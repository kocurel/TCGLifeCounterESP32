#include "ChangeHistory.h"

#include <stdlib.h>

#include "esp_heap_caps.h"
#include "memory.h"

void ChangeHistory_initialize(ChangeHistory* history) {
    history->head = 0;
    history->count = 0;
}

ChangeHistory* ChangeHistory_create() {
    ChangeHistory* newHistory = (ChangeHistory*)malloc(sizeof(ChangeHistory));
    for (int i = 0; i < HISTORY_MAX_CAPACITY - 1; i++) {
        ValueChange emptyValueChange = {0, 0, 0, 0};
        newHistory->changes[i] = emptyValueChange;
        newHistory->count = 0;
        newHistory->head = 0;
    }
    return newHistory;
}

void ChangeHistory_add_change(ChangeHistory* history, ValueChange new_change) {
    // 1. Write the new change to the current head position
    history->changes[history->head] = new_change;

    // 2. Advance the head index and wrap it around using the modulo operator
    history->head = (history->head + 1) % HISTORY_MAX_CAPACITY;

    // 3. Update the count
    if (history->count < HISTORY_MAX_CAPACITY) {
        // If the buffer is not full, increment the count
        history->count++;
    }
    // If the buffer IS full, the new item overwrote the oldest item,
    // so the count stays at HISTORY_MAX_CAPACITY.
}

ValueChange* ChangeHistory_get_changes(const ChangeHistory* history,
                                       int* out_count) {
    *out_count = history->count;
    ValueChange* changes_copy = (ValueChange*)heap_caps_malloc(
        history->count * sizeof(ValueChange), MALLOC_CAP_DEFAULT);
    if (!changes_copy) {
        *out_count = 0;
        return NULL;  // Memory allocation failed
    }

    // 1. Calculate the starting position (the oldest element)
    int oldest_start_index =
        (history->head - history->count + HISTORY_MAX_CAPACITY) %
        HISTORY_MAX_CAPACITY;
    for (int i = 0; i < history->count; i++) {
        // 2. Read from the starting position + offset (i)
        int index = (oldest_start_index + i) % HISTORY_MAX_CAPACITY;

        changes_copy[i] = history->changes[index];
    }
    return changes_copy;
}
// void ChangeHistory_undo_changes(ChangeHistory* history, Players* players,
//                                 int num_changes) {
//     if (num_changes <= 0 || history->count == 0) {
//         return;  // Nothing to undo
//     }

//     // Calculate the actual number of changes we can undo
//     int changes_to_undo =
//         (num_changes < history->count) ? num_changes : history->count;

//     for (int i = 0; i < changes_to_undo; i++) {
//         // Calculate the index of the change to undo
//         int index = (history->head - 1 - i + HISTORY_MAX_CAPACITY) %
//                     HISTORY_MAX_CAPACITY;
//         ValueChange change_to_undo = history->changes[index];

//         players->players_data[change_to_undo.player_index]
//             .player_values[change_to_undo.value_index] =
//             change_to_undo.old_value;
//     }
//     // Move the head back by the number of changes to undo
//     history->head = (history->head - changes_to_undo + HISTORY_MAX_CAPACITY)
//     %
//                     HISTORY_MAX_CAPACITY;

//     // Decrease the count accordingly
//     history->count -= changes_to_undo;
// }