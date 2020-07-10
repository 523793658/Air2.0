#include <list>
#include <map>
#include "algorithm"
#include "pi_mesh.h"
#include "pi_vector3.h"
#include "pi_mesh_prog.h"
using namespace std;

class Triangle;
class Vertex;
using namespace std;
class Triangle {
public:
	Vertex *         vertex[3];
	PiVector3           normal;
	PiVector3			origin_normal;
	Triangle(Vertex *v0, Vertex *v1, Vertex *v2);
	~Triangle();
	void             ComputeNormal();
	void             ReplaceVertex(Vertex *vold, Vertex *vnew);
	int              HasVertex(Vertex *v);
};
class Vertex {
public:
	PiVector3           position; 
	float			uv[2];
	int              id;       
	list<Vertex *>   neighbor;
	list<Triangle *> face;    
	float            objdist; 
	float			 objBoundary;
	PiBool			 is_Boundary;
	Vertex *         collapse;
	Vertex(PiVector3 v, int _id, float[2]);
	~Vertex();
	void             RemoveIfNonNeighbor(Vertex *n);
};
list<Vertex *>   vertices;
list<Triangle *> triangles;


Triangle::Triangle(Vertex *v0, Vertex *v1, Vertex *v2){
	PI_ASSERT(v0 != v1 && v1 != v2 && v2 != v0, "error");
	vertex[0] = v0;
	vertex[1] = v1;
	vertex[2] = v2;
	ComputeNormal();
	triangles.push_back(this);
	for (int i = 0; i<3; i++) {
		vertex[i]->face.push_back(this);
		for (int j = 0; j<3; j++) if (i != j) {
			if (std::find(vertex[i]->neighbor.begin(), vertex[i]->neighbor.end(), vertex[j]) == vertex[i]->neighbor.end())
			{
				vertex[i]->neighbor.push_back(vertex[j]);
			}
		}
	}
}
Triangle::~Triangle(){
	int i;
	triangles.remove(this);
	for (i = 0; i<3; i++) {
		if (vertex[i]) vertex[i]->face.remove(this);
	}
	for (i = 0; i<3; i++) {
		int i2 = (i + 1) % 3;
		if (!vertex[i] || !vertex[i2]) continue;
		vertex[i]->RemoveIfNonNeighbor(vertex[i2]);
		vertex[i2]->RemoveIfNonNeighbor(vertex[i]);
	}
}
int Triangle::HasVertex(Vertex *v) {
	return (v == vertex[0] || v == vertex[1] || v == vertex[2]);
}
void Triangle::ComputeNormal(){
	PiVector3 v0 = vertex[0]->position;
	PiVector3 v1 = vertex[1]->position;
	PiVector3 v2 = vertex[2]->position;
	PiVector3 e1, e2;
	pi_vec3_sub(&e1, &v1, &v0);
	pi_vec3_sub(&e2, &v2, &v1);
	pi_vec3_cross(&normal, &e1, &e2);
	if (pi_vec3_len(&normal) == 0)return;
	pi_vec3_normalise(&normal, &normal);
}
void Triangle::ReplaceVertex(Vertex *vold, Vertex *vnew) {
	PI_ASSERT(vold && vnew, "error");
	PI_ASSERT(vold == vertex[0] || vold == vertex[1] || vold == vertex[2], "error");
	PI_ASSERT(vnew != vertex[0] && vnew != vertex[1] && vnew != vertex[2], "error");
	if (vold == vertex[0]){
		vertex[0] = vnew;
	}
	else if (vold == vertex[1]){
		vertex[1] = vnew;
	}
	else {
		PI_ASSERT(vold == vertex[2], "error");
		vertex[2] = vnew;
	}
	int i;
	vold->face.remove(this);
	vnew->face.push_back(this);
	for (i = 0; i < 3; i++) {
		vold->RemoveIfNonNeighbor(vertex[i]);
		vertex[i]->RemoveIfNonNeighbor(vold);
	}
	for (i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) if (i != j) {
			if (std::find(vertex[i]->neighbor.begin(), vertex[i]->neighbor.end(), vertex[j]) == vertex[i]->neighbor.end())
			{
				vertex[i]->neighbor.push_back(vertex[j]);
			}
		}
	}
	ComputeNormal();
}

Vertex::Vertex(PiVector3 v, int _id, float uv[2]) {
	position = v;
	id = _id;
	vertices.push_back(this);
}

Vertex::~Vertex(){
	while (neighbor.size()) {
		list<Vertex*>::iterator it;
		it = neighbor.begin();
		(*it)->neighbor.remove(this);
		neighbor.remove(*it);
	}
	vertices.remove(this);
}
void Vertex::RemoveIfNonNeighbor(Vertex *n) {
	if (std::find(neighbor.begin(), neighbor.end(), n) == neighbor.end()) return;
	for (auto it = face.begin(); it != face.end(); it++)
	{
		if ((*it)->HasVertex(n)){
			return;
		}
	}
	neighbor.remove(n);
}


