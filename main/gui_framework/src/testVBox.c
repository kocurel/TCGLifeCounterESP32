#include <stdio.h>

#include "GUIFramework.h"  // Include your framework's public API
#include "GUItest.h"
#include "GUItests.h"
#include "esp_log.h"  // Useful for printing messages during the test

void test_VBox_is_properly_initiated() {
    GUIVBox* vbox = GUIVBox_new();
    TEST_ASSERT_NOT_NULL(vbox);
    TEST_ASSERT_NOT_NULL(vbox->base.base.layout);
    TEST_ASSERT_NOT_NULL(vbox->base.base.draw);
    TEST_ASSERT_NOT_NULL(vbox->base.base.delete);
    TEST_ASSERT_NOT_NULL(vbox->base.add_child);
    TEST_ASSERT_NOT_NULL(vbox->base.children);
    TEST_ASSERT_EQUAL_INT32(0, vbox->base.count);
    TEST_ASSERT_EQUAL_INT32(0, vbox->base.padding);
    TEST_ASSERT_EQUAL_INT32(0, vbox->base.spacing);
    TEST_ASSERT_EQUAL_UINT8(0, vbox->base.base.x);
    TEST_ASSERT_EQUAL_UINT8(0, vbox->base.base.y);
    TEST_ASSERT_EQUAL_UINT8(0, vbox->base.base.width);
    TEST_ASSERT_EQUAL_UINT8(0, vbox->base.base.height);
    GUI_DELETE(vbox);
}
void test_HBox_is_properly_initiated() {
    GUIHBox* hbox = GUIVBox_new();
    TEST_ASSERT_NOT_NULL(hbox);
    TEST_ASSERT_NOT_NULL(hbox->base.base.layout);
    TEST_ASSERT_NOT_NULL(hbox->base.base.draw);
    TEST_ASSERT_NOT_NULL(hbox->base.base.delete);
    TEST_ASSERT_NOT_NULL(hbox->base.add_child);
    TEST_ASSERT_NOT_NULL(hbox->base.children);
    TEST_ASSERT_EQUAL_INT32(0, hbox->base.count);
    TEST_ASSERT_EQUAL_INT32(0, hbox->base.padding);
    TEST_ASSERT_EQUAL_INT32(0, hbox->base.spacing);
    TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.x);
    TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.y);
    TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.width);
    TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.height);
    GUI_DELETE(hbox);
}