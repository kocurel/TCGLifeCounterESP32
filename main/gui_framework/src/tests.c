// #include <stdio.h>

// #include "GUIFramework.h"  // Include your framework's public API
// #include "GUItests.h"
// #include "esp_log.h"  // Useful for printing messages during the test

// void test_VBox_is_properly_initiated() {
//     GUIVBox* vbox = GUIVBox_new();
//     TEST_ASSERT_NOT_NULL(vbox);
//     TEST_ASSERT_NOT_NULL(vbox->base.base.layout);
//     TEST_ASSERT_NOT_NULL(vbox->base.base.draw);
//     TEST_ASSERT_NOT_NULL(vbox->base.base.delete);
//     TEST_ASSERT_NOT_NULL(vbox->base.add_child);
//     TEST_ASSERT_NOT_NULL(vbox->base.children);
//     TEST_ASSERT_EQUAL_INT32(0, vbox->base.count);
//     TEST_ASSERT_EQUAL_INT32(0, vbox->base.padding);
//     TEST_ASSERT_EQUAL_INT32(0, vbox->base.spacing);
//     TEST_ASSERT_EQUAL_UINT8(0, vbox->base.base.x);
//     TEST_ASSERT_EQUAL_UINT8(0, vbox->base.base.y);
//     TEST_ASSERT_EQUAL_UINT8(0, vbox->base.base.width);
//     TEST_ASSERT_EQUAL_UINT8(0, vbox->base.base.height);
//     GUI_DELETE(vbox);
// }
// void test_HBox_is_properly_initiated() {
//     GUIHBox* hbox = GUIHBox_new();
//     TEST_ASSERT_NOT_NULL(hbox);
//     TEST_ASSERT_NOT_NULL(hbox->base.base.layout);
//     TEST_ASSERT_NOT_NULL(hbox->base.base.draw);
//     TEST_ASSERT_NOT_NULL(hbox->base.base.delete);
//     TEST_ASSERT_NOT_NULL(hbox->base.add_child);
//     TEST_ASSERT_NOT_NULL(hbox->base.children);
//     TEST_ASSERT_EQUAL_INT32(0, hbox->base.count);
//     TEST_ASSERT_EQUAL_INT32(0, hbox->base.padding);
//     TEST_ASSERT_EQUAL_INT32(0, hbox->base.spacing);
//     TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.x);
//     TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.y);
//     TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.width);
//     TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.height);
//     GUI_DELETE(hbox);
// }
// void test_Label_is_properly_initiated() {
//     GUILabel* label = GUILabel_new();
//     TEST_ASSERT_NOT_NULL(label);
//     TEST_ASSERT_NULL(label->base.layout);
//     TEST_ASSERT_NOT_NULL(label->base.draw);
//     TEST_ASSERT_NOT_NULL(label->base.delete);
//     TEST_ASSERT_NOT_NULL(label->text);
//     TEST_ASSERT_EQUAL_INT32(10, label->font_size);
//     TEST_ASSERT_FALSE(label->isUpsideDown);
//     TEST_ASSERT_EQUAL_UINT8(0, label->base.x);
//     TEST_ASSERT_EQUAL_UINT8(0, label->base.y);
//     TEST_ASSERT_EQUAL_UINT8(0, label->base.width);
//     TEST_ASSERT_EQUAL_UINT8(0, label->base.height);
//     TEST_ASSERT_EQUAL_STRING("", label->text);
//     GUI_DELETE(label);
// }

// // Global flag to track if the mock delete was called
// int test_variable1 = 0;

// // The Mock Delete Function: Increments the counter AND frees the memory.
// void mock_delete_func(GUIComponent* self) {
//     test_variable1 += 1;
//     free(self);  // CRITICAL: This allows the test to be valid memory-wise.
// }

// // Helper to create a heap-allocated mock component (mimicking GUILabel_new,
// // etc.)
// GUIComponent* create_mock_component(void) {
//     // Allocate the memory on the heap
//     GUIComponent* comp = (GUIComponent*)malloc(sizeof(GUIComponent));
//     if (comp == NULL) {
//         // In a real test, you would assert failure here, but for simplicity:
//         return NULL;
//     }
//     // Initialize its V-table pointer to our spy function
//     comp->delete = mock_delete_func;
//     return comp;
// }

