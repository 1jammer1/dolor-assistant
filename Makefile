CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lportaudio -lpv_porcupine -lpocketsphinx -lespeak-ng -lcurl

# Directories
SRC_DIR = .
BUILD_DIR = build

# Executable
TARGET = $(BUILD_DIR)/dolor-assistant

# Source files
SRCS = $(SRC_DIR)/main.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Default target
all: directories $(TARGET)

# Rule to create build directory
directories:
	@mkdir -p $(BUILD_DIR)

# Rule to build executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -rf $(BUILD_DIR)

# Detect the package manager
detect-distro:
	@if [ -f /etc/debian_version ]; then \
		echo "DEB"; \
	elif [ -f /etc/fedora-release ]; then \
		echo "DNF"; \
	elif [ -f /etc/arch-release ]; then \
		echo "PACMAN"; \
	elif [ -f /etc/alpine-release ]; then \
		echo "APK"; \
	elif [ -f /etc/SuSE-release ] || [ -f /etc/opensuse-release ]; then \
		echo "ZYPPER"; \
	elif [ -f /etc/gentoo-release ]; then \
		echo "EMERGE"; \
	else \
		echo "UNKNOWN"; \
	fi

# Install dependencies
install-deps:
	@echo "Detecting Linux distribution..."
	@PKG_MANAGER=$$($(MAKE) -s detect-distro); \
	case $$PKG_MANAGER in \
		"DEB") \
			echo "Debian/Ubuntu detected. Using apt..."; \
			sudo apt-get update; \
			sudo apt-get install -y build-essential portaudio19-dev curl libcurl4-openssl-dev; \
			sudo apt-get install -y espeak-ng espeak-ng-data libespeak-ng-dev; \
			sudo apt-get install -y pocketsphinx libpocketsphinx-dev pocketsphinx-en-us; \
			;; \
		"DNF") \
			echo "Fedora/RHEL detected. Using dnf..."; \
			sudo dnf update -y; \
			sudo dnf install -y gcc make portaudio-devel curl libcurl-devel; \
			sudo dnf install -y espeak-ng espeak-ng-devel; \
			sudo dnf install -y pocketsphinx pocketsphinx-devel; \
			;; \
		"PACMAN") \
			echo "Arch Linux detected. Using pacman..."; \
			sudo pacman -Syu --noconfirm; \
			sudo pacman -S --noconfirm base-devel portaudio curl; \
			sudo pacman -S --noconfirm espeak-ng; \
			sudo pacman -S --noconfirm pocketsphinx; \
			;; \
		"APK") \
			echo "Alpine Linux detected. Using apk..."; \
			sudo apk update; \
			sudo apk add build-base portaudio-dev curl curl-dev; \
			sudo apk add espeak-ng espeak-ng-dev; \
			sudo apk add pocketsphinx pocketsphinx-dev; \
			;; \
		"ZYPPER") \
			echo "openSUSE detected. Using zypper..."; \
			sudo zypper refresh; \
			sudo zypper install -y gcc make portaudio-devel libcurl-devel curl; \
			sudo zypper install -y espeak-ng espeak-ng-devel; \
			sudo zypper install -y pocketsphinx pocketsphinx-devel; \
			;; \
		"EMERGE") \
			echo "Gentoo detected. Using emerge..."; \
			sudo emerge --sync; \
			sudo emerge -av media-libs/portaudio net-misc/curl; \
			sudo emerge -av app-accessibility/espeak-ng; \
			sudo emerge -av app-accessibility/pocketsphinx; \
			;; \
		*) \
			echo "Unknown distribution. Please install the following packages manually:"; \
			echo "- Build tools (gcc, make)"; \
			echo "- PortAudio development files"; \
			echo "- cURL and its development files"; \
			echo "- eSpeak-NG and its development files"; \
			echo "- PocketSphinx and its development files"; \
			;; \
	esac
	@echo "Note: You need to manually install Porcupine from https://github.com/Picovoice/porcupine"
	@echo "Note: You need to manually install Ollama from https://github.com/ollama/ollama"

# Run the assistant
run: all
	@echo "Starting Dolor Assistant..."
	$(TARGET)

# Phony targets
.PHONY: all clean directories detect-distro install-deps run