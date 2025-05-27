#ifndef FX_COORDINATE_MODULE_H
#define FX_COORDINATE_MODULE_H

#include "flecs.h"
#include "stdint.h"

typedef struct {
  int32_t x, y;
} Position, Extent, Velocity;

void Move(ecs_iter_t *it);

void Fx_Draw_Debug_Rect(ecs_iter_t *it);

#endif