// void test_Container_add_child_recursive_delete() {
//     // 1. ARRANGE
//     GUIVBox* vbox = GUIVBox_new();  // Heap allocated

//     // Allocate all components on the HEAP using the helper
//     GUIComponent* test_comp1 = create_mock_component();
//     GUIComponent* test_comp2 = create_mock_component();
//     GUIComponent* test_comp3 = create_mock_component();
//     GUIComponent* test_comp4 = create_mock_component();

//     // Set the counter and add children
//     test_variable1 = 0;

//     GUI_ADD_CHILD(vbox, test_comp1);
//     GUI_ADD_CHILD(vbox, test_comp2);
//     GUI_ADD_CHILD(vbox, test_comp3);
//     GUI_ADD_CHILD(vbox, test_comp4);

//     // 2. ASSERT (Functionality Check)
//     TEST_ASSERT_NOT_NULL(vbox);
//     TEST_ASSERT_EQUAL_INT32(4, vbox->base.count);

//     // Check internal array pointers (Ensures assignment worked)
//     TEST_ASSERT_EQUAL_PTR(test_comp1, vbox->base.children[0]);

//     // 3. ACT (Recursive Delete)
//     GUI_DELETE(vbox);  // Calls VBox_delete -> loops and calls
//                        // mock_delete_func(comp) 4 times

//     // 4. ASSERT (Memory Safety Check)
//     // Check that the mock function was called exactly 4 times (meaning all
//     // children were deleted)
//     TEST_ASSERT_EQUAL_INT32(4, test_variable1);

//     // No need for further cleanup; GUI_DELETE(vbox) should have handled all
//     // memory.
// }

// void test_Nested_Container_Layout() {
//     // --- 1. ARRANGE ---

//     // Root VBox (100x100 at 0,0 - splits space vertically)
//     GUIVBox* vbox = GUIVBox_new();
//     GUI_SET_POS(vbox, 0, 0);
//     GUI_SET_SIZE(vbox, 100, 100);
//     GUI_SET_PADDING(vbox, 0);
//     GUI_SET_SPACING(vbox, 0);

//     // Child 1: A simple label (L1)
//     GUILabel* l1 = GUILabel_new();

//     // Child 2: A container (H1)
//     GUIHBox* hbox = GUIHBox_new();

//     // Children of HBox (L2, L3 - will split HBox width horizontally)
//     GUILabel* l2 = GUILabel_new();
//     GUILabel* l3 = GUILabel_new();

//     // Build the tree
//     GUI_ADD_CHILD(hbox, l2);
//     GUI_ADD_CHILD(hbox, l3);  // HBox now has L2, L3
//     GUI_ADD_CHILD(vbox, l1);
//     GUI_ADD_CHILD(vbox, hbox);  // VBox now has L1, HBox

//     // --- 2. ACT ---
//     // This call must recursively trigger layout for VBox, then HBox, then
//     // L2/L3.
//     GUI_UPDATE_LAYOUT(vbox);

//     // --- 3. ASSERTIONS ---

//     // A. Check Root VBox Children (Vertical Split)
//     // VBox has 100px height for 2 children => 50px each.

//     // L1 (Child 1 of VBox)
//     TEST_ASSERT_EQUAL_UINT8(100, l1->base.width);
//     TEST_ASSERT_EQUAL_UINT8(50, l1->base.height);
//     TEST_ASSERT_EQUAL_UINT8(0, l1->base.x);
//     TEST_ASSERT_EQUAL_UINT8(0, l1->base.y);  // Top half

//     // HBox (Child 2 of VBox)
//     TEST_ASSERT_EQUAL_UINT8(100, hbox->base.base.width);
//     TEST_ASSERT_EQUAL_UINT8(50, hbox->base.base.height);
//     TEST_ASSERT_EQUAL_UINT8(0, hbox->base.base.x);
//     TEST_ASSERT_EQUAL_UINT8(50,
//                             hbox->base.base.y);  // Bottom half (starts at
//                             Y=50)

//     // B. Check Nested HBox Children (Horizontal Split, Offset Coordinates)
//     // HBox has 100px width for 2 children => 50px each.