float ComputeEdgeCollapseCost(Vertex *u, Vertex *v) {
	PiVector3 dir;
	pi_vec3_sub(&dir, &v->position, &u->position);
	float edgelength = pi_vec3_len(&dir);
	float curvature = 0;
	pi_vec3_normalise(&dir, &dir);
	float dirValue = 10000000;

	for (auto n : u->neighbor)
	{
		PiVector3 dir1;
		pi_vec3_sub(&dir1, &n->position, &u->position);
		pi_vec3_normalise(&dir1, &dir1);
		
		float d = (1 + (pi_vec3_dot(&dir1, &dir))) / 2;
		dirValue = min(d, dirValue);
	}
	dirValue += 0.001;


	// find the "sides" triangles that are on the edge uv
	list<Triangle *> sides;
	for (auto it : u->face)
	{
		if (it->HasVertex(v))
		{
			sides.push_back(it);
		}
	}


	// use the triangle facing most away from the sides 
	// to determine our curvature term
	for (auto i : u->face)
	{
		float mincurv = 1;
		for (auto j : sides)
		{
			float dotprod = pi_vec3_dot(&i->normal, &j->normal);
			mincurv = min(mincurv, (1 - dotprod) / 2.0f);
		}
		curvature = max(curvature, mincurv);
	}
	curvature += 0.001;
	// the more coplanar the lower the curvature term   
	return curvature * dirValue * edgelength;
}

void ComputeEdgeCostAtVertex(Vertex *v) {
	// compute the edge collapse cost for all edges that start
	// from vertex v.  Since we are only interested in reducing
	// the object by selecting the min cost edge at each step, we
	// only cache the cost of the least cost edge at this vertex
	// (in member variable collapse) as well as the value of the 
	// cost (in member variable objdist).
	if (v->is_Boundary)
	{
		v->objdist = 1000000;
		return;
	}
	if (v->neighbor.size() == 0) {
		// v doesn't have neighbors so it costs nothing to collapse
		v->collapse = NULL;
		v->objdist = -0.01f;
		return;
	}
	v->objdist = 1000000;
	v->collapse = NULL;
	// search all neighboring edges for "least cost" edge
	for (auto i : v->neighbor)
	{
		float dist;
		dist = ComputeEdgeCollapseCost(v, i);
		if (dist < v->objdist)
		{
			v->collapse = i;
			v->objdist = dist;
		}
	}
}
void ComputeAllEdgeCollapseCosts() {
	// For all the edges, compute the difference it would make
	// to the model if it was collapsed.  The least of these
	// per vertex is cached in each vertex object.
	for (auto vertex : vertices)
	{
		ComputeEdgeCostAtVertex(vertex);
	}
}

void Collapse(Vertex *u, Vertex *v){
	// Collapse the edge uv by moving vertex u onto v
	// Actually remove tris on uv, then update tris that
	// have u to have v, and then remove u.
	if (!v) {
		// u is a vertex all by itself so just delete it
		delete u;
		return;
	}
	int i;
	list<Vertex *>tmp;
	// make tmp a list of all the neighbors of u
	for (auto vertex : u->neighbor)
	{
		tmp.push_back(vertex);
	}
	// delete triangles on edge uv:

	for (i = u->face.size() - 1; i >= 0; i--)
	{
		auto it = u->face.begin();
		advance(it, i);
		if ((*it)->HasVertex(v))
		{
			delete (*it);
		}
	}
	// update remaining triangles to have v instead of u
	for (i = u->face.size() - 1; i >= 0; i--)
	{
		auto it = u->face.begin();
		advance(it, i);
		(*it)->ReplaceVertex(u, v);
	}
	delete u;
	// recompute the edge collapse costs for neighboring vertices
	for (auto it : tmp)
	{
		ComputeEdgeCostAtVertex(it);
	}
}

Vertex *MinimumCostEdge(){
	Vertex *mn = *vertices.begin();
	for (auto vertex : vertices)
	{
		if (vertex->objdist < mn->objdist)
		{
			mn = vertex;
		}
	}
	return mn;
}


