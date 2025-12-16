idf_component_register(SRCS "console_init.c"  "model/Button.c"  "ViewController.c" "main.c"
                        "model/ChangeHistory.c"
                        "model/DiceRoller.c"
                        "model/Game.c"
                       
                        "app/pages/ChangeHistoryPage.c"
                        "app/pages/CommanderPage.c"
                        "app/pages/MainPage.c"
                        "app/pages/MenuPage.c"
                        "app/pages/PlayerPage.c"
                        "app/pages/ValueEditorPage.c"
                        "app/PageManager.c"
                        "app/GameDelegates.c"
                        PRIV_REQUIRES spi_flash driver console
                        REQUIRES "gui_framework"
                        INCLUDE_DIRS "."
)