//     // L2 (Child 1 of HBox)
//     TEST_ASSERT_EQUAL_UINT8(50, l2->base.width);
//     TEST_ASSERT_EQUAL_UINT8(50, l2->base.height);
//     TEST_ASSERT_EQUAL_UINT8(0, l2->base.x);  // Inherits HBox X (0)
//     TEST_ASSERT_EQUAL_UINT8(
//         50, l2->base.y);  // Inherits HBox Y (50) - **CRITICAL OFFSET CHECK**

//     // L3 (Child 2 of HBox)
//     TEST_ASSERT_EQUAL_UINT8(50, l3->base.width);
//     TEST_ASSERT_EQUAL_UINT8(50, l3->base.height);
//     TEST_ASSERT_EQUAL_UINT8(50, l3->base.x);  // Offset by L2 width
//     TEST_ASSERT_EQUAL_UINT8(50, l3->base.y);  // Inherits HBox Y (50)

//     // --- 4. CLEANUP ---
//     GUI_DELETE(vbox);
// }

// void test_Safety_Null_Inputs() {
//     // 1. ARRANGE
//     // Use NULL for all components and containers
//     GUIComponent* null_comp = NULL;
//     GUILabel* null_label = NULL;
//     GUIVBox* null_vbox = NULL;
//     GUIVBox* real_vbox = NULL;

//     // Create a real component needed for the GUI_ADD_CHILD test,
//     // to ensure the function bails on the container, not the child.
//     GUILabel* real_label = GUILabel_new();

//     // An arbitrary variable to check if a function was accidentally executed
//     // (though crashing is the main failure mode)
//     int dummy_check = 0;

//     // --- 2. ACT: Test all macros with NULL ---
//     // (The primary assertion is that the entire block executes without
//     // crashing)

//     // GUIComponent Functions
//     GUI_SET_SIZE(null_comp, 10, 10);
//     GUI_SET_POS(null_comp, 5, 5);
//     GUI_DRAW(null_comp);
//     GUI_DELETE(null_comp);  // Crucial check for safe no-op

//     // GUIContainer Functions
//     GUI_ADD_CHILD(null_vbox, real_label);  // NULL Container, valid Child
//     GUI_ADD_CHILD(real_vbox, null_comp);   // Valid Container, NULL Child
//                                            // (assuming Lable is not
//                                            container)
//     GUI_UPDATE_LAYOUT(null_vbox);
//     GUI_SET_SPACING(null_vbox, 1);
//     GUI_SET_PADDING(null_vbox, 1);

//     // GUILabel Functions
//     GUI_SET_TEXT(null_label, "Should not crash");
//     GUI_SET_FONT_SIZE(null_label, 10);
//     GUI_SET_FONT_SIZE(null_label, 100);
//     GUI_SET_FONT_SIZE(null_label, -1);
//     GUI_SET_TEXT_UPSIDE_DOWN(null_label, 1);
//     GUI_SET_TEXT_UPSIDE_DOWN(null_label, 0);

//     // --- 3. ASSERT ---
//     // The test reaching this line is the SUCCESS assertion.
//     // If any function lacked a NULL check, the test would have crashed.
//     TEST_PASS();

//     // --- 4. CLEANUP ---
//     // Clean up the one component we successfully allocated
//     GUI_DELETE(real_label);
//     GUI_DELETE(real_vbox);

//     // Note: If you were modifying a global state variable (like a screen
//     buffer
//     // counter), you would assert that variable remained unchanged.
// }

// void test_VBox_Full_Height_Distribution() {
//     // 1. ARRANGE
//     GUIVBox* vbox = GUIVBox_new();

//     // Set up dimensions that result in a remainder
//     // Height 100, 3 Children -> 33.33px per child
//     GUI_SET_POS(vbox, 0, 0);
//     GUI_SET_SIZE(vbox, 50, 100);
//     GUI_SET_PADDING(vbox, 0);
//     GUI_SET_SPACING(vbox, 0);  // Keep spacing 0 to make math easy

//     // Create 3 Children
//     GUILabel* l1 = GUILabel_new();
//     GUILabel* l2 = GUILabel_new();
//     GUILabel* l3 = GUILabel_new();

//     GUI_ADD_CHILD(vbox, l1);
//     GUI_ADD_CHILD(vbox, l2);
//     GUI_ADD_CHILD(vbox, l3);

//     // 2. ACT
//     GUI_UPDATE_LAYOUT(vbox);

