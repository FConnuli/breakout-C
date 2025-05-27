#include "fx_collision_module.h"
#include "flecs.h"
#include "fx_arena.h"
#include "fx_scratch.h"
#include <stdint.h>


ECS_COMPONENT_DECLARE(Position);

ECS_COMPONENT_DECLARE(Velocity);

ECS_COMPONENT_DECLARE(Extent);

ECS_COMPONENT_DECLARE(CollisionRealm);

ECS_TAG_DECLARE(CollisionRoot);

ECS_TAG_DECLARE(ActiveCollider);

ECS_TAG_DECLARE(PassiveCollider);

ECS_TAG_DECLARE(CollisionWith);

ECS_TAG_DECLARE(CollisionIn);

ECS_TAG_DECLARE(ElementOf);

#define TOTAL_SPACIAL_HASH_BINS  4096

#define BIN_WIDTH 25600

typedef struct {
  ecs_query_t *active, 
              *passive;
} CollisionRealm;

typedef struct Node {
    ecs_entity_t entity;
    uint32_t next_index;
} Node;

inline int32_t abs(int32_t num) {
    return num & INT32_MIN;
}

uint16_t hash(int32_t x, int32_t y) {
    return 
        ((uint16_t) (x /BIN_WIDTH))%64 + 
        ((((uint16_t) (y /BIN_WIDTH))%64) << 3);
}

// SYSTEM
void Fx_Collide(ecs_iter_t *it) {
    CollisionRealm *realm = ecs_field(it, CollisionRealm, 0);
    ecs_remove_all(it->world, ecs_pair(CollisionWith, EcsWildcard));
    ecs_remove_all(it->world, ecs_pair(CollisionIn, EcsWildcard));
    ecs_remove_all(it->world, ecs_pair(ElementOf, EcsWildcard));

    for (int i = 0; i < it->count; i++) {
        ecs_iter_t passive_it = ecs_query_iter(it->world, realm[i].passive);

        uint32_t spacial_hash_map[TOTAL_SPACIAL_HASH_BINS] = {0};
        fx_arena_alloc(&scratch_list, sizeof(Node));

        while (ecs_query_next(&passive_it)){
            Position *passive_p = ecs_field(&passive_it, Position, 0);
            Extent *passive_e = ecs_field(&passive_it, Extent, 1);
            for (int k = 0; k < passive_it.count; k++) {
                for(int32_t x = passive_p[k].x - passive_e[k].x - BIN_WIDTH; x <= passive_p[k].x + passive_e[k].x; x += BIN_WIDTH) {
                    for(int32_t y = passive_p[k].y - passive_e[k].y - BIN_WIDTH; y <= passive_p[k].y + passive_e[k].y; y += BIN_WIDTH) {
                        uint16_t hash_index = hash(x, y);
                        fx_arena_alloc(&scratch_list, sizeof(Node));
                        uint32_t node_index = (scratch_list.mem.size/sizeof(Node)) - 1;
                        ((Node*)scratch_list.mem.ptr)[node_index] = (Node){
                            .entity = passive_it.entities[k],
                            .next_index = spacial_hash_map[hash_index],
                        };
                        spacial_hash_map[hash_index] = node_index;
                    }
                }
            }
        }
        
        ecs_iter_t active_it = ecs_query_iter(it->world, realm[i].active);
        while (ecs_query_next(&active_it)){
            Position *active_p = ecs_field(&active_it, Position, 0);
            Extent *active_e = ecs_field(&active_it, Extent, 1);
            for (int j = 0; j < active_it.count; j++) {        
                ecs_entity_t active_root = active_it.entities[j];
                while (ecs_has_pair(it->world, active_root, EcsChildOf, EcsWildcard)) {
                    active_root = ecs_get_parent(it->world, active_root);
                }
                
                for(int32_t x = active_p[j].x - active_e[j].x - BIN_WIDTH; x <= active_p[j].x + active_e[j].x; x += BIN_WIDTH) {
                    for(int32_t y = active_p[j].y - active_e[j].y - BIN_WIDTH; y <= active_p[j].y + active_e[j].y; y += BIN_WIDTH) {
                        uint16_t hash_index = hash(x, y);
                        uint32_t passive_idx = spacial_hash_map[hash_index];
                while (passive_idx) {
                    Node passive_node = ((Node*) scratch_list.mem.ptr)[passive_idx];
                    passive_idx = passive_node.next_index;

                    const Position* passive_p = ecs_get(it->world, passive_node.entity, Position);
                    const Extent* passive_e = ecs_get(it->world, passive_node.entity, Extent);

                    int k = 0;    
                        ecs_entity_t passive_root = passive_node.entity; 
                        while (ecs_has_pair(it->world, passive_root, EcsChildOf, EcsWildcard)) {
                            passive_root = ecs_get_parent(it->world, passive_root);
                        }
                        if ( 
                                abs(passive_p[k].x - active_p[j].x) < (passive_e[k].x + active_e[j].x)/2 &&
                                abs(passive_p[k].y - active_p[j].y) < (passive_e[k].y + active_e[j].y)/2 ) {
                            ecs_add_pair(it->world, active_root, CollisionWith, passive_root);
                            ecs_add_pair(it->world, passive_root, CollisionWith, active_root);
                            ecs_add_pair(it->world, active_root, CollisionIn, it->entities[i]);
                            ecs_add_pair(it->world, passive_root, CollisionIn, it->entities[i]);
                        }  
                    
                }
                    }
                }
            }
        }
        fx_arena_free(&scratch_list);
    }
}