PiMesh* PI_API pi_mesh_prog(PiMesh* mesh, MeshReduceType type)
{
	EGeometryType egt;
	uint i, vertexNum, indexNum, texCoordNum, texSize;

	EIndexType indexType;

	PiVector3* vDatas = pi_mesh_get_pos(mesh, &egt, &vertexNum);
	float* uvDatas = pi_mesh_get_texcoord(mesh, 0, &texCoordNum, &texSize);
	void* indexData = pi_mesh_get_index(mesh, &indexType, &indexNum);
	

	for (i = 0; i < vertexNum; i++)
	{
		new Vertex(vDatas[i], i, &uvDatas[i * 2]);
	}

	for (i = 0; i < indexNum / 3; i++)
	{
		int vi0, vi1, vi2;
		if (indexType == EINDEX_16BIT)
		{
			vi0 = (int)((uint16*)indexData)[i * 3];
			vi1 = (int)((uint16*)indexData)[i * 3 + 1];
			vi2 = (int)((uint16*)indexData)[i * 3 + 2];
		}
		else
		{
			vi0 = (int)((uint32*)indexData)[i * 3];
			vi1 = (int)((uint32*)indexData)[i * 3 + 1];
			vi2 = (int)((uint32*)indexData)[i * 3 + 2];
		}
		auto v0 = vertices.begin();
		advance(v0, vi0);

		auto v1 = vertices.begin();
		advance(v1, vi1);

		auto v2 = vertices.begin();
		advance(v2, vi2);
		Triangle* tri = new Triangle(*v0, *v1, *v2);
		tri->ComputeNormal();
		pi_vec3_copy(&tri->origin_normal, &tri->normal);
	}

	for (auto v : vertices)
	{
		float angle = 0.0;
		for (auto face : v->face)
		{
			for (int i = 0; i < 3; i++)
			{
				PiVector3 e1, e2;
				if (face->vertex[i] == v)
				{
					pi_vec3_sub(&e1, &face->vertex[(i + 1) % 3]->position, &v->position);
					pi_vec3_sub(&e2, &face->vertex[(i + 2) % 3]->position, &v->position);
					float x = pi_vec3_dot(&e1, &e2);
					x = x / (pi_vec3_len(&e1) * pi_vec3_len(&e2));
					angle += pi_math_acos(x);
				}
			}
		}
		if (angle < 6.27)
		{
			v->is_Boundary = TRUE;
		}
		else
		{
			v->is_Boundary = FALSE;
		}
	}

	ComputeAllEdgeCollapseCosts();
	while (vertices.size() > 3)
	{
		Vertex* mn = MinimumCostEdge();
		if (type == MRDT_WATER)
		{
			if (mn->objdist < 0.001)
			{
				Collapse(mn, mn->collapse);
			}
			else
			{
				break;
			}
		}
		else if (type == MRDT_TERRAIN)
		{
			if (mn->objdist < 0.0001)
			{
				Collapse(mn, mn->collapse);
			}
			else
			{
				break;
			}
		}
		else
		{
			if (mn->objdist < 0.0001)
			{
				Collapse(mn, mn->collapse);
			}
			else
			{
				break;
			}
		}
	}
	for (auto face : triangles)
	{
		face->ComputeNormal();
	}


	PiVector3* posData = pi_new(PiVector3, vertices.size());
	PiVector3* norData = pi_new0(PiVector3, vertices.size());
	float* uvData = pi_new0(float, vertices.size()*texSize);
	void* iData;
	if (indexType == EINDEX_16BIT)
	{
		iData = pi_new0(uint16, triangles.size() * 3);
	}
	else
	{
		iData = pi_new0(uint32, triangles.size() * 3);
	}
	map<int, int> verMap;
	i = 0;
	for (list<Vertex*>::iterator iter = vertices.begin(); iter != vertices.end(); iter++, i++)
	{
		pi_vec3_copy(&posData[i], &(*iter)->position);
		for (auto face : (*iter)->face)
		{
			pi_vec3_add(&norData[i], &norData[i], &face->normal);
		}
		pi_vec3_normalise(&norData[i], &norData[i]);
		pi_memcmp_inline(&uvData[i * texSize], (*iter)->uv, sizeof(float) * texSize);
		verMap.insert(std::pair<int, int>(((*iter)->id), i));
	}
	i = 0;
	for (list<Triangle*>::iterator it = triangles.begin(); it != triangles.end(); i++, it++)
	{
		int index0 = verMap[(*it)->vertex[0]->id];
		int index1 = verMap[(*it)->vertex[1]->id];
		int index2 = verMap[(*it)->vertex[2]->id];
		PiBool is_revert = FALSE;
// 		if (pi_vec3_dot(&(*it)->origin_normal, &(*it)->normal) < 0.1)
// 		{
// 			is_revert = TRUE;
// 		}
		if (indexType == EINDEX_16BIT)
		{
			if (is_revert){
				*(((uint16*)iData) + i * 3) = (uint16)index1;
				*(((uint16*)iData) + i * 3 + 1) = (uint16)index0;
			}
			else
			{
				*(((uint16*)iData) + i * 3) = (uint16)index0;
				*(((uint16*)iData) + i * 3 + 1) = (uint16)index1;
			}
			*(((uint16*)iData) + i * 3 + 2) = (uint16)index2;
		}
		else
		{
			if (is_revert)
			{
				*(((uint32*)iData) + i * 3) = (uint32)index1;
				*(((uint32*)iData) + i * 3 + 1) = (uint32)index0;
			}
			else
			{
				*(((uint32*)iData) + i * 3) = (uint32)index0;
				*(((uint32*)iData) + i * 3 + 1) = (uint32)index1;
			}
			*(((uint32*)iData) + i * 3 + 2) = (uint32)index2;
		}
	}

	

	


	PiMesh* outputMesh = pi_mesh_create(EGOT_TRIANGLE_LIST, indexType, vertices.size(), posData, NULL, norData, uvData, triangles.size() * 3, iData);

	pi_free(posData);
	pi_free(norData);
	pi_free(uvData);
	pi_free(iData);

	while (triangles.size())
	{
		delete (*triangles.begin());
	}

	while (vertices.size())
	{
		delete (*vertices.begin());
	}
	return outputMesh;
}