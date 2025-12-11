#ifndef CHANGEHISTORYMODEL_H
#define CHANGEHISTORYMODEL_H
#include <memory.h>

// #include "Players.h"
#include "ValueChange.h"
#define HISTORY_MAX_CAPACITY 10000
typedef struct {
    ValueChange changes[HISTORY_MAX_CAPACITY];
    int head;
    int count;
} ChangeHistory;

ChangeHistory* ChangeHistory_create();

void ChangeHistory_add_change(ChangeHistory* history, ValueChange new_change);
void ChangeHistory_initialize(ChangeHistory* history);
ValueChange* ChangeHistory_get_changes(const ChangeHistory* history,
                                       int* out_count);
// void ChangeHistory_undo_changes(ChangeHistory* history, Players* players,
//                                 int num_changes);
#endif  // CHANGEHISTORYMODEL_H