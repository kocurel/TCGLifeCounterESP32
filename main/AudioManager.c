#include "AudioManager.h"

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "model/Settings.h"

// Cache głośności (0-100)
static uint8_t s_volume_percent = 50;

typedef struct {
    uint32_t frequency;
    uint32_t duration;
} ToneRequest;

static QueueHandle_t s_audio_queue = NULL;

static void audio_task(void* arg) {
    ToneRequest req;
    while (1) {
        if (xQueueReceive(s_audio_queue, &req, portMAX_DELAY)) {
            // Obliczamy wypełnienie na podstawie głośności
            // Max duty dla 13-bit to 8191. 50% (max głośność) to 4095.
            uint32_t max_audible_duty = 4095;
            uint32_t calculated_duty =
                (max_audible_duty * s_volume_percent) / 100;

            ledc_set_freq(BUZZER_MODE, BUZZER_TIMER, req.frequency);
            ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, calculated_duty);
            ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);

            vTaskDelay(pdMS_TO_TICKS(req.duration));

            ledc_stop(BUZZER_MODE, BUZZER_CHANNEL, 0);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}

// Nowa funkcja do zmiany głośności
void AudioManager_set_volume(uint8_t volume_percent) {
    if (volume_percent > 100) volume_percent = 100;
    s_volume_percent = volume_percent;
}

void AudioManager_init(void) {
    GameSettings settings = SettingsModel_get();
    AudioManager_set_volume(settings.sound_loudness);
    // 1. Configure LEDC Timer
    ledc_timer_config_t ledc_timer = {.speed_mode = BUZZER_MODE,
                                      .timer_num = BUZZER_TIMER,
                                      .duty_resolution = BUZZER_DUTY_RES,
                                      .freq_hz = 4000,  // Default frequency
                                      .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&ledc_timer);

    // 2. Configure LEDC Channel
    ledc_channel_config_t ledc_channel = {.speed_mode = BUZZER_MODE,
                                          .channel = BUZZER_CHANNEL,
                                          .timer_sel = BUZZER_TIMER,
                                          .intr_type = LEDC_INTR_DISABLE,
                                          .gpio_num = BUZZER_PIN,
                                          .duty = 0,  // Start with 0 (Silence)
                                          .hpoint = 0};
    ledc_channel_config(&ledc_channel);

    // 3. Create Queue and Task
    s_audio_queue = xQueueCreate(AUDIO_QUEUE_LEN, sizeof(ToneRequest));

    // Stack size 2048 is usually sufficient for simple tasks
    xTaskCreate(audio_task, "AudioTask", 2048, NULL, 5, NULL);
}

void AudioManager_play_tone(uint32_t freq_hz, uint32_t duration_ms) {
    if (s_audio_queue == NULL) return;

    ToneRequest req = {.frequency = freq_hz, .duration = duration_ms};
    // Don't wait if queue is full (UI responsiveness > dropping a beep)
    xQueueSend(s_audio_queue, &req, 0);
}

void AudioManager_play_sound(SystemSound sound) {
    switch (sound) {
        case SOUND_UI_MOVE:
            AudioManager_play_tone(800, 20);  // Very short click
            break;
        case SOUND_UI_SELECT:
            AudioManager_play_tone(1200, 100);  // High pitch confirm
            break;
        case SOUND_UI_CANCEL:
            AudioManager_play_tone(300, 150);  // Low pitch cancel
            break;
        case SOUND_UI_ERROR:
            AudioManager_play_tone(150, 300);  // Low buzz
            break;
        case SOUND_DICE_ROLL:
            AudioManager_play_tone(2000, 10);  // High tick
            break;
        case SOUND_GAME_START:
            // Example of queuing multiple tones for a melody
            AudioManager_play_tone(523, 100);  // C5
            AudioManager_play_tone(659, 100);  // E5
            AudioManager_play_tone(783, 200);  // G5
            break;
    }
}