// SYSTEM
void Fx_Collide_old(ecs_iter_t *it) {
    CollisionRealm *realm = ecs_field(it, CollisionRealm, 0);
    ecs_remove_all(it->world, ecs_pair(CollisionWith, EcsWildcard));
    ecs_remove_all(it->world, ecs_pair(CollisionIn, EcsWildcard));
    ecs_remove_all(it->world, ecs_pair(ElementOf, EcsWildcard));

    for (int i = 0; i < it->count; i++) {
        ecs_iter_t active_it = ecs_query_iter(it->world, realm[i].active);
        while (ecs_query_next(&active_it)){
            Position *active_p = ecs_field(&active_it, Position, 0);
            Extent *active_e = ecs_field(&active_it, Extent, 1);
            for (int j = 0; j < active_it.count; j++) {        
                ecs_entity_t active_root = active_it.entities[j];
                while (ecs_has_pair(it->world, active_root, EcsChildOf, EcsWildcard)) {
                    active_root = ecs_get_parent(it->world, active_root);
                }
                ecs_iter_t passive_it = ecs_query_iter(it->world, realm[i].passive);
                while (ecs_query_next(&passive_it)){
                    Position *passive_p = ecs_field(&passive_it, Position, 0);
                    Extent *passive_e = ecs_field(&passive_it, Extent, 1);

                    for (int k = 0; k < passive_it.count; k++) {    
                        ecs_entity_t passive_root = passive_it.entities[k];
                        while (ecs_has_pair(it->world, passive_root, EcsChildOf, EcsWildcard)) {
                            passive_root = ecs_get_parent(it->world, passive_root);
                        }
                        if ( 
                                abs(passive_p[k].x - active_p[j].x) < (passive_e[k].x + active_e[j].x)/2 &&
                                abs(passive_p[k].y - active_p[j].y) < (passive_e[k].y + active_e[j].y)/2 ) {
                            ecs_add_pair(it->world, active_root, CollisionWith, passive_root);
                            ecs_add_pair(it->world, passive_root, CollisionWith, active_root);
                            ecs_add_pair(it->world, active_root, CollisionIn, it->entities[i]);
                            ecs_add_pair(it->world, passive_root, CollisionIn, it->entities[i]);
                        }  
                    }
                }
            }
        }
    }
}

void FxCollisionImport(ecs_world_t* w) {
    ECS_MODULE(w, FxCollision);

    ECS_COMPONENT_DEFINE(w, Position);
    ECS_COMPONENT_DEFINE(w, Velocity);
    ECS_COMPONENT_DEFINE(w, Extent);
    ECS_COMPONENT_DEFINE(w, CollisionRealm);
    ECS_TAG_DEFINE(w, CollisionRoot);
    ECS_TAG_DEFINE(w, ActiveCollider);
    ECS_TAG_DEFINE(w, PassiveCollider);
    ECS_TAG_DEFINE(w, CollisionWith);
    ECS_TAG_DEFINE(w, CollisionIn);
    ECS_TAG_DEFINE(w, ElementOf);
    ecs_add_id(w, ecs_id(CollisionWith), EcsRelationship);
    ecs_add_id(w, ecs_id(CollisionIn), EcsRelationship);
    ecs_add_id(w, ecs_id(ActiveCollider), EcsRelationship);
    ecs_add_id(w, ecs_id(PassiveCollider), EcsRelationship);
 

    ECS_SYSTEM(w, Fx_Collide, EcsPostUpdate, CollisionRealm);
}

ecs_entity_t fx_collision_create_realm(ecs_world_t *w) {
    ecs_entity_t realm = ecs_new(w);
    ecs_set(w, realm, CollisionRealm, {
            .active = ecs_query(w ,{ .terms = {
                    {ecs_id(Position), .src = {}}, 
                    {ecs_id(Extent), .src = {}},
                    {ecs_pair(ActiveCollider, realm), .src = {}},
                    } }),      
            .passive = ecs_query(w , { .terms = {
                    {ecs_id(Position), .src = {}}, 
                    {ecs_id(Extent), .src = {}},
                    {ecs_pair(PassiveCollider, realm), .src = {}},
                    } }),              });
    return realm;
}

void fx_collision_register_active_collider(ecs_world_t* w, ecs_entity_t realm, ecs_entity_t collider) {
    ecs_add_pair(w, collider, ActiveCollider, realm);
}

void fx_collision_register_passive_collider(ecs_world_t* w, ecs_entity_t realm, ecs_entity_t collider) {
    ecs_add_pair(w, collider, PassiveCollider, realm);
}
