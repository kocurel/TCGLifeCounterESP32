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
    GUIHBox* hbox = GUIHBox_new();
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
void test_Label_is_properly_initiated() {
    GUILabel* label = GUILabel_new();
    TEST_ASSERT_NOT_NULL(label);
    TEST_ASSERT_NULL(label->base.layout);
    TEST_ASSERT_NOT_NULL(label->base.draw);
    TEST_ASSERT_NOT_NULL(label->base.delete);
    TEST_ASSERT_NOT_NULL(label->text);
    TEST_ASSERT_EQUAL_INT32(10, label->font_size);
    TEST_ASSERT_FALSE(label->isUpsideDown);
    TEST_ASSERT_EQUAL_UINT8(0, label->base.x);
    TEST_ASSERT_EQUAL_UINT8(0, label->base.y);
    TEST_ASSERT_EQUAL_UINT8(0, label->base.width);
    TEST_ASSERT_EQUAL_UINT8(0, label->base.height);
    TEST_ASSERT_EQUAL_STRING("", label->text);
    GUI_DELETE(label);
}

// Global flag to track if the mock delete was called
int test_variable1 = 0;

// The Mock Delete Function: Increments the counter AND frees the memory.
void mock_delete_func(GUIComponent* self) {
    test_variable1 += 1;
    free(self);  // CRITICAL: This allows the test to be valid memory-wise.
}

// Helper to create a heap-allocated mock component (mimicking GUILabel_new,
// etc.)
GUIComponent* create_mock_component(void) {
    // Allocate the memory on the heap
    GUIComponent* comp = (GUIComponent*)malloc(sizeof(GUIComponent));
    if (comp == NULL) {
        // In a real test, you would assert failure here, but for simplicity:
        return NULL;
    }
    // Initialize its V-table pointer to our spy function
    comp->delete = mock_delete_func;
    return comp;
}

void test_Container_add_child_recursive_delete() {
    // 1. ARRANGE
    GUIVBox* vbox = GUIVBox_new();  // Heap allocated

    // Allocate all components on the HEAP using the helper
    GUIComponent* test_comp1 = create_mock_component();
    GUIComponent* test_comp2 = create_mock_component();
    GUIComponent* test_comp3 = create_mock_component();
    GUIComponent* test_comp4 = create_mock_component();

    // Set the counter and add children
    test_variable1 = 0;

    GUI_ADD_CHILD(vbox, test_comp1);
    GUI_ADD_CHILD(vbox, test_comp2);
    GUI_ADD_CHILD(vbox, test_comp3);
    GUI_ADD_CHILD(vbox, test_comp4);

    // 2. ASSERT (Functionality Check)
    TEST_ASSERT_NOT_NULL(vbox);
    TEST_ASSERT_EQUAL_INT32(4, vbox->base.count);

    // Check internal array pointers (Ensures assignment worked)
    TEST_ASSERT_EQUAL_PTR(test_comp1, vbox->base.children[0]);

    // 3. ACT (Recursive Delete)
    GUI_DELETE(vbox);  // Calls VBox_delete -> loops and calls
                       // mock_delete_func(comp) 4 times

    // 4. ASSERT (Memory Safety Check)
    // Check that the mock function was called exactly 4 times (meaning all
    // children were deleted)
    TEST_ASSERT_EQUAL_INT32(4, test_variable1);

    // No need for further cleanup; GUI_DELETE(vbox) should have handled all
    // memory.
}

void test_Nested_Container_Layout() {
    // --- 1. ARRANGE ---

    // Root VBox (100x100 at 0,0 - splits space vertically)
    GUIVBox* vbox = GUIVBox_new();
    GUI_SET_POS(vbox, 0, 0);
    GUI_SET_SIZE(vbox, 100, 100);
    GUI_SET_PADDING(vbox, 0);
    GUI_SET_SPACING(vbox, 0);

    // Child 1: A simple label (L1)
    GUILabel* l1 = GUILabel_new();

    // Child 2: A container (H1)
    GUIHBox* hbox = GUIHBox_new();

    // Children of HBox (L2, L3 - will split HBox width horizontally)
    GUILabel* l2 = GUILabel_new();
    GUILabel* l3 = GUILabel_new();

    // Build the tree
    GUI_ADD_CHILD(hbox, l2);
    GUI_ADD_CHILD(hbox, l3);  // HBox now has L2, L3
    GUI_ADD_CHILD(vbox, l1);
    GUI_ADD_CHILD(vbox, hbox);  // VBox now has L1, HBox

    // --- 2. ACT ---
    // This call must recursively trigger layout for VBox, then HBox, then
    // L2/L3.
    GUI_UPDATE_LAYOUT(vbox);

    // --- 3. ASSERTIONS ---

    // A. Check Root VBox Children (Vertical Split)
    // VBox has 100px height for 2 children => 50px each.

    // L1 (Child 1 of VBox)
    TEST_ASSERT_EQUAL_UINT8(100, l1->base.width);
    TEST_ASSERT_EQUAL_UINT8(50, l1->base.height);
    TEST_ASSERT_EQUAL_UINT8(0, l1->base.x);
    TEST_ASSERT_EQUAL_UINT8(0, l1->base.y);  // Top half

    // HBox (Child 2 of VBox)
    TEST_ASSERT_EQUAL_UINT8(100, hbox->base.base.width);
    TEST_ASSERT_EQUAL_UINT8(50, hbox->base.base.height);
    TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.x);
    TEST_ASSERT_EQUAL_UINT8(50,
                            hbox->base.base.y);  // Bottom half (starts at Y=50)

    // B. Check Nested HBox Children (Horizontal Split, Offset Coordinates)
    // HBox has 100px width for 2 children => 50px each.

    // L2 (Child 1 of HBox)
    TEST_ASSERT_EQUAL_UINT8(50, l2->base.width);
    TEST_ASSERT_EQUAL_UINT8(50, l2->base.height);
    TEST_ASSERT_EQUAL_UINT8(0, l2->base.x);  // Inherits HBox X (0)
    TEST_ASSERT_EQUAL_UINT8(
        50, l2->base.y);  // Inherits HBox Y (50) - **CRITICAL OFFSET CHECK**

    // L3 (Child 2 of HBox)
    TEST_ASSERT_EQUAL_UINT8(50, l3->base.width);
    TEST_ASSERT_EQUAL_UINT8(50, l3->base.height);
    TEST_ASSERT_EQUAL_UINT8(50, l3->base.x);  // Offset by L2 width
    TEST_ASSERT_EQUAL_UINT8(50, l3->base.y);  // Inherits HBox Y (50)

    // --- 4. CLEANUP ---
    GUI_DELETE(vbox);
}
