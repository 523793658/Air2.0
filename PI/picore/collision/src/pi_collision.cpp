#include <pi_collision.h>

#define PI_MAT_2_ICE_MAT(piMat, iceMat) \
	iceMat.m[0][0] = piMat.m[0][0]; \
	iceMat.m[0][1] = piMat.m[1][0]; \
	iceMat.m[0][2] = piMat.m[2][0]; \
	iceMat.m[0][3] = piMat.m[3][0]; \
	iceMat.m[1][0] = piMat.m[0][1]; \
	iceMat.m[1][1] = piMat.m[1][1]; \
	iceMat.m[1][2] = piMat.m[2][1]; \
	iceMat.m[1][3] = piMat.m[3][1]; \
	iceMat.m[2][0] = piMat.m[0][2]; \
	iceMat.m[2][1] = piMat.m[1][2]; \
	iceMat.m[2][2] = piMat.m[2][2]; \
	iceMat.m[2][3] = piMat.m[3][2]; \
	iceMat.m[3][0] = piMat.m[0][3]; \
	iceMat.m[3][1] = piMat.m[1][3]; \
	iceMat.m[3][2] = piMat.m[2][3]; \
	iceMat.m[3][3] = piMat.m[3][3]

static void get_face_normal(PiVector3 *dst, const PiVector3 *pa, const PiVector3 *pb, const PiVector3 *pc)
{
	PiVector3 ab, bc;

	pi_vec3_sub(&ab, pb, pa);
	pi_vec3_sub(&bc, pc, pb);
	pi_vec3_cross(dst, &ab, &bc);
	pi_vec3_normalise(dst, dst);
}

Opcode::Model* PI_API pi_collision_obj_new(PiMesh* mesh)
{

	void *vertex_buffer, *index_buffer;
	uint num_vertices, num_indices;

	EGeometryType geometry_type;
	EIndexType index_type;

	Opcode::Model* opc_model;
	Opcode::OPCODECREATE opcc;

	Opcode::MeshInterface* meshInterface;

	vertex_buffer = pi_mesh_get_pos(mesh, &geometry_type, &num_vertices);
	index_buffer = pi_mesh_get_index(mesh, &index_type, &num_indices);
	PI_ASSERT(geometry_type == EGOT_TRIANGLE_LIST, "only triangle list supported");
	PI_ASSERT(num_indices % 3 == 0 && index_type == EINDEX_32BIT, "only 3x bit32 index supported");

	meshInterface = new Opcode::MeshInterface;
	meshInterface->SetNbTriangles(num_indices / 3);
	meshInterface->SetNbVertices(num_vertices);
	meshInterface->SetPointers((IceMaths::IndexedTriangle*)index_buffer, (IceMaths::Point*)vertex_buffer);

	opcc.mIMesh = meshInterface;
	opcc.mSettings.mRules = Opcode::SPLIT_GEOM_CENTER | Opcode::SPLIT_SPLATTER_POINTS;
	opcc.mSettings.mLimit = 1;
	opcc.mNoLeaf = true;
	opcc.mQuantized = false;
	opcc.mKeepOriginal = false;

	opc_model = new Opcode::Model;
	opc_model->Build(opcc);
	return opc_model;
}
void PI_API pi_collision_obj_free(Opcode::Model* opc_model)
{
	const Opcode::MeshInterface* meshInterface;
	meshInterface = opc_model->GetMeshInterface();
	if (meshInterface) 
	{
		delete meshInterface;
	}
	delete opc_model;
}


PiCollisionModel* PI_API pi_collision_model_build(Opcode::Model* opc_model)
{
	PiCollisionModel* model = pi_new0(PiCollisionModel, 1);
	PiMatrix4 mat;
	model->opc_model = opc_model;
	pi_mat4_set_identity(&mat);
	pi_collision_model_update_transform(model, &mat);
	return model;
}

void PI_API pi_collision_model_destroy(PiCollisionModel* model)
{
	pi_free(model);
}


PiCollisionModel* PI_API pi_collision_model_new(PiMesh* mesh)
{
	PiCollisionModel* model = pi_new0(PiCollisionModel, 1);
	PiMatrix4 mat;
	pi_collision_model_update_mesh(model, mesh);
	pi_mat4_set_identity(&mat);
	pi_collision_model_update_transform(model, &mat);

	return model;
}

void PI_API pi_collision_model_update_mesh(PiCollisionModel* model, PiMesh* mesh)
{
	//TODO:可能可以使用更轻量的更新方法而非完全重建opc_model
	void *vertex_buffer, *index_buffer;
	uint num_vertices, num_indices;
	EGeometryType geometry_type;
	EIndexType index_type;
	Opcode::Model* opc_model;
	Opcode::OPCODECREATE opcc;
	Opcode::MeshInterface* meshInterface;

	vertex_buffer = pi_mesh_get_pos(mesh, &geometry_type, &num_vertices);
	index_buffer = pi_mesh_get_index(mesh, &index_type, &num_indices);
	PI_ASSERT(geometry_type == EGOT_TRIANGLE_LIST, "only triangle list supported");
	PI_ASSERT(num_indices % 3 == 0 && index_type == EINDEX_32BIT, "only 3x bit32 index supported");

	meshInterface = new Opcode::MeshInterface;
	meshInterface->SetNbTriangles(num_indices / 3);
	meshInterface->SetNbVertices(num_vertices);
	meshInterface->SetPointers((IceMaths::IndexedTriangle*)index_buffer, (IceMaths::Point*)vertex_buffer);

	opcc.mIMesh = meshInterface;
	opcc.mSettings.mRules = Opcode::SPLIT_GEOM_CENTER | Opcode::SPLIT_SPLATTER_POINTS;
	opcc.mSettings.mLimit = 1;
	opcc.mNoLeaf = true;
	opcc.mQuantized = false;
	opcc.mKeepOriginal = false;

	opc_model = new Opcode::Model;
	opc_model->Build(opcc);

	//delete meshInterface;
	delete model->opc_model;
	model->opc_model = opc_model; 
}

void PI_API pi_collision_model_update_transform(PiCollisionModel* model, PiMatrix4* world_matrix) 
{
	PI_MAT_2_ICE_MAT((*world_matrix), model->world_matrix);
}

void PI_API pi_collision_model_free(PiCollisionModel* model)
{
	const Opcode::MeshInterface* meshInterface;
	meshInterface = model->opc_model->GetMeshInterface();
	if (meshInterface)
	{
		delete meshInterface;
	}
	delete model->opc_model;
	pi_free(model);
}

void PI_API pi_collision_ray_collide(PiCollisionModel* model, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, PiBool first_contact, PiBool closest_hit, PiBytes* hit_pront_buffer, PiBytes* hit_normal_buffer)
{
	float dx, dy, dz, dist;
	PiBool noErr;
	Opcode::RayCollider collider;
	Opcode::CollisionFaces collisionFaces;
	Opcode::VertexPointers vps;
	IceMaths::Ray ray;

	dx = end_x - start_x;
	dy = end_y - start_y;
	dz = end_z - start_z;
	dist = pi_math_sqrt(dx*dx + dy*dy + dz*dz);
	dx /= dist;
	dy /= dist;
	dz /= dist;

	collider.SetFirstContact(first_contact);
	collider.SetCulling(false);
	collider.SetClosestHit(closest_hit);
	collider.SetMaxDist(dist);
	collider.SetDestination(&collisionFaces);
	collider.SetTemporalCoherence(false);

	ray.mOrig.Set(start_x, start_y, start_z);
	ray.mDir.Set(dx, dy, dz);

	noErr = collider.Collide(ray, (*model->opc_model), &model->world_matrix);
	if(noErr && collider.GetContactStatus() && collider.GetNbIntersections()) {
		uint i, faceId;
		float mDistance;
		PiVector3 mPoint;
		PiVector3 mNormal;

		pi_bytes_write_int32(hit_pront_buffer, collisionFaces.GetNbFaces());
		for (i = 0; i < collisionFaces.GetNbFaces(); i++)
		{
			mDistance = collisionFaces.GetFaces()[i].mDistance;
			faceId = collisionFaces.GetFaces()[i].mFaceID;
			pi_vec3_set(&mPoint, start_x, start_y, start_z);
			mPoint.x += dx * mDistance;
			mPoint.y += dy * mDistance;
			mPoint.z += dz * mDistance;

			pi_bytes_write_float(hit_pront_buffer, mPoint.x);
			pi_bytes_write_float(hit_pront_buffer, mPoint.y);
			pi_bytes_write_float(hit_pront_buffer, mPoint.z);

			if(hit_normal_buffer != NULL) {
				model->opc_model->GetMeshInterface()->GetTriangle(vps, faceId);
				get_face_normal(&mNormal, (PiVector3*)vps.Vertex[0], (PiVector3*)vps.Vertex[1], (PiVector3*)vps.Vertex[2]);
				pi_bytes_write_float(hit_normal_buffer, mNormal.x);
				pi_bytes_write_float(hit_normal_buffer, mNormal.y);
				pi_bytes_write_float(hit_normal_buffer, mNormal.z);
			}
		}
	}
	else
	{
		pi_bytes_write_int32(hit_pront_buffer, 0);
	}
}

PiBool PI_API pi_collision_model_collide(PiCollisionModel* model0, PiCollisionModel* model1)
{
	Opcode::AABBTreeCollider collider;
	Opcode::BVTCache cache;
	PiBool noErr = false;

	collider.SetFirstContact(true);
	collider.SetFullBoxBoxTest(false);
	collider.SetFullPrimBoxTest(false);
	collider.SetTemporalCoherence(false);

	cache.Model0 = model0->opc_model;
	cache.Model1 = model1->opc_model;

	PI_ASSERT(collider.ValidateSettings() == 0, "collider is not ready yet");
	noErr = collider.Collide(cache, &model0->world_matrix, &model1->world_matrix);

	if (noErr && collider.GetContactStatus() && collider.GetNbPairs() > 0) {
		return TRUE;
	}

	return FALSE;
}