//     // 3. ASSERT
//     // Available Height = 100
//     // Count = 3
//     // Base Height = 100 / 3 = 33
//     // Remainder = 100 % 3 = 1

//     // Child 1 (Index 0)
//     TEST_ASSERT_EQUAL_UINT8(33, l1->base.height);
//     TEST_ASSERT_EQUAL_UINT8(0, l1->base.y);

//     // Child 2 (Index 1)
//     TEST_ASSERT_EQUAL_UINT8(33, l2->base.height);
//     TEST_ASSERT_EQUAL_UINT8(33, l2->base.y);  // Starts after l1 (0 + 33)

//     // Child 3 (Index 2 - The Last Child)
//     // This one should absorb the remainder!
//     // Expected Height = 33 + 1 = 34
//     TEST_ASSERT_EQUAL_UINT8(34, l3->base.height);
//     TEST_ASSERT_EQUAL_UINT8(66, l3->base.y);  // Starts after l2 (33 + 33)

//     // Verify Total Coverage
//     // Last child Y (66) + Last child Height (34) should equal Total Height
//     // (100)
//     TEST_ASSERT_EQUAL_INT(100, l3->base.y + l3->base.height);

//     // 4. CLEANUP
//     GUI_DELETE(vbox);
// }

// void test_HBox_Full_Width_Distribution() {
//     // 1. ARRANGE
//     GUIHBox* hbox = GUIHBox_new();

//     // Set up dimensions that result in a remainder
//     // Width 100, 3 Children -> 33.33px per child
//     // Start at X=10, Y=10 to ensure relative positioning works
//     GUI_SET_POS(hbox, 10, 10);
//     GUI_SET_SIZE(hbox, 100, 50);
//     GUI_SET_PADDING(hbox, 0);
//     GUI_SET_SPACING(hbox, 0);  // Keep spacing 0 for simple math

//     // Create 3 Children
//     GUILabel* l1 = GUILabel_new();
//     GUILabel* l2 = GUILabel_new();
//     GUILabel* l3 = GUILabel_new();

//     GUI_ADD_CHILD(hbox, l1);
//     GUI_ADD_CHILD(hbox, l2);
//     GUI_ADD_CHILD(hbox, l3);

//     // 2. ACT
//     GUI_UPDATE_LAYOUT(hbox);

//     // 3. ASSERT
//     // Available Width = 100
//     // Count = 3
//     // Base Width = 100 / 3 = 33
//     // Remainder = 100 % 3 = 1

//     // --- CHILD 1 (Index 0) ---
//     TEST_ASSERT_EQUAL_UINT8(33, l1->base.width);
//     TEST_ASSERT_EQUAL_UINT8(50, l1->base.height);  // Full height inherited
//     TEST_ASSERT_EQUAL_UINT8(10, l1->base.x);       // Start X (10) + Padding
//     (0) TEST_ASSERT_EQUAL_UINT8(10, l1->base.y);       // Start Y (10) +
//     Padding (0)

//     // --- CHILD 2 (Index 1) ---
//     TEST_ASSERT_EQUAL_UINT8(33, l2->base.width);
//     TEST_ASSERT_EQUAL_UINT8(50, l2->base.height);
//     // X = Start(10) + Padding(0) + 1 * (Width(33) + Spacing(0)) = 43
//     TEST_ASSERT_EQUAL_UINT8(43, l2->base.x);
//     TEST_ASSERT_EQUAL_UINT8(10, l2->base.y);

//     // --- CHILD 3 (Index 2 - The Last Child) ---
//     // This one should absorb the remainder!
//     // Expected Width = 33 + 1 = 34
//     TEST_ASSERT_EQUAL_UINT8(34, l3->base.width);
//     TEST_ASSERT_EQUAL_UINT8(50, l3->base.height);
//     // X = Start(10) + 0 + 2 * (33 + 0) = 10 + 66 = 76
//     TEST_ASSERT_EQUAL_UINT8(76, l3->base.x);
//     TEST_ASSERT_EQUAL_UINT8(10, l3->base.y);

//     // Verify Total Coverage
//     // Last child X (76) + Last child Width (34) should equal Parent X (10) +
//     // Width (100)
//     TEST_ASSERT_EQUAL_INT(110, l3->base.x + l3->base.width);

//     // 4. CLEANUP
//     GUI_DELETE(hbox);
// }