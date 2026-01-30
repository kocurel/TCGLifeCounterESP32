// #include "Assert.h"  // Your custom asserts for additional logic checks
// #include "Game.h"
// #include "unity.h"

// // This runs before every test case
// void setUp(void) { Game_init(); }

// // This runs after every test case
// void tearDown(void) {
//     // Reset state if necessary
// }

// void test_history_basic_undo_redo(void) {
//     // Initial HP is 20
//     uint8_t p0 = 0;
//     uint8_t hp_val = 0;

//     Game_set_value(15, p0, hp_val);  // Change 1: 20 -> 15
//     Game_set_value(10, p0, hp_val);  // Change 2: 15 -> 10

//     TEST_ASSERT_EQUAL_INT32(10, Game_get_value(p0, hp_val));

//     Game_undo();  // Back to 15
//     TEST_ASSERT_EQUAL_INT32(15, Game_get_value(p0, hp_val));

//     Game_undo();  // Back to 20
//     TEST_ASSERT_EQUAL_INT32(20, Game_get_value(p0, hp_val));

//     Game_redo();  // Forward to 15
//     TEST_ASSERT_EQUAL_INT32(15, Game_get_value(p0, hp_val));
// }

// void test_history_seek_navigation(void) {
//     uint8_t p0 = 0;
//     uint8_t hp_val = 0;

//     // Create a timeline: 20 -> 19 -> 18 -> 17 -> 16
//     for (int i = 19; i >= 16; i--) {
//         Game_set_value(i, p0, hp_val);
//     }

//     // Seek to 3 steps back (should be 19 HP)
//     Game_seek_history(3);
//     TEST_ASSERT_EQUAL_INT32(19, Game_get_value(p0, hp_val));

//     // Seek to 0 steps back (the present: 16 HP)
//     Game_seek_history(0);
//     TEST_ASSERT_EQUAL_INT32(16, Game_get_value(p0, hp_val));
// }

// void test_history_truncation_on_new_action(void) {
//     uint8_t p0 = 0;
//     uint8_t hp_val = 0;

//     Game_set_value(15, p0, hp_val);  // Step 1
//     Game_set_value(10, p0, hp_val);  // Step 2

//     Game_undo();  // Back to 15. Cursor is now at index 1, Head is at
//     index 2.

//     // Perform a NEW action. This should wipe the "Redo" to 10.
//     Game_set_value(30, p0, hp_val);

//     // Try to redo. It should do nothing because the future was truncated.
//     Game_redo();
//     TEST_ASSERT_EQUAL_INT32(30, Game_get_value(p0, hp_val));

//     Game_undo();  // Should go back to 15
//     TEST_ASSERT_EQUAL_INT32(15, Game_get_value(p0, hp_val));
// }

// void test_history_buffer_wrap_around(void) {
//     uint8_t p0 = 0;
//     uint8_t hp_val = 0;

//     // Fill the buffer beyond capacity
//     // HISTORY_MAX_CAPACITY is 2000
//     for (int i = 0; i < HISTORY_MAX_CAPACITY + 10; i++) {
//         Game_set_value(i, p0, hp_val);
//     }

//     // The count should stay capped
//     TEST_ASSERT_EQUAL_INT32(HISTORY_MAX_CAPACITY,
//                             Game_get_change_history_count());

//     // We should be able to undo exactly 2000 times, but not 2001.
//     for (int i = 0; i < HISTORY_MAX_CAPACITY; i++) {
//         Game_undo();
//     }

//     int32_t val_at_floor = Game_get_value(p0, hp_val);

//     // Attempting one more undo should change nothing (hitting the tail)
//     Game_undo();
//     TEST_ASSERT_EQUAL_INT32(val_at_floor, Game_get_value(p0, hp_val));
// }

// void run_all_game_tests(void) {
//     UNITY_BEGIN();
//     RUN_TEST(test_history_basic_undo_redo);
//     RUN_TEST(test_history_seek_navigation);
//     RUN_TEST(test_history_truncation_on_new_action);
//     RUN_TEST(test_history_buffer_wrap_around);
//     UNITY_END();
// }