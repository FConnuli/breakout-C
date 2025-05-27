#include "raylib.h"
#include "fx_coordinate_module.h"

void Move(ecs_iter_t *it) {
  Position *p = ecs_field(it, Position, 0);
  Velocity *v = ecs_field(it, Velocity, 1);

  for (int i = 0; i < it->count; i++) {
    p[i].x += v[i].x;
    p[i].y += v[i].y;
  }
}

// each 100 positional units translate to 1 pixel
void Fx_Draw_Debug_Rect(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    Extent *e = ecs_field(it, Extent, 1);
    
    for (int i = 0; i < it->count; i++) {
        DrawRectangle(
                p[i].x/100 - e[i].x/200, 
                p[i].y/100 - e[i].y/200, 
                e[i].x/100,  
                e[i].y/100, 
                RED);
    }
}
