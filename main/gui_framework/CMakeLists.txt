idf_component_register(
    SRCS "src/GUIComponents.c" "src/GUIRenderer.c" "src/GUIComponent.c" "src/GUILabel.c" "src/GUIContainer.c" "src/GUIVBox.c" "src/GUIHBox.c"
        "src/testVBox.c"
    REQUIRES "u8g2" "u8g2-hal-esp-idf" "unity"
    INCLUDE_DIRS "include" "include/test"
    )