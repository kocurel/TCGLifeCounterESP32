#ifndef CHANGEHISTORYMODEL_H
#define CHANGEHISTORYMODEL_H
#include "Game.h"

ChangeHistory* ChangeHistory_create();

void ChangeHistory_add_change(ChangeHistory* history, ValueChange new_change);
ValueChange* ChangeHistory_get_changes(const ChangeHistory* history,
                                       int* out_count);
// void ChangeHistory_undo_changes(ChangeHistory* history, Players* players,
//                                 int num_changes);
#endif  // CHANGEHISTORYMODEL_H