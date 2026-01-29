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

// Initialize the LEDC timer and background task
void AudioManager_init(void);

// Play a predefined system sound (Non-blocking)
void AudioManager_play_sound(SystemSound sound);

// Play a custom raw frequency (Non-blocking)
void AudioManager_play_tone(uint32_t freq_hz, uint32_t duration_ms);

#endif  // AUDIO_MANAGER_H