/**
 * Dolor Assistant
 * An AI full screen assistant integrating wake word detection, speech recognition,
 * AI processing, and voice synthesis.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

// PortAudio for audio I/O
#include <portaudio.h>

// Porcupine for wake word detection
#include <pv_porcupine.h>

// PocketSphinx for speech recognition
#include <pocketsphinx.h>

// eSpeak-NG for voice synthesis
#include <espeak-ng/speak_lib.h>

// Constants
#define SAMPLE_RATE 16000
#define FRAMES_PER_BUFFER 512
#define CHANNELS 1
#define WAKE_WORD "dolor"
#define OLLAMA_ENDPOINT "http://localhost:11434/api/generate"

// Global variables
volatile bool is_running = true;
volatile bool is_listening = false;
pv_porcupine_t* porcupine = NULL;
ps_decoder_t* ps = NULL;
cmd_ln_t* config = NULL;

// Signal handler to gracefully exit
void signal_handler(int signal) {
    fprintf(stderr, "\nCaught signal %d, shutting down...\n", signal);
    is_running = false;
}

// Callback function for PortAudio
static int audio_callback(const void* input_buffer, void* output_buffer,
                          unsigned long frames_per_buffer,
                          const PaStreamCallbackTimeInfo* time_info,
                          PaStreamCallbackFlags status_flags,
                          void* user_data) {
    
    const int16_t* buffer = (const int16_t*)input_buffer;
    
    if (is_listening) {
        // If we're actively listening, send audio to PocketSphinx
        ps_process_raw(ps, buffer, frames_per_buffer, FALSE, FALSE);
    } else {
        // Otherwise, check for wake word using Porcupine
        int32_t result;
        pv_porcupine_process(porcupine, buffer, &result);
        
        if (result) {
            printf("Wake word detected! Listening...\n");
            is_listening = true;
            
            // Reset PocketSphinx for new utterance
            ps_start_utt(ps);
        }
    }
    
    return paContinue;
}

// Function to send text to Ollama
char* query_ollama(const char* text) {
    char command[1024];
    char* response = malloc(4096);
    FILE* fp;
    
    if (!response) {
        return NULL;
    }
    
    // Create curl command to send to Ollama
    snprintf(command, sizeof(command), 
             "curl -s -X POST %s -d '{\"model\":\"phi4-mini\",\"prompt\":\"%s\"}' | jq -r '.response'",
             OLLAMA_ENDPOINT, text);
    
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run command to Ollama\n");
        free(response);
        return NULL;
    }
    
    // Read the output
    if (fgets(response, 4096, fp) == NULL) {
        fprintf(stderr, "Failed to get response from Ollama\n");
        free(response);
        pclose(fp);
        return NULL;
    }
    
    pclose(fp);
    return response;
}

// Function to speak text using eSpeak-NG
void speak_text(const char* text) {
    espeak_Synth(text, strlen(text) + 1, 0, POS_CHARACTER, 0, 
                 espeakCHARS_AUTO, NULL, NULL);
    espeak_Synchronize();
}

int main() {
    PaError pa_err;
    PaStream* stream = NULL;
    int status;
    pv_status_t pv_status;
    
    // Setup signal handler
    signal(SIGINT, signal_handler);
    
    printf("Initializing Dolor Assistant...\n");
    
    // Initialize PortAudio
    pa_err = Pa_Initialize();
    if (pa_err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(pa_err));
        goto cleanup;
    }
    
    // Initialize Porcupine (wake word detector)
    const char* porcupine_model_path = "./porcupine_params.pv";
    const char* keyword_path = "./dolor_keyword.ppn";
    const float sensitivity = 0.5f;
    
    pv_status = pv_porcupine_init(porcupine_model_path, 1, &keyword_path,
                                &sensitivity, &porcupine);
    if (pv_status != PV_STATUS_SUCCESS) {
        fprintf(stderr, "Failed to initialize Porcupine with status %d\n", pv_status);
        goto cleanup;
    }
    
    // Initialize PocketSphinx
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                        "-hmm", "/usr/local/share/pocketsphinx/model/en-us/en-us",
                        "-lm", "/usr/local/share/pocketsphinx/model/en-us/en-us.lm.bin",
                        "-dict", "/usr/local/share/pocketsphinx/model/en-us/cmudict-en-us.dict",
                        "-samprate", "16000",
                        NULL);
    
    if (config == NULL) {
        fprintf(stderr, "Failed to create PocketSphinx config\n");
        goto cleanup;
    }
    
    ps = ps_init(config);
    if (ps == NULL) {
        fprintf(stderr, "Failed to initialize PocketSphinx\n");
        goto cleanup;
    }
    
    // Initialize eSpeak-NG
    if (espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0) < 0) {
        fprintf(stderr, "Failed to initialize eSpeak-NG\n");
        goto cleanup;
    }
    
    espeak_SetVoiceByName("en");
    
    // Open PortAudio stream
    pa_err = Pa_OpenDefaultStream(&stream, CHANNELS, 0, paInt16, SAMPLE_RATE,
                                FRAMES_PER_BUFFER, audio_callback, NULL);
    if (pa_err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(pa_err));
        goto cleanup;
    }
    
    pa_err = Pa_StartStream(stream);
    if (pa_err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(pa_err));
        goto cleanup;
    }
    
    // Startup message
    printf("Dolor Assistant ready. Say '%s' to activate.\n", WAKE_WORD);
    speak_text("Dolor Assistant is ready. Say dolor to activate.");
    
    // Main loop
    while (is_running) {
        if (is_listening) {
            // Check if speech recognition has detected an end of utterance
            const char* hyp;
            if (ps_get_in_speech(ps) == 0) {
                // Speech ended, get the hypothesis
                ps_end_utt(ps);
                hyp = ps_get_hyp(ps, NULL);
                
                if (hyp != NULL && strlen(hyp) > 0) {
                    printf("You said: %s\n", hyp);
                    
                    // Query Ollama with the recognized text
                    char* ai_response = query_ollama(hyp);
                    if (ai_response) {
                        printf("AI response: %s\n", ai_response);
                        
                        // Speak the response
                        speak_text(ai_response);
                        free(ai_response);
                    }
                }
                
                // Reset for next wake word
                is_listening = false;
                printf("Listening ended. Say '%s' to activate again.\n", WAKE_WORD);
            }
        }
        
        // Sleep to prevent busy waiting
        usleep(100000); // 100ms
    }
    
cleanup:
    printf("Cleaning up resources...\n");
    
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
    }
    
    Pa_Terminate();
    
    if (porcupine) {
        pv_porcupine_delete(porcupine);
    }
    
    if (ps) {
        ps_free(ps);
    }
    
    if (config) {
        cmd_ln_free_r(config);
    }
    
    espeak_Terminate();
    
    printf("Dolor Assistant terminated.\n");
    return 0;
}