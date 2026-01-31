#include "AudioManager.h"

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "model/Settings.h"

/* --- Types and Private State --- */

typedef struct {
    uint32_t frequency;
    uint32_t duration;
} ToneRequest;

static uint8_t s_volume_percent = 50;  // Cache for volume level (0-100)
static QueueHandle_t s_audio_queue = NULL;

/* --- Audio Engine Task --- */

/**
 * High-priority task that processes tone requests from the queue.
 * Controls the LEDC (PWM) hardware to generate audible frequencies.
 */
static void audio_task(void* arg) {
    ToneRequest req;
    while (1) {
        if (xQueueReceive(s_audio_queue, &req, portMAX_DELAY)) {
            /* * Volume Control logic:
             * For a 13-bit resolution, max duty is 8191.
             * To maximize audible volume, we use 50% duty cycle (4095) as 100%
             * volume.
             */
            uint32_t max_audible_duty = 4095;
            uint32_t calculated_duty =
                (max_audible_duty * s_volume_percent) / 100;

            // Apply frequency and duty cycle
            ledc_set_freq(BUZZER_MODE, BUZZER_TIMER, req.frequency);
            ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, calculated_duty);
            ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);

            // Sustain tone for requested duration
            vTaskDelay(pdMS_TO_TICKS(req.duration));

            // Stop the sound and enforce a short silence (gap) between tones
            ledc_stop(BUZZER_MODE, BUZZER_CHANNEL, 0);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}

/* --- Public API --- */

/**
 * Updates the global volume level used by the audio task.
 */
void AudioManager_set_volume(uint8_t volume_percent) {
    if (volume_percent > 100) volume_percent = 100;
    s_volume_percent = volume_percent;
}

/**
 * Configures LEDC hardware and starts the asynchronous audio processing task.
 */
void AudioManager_init(void) {
    // 1. Load initial volume from persistent settings
    GameSettings settings = SettingsModel_get();
    AudioManager_set_volume(settings.sound_loudness);

    // 2. Configure LEDC Timer
    ledc_timer_config_t ledc_timer = {.speed_mode = BUZZER_MODE,
                                      .timer_num = BUZZER_TIMER,
                                      .duty_resolution = BUZZER_DUTY_RES,
                                      .freq_hz = 4000,
                                      .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&ledc_timer);

    // 3. Configure LEDC Channel
    ledc_channel_config_t ledc_channel = {.speed_mode = BUZZER_MODE,
                                          .channel = BUZZER_CHANNEL,
                                          .timer_sel = BUZZER_TIMER,
                                          .intr_type = LEDC_INTR_DISABLE,
                                          .gpio_num = BUZZER_PIN,
                                          .duty = 0,  // Initial silence
                                          .hpoint = 0};
    ledc_channel_config(&ledc_channel);

    // 4. Create Queue and Task
    s_audio_queue = xQueueCreate(AUDIO_QUEUE_LEN, sizeof(ToneRequest));
    xTaskCreate(audio_task, "AudioTask", 2048, NULL, 5, NULL);
}

/**
 * Non-blocking function to queue a single tone.
 */
void AudioManager_play_tone(uint32_t freq_hz, uint32_t duration_ms) {
    if (s_audio_queue == NULL) return;

    ToneRequest req = {.frequency = freq_hz, .duration = duration_ms};

    // Non-blocking send: prioritize UI responsiveness over guaranteed audio
    xQueueSend(s_audio_queue, &req, 0);
}

/**
 * High-level API to play predefined system sound effects.
 */
void AudioManager_play_sound(SystemSound sound) {
    switch (sound) {
        case SOUND_UI_MOVE:
            AudioManager_play_tone(800, 20);
            break;
        case SOUND_UI_SELECT:
            AudioManager_play_tone(1200, 100);
            break;
        case SOUND_UI_CANCEL:
            AudioManager_play_tone(300, 150);
            break;
        case SOUND_UI_ERROR:
            AudioManager_play_tone(150, 300);
            break;
        case SOUND_DICE_ROLL:
            AudioManager_play_tone(2000, 10);
            break;
        case SOUND_GAME_START:
            // Queue multiple tones to create a chord/melody
            AudioManager_play_tone(523, 100);  // C5
            AudioManager_play_tone(659, 100);  // E5
            AudioManager_play_tone(783, 200);  // G5
            break;
    }
}