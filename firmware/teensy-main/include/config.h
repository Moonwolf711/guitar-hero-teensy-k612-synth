/**
 * Configuration and Pin Definitions
 * Hardware connections and system parameters
 */

#ifndef CONFIG_H
#define CONFIG_H

// Version information
#define FIRMWARE_VERSION "1.0.0"
#define FIRMWARE_DATE __DATE__

// Audio configuration
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BLOCK_SIZE 128
#define NUM_VOICES 6
#define AUDIO_MEMORY_BLOCKS 64

// Performance limits
#define MAX_CPU_USAGE 80.0f      // Warning threshold
#define MAX_MEMORY_USAGE 48       // Audio memory blocks

// USB Host configuration
#define USB_HOST_SPEED 12000000   // 12MHz for full-speed USB

// Hardware pin definitions for Teensy 4.1
// I2S Audio output pins (PCM5102A DAC)
#define I2S_BCLK_PIN 21          // Bit clock
#define I2S_LRCLK_PIN 20         // Word select / LR clock
#define I2S_DATA_PIN 7           // Data out
#define I2S_MCLK_PIN 23          // Master clock (optional)

// ESP8266 Serial connection
#define ESP_TX_PIN 1             // Serial1 TX
#define ESP_RX_PIN 0             // Serial1 RX
#define ESP_RESET_PIN 2          // ESP8266 reset (optional)
#define ESP_ENABLE_PIN 3         // ESP8266 chip enable (optional)

// Status LEDs (optional)
#define LED_POWER_PIN 13         // Built-in LED
#define LED_ACTIVITY_PIN 4       // Activity indicator
#define LED_ERROR_PIN 5          // Error indicator

// SD Card (built into Teensy 4.1)
#define SD_CS_PIN BUILTIN_SDCARD

// Debug serial port
#define DEBUG_SERIAL Serial
#define DEBUG_BAUD 115200

// MIDI configuration
#define MIDI_CHANNEL 1
#define MIDI_VELOCITY_DEFAULT 100

// Control ranges
#define WHAMMY_DEADZONE 10       // Ignore small whammy movements
#define TILT_DEADZONE 1000       // Ignore small tilt changes

// Timing constants (milliseconds)
#define CONTROLLER_POLL_RATE 1   // 1ms = 1000Hz polling
#define ESP_UPDATE_RATE 100      // Update ESP every 100ms
#define PERF_REPORT_RATE 1000    // Performance report every second

// Default synthesizer parameters
#define DEFAULT_WAVEFORM WAVEFORM_SAWTOOTH
#define DEFAULT_FILTER_FREQ 2000.0f
#define DEFAULT_FILTER_RES 2.0f
#define DEFAULT_REVERB_MIX 0.3f
#define DEFAULT_DELAY_MIX 0.2f
#define DEFAULT_DELAY_TIME 150.0f

// Network configuration (for ESP8266)
#define DEFAULT_WIFI_SSID "GuitarHeroSynth"
#define DEFAULT_WIFI_PASS "music123"
#define DEFAULT_AP_MODE true     // Start in Access Point mode
#define DEFAULT_WEB_PORT 80
#define DEFAULT_OSC_PORT 8000

// Memory management
#define USE_EXTERNAL_PSRAM false  // Teensy 4.1 with PSRAM expansion
#define PSRAM_SIZE_MB 8          // If PSRAM installed

// Debugging flags
#define DEBUG_USB_HOST 0         // Print USB Host debug info
#define DEBUG_AUDIO 0            // Print audio system debug info
#define DEBUG_CONTROLLER 1       // Print controller state
#define DEBUG_PERFORMANCE 1      // Print performance metrics

// Safety features
#define ENABLE_WATCHDOG true     // Enable watchdog timer
#define WATCHDOG_TIMEOUT 1000    // 1 second timeout
#define ENABLE_BROWNOUT true     // Enable brownout detection
#define AUTO_SAVE_SETTINGS true  // Save settings to EEPROM

// Audio codec selection
// Comment out one of these based on your hardware
#define USE_PCM5102A             // Recommended: Simple I2S DAC
// #define USE_SGTL5000          // Alternative: Audio shield codec

// Controller compatibility modes
#define SUPPORT_XBOX360 true
#define SUPPORT_PS3 false        // Future expansion
#define SUPPORT_WII false        // Future expansion

// Helper macros
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define MAP_FLOAT(x, in_min, in_max, out_min, out_max) \
    (((x) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))

#endif // CONFIG_H