/* This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 */

#include "gtest/gtest.h"

#include <pi_spatial.h>

static PiBool _is_aabb_equal(PiAABBBox *a, PiAABBBox *b)
{
	return pi_vec3_is_equal(&a->minPt, &b->minPt)
	       && pi_vec3_is_equal(&a->maxPt, &b->maxPt);
}

// Geometry同步Geometry，测试transform属性
TEST(SpatialSyncTest, G_G_Transform)
{
	PiSpatial *parent = pi_spatial_geometry_create();
	PiSpatial *child = pi_spatial_geometry_create();
	PiSpatial *grand = pi_spatial_geometry_create();

	pi_spatial_attach_sync(parent, child);
	pi_spatial_attach_sync(child, grand);

	pi_spatial_set_local_translation(parent, 20, 0, 0);

	pi_spatial_update(parent);

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_translation(grand), pi_spatial_get_world_translation(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_translation(child), pi_spatial_get_world_translation(parent)));

	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_world_rotation(grand), pi_spatial_get_world_rotation(child)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_world_rotation(child), pi_spatial_get_world_rotation(parent)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_scaling(grand), pi_spatial_get_world_scaling(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_scaling(child), pi_spatial_get_world_scaling(parent)));

	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_world_transform(child), pi_spatial_get_world_transform(parent)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_world_transform(child), pi_spatial_get_world_transform(parent)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_translation(grand), pi_spatial_get_local_translation(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_translation(child), pi_spatial_get_local_translation(parent)));

	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_local_rotation(grand), pi_spatial_get_local_rotation(child)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_local_rotation(child), pi_spatial_get_local_rotation(parent)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_scaling(grand), pi_spatial_get_local_scaling(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_scaling(child), pi_spatial_get_local_scaling(parent)));

	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_local_transform(child), pi_spatial_get_local_transform(parent)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_local_transform(child), pi_spatial_get_local_transform(parent)));

	pi_spatial_destroy(parent);
	pi_spatial_destroy(child);
	pi_spatial_destroy(grand);
}

// Geometry同步Geometry，子Geometry不设置自己的AABB，测试aabb属性
TEST(SpatialSyncTest, G_G_AABB_NoSetLocalAABB)
{
	PiSpatial *parent = pi_spatial_geometry_create();
	PiSpatial *child = pi_spatial_geometry_create();

	pi_spatial_set_local_translation(parent, 20, 0, 0);

	pi_spatial_attach_sync(parent, child);

	{
		PiVector3 pt;
		PiAABBBox aabb;
		pi_aabb_init(&aabb);

		pi_vec3_set(&pt, 0.0f, 0.0f, 0.0f);
		pi_aabb_add_point(&aabb, &pt);

		pi_vec3_set(&pt, 1.0f, 1.0f, 1.0f);
		pi_aabb_add_point(&aabb, &pt);

		pi_geometry_set_local_aabb(parent, &aabb);
	}

	pi_spatial_update(parent);

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_translation(parent), pi_spatial_get_world_translation(child)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_world_rotation(parent), pi_spatial_get_world_rotation(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_scaling(parent), pi_spatial_get_world_scaling(child)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_world_transform(parent), pi_spatial_get_world_transform(child)));
	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_world_aabb(parent), pi_spatial_get_world_aabb(child)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_translation(parent), pi_spatial_get_local_translation(child)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_local_rotation(parent), pi_spatial_get_local_rotation(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_scaling(parent), pi_spatial_get_local_scaling(child)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_local_transform(parent), pi_spatial_get_local_transform(child)));
	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_local_aabb(parent), pi_spatial_get_local_aabb(child)));

	pi_spatial_destroy(parent);
	pi_spatial_destroy(child);
}

// Geometry同步Geometry，子Geometry设置自己的AABB，测试aabb属性
TEST(SpatialSyncTest, G_G_AABB_SetLocalAABB)
{
	PiSpatial *parent = pi_spatial_geometry_create();
	PiSpatial *child = pi_spatial_geometry_create();

	PiVector3 tr;
	pi_vec3_set(&tr, 20.0f, 0.0f, 0.0f);

	pi_spatial_set_local_translation(parent, tr.x, tr.y, tr.z);

	pi_spatial_attach_sync(parent, child);

	{
		PiVector3 pt;
		PiAABBBox aabb;
		pi_aabb_init(&aabb);

		pi_vec3_set(&pt, 0.0f, 0.0f, 0.0f);
		pi_aabb_add_point(&aabb, &pt);

		pi_vec3_set(&pt, 1.0f, 1.0f, 1.0f);
		pi_aabb_add_point(&aabb, &pt);

		pi_geometry_set_local_aabb(parent, &aabb);
	}

	PiVector3 pt;

	PiAABBBox child_local;
	pi_aabb_init(&child_local);

	pi_vec3_set(&pt, 0.0f, 0.0f, 0.0f);
	pi_aabb_add_point(&child_local, &pt);

	pi_vec3_set(&pt, 2.0f, 2.0f, 2.0f);
	pi_aabb_add_point(&child_local, &pt);

	pi_geometry_set_local_aabb(child, &child_local);

	pi_spatial_update(parent);

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_translation(parent), pi_spatial_get_world_translation(child)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_world_rotation(parent), pi_spatial_get_world_rotation(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_scaling(parent), pi_spatial_get_world_scaling(child)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_world_transform(parent), pi_spatial_get_world_transform(child)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_translation(parent), pi_spatial_get_local_translation(child)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_local_rotation(parent), pi_spatial_get_local_rotation(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_scaling(parent), pi_spatial_get_local_scaling(child)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_local_transform(parent), pi_spatial_get_local_transform(child)));

	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_local_aabb(child), &child_local));

	PiAABBBox child_world;
	pi_aabb_init(&child_world);

	pi_vec3_set(&pt, tr.x + child_local.minPt.x, tr.y + child_local.minPt.y, tr.z + child_local.minPt.z);
	pi_aabb_add_point(&child_world, &pt);

	pi_vec3_set(&pt, tr.x + child_local.maxPt.x, tr.y + child_local.maxPt.y, tr.z + child_local.maxPt.z);
	pi_aabb_add_point(&child_world, &pt);
	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_world_aabb(child), &child_world));

	pi_spatial_destroy(parent);
	pi_spatial_destroy(child);
}

// Geometry同步Node，子Geometry不设置自己的AABB，测试aabb属性
TEST(SpatialSyncTest, N_G_Transform_AABB_NoSetLocalAABB)
{
	PiSpatial *parent = pi_spatial_node_create();
	PiSpatial *child = pi_spatial_geometry_create();
	PiSpatial *sync = pi_spatial_geometry_create();

	pi_node_attach_child(parent, child);
	pi_spatial_attach_sync(parent, sync);

	pi_spatial_set_local_translation(child, 0, 20, 0);
	pi_spatial_set_local_translation(parent, 20, 0, 0);

	PiVector3 pt;
	PiAABBBox child_local;
	pi_aabb_init(&child_local);

	pi_vec3_set(&pt, 0.0f, 0.0f, 0.0f);
	pi_aabb_add_point(&child_local, &pt);

	pi_vec3_set(&pt, 1.0f, 1.0f, 1.0f);
	pi_aabb_add_point(&child_local, &pt);

	pi_geometry_set_local_aabb(child, &child_local);

	pi_spatial_update(parent);

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_translation(sync), pi_spatial_get_world_translation(parent)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_world_rotation(sync), pi_spatial_get_world_rotation(parent)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_scaling(sync), pi_spatial_get_world_scaling(parent)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_world_transform(sync), pi_spatial_get_world_transform(parent)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_translation(sync), pi_spatial_get_local_translation(parent)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_local_rotation(sync), pi_spatial_get_local_rotation(parent)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_scaling(sync), pi_spatial_get_local_scaling(parent)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_local_transform(sync), pi_spatial_get_local_transform(parent)));

	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_local_aabb(sync), pi_spatial_get_local_aabb(parent)));
	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_world_aabb(sync), pi_spatial_get_world_aabb(parent)));

	pi_spatial_destroy(parent);
	pi_spatial_destroy(child);
	pi_spatial_destroy(sync);
}

/* 
 * node，geometry组成父子
 * 另有一个parent_sync的geometry同步node，一个child_sync的geometry同步geometry，
 * geometry设LOCALAABB，parent_sync和child_sync不设LOCALAABB 
 */
