#ifndef _Physics_Wrap_H_
#define _Physics_Wrap_H_

#include "pi_lib.h"
#include "physics_engine.h"
#include "physics_actor.h"
#include "physics_ragdoll.h"
#include "pi_transform.h"
#include "physics_scene.h"
#include "physics_mesh.h"
#include "physics_offline_tools.h"
PI_BEGIN_DECLS

void PI_API physics_engine_init(PhysicsEngine* engine, InitialType type, char* url);

void PI_API physics_engine_simulate(PhysicsEngine* engine, float tpf);

void PI_API physics_engine_release(PhysicsEngine* engine);

void PI_API physics_actor_get_mat(PiActor* actor, PiMatrix4* mat);

void PI_API physics_actor_apply_transform(PiActor* actor, TransformData* transform);

void PI_API physics_actor_get_transform(PiActor* actor, TransformData* transform);

void PI_API physics_actor_set_kinematic(PiActor* actor, PiBool is_kinematic);

void PI_API physics_actor_init(PiActor* actor);

void PI_API physics_actor_set_linear_velocity(PiActor* actor, float vx, float vy, float vz);

void PI_API physics_actor_set_force(PiActor* actor, float vx, float vy, float vz, int modeType);

void PI_API physics_actor_wakeup(PiActor * actor);

void PI_API physics_actor_set_angular_velocity(PiActor* actor, PiVector3* v);

void PI_API physics_actor_set_kinematic_target(PiActor* actor, TransformData* destination);

void PI_API physics_actor_set_sleep(PiActor* actor);

PiBool PI_API physics_actor_is_sleep(PiActor* actor);

void PI_API physics_actor_set_linear_damp(PiActor* actor, float damp);

void PI_API physics_actor_set_angular_damp(PiActor* actor, float damp);

void PI_API physics_actor_release(PiActor* actor);

void PI_API physics_collection_load(PiCollection* collection, uint8_t* data, uint size);

void PI_API physics_collection_free(PiCollection* collection);

void PI_API physics_collection_clear(PiCollection* collection);

void PI_API physics_aggregate_init(PiAggregate* agg);

void PI_API physics_aggregate_release(PiAggregate* agg);

void PI_API physics_aggregate_add(PiAggregate* agg, PiActor* actor);

void PI_API physics_scene_create(PiPhysicsScene* scene, PiVector3* gravity, void* filter);

void PI_API physics_scene_free(PiPhysicsScene* scene);

void PI_API physics_scene_add_actor(PiPhysicsScene* scene, PiActor* actor);

void PI_API physics_scene_remove_actor(PiPhysicsScene* scene, PiActor* actor);

void PI_API physics_scene_remove_aggregate(PiPhysicsScene* scene, PiAggregate* agg);

void PI_API physics_scene_add_aggregate(PiPhysicsScene* scene, PiAggregate* agg);

void PI_API physics_scene_simulate(PiPhysicsScene* scene, float tpf);

void PI_API physics_scene_fetch_results(PiPhysicsScene* scene, PiBool block, uint32* error_state);

void PI_API physics_geometry_init(PiGeometry* g);

void PI_API physics_geometry_release(PiGeometry* g);

void PI_API physics_material_create(PiPhyscisMaterial* material, float staticFriction, float dynamicFriction, float restitution);

void PI_API physics_material_free(PiPhyscisMaterial* material);

void PI_API physics_engine_attach_debug_scene(PiPhysicsScene* scene);

void PI_API physics_engine_detach_debug_scene(PiPhysicsScene* scene);

void* PI_API physics_get_defualt_filter_shader();

uint PI_API physics_cook_terrain(uint32 columns, uint32 rows, float* heightBuffer, void** output);

void PI_API physics_cook_triangle_mesh(PiMesh* mesh, PhysicsTriangleMesh* triangleMesh);

void PI_API physics_triangle_mesh_free(PhysicsTriangleMesh* mesh);

void PI_API physics_height_field_init(HeightField* heightField, void* data, uint size);

void PI_API physics_height_field_free(HeightField* heightField);

PI_END_DECLS
#endif