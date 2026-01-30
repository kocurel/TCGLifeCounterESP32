#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdint.h>

// Standard UI sounds for consistent feedback
typedef enum {
    SOUND_UI_MOVE = 0,  // Short click (navigation)
    SOUND_UI_SELECT,    // High pitch beep (accept)
    SOUND_UI_CANCEL,    // Low pitch beep (back)
    SOUND_UI_ERROR,     // Buzz/Error sound
    SOUND_GAME_START,   // Ascending arpeggio
    SOUND_DICE_ROLL     // Short tick for animation
} SystemSound;

#define BUZZER_PIN 20
#define BUZZER_TIMER LEDC_TIMER_0
#define BUZZER_MODE LEDC_LOW_SPEED_MODE
#define BUZZER_CHANNEL LEDC_CHANNEL_0
#define BUZZER_DUTY_50 2048  // 50% duty cycle for max volume
#define BUZZER_DUTY_RES LEDC_TIMER_13_BIT
#define BUZZER_MAX_DUTY 8191

// Funkcja do dynamicznej zmiany głośności
// Queue configuration
#define AUDIO_QUEUE_LEN 10
// Initialize the LEDC timer and background task
void AudioManager_init(void);

void AudioManager_set_volume(uint8_t volume_percent);
// Play a predefined system sound (Non-blocking)
void AudioManager_play_sound(SystemSound sound);

// Play a custom raw frequency (Non-blocking)
void AudioManager_play_tone(uint32_t freq_hz, uint32_t duration_ms);

#endif  // AUDIO_MANAGER_H