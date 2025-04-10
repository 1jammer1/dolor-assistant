# Dolor Assistant 
A full-screen AI assistant made to run on any ***okay*** computer from the past 10 years. It uses wake word detection, speech recognition, AI processing, and voice synthesis to create a complete voice assistant experience.

## Overview
Dolor Assistant is a lightweight yet powerful voice-controlled AI assistant that:
1. Listens for a wake word ("dolor")
2. Records your voice command
3. Sends the speech to an AI model
4. Speaks the response back to you

## Components
- [Porcupine](https://github.com/Picovoice/porcupine) - Wake word detection
- [PocketSphinx](https://github.com/cmusphinx/pocketsphinx/) - Speech recognition
- [PortAudio](https://github.com/PortAudio/portaudio) - Audio I/O handling
- [Ollama](https://github.com/ollama/ollama) - Local AI server
- [eSpeak-NG](https://github.com/espeak-ng/espeak-ng) - Text-to-speech synthesis

## Installation

### Prerequisites
- Linux-based system
- C compiler (gcc)
- Make
- Internet connection for downloading dependencies

### Dependencies Installation
1. Run the included dependency installer:
```
make install-deps
```

2. Install Porcupine manually:
   - Clone [Porcupine](https://github.com/Picovoice/porcupine)
   - Follow their installation instructions for C library
   - Create a keyword file for "dolor" using their console

3. Install Ollama:
   - Follow instructions at [Ollama Installation](https://github.com/ollama/ollama#installation)
   - Download a model: `ollama pull phi4-mini`

### Building
```
make all
```

### Running
```
make run
```

## Usage
1. Start the assistant
2. Wait for the "Dolor Assistant is ready" message
3. Say "dolor" to activate listening
4. Speak your query
5. Listen to the response
6. Repeat from step 3

## Why the fuck
very simple, because [Machine Love - JamieP](https://www.youtube.com/watch?v=sqK-jh4TDXo).

## Minimum requirements
fuck you, if i want this to run on something i will make it. dont fucking test me.