TEST(SpatialSyncTest, N_G_G_G_Transform_AABB_NoSetLocalAABB)
{
	PiSpatial *parent = pi_spatial_node_create();
	PiSpatial *child = pi_spatial_geometry_create();
	PiSpatial *parent_sync = pi_spatial_geometry_create();
	PiSpatial *child_sync = pi_spatial_geometry_create();

	pi_spatial_attach_sync(parent, parent_sync);

	pi_spatial_set_local_translation(parent, 20.0f, 0.0f, 0.0f);
	pi_spatial_set_local_rotation(parent, 0.0f, 1.0f, 0.0f, 0.0f);
	pi_spatial_update(parent);
	
	PiVector3 pt;
	PiAABBBox child_local;
	pi_aabb_init(&child_local);
	pi_vec3_set(&pt, 0.0f, 0.0f, 0.0f);
	pi_aabb_add_point(&child_local, &pt);
	pi_vec3_set(&pt, 1.0f, 1.0f, 1.0f);
	pi_aabb_add_point(&child_local, &pt);
	pi_geometry_set_local_aabb(child, &child_local);
	
	pi_spatial_attach_sync(child, child_sync);

	pi_node_attach_child(parent, child);

	pi_spatial_update(parent);

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_translation(parent_sync), pi_spatial_get_world_translation(parent)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_world_rotation(parent_sync), pi_spatial_get_world_rotation(parent)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_scaling(parent_sync), pi_spatial_get_world_scaling(parent)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_world_transform(parent_sync), pi_spatial_get_world_transform(parent)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_translation(parent_sync), pi_spatial_get_local_translation(parent)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_local_rotation(parent_sync), pi_spatial_get_local_rotation(parent)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_scaling(parent_sync), pi_spatial_get_local_scaling(parent)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_local_transform(parent_sync), pi_spatial_get_local_transform(parent)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_translation(child), pi_spatial_get_world_translation(parent)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_world_rotation(child), pi_spatial_get_world_rotation(parent)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_scaling(child), pi_spatial_get_world_scaling(parent)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_world_transform(child), pi_spatial_get_world_transform(parent)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_translation(child_sync), pi_spatial_get_world_translation(child)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_world_rotation(child_sync), pi_spatial_get_world_rotation(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_world_scaling(child_sync), pi_spatial_get_world_scaling(child)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_world_transform(child_sync), pi_spatial_get_world_transform(child)));

	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_translation(child_sync), pi_spatial_get_local_translation(child)));
	ASSERT_EQ(TRUE, pi_quat_is_equal(pi_spatial_get_local_rotation(child_sync), pi_spatial_get_local_rotation(child)));
	ASSERT_EQ(TRUE, pi_vec3_is_equal(pi_spatial_get_local_scaling(child_sync), pi_spatial_get_local_scaling(child)));
	ASSERT_EQ(TRUE, pi_mat4_is_equal(pi_spatial_get_local_transform(child_sync), pi_spatial_get_local_transform(child)));

	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_local_aabb(parent_sync), pi_spatial_get_local_aabb(parent)));
	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_world_aabb(parent_sync), pi_spatial_get_world_aabb(parent)));

	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_local_aabb(child), pi_spatial_get_local_aabb(parent)));
	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_world_aabb(child), pi_spatial_get_world_aabb(parent)));

	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_local_aabb(child_sync), pi_spatial_get_local_aabb(child)));
	ASSERT_EQ(TRUE, _is_aabb_equal(pi_spatial_get_world_aabb(child_sync), pi_spatial_get_world_aabb(child)));

	pi_spatial_destroy(parent);
	pi_spatial_destroy(child);
	pi_spatial_destroy(parent_sync);
	pi_spatial_destroy(child_sync);
}

TEST(SpatialSyncTest, AttachProblem)
{
	PiSpatial *a = pi_spatial_node_create();
	PiSpatial *b = pi_spatial_node_create();
	PiSpatial *c = pi_spatial_geometry_create();

	PiVector3 pt;
	PiAABBBox local;
	
	// init aabb of c
	pi_aabb_init(&local);
	pi_vec3_set(&pt, 0.0f, 0.0f, 0.0f);
	pi_aabb_add_point(&local, &pt);
	pi_vec3_set(&pt, 1.0f, 1.0f, 1.0f);
	pi_aabb_add_point(&local, &pt);
	pi_geometry_set_local_aabb(c, &local);

	pi_spatial_set_local_translation(b, 1.0f, 1.0f, 1.0f);
	pi_spatial_set_local_translation(a, 2.0f, 2.0f, 2.0f);

	pi_node_attach_child(a, b);
	
	pi_node_attach_child(b, c);
	
	PiAABBBox aabb_assert;
	pi_aabb_init(&aabb_assert);
	pi_vec3_set(&pt, 3.0f, 3.0f, 3.0f);
	pi_aabb_add_point(&aabb_assert, &pt);
	pi_vec3_set(&pt, 4.0f, 4.0f, 4.0f);
	pi_aabb_add_point(&aabb_assert, &pt);
	
	PiAABBBox* aabb = pi_spatial_get_world_aabb(c);
	ASSERT_EQ(TRUE, _is_aabb_equal(aabb, &aabb_assert));
}