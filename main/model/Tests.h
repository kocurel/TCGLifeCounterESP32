#ifndef TESTS_H
#define TESTS_H

/* * Individual Test Cases
 * These can be called manually if you want to isolate a specific bug.
 */
void test_history_basic_undo_redo(void);
void test_history_seek_navigation(void);
void test_history_truncation_on_new_action(void);
void test_history_buffer_wrap_around(void);

/**
 * @brief Executes all registered Game logic tests.
 * * This is the primary entry point for manual testing.
 * Ensure UNITY_BEGIN() has been called before invoking this.
 */
void run_all_game_tests(void);

#endif  // TESTS_H