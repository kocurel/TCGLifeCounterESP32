idf_component_register(SRCS "console_init.c"  "Button.c" "ViewModel.c" "ViewController.c" "main.c"
                        "ChangeHistory.c"
                        "DiceRoller.c"
                        "Game.c"
                       
                       
                       
                        "Button.c"
                        "ViewModel.c"

                        PRIV_REQUIRES spi_flash driver console
                        REQUIRES "gui_framework"
                        INCLUDE_DIRS "../components/u8g2/csrc"
                                    "../components/u8g2-hal-esp-idf/include"
                                    ".")
