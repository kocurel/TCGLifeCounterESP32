#ifndef VALUECHANGEMODEL_H
#define VALUECHANGEMODEL_H
#include <stdint.h>
typedef struct {
    uint8_t player_index;
    uint8_t value_index;
    int32_t change;
} ValueChange;
#endif  // VALUECHANGEMODEL_H