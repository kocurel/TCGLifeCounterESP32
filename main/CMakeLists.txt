idf_component_register(SRCS 
                        "System.c"
                        "PowerManager.c"
                        "AudioManager.c"
                        "keypad.c" "battery.c" "console_init.c"  "model/Button.c"  "ViewController.c" "main.c"
                        "model/DiceRoller.c"
                        "model/Game.c"
                        "model/Settings.c"
                        "model/Tests.c"

                        "app/pages/ChangeHistoryPage.c"
                        "app/pages/CommanderPage.c"
                        "app/pages/MainPage.c"
                        "app/pages/SettingsPage.c"
                        "app/pages/MenuPage.c"
                        "app/pages/PlayerPage.c"
                        "app/pages/ValueEditorPage.c"
                        "app/pages/DicePage.c"
                        "app/PageManager.c"
                        "app/GameDelegates.c"
                        PRIV_REQUIRES spi_flash driver console esp_adc nvs_flash esp_timer
                        REQUIRES "gui_framework" "unity"
                        INCLUDE_DIRS "."
)