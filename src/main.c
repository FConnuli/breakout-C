#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "flecs/addons/json.h"


#include "flecs.h"


#include "fx_coordinate_module.h"

#include "fx_collision_module.h"


#include "rlImGui.h"



#include "fx_arena.h"
#include "fx_scratch.h"

fx_arena_t scratch_arena;
fx_arena_t scratch_list;

void Fx_Draw_Debug_Position(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    
    for (int i = 0; i < it->count; i++) {
        DrawCircle(p[i].x/100, p[i].y/100, 4.0, GREEN);
    }
}


void Fx_Draw_2d(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    Texture2D *t = ecs_field(it, Texture2D, 1);
    
    for (int i = 0; i < it->count; i++) {
        DrawTexture(*t, p[i].x/100 - t[i].width/2, p[i].y/100 - t[i].height, BLUE);
    }
}

void Fx_Breakout_Collide(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    Extent *e = ecs_field(it, Extent, 1);
    ecs_query_t* active_query = ecs_query(it->world, {
            .terms = { 
            { .id = ecs_id(Position) }, 
            { .id = ecs_id(Extent) }, 
            },
    });
    ecs_iter_t active_it = ecs_query_iter(it->world, active_query);
    while (ecs_query_next(&active_it)) {
        Position *active_p = ecs_field(&active_it, Position, 0);
        Extent *active_e = ecs_field(&active_it, Extent, 1);
        for (int j = 0; j < active_it.count; j++ ) {
            for (int i = 0; i < it->count; i++) {
                if ( 
                        abs(p[i].x - active_p[j].x) <= e[i].x + active_e[i].x &&
                        abs(p[i].y - active_p[j].y) <= e[i].y + active_e[i].y ) {
                }
            }
        }
    }

}

void Fx_Breakout_Ball_Collision_Handler(ecs_iter_t *it) {
    Position *p = ecs_field(it, Position, 0);
    Velocity *v = ecs_field(it, Velocity, 1);
    Extent *e = ecs_field(it, Extent, 2);
    for (int i = 0; i < it->count; i++) {
        if (!ecs_has_pair(it->world, it->entities[i], CollisionWith, EcsWildcard)) continue;
        Position prev_pos = {p[i].x - v[i].x, p[i].y - v[i].y};
        int32_t j = 0;
        ecs_entity_t colliding_with;
        int32_t x_t = INT32_MAX;
        int32_t y_t = INT32_MAX;
        int32_t velocity_modifer = v[i].x;
        while ((colliding_with = ecs_get_target(it->world, it->entities[i], CollisionWith, j++))) {
            int32_t temp;
            const Position *collider_pos = ecs_get_id(it->world, colliding_with, ecs_id(Position));
            const Extent *collider_extent = ecs_get_id(it->world, colliding_with, ecs_id(Extent));

            if (!(collider_pos && collider_extent)) continue;
            if (v[i].x > 0) {
                temp = (collider_pos->x - collider_extent->x/2 - prev_pos.x - e[i].x/2)/v[i].x;
                if (temp < x_t) {
                    x_t = temp;
                }
            }
            if (v[i].x < 0) {
                temp = -(prev_pos.x - collider_pos->x - collider_extent->x/2  - e[i].x/2)/v[i].x;
                if (temp < x_t) {
                    x_t = temp;
                }
            }
            if (v[i].y > 0) {
                temp = (collider_pos->y - collider_extent->y/2 - prev_pos.y - e[i].y/2)/v[i].y;
                if (temp < y_t) {
                    y_t = temp;
                }
            }
            if (v[i].y < 0) {
                temp = -(prev_pos.y - collider_pos->y - collider_extent->y/2  - e[i].y/2)/v[i].y;
                if (temp < y_t) {
                    y_t = temp;
                }
            }
            const Velocity *collider_velocity = ecs_get_id(it->world, colliding_with, ecs_id(Velocity));
            if (collider_velocity) {
                if (collider_velocity->x > 400 || collider_velocity->x < -400 ) {
                    velocity_modifer = collider_velocity->x;
                } else {
                    if (v[i].x > 0) {
                    velocity_modifer = 400; 

                    } else {
                    velocity_modifer  = -400; 

                    }
                }
            }
            

        
        }    
        if (y_t >= x_t) {
            p[i].y = prev_pos.y + (v[i].y * y_t);
            v[i].x = velocity_modifer;
            v[i].y = -v[i].y;
        } else {
            p[i].x = prev_pos.x + (v[i].x * x_t);
            v[i].x = -v[i].x;
            if (velocity_modifer != -v[i].x) {
                v[i].x += velocity_modifer;
            }
        }
    }
}

void Fx_Breakout_Paddle_Collision_Handler(ecs_iter_t *it) { 
    Position *p = ecs_field(it, Position, 0);
    Velocity *v = ecs_field(it, Velocity, 1);
    Extent *e = ecs_field(it, Extent, 2);
    for (int i = 0; i < it->count; i++) {
        if (!ecs_has_pair(it->world, it->entities[i], CollisionWith, EcsWildcard)) continue;
        int32_t j = 0;
        ecs_entity_t colliding_with;
        while ((colliding_with = ecs_get_target(it->world, it->entities[i], CollisionWith, j++))) {
            if (ecs_has_id(it->world, colliding_with, ecs_id(Velocity))) continue;
            const Position *collider_pos = ecs_get_id(it->world, colliding_with, ecs_id(Position));
            const Extent *collider_extent = ecs_get_id(it->world, colliding_with, ecs_id(Extent));
            if (p[i].x > collider_pos->x) {
                p[i].x = collider_pos->x + collider_extent->x/2 + e[i].x/2;
            } else {
                p[i].x = collider_pos->x - collider_extent->x/2 - e[i].x/2;
            }
            v[i].x = 0;
    }
    }

}

void Fx_Breakout_Brick_Collision_Handler(ecs_iter_t *it) {
    for (int i = 0; i < it->count; i++) {
        ecs_delete(it->world, it->entities[i]);
    }
}

void Fx_Breakout_Paddle_Behavior(ecs_iter_t *it) {
    Velocity *v = ecs_field(it, Velocity, 0);
    for (int i = 0; i < it->count; i++) {
        if (IsKeyDown(KEY_LEFT)) {
            v[i].x -= 220;

        }
        else if (IsKeyDown(KEY_RIGHT)) {
            v[i].x += 220;
        
        }
        else {
            if (v[i].x > 0) {
                v[i].x -= 90;
            }
            if (v[i].x < 0) {
                v[i].x += 90;
            }
            if (abs(v[i].x) < 100) {
                v[i].x = 0;
            }
        }
        if (v[i].x > 900) {
            v[i].x = 900;
        }
        if (v[i].x < -900) {
            v[i].x = -900;
        }
    }
}


void start_child_scene(void* (*scene)()) {
    void* (*next_scene)();
    next_scene = scene;
    while (next_scene) {
        next_scene = (next_scene)();
    }
}

 

    // create collision realm

void* scene_breakout() {
    void* (*next_scene)() = NULL;
    // LEVEL INIT
    ecs_world_t *ecs = ecs_init();

    ECS_IMPORT(ecs, FxCollision);
    
    ECS_TAG(ecs, Point_Draw);
    ECS_TAG(ecs, Breakable);
    ECS_TAG(ecs, Paddle);
    ECS_TAG(ecs, Ball);

    
    ECS_SYSTEM(ecs, Move, EcsOnUpdate, fx.collision.Position, fx.collision.Velocity);
    ECS_SYSTEM(ecs, Fx_Breakout_Paddle_Behavior, EcsOnLoad, [out]fx.collision.Velocity, Paddle);
    ECS_SYSTEM(ecs, Fx_Draw_Debug_Position, 0, fx.collision.Position, Point_Draw);
    ECS_SYSTEM(ecs, Fx_Draw_Debug_Rect, 0, fx.collision.Position, fx.collision.Extent);
 

    // create collision realms
    ecs_entity_t realm = fx_collision_create_realm(ecs);
    ecs_entity_t wall_realm = fx_collision_create_realm(ecs);

        ecs_system(ecs, {
            .entity = ecs_entity(ecs, {
                    .name = "Fx_Breakout_Ball_Collision_Handler",
                    .add = ecs_ids( ecs_dependson(EcsPreUpdate) )
            }),
            .query.terms = {
                {ecs_id(Position), .src = {}},
                {ecs_id(Velocity), .src = {}},
                {ecs_id(Extent), .src = {}},
                {ecs_id(Ball), .src = {}},
                {ecs_pair(CollisionIn, realm), .src = {}},
            },
            .callback = Fx_Breakout_Ball_Collision_Handler,
            });
        ecs_system(ecs, {
            .entity = ecs_entity(ecs, {
                    .name = "Fx_Breakout_Brick_Collision_Handler",
                    .add = ecs_ids( ecs_dependson(EcsPreUpdate) )
            }),
            .query.terms = {
                {ecs_id(Breakable), .src = {}},
                {ecs_pair(CollisionIn, realm), .src = {}},
            },
            .callback = Fx_Breakout_Brick_Collision_Handler,
            });
        ecs_system(ecs, {
            .entity = ecs_entity(ecs, {
                    .name = "Fx_Breakout_Paddle_Collision_Handler",
                    .add = ecs_ids( ecs_dependson(EcsPreStore) )
            }),
            .query.terms = {
                {ecs_id(Position), .src = {}},
                {ecs_id(Velocity), .src = {}},
                {ecs_id(Extent), .src = {}},
                {ecs_id(Paddle), .src = {}},
                {ecs_pair(CollisionIn, wall_realm), .src = {}},
            },
            .callback = Fx_Breakout_Paddle_Collision_Handler,
            });
     


    // create brick grid
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 15; j++) {
            ecs_entity_t e = ecs_new(ecs);
            ecs_set(ecs, e, Position, { i * 8000 + 24000, j * 3000 + 7000 });
            ecs_set(ecs, e, Extent, { 7000, 2000 });
            ecs_add(ecs, e, Breakable);
            fx_collision_register_passive_collider(ecs, realm, e);
        }
    }
    
    // setup level bounds
    ecs_entity_t e;
    e = ecs_new(ecs);
    ecs_add(ecs, e, Point_Draw);
    ecs_set(ecs, e, Position, { 96000, 0 });
    ecs_set(ecs, e, Extent, { 7000000, 2000 });
    fx_collision_register_passive_collider(ecs, realm, e);
    fx_collision_register_passive_collider(ecs, wall_realm, e);
    
    
    e = ecs_new(ecs);
    ecs_add(ecs, e, Point_Draw);
    ecs_set(ecs, e, Position, { 0, 54000 });
    ecs_set(ecs, e, Extent, { 2000, 70000000 });
    fx_collision_register_passive_collider(ecs, realm, e);
    fx_collision_register_passive_collider(ecs, wall_realm, e);
    
    e = ecs_new(ecs);
    ecs_add(ecs, e, Point_Draw);
    ecs_set(ecs, e, Position, { 192000, 54000 });
    ecs_set(ecs, e, Extent, { 2000, 70000000 });
    fx_collision_register_passive_collider(ecs, realm, e);
    fx_collision_register_passive_collider(ecs, wall_realm, e);
    
    e = ecs_new(ecs);
    ecs_add(ecs, e, Point_Draw);
    ecs_add(ecs, e, Paddle);
    ecs_set(ecs, e, Position, { 96000, 100000 });
    ecs_set(ecs, e, Velocity, { 0, 0 });
    ecs_set(ecs, e, Extent, { 25600, 3200 });
    fx_collision_register_passive_collider(ecs, realm, e);
    fx_collision_register_active_collider(ecs, wall_realm, e);
    
    
    // create ball
    e = ecs_new(ecs);
    ecs_add(ecs, e, Point_Draw);
    ecs_set(ecs, e, Position, { 36000, 54000 });
    ecs_set(ecs, e, Extent, { 2000, 2000 });
    ecs_set(ecs, e, Velocity, { 400, -400 });
    ecs_add(ecs, e, Ball);
    fx_collision_register_active_collider(ecs, realm, e);

    ecs_query_t *all = ecs_query(ecs, {.expr="_"});

    // Main game loop
    while (!WindowShouldClose() && !next_scene)    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        

            ecs_progress(ecs, 0);
            if (IsKeyPressed(KEY_R)) {
                next_scene = &scene_breakout;
            }



        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
                ecs_run(ecs, ecs_id(Fx_Draw_Debug_Rect), 0, 0);
                DrawFPS(10, 10);
                char str[30];
                int32_t entity_count = 0;
                ecs_iter_t all_iter = ecs_query_iter(ecs, all);
                while (ecs_iter_next(&all_iter)) {
                    entity_count += all_iter.count;
                }
                sprintf(str, "entities: %8d", entity_count);
                DrawText(str, 20, 22, 30, GREEN);
        EndDrawing();
        fx_arena_free(&scratch_arena);
        //----------------------------------------------------------------------------------
    }
    
    return next_scene;
}

void* scene_title_screen() {
    void* (*next_scene)() = NULL;


    while (!WindowShouldClose() && !next_scene)    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        if (IsKeyPressed(KEY_SPACE)) {
            next_scene = &scene_breakout;
        }


        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(CLITERAL(Color) {160, 0, 0, 255});
            
                
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    return next_scene;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - tesseract view");

    scratch_arena = fx_arena_create();
    scratch_list = fx_arena_create();
    

    rlImGuiSetup(true);
    
    SetTargetFPS(60);                
    start_child_scene(&scene_title_screen);
    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}

