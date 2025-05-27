#ifndef FX_COLLISION_MODULE_H
#define FX_COLLISION_MODULE_H

#include "flecs.h"
#include "flecs/addons/flecs_c.h"
#include "flecs/addons/module.h"
#include "fx_coordinate_module.h"


extern ECS_COMPONENT_DECLARE(CollisionRealm);

extern ECS_COMPONENT_DECLARE(Position);

extern ECS_COMPONENT_DECLARE(Velocity);

extern ECS_COMPONENT_DECLARE(Extent);

extern ECS_COMPONENT_DECLARE(CollisionRealm);

extern ECS_TAG_DECLARE(CollisionRoot);

extern ECS_TAG_DECLARE(ActiveCollider);

extern ECS_TAG_DECLARE(PassiveCollider);

extern ECS_TAG_DECLARE(CollisionWith);

extern ECS_TAG_DECLARE(CollisionIn);

//extern ECS_TAG_DECLARE(ElementOf);


void FxCollisionImport(ecs_world_t* w);

ecs_entity_t fx_collision_create_realm(ecs_world_t* w);

void fx_collision_register_active_collider(ecs_world_t* w, ecs_entity_t realm, ecs_entity_t collider);

void fx_collision_register_passive_collider(ecs_world_t* w, ecs_entity_t realm, ecs_entity_t collider);

#endif
