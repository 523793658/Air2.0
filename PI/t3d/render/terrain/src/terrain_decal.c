
#include "terrain_decal.h"

#define TERRAIN_DECAL_COUNT 1000

static float _min_value(float a, float b, float c, float d)
{
	float value = a;
	if (value > b)
	{
		value = b;
	}
	if (value > c)
	{
		value = c;
	}
	if (value > d)
	{
		value = d;
	}
	return value;
}

static float _max_value(float a, float b, float c, float d)
{
	float value = a;
	if (value < b)
	{
		value = b;
	}
	if (value < c)
	{
		value = c;
	}
	if (value < d)
	{
		value = d;
	}
	return value;
}

static void _extend_segment(PiVector3 *p1, PiVector3 *p2)
{
	PiVector3 dir;
	pi_vec3_sub(&dir, p1, p2);
	pi_vec3_normalise(&dir, &dir);
	pi_vec3_scale(&dir, &dir, 0.1f);
	pi_vec3_add(p1, p1, &dir);
	pi_vec3_sub(p2, p2, &dir);
}

static float _point_to_segment_dis(const PiVector3 *point, const PiVector3 *seg_point1, const PiVector3 *seg_point2)
{
	float a = seg_point2->z - seg_point1->z;
	float b = seg_point1->x - seg_point2->x;
	float c = seg_point2->x * seg_point1->z - seg_point1->x * seg_point2->z;
	PI_ASSERT(!pi_vec3_is_equal(seg_point1, seg_point2), "点到直线的距离算法中，构建直线的两点不能重合");
	return pi_math_abs(a * point->x + b * point->z + c) / pi_math_sqrt(a * a + b * b);
}

static PiBool _point_on_segment(const PiVector3 *segment_p1, const PiVector3 *segment_p2, const PiVector3 *point)
{
	/*叉积是否为0，判断是否在同一直线上*/
	double max_x, min_x, max_z, min_z, value;
	PiVector3 p1 = *segment_p1;
	PiVector3 p2 = *segment_p2;
	_extend_segment(&p1, &p2);
	value = (p1.x - point->x) * (p2.z - point->z) - (p2.x - point->x) * (p1.z - point->z);
	if (value < -FLOAT_TOLERANCE || value > FLOAT_TOLERANCE)
	{
		return FALSE;
	}
	/*判断是否在线段上*/
	max_x = p1.x > p2.x ? p1.x : p2.x;
	min_x = p1.x < p2.x ? p1.x : p2.x;
	max_z = p1.z > p2.z ? p1.z : p2.z;
	min_z = p1.z < p2.z ? p1.z : p2.z;
	min_x -= FLOAT_TOLERANCE;
	min_z -= FLOAT_TOLERANCE;
	max_x += FLOAT_TOLERANCE;
	max_z += FLOAT_TOLERANCE;
	if (point->x >= min_x && point->x <= max_x && point->z >= min_z && point->z <= max_z)
	{
		return TRUE;
	}
	return FALSE;
}

/*求直线与平面的交点*/
static PiBool _line_intersect_plane(PiVector3 *dest, const PiVector3 *plane_dir, const PiVector3 *plane_point, const PiVector3 *line_dir, const PiVector3 *line_point)
{
	double vp1, vp2, vp3, n1, n2, n3, v1, v2, v3, m1, m2, m3, t, vpt;
	vp1 = plane_dir->x;
	vp2 = plane_dir->y;
	vp3 = plane_dir->z;
	n1 = plane_point->x;
	n2 = plane_point->y;
	n3 = plane_point->z;
	v1 = line_dir->x;
	v2 = line_dir->y;
	v3 = line_dir->z;
	m1 = line_point->x;
	m2 = line_point->y;
	m3 = line_point->z;
	vpt = v1 * vp1 + v2 * vp2 + v3 * vp3;
	/*首先判断直线是否与平面平行*/
	if (vpt > -FLOAT_TOLERANCE && vpt < FLOAT_TOLERANCE)
	{
		return FALSE;
	}
	else
	{
		t = ((n1 - m1) * vp1 + (n2 - m2) * vp2 + (n3 - m3) * vp3) / vpt;
		dest->x = (float)(m1 + v1 * t);
		dest->y = (float)(m2 + v2 * t);
		dest->z = (float)(m3 + v3 * t);
		return TRUE;
	}
}

static PiBool _segment_intersect_plane(PiVector3 *dest, const PiVector3 *plane_dir, const PiVector3 *plane_point, PiVector3 *seg_point_a, PiVector3 *seg_point_b)
{
	PiVector3 dir;
	pi_vec3_sub(&dir, seg_point_a, seg_point_b);
	/*先保证直线与平面相交, 然后判断是否在线段中*/
	if (_line_intersect_plane(dest, plane_dir, plane_point, &dir, seg_point_b))
	{
		if (_point_on_segment(seg_point_a, seg_point_b, dest))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/* Determine whether a ray intersect with a triangle
 * Parameters
 * orig: origin of the ray
 * dir: direction of the ray
 * v0, v1, v2: vertices of triangle
 * t(out): weight of the intersection for the ray
 * u(out), v(out): barycentric coordinate of intersection
*/
static PiBool intersect_triangle(const PiVector3 *orig, const PiVector3 *dir, const PiVector3 *v0, const PiVector3 *v1, const PiVector3 *v2, float *u, float *v)
{
	PiVector3 edge1, edge2, pvec, tvec, qvec;
	double det, fInvDet;
	/*E1*/
	pi_vec3_sub(&edge1, v1, v0);
	/*E2*/
	pi_vec3_sub(&edge2, v2, v0);
	/*P*/
	pi_vec3_cross(&pvec, dir, &edge2);
	/*determinant*/
	det = pi_vec3_dot(&edge1, &pvec);
	if (det > 0)
	{
		pi_vec3_sub(&tvec, orig, v0);
	}
	else
	{
		pi_vec3_sub(&tvec, v0, orig);
		det = -det;
	}
	*u = pi_vec3_dot(&tvec, &pvec);
	pi_vec3_cross(&qvec, &tvec, &edge1);
	*v = pi_vec3_dot(dir, &qvec);
	/*Calculate t, scale parameters, ray intersects triangle*/
	fInvDet = 1.0f / det;
	*u *= (float)fInvDet;
	*v *= (float)fInvDet;
	return TRUE;
}

static PiBool _point_is_in_triangle(PiVector3 *dest, const PiVector3 *point_a, const PiVector3 *point_b, const PiVector3 *point_c, const PiVector3 *point, PiBool is_tri)
{
	PiVector3 v0, v1, v2;
	double dot00, dot01, dot02, dot11, dot12;
	double inver_deno, u, v;

	PiVector3 a = *point_a;
	PiVector3 b = *point_b;
	PiVector3 c = *point_c;
	PiVector3 p = *point;
	a.y = b.y = c.y = p.y = 0;

	pi_vec3_sub(&v0, &c, &a);
	pi_vec3_sub(&v1, &b, &a);
	pi_vec3_sub(&v2, &p, &a);

	dot00 = pi_vec3_dot(&v0, &v0);
	dot01 = pi_vec3_dot(&v0, &v1);
	dot02 = pi_vec3_dot(&v0, &v2);
	dot11 = pi_vec3_dot(&v1, &v1);
	dot12 = pi_vec3_dot(&v1, &v2);
	inver_deno = 1.0f / (dot00 * dot11 - dot01 * dot01);
	u = (dot11 * dot02 - dot01 * dot12) * inver_deno;
	if (u < 0 || u > 1) /*if u out of range, return directly*/
	{
		return FALSE;
	}
	v = (dot00 * dot12 - dot01 * dot02) * inver_deno;
	if (v < 0 || v > 1) /*if v out of range, return directly*/
	{
		return FALSE;
	}
	if (u + v <= 1)
	{
		if (is_tri)
		{
			pi_vec3_set(dest, point->x, point->y, point->z);
		}
		else
		{
			PiVector3 orig, dir;
			float u, v;
			pi_vec3_set(&orig, point->x, 0, point->z);
			pi_vec3_set(&dir, 0, 1.0f, 0);
			intersect_triangle(&orig, &dir, point_a, point_b, point_c, &u, &v);
			a.y = point_a->y + u * (point_b->y - point_a->y) + v * (point_c->y - point_a->y);
			pi_vec3_set(dest, point->x, a.y, point->z);
		}
		return TRUE;
	}
	return FALSE;
}

static PiBool _two_segment_is_intersect(PiVector3 *dest, const PiVector3 *point_a, const PiVector3 *point_b, const PiVector3 *point_c, const PiVector3 *point_d)
{
	/*解线性方程组, 求线段交点*/
	double denominator = (point_b->z - point_a->z) * (point_d->x - point_c->x) - (point_a->x - point_b->x) * (point_c->z - point_d->z);
	double x, z;
	if (denominator > -FLOAT_TOLERANCE && denominator < FLOAT_TOLERANCE)
	{
		return FALSE;
	}
	/*线段所在直线的交点坐标 (x , y)*/
	x = ((point_b->x - point_a->x) * (point_d->x - point_c->x) * (point_c->z - point_a->z) +
		(point_b->z - point_a->z) * (point_d->x - point_c->x) * point_a->x -
		(point_d->z - point_c->z) * (point_b->x - point_a->x) * point_c->x) / denominator;
	z = -((point_b->z - point_a->z) * (point_d->z - point_c->z) * (point_c->x - point_a->x) +
		(point_b->x - point_a->x) * (point_d->z - point_c->z) * point_a->z -
		(point_d->x - point_c->x) * (point_b->z - point_a->z) * point_c->z) / denominator;
	/*判断交点是否在两条线段上*/
	if ((x - point_a->x) * (x - point_b->x) < FLOAT_TOLERANCE &&
		(z - point_a->z) * (z - point_b->z) < FLOAT_TOLERANCE &&
		(x - point_c->x) * (x - point_d->x) < FLOAT_TOLERANCE &&
		(z - point_c->z) * (z - point_d->z) < FLOAT_TOLERANCE)
	{
		if (point_a->y - point_b->y == 0)
		{
			dest->y = point_a->y;
		}
		else if (point_a->y < point_b->y)
		{
			/*在XZ平面，点p与A点的距离*/
			double dis_p_a = pi_math_sqrt((float)((x - point_a->x) * (x - point_a->x) + (z - point_a->z) * (z - point_a->z)));
			/*点B与A点的距离*/
			double dis_b_a = pi_math_sqrt((point_b->x - point_a->x) * (point_b->x - point_a->x) + (point_b->z - point_a->z) * (point_b->z - point_a->z));
			/*B点Y值*/
			double dis_b_y= pi_math_abs(point_b->y - point_a->y);
			dest->y = (float)(point_a->y + (dis_p_a * dis_b_y) / dis_b_a);

		}
		else
		{
			double dis_p_a = pi_math_sqrt((float)((x - point_b->x) * (x - point_b->x) + (z - point_b->z) * (z - point_b->z)));
			/*点B与A点的距离*/
			double dis_b_a = pi_math_sqrt((point_a->x - point_b->x) * (point_a->x - point_b->x) + (point_a->z - point_b->z) * (point_a->z - point_b->z));
			/*B点Y值*/
			double dis_b_y = pi_math_abs(point_a->y - point_b->y);
			dest->y = (float)(point_b->y + (dis_p_a * dis_b_y) / dis_b_a);
		}
		dest->x = (float)x;
		dest->z = (float)z;
		return TRUE;
	}
	return FALSE;
}

static PiBool _two_segment_is_intersect_no_y(PiVector3 *dest, const PiVector3 *point_a, const PiVector3 *point_b, const PiVector3 *point_c, const PiVector3 *point_d)
{
	/*解线性方程组, 求线段交点*/
	double denominator = (point_b->z - point_a->z) * (point_d->x - point_c->x) - (point_a->x - point_b->x) * (point_c->z - point_d->z);
	double x, z;
	if (denominator > -FLOAT_TOLERANCE && denominator < FLOAT_TOLERANCE)
	{
		return FALSE;
	}
	/*线段所在直线的交点坐标 (x , y)*/
	x = ((point_b->x - point_a->x) * (point_d->x - point_c->x) * (point_c->z - point_a->z) +
		(point_b->z - point_a->z) * (point_d->x - point_c->x) * point_a->x -
		(point_d->z - point_c->z) * (point_b->x - point_a->x) * point_c->x) / denominator;
	z = -((point_b->z - point_a->z) * (point_d->z - point_c->z) * (point_c->x - point_a->x) +
		(point_b->x - point_a->x) * (point_d->z - point_c->z) * point_a->z -
		(point_d->x - point_c->x) * (point_b->z - point_a->z) * point_c->z) / denominator;
	/*判断交点是否在两条线段上*/
	if ((x - point_a->x) * (x - point_b->x) <= FLOAT_TOLERANCE &&
		(z - point_a->z) * (z - point_b->z) <= FLOAT_TOLERANCE &&
		(x - point_c->x) * (x - point_d->x) <= FLOAT_TOLERANCE &&
		(z - point_c->z) * (z - point_d->z) <= FLOAT_TOLERANCE)
	{
		dest->x = (float)x;
		dest->z = (float)z;
		dest->y = 0;
		return TRUE;
	}
	return FALSE;
}

static void _point_in_tri(const PiVector3 *point, const PiVector3 *point_a, const PiVector3 *point_b, const PiVector3 *point_c, PiVector3 pt_array[], uint *count, PiBool isTri)
{
	if (_point_is_in_triangle(&pt_array[*count], point_a, point_b, point_c, point, isTri))
	{
		(*count)++;
	}
}

static void _segment_intersect_segment(const PiVector3 *point_a, const PiVector3 *point_b, const PiVector3 *point_c, const PiVector3 *point_d, PiVector3 pt_array[], uint *count)
{
	if (_two_segment_is_intersect(&pt_array[*count], point_a, point_b, point_c, point_d))
	{
		(*count)++;
	}
}

static void _proc_point(const PiVector3 *tri_point, const PiVector3 *rect_point, PiVector3 pt_array[], uint *count)
{
	uint i;
	for (i = 0; i < 3; i++)
	{
		_point_in_tri(&tri_point[i], &rect_point[0], &rect_point[1], &rect_point[2], pt_array, count, TRUE);
	}
	for (i = 0; i < 3; i++)
	{
		_point_in_tri(&rect_point[i], &tri_point[0], &tri_point[1], &tri_point[2], pt_array, count, FALSE);
	}
}

static void _proc_segment(const PiVector3 tri_segment[3][2], const PiVector3 rect_segment[4][2], PiVector3 pt_array[], uint *count)
{
	uint i, j;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 4; j++)
		{
			_segment_intersect_segment(&tri_segment[i][0], &tri_segment[i][1], &rect_segment[j][0], &rect_segment[j][1], pt_array, count);
		}
	}
}

static void _remove_duplates(PiVector3 pt_array[], uint * point_count)
{
	uint count = *point_count;
	uint i, j, same_count;
	for (i = 0; i < count; i++)
	{
		same_count = 0;
		for (j = i + 1; j < count; j++)
		{
			if (pi_vec3_is_equal(&pt_array[j], &pt_array[i]))
			{
				same_count++;
			}
			else
			{
				pt_array[j - same_count] = pt_array[j];
			}
		}
		count -= same_count;
	}
	*point_count = count;
}

static void _sort_vertex_to_convex(PiVector3 pt_array[], uint count)
{
	uint i, j, max_id = 0, temp_Id = 0, swap_id;
	float max_z = pt_array[0].z, swap_angle;
	uint *id_list = pi_new0(uint, count - 1);
	float *angle_list = pi_new0(float, count - 1);
	PiVector3 temp;
	PiVector3 right;
	PiVector3 *point_array = pi_new(PiVector3, count);
	right.y = right.z = 0;
	right.x = 1.0f;
	/*赋值*/
	for (i = 0; i < count; ++i)
	{
		point_array[i] = pt_array[i];
	}
	/*首先找到Z轴坐标最大的那个点*/
	for (i = 1; i < count; ++i)
	{
		if (max_z < point_array[i].z)
		{
			max_z = point_array[i].z;
			max_id = i;
		}
	}
	/*然后以Z轴值最大的这个点最为起点，分别于其他的点连线，求出这条线与X轴的夹角*/
	temp.y = 0;
	for (i = 0; i < count; ++i)
	{
		if (i != max_id)
		{
			temp.x = point_array[i].x - point_array[max_id].x;
			temp.z = point_array[i].z - point_array[max_id].z;
			angle_list[temp_Id] = pi_vec3_angle(&temp, &right);
			id_list[temp_Id] = i;
			temp_Id++;
		}
	}
	/*将夹角从小到大排序*/
	for (i = 0; i < temp_Id; i++)
	{
		for (j = 1; j < temp_Id - i; j++)
		{
			if (angle_list[j - 1] > angle_list[j])
			{
				swap_angle = angle_list[j - 1];
				angle_list[j - 1] = angle_list[j];
				angle_list[j] = swap_angle;
				/*对应ID交换*/
				swap_id = id_list[j - 1];
				id_list[j - 1] = id_list[j];
				id_list[j] = swap_id;
			}
		}
	}
	/*最后将整理好的值赋给pt_array*/
	pt_array[0] = point_array[max_id];
	for (i = 0; i < count - 1; ++i)
	{
		pt_array[i + 1] = point_array[id_list[i]];
	}
}

static void _create_vertex_buffer(PiVector3 dest[], PiVector3 src[], uint *count)
{
	uint i, id = 0;
	uint vertex_count = 0;
	/*首先删除重复的点*/
	_remove_duplates(src, count);
	if (*count < 3)
	{
		*count = 0;
		return;
	}
	/*然后生成凸包图形*/
	_sort_vertex_to_convex(src, *count);
	/*创建顶点索引，一开始将顶点和索引一一对应， 以后再整理*/
	vertex_count = (*count - 2) * 3;
	for (i = 2; i < *count; i++)
	{
		dest[id++] = src[0];
		dest[id++] = src[i - 1];
		dest[id++] = src[i];
	}
	PI_ASSERT(vertex_count == id, "地形贴花网格的顶点数目错误");
	*count = vertex_count;
}

static void _buffer_push_back(PiVector3 *dest, uint *dest_count, const PiVector3 *src, const uint src_count)
{
	uint i, id = *dest_count;
	for (i = 0; i < src_count; i++)
	{
		dest[id++] = src[i];
	}
	(*dest_count) += src_count;
}

static float _get_intersect_y(PiVector3 *point1, PiVector3 *point2, PiVector3 *point)
{
	float y;
	if (point1->y - point2->y == 0)
	{
		y = point1->y;
	}
	else if (point1->y < point2->y)
	{
		/*在XZ平面，点p与A点的距离*/
		float dis_p_a = pi_math_sqrt((point->x - point1->x) * (point->x - point1->x) + (point->z - point1->z) * (point->z - point1->z));
		/*点B与A点的距离*/
		float dis_b_a = pi_math_sqrt((point2->x - point1->x) * (point2->x - point1->x) + (point2->z - point1->z) * (point2->z - point1->z));
		/*B点Y值*/
		float dis_b_y = pi_math_abs(point2->y - point1->y);
		y = point1->y + (dis_p_a * dis_b_y) / dis_b_a;
	}
	else
	{
		float dis_p_a = pi_math_sqrt((point->x - point2->x) * (point->x - point2->x) + (point->z - point2->z) * (point->z - point2->z));
		/*点B与A点的距离*/
		float dis_b_a= pi_math_sqrt((point1->x - point2->x) * (point1->x - point2->x) + (point1->z - point2->z) * (point1->z - point2->z));
		/*B点Y值*/
		float dis_b_y= pi_math_abs(point1->y - point2->y);
		y = point2->y + (dis_p_a * dis_b_y) / dis_b_a;
	}
	return y;
}

static float _decal_tex_dis(TerrainDecal *decal, PiVector3 *point, PiVector3 *point_a, PiVector3 *point_b)
{
	uint i, id = 0;
	PiVector3 up, dir1, dir2, point1, point2, point_new;
	DecalRect rect = decal->rect;
	up.x = up.z = dir1.y = dir2.y = point1.y = point2.y = point_new.y = 0;
	up.y = 1.0f;
	dir1.x = point_a->x - point_b->x;
	dir1.z = point_a->z - point_b->z;
	pi_vec3_normalise(&dir1, &dir1);
	point_a->x += dir1.x * 2.0f;
	point_a->z += dir1.z * 2.0f;
	point_b->x -= dir1.x * 2.0f;
	point_b->z -= dir1.z * 2.0f;
	pi_vec3_cross(&dir2, &up, &dir1);
	pi_vec3_normalise(&dir2, &dir2);
	point1.x = point->x - dir2.x * (rect.aabb_max_x - rect.aabb_min_x) * 2.0f;
	point1.z = point->z - dir2.z * (rect.aabb_max_z - rect.aabb_min_z) * 2.0f;
	point2.x = point->x + dir2.x * (rect.aabb_max_x - rect.aabb_min_x) * 2.0f;
	point2.z = point->z + dir2.z * (rect.aabb_max_z - rect.aabb_min_z) * 2.0f;
	PI_ASSERT(_two_segment_is_intersect_no_y(&point_new, &point1, &point2, point_a, point_b), "decal tex坐标必然与边相交");
	point_a->x -= dir1.x * 2.0f;
	point_a->z -= dir1.z * 2.0f;
	point_b->x += dir1.x * 2.0f;
	point_b->z += dir1.z * 2.0f;
	for (i = 0; i < rect.tri_segment_count; i++)
	{
		if (_point_on_segment(&rect.tri_segment[i][0], &rect.tri_segment[i][1], &point_new))
		{
			id++;
			if (id == 1)
			{
				point_new.y = _get_intersect_y(&rect.tri_segment[i][0], &rect.tri_segment[i][1], &point_new);
			}
			else
			{
				break;
			}
		}
	}
	PI_ASSERT(id, "decal tex坐标必然有y值");
	return pi_vec3_distance(point, &point_new);
}

static void _get_plane_vertex(PiVector3 dest_point[], float dest_dis[], uint *dest_count, PiVector3 src[], uint count, const PiVector3 *plane_parallel_dir, const PiVector3 *plane_point, PiBool is_front)
{
	uint i;
	PiVector3 up, normal;
	float dot, product;
	up.x = up.z = 0;
	up.y = 1.0f;
	*dest_count = 0;
	if (is_front)
	{
		pi_vec3_cross(&normal, &up, plane_parallel_dir);
	}
	else
	{
		pi_vec3_cross(&normal, plane_parallel_dir, &up);
	}
	dot = pi_vec3_dot(&normal, plane_point);
	/*只保留大于或者等于0的数, 也就是在平面正面的点*/
	for (i = 0; i < count; i++)
	{
		product = pi_vec3_dot(&normal, &src[i]) - dot;
		if (product >= 0)
		{
			dest_point[*dest_count] = src[i];
			dest_dis[*dest_count] = product / pi_vec3_len(&normal);
			(*dest_count)++;
		}
	}
}

static void _sort_decal_point_dis(PiVector3 point[], float distance[], uint count)
{
	uint i, j;
	float swap_angle;
	PiVector3 swap_point;
	for (i = 0; i < count; i++)
	{
		for (j = 1; j < count - i; j++)
		{
			if (distance[j - 1] > distance[j])
			{
				swap_angle = distance[j - 1];
				distance[j - 1] = distance[j];
				distance[j] = swap_angle;
				/*对应顶点交换*/
				swap_point = point[j - 1];
				point[j - 1] = point[j];
				point[j] = swap_point;
			}
		}
	}
}

static void _decal_tex_dis_plane(TerrainDecal *decal, const PiVector3 *point, const PiVector3 *point_a, const PiVector3 *point_b, float *af, float *bf, uint type)
{
	uint i, count = 0, front_count = 0, back_count = 0;
	PiVector3 dir, up, point_new[TERRAIN_DECAL_COUNT], point_front[TERRAIN_DECAL_COUNT], point_back[TERRAIN_DECAL_COUNT];
	float dis_point_front[TERRAIN_DECAL_COUNT], dis_point_back[TERRAIN_DECAL_COUNT];
	dir.y = 0;
	dir.x = point_a->x - point_b->x;
	dir.z = point_a->z - point_b->z;
	up.x = up.z = 0;
	up.y = 1.0f;
	for (i = 0; i < decal->rect.tri_segment_count; i++)
	{
		if (_segment_intersect_plane(&point_new[count], &dir, point, &decal->rect.tri_segment[i][0], &decal->rect.tri_segment[i][1]))
		{
			count++;
		}
	}
	PI_ASSERT(count, "decal tex坐标必然有y值");
	/*去掉重复的点*/
	_remove_duplates(point_new, &count);
	/*构建平面 分出正面和负面的点*/
	_get_plane_vertex(point_front, dis_point_front, &front_count, point_new, count, &dir, point, TRUE);
	_get_plane_vertex(point_back, dis_point_back, &back_count, point_new, count, &dir, point, FALSE);
	_sort_decal_point_dis(point_front, dis_point_front, front_count);
	_sort_decal_point_dis(point_back, dis_point_back, back_count);
	if (type == 0)
	{
		*af = *bf = 0;
		for (i = 1; i < front_count; i++)
		{
			pi_vec3_sub(&dir, &point_front[i], &point_front[i - 1]);
			*af += pi_vec3_len(&dir);
		}
		for (i = 1; i < back_count; i++)
		{
			pi_vec3_sub(&dir, &point_back[i], &point_back[i - 1]);
			*bf += pi_vec3_len(&dir);
		}
	}
	else if (type == 1)
	{
		*af = *bf = 0;
		for (i = 0; i < front_count; i++)
		{
			*af += dis_point_front[i];
		}
		for (i = 0; i < back_count; i++)
		{
			*bf += dis_point_back[i];
		}
		*af /= front_count;
		*bf /= back_count;
	}
}

static void _create_tex_coord(TerrainDecal *decal, float *tex_buffer, PiVector3 *buffer, uint count, uint type)
{
	uint i;
	float af, bf;
	PiVector3 up, dir, point1, point2;
	for (i = 0; i < count; i++)
	{
		up.x = up.z = dir.y = point1.y = point2.y = 0;
		up.y = 1.0f;
		if (type == 0)
		{
			af = _point_to_segment_dis(&buffer[i], &decal->pos_a, &decal->pos_b);
			bf = _point_to_segment_dis(&buffer[i], &decal->pos_d, &decal->pos_c);
			tex_buffer[i * 2 + 0] = af / (af + bf);
			af = _point_to_segment_dis(&buffer[i], &decal->pos_a, &decal->pos_d);
			bf = _point_to_segment_dis(&buffer[i], &decal->pos_b, &decal->pos_c);
			tex_buffer[i * 2 + 1] = af / (af + bf);
		}
		else if (type == 1)
		{
			af = _decal_tex_dis(decal, &buffer[i], &decal->pos_a, &decal->pos_b);
			bf = _decal_tex_dis(decal, &buffer[i], &decal->pos_d, &decal->pos_c);
			tex_buffer[i * 2 + 0] = af / (af + bf);
			af = _decal_tex_dis(decal, &buffer[i], &decal->pos_d, &decal->pos_a);
			bf = _decal_tex_dis(decal, &buffer[i], &decal->pos_b, &decal->pos_c);
			tex_buffer[i * 2 + 1] = af / (af + bf);
		}
		else if (type == 2)
		{
			_decal_tex_dis_plane(decal, &buffer[i], &decal->pos_a, &decal->pos_b, &af, &bf, 0);
			tex_buffer[i * 2 + 0] = af / (af + bf);
			_decal_tex_dis_plane(decal, &buffer[i], &decal->pos_d, &decal->pos_a, &af, &bf, 0);
			tex_buffer[i * 2 + 1] = af / (af + bf);
		}
		else if (type == 3)
		{
			_decal_tex_dis_plane(decal, &buffer[i], &decal->pos_a, &decal->pos_b, &af, &bf, 1);
			tex_buffer[i * 2 + 0] = af / (af + bf);
			_decal_tex_dis_plane(decal, &buffer[i], &decal->pos_d, &decal->pos_a, &af, &bf, 1);
			tex_buffer[i * 2 + 1] = af / (af + bf);
		}
	}
}

static void _remove_duplates_vertex(PiVector3 *vertex_buffer, uint *vertex_count, uint *index_buffer)
{
	uint i, j;
	uint count = *vertex_count;
	PiVector3 *compare_vertex = pi_new0(PiVector3, count);
	pi_memcpy(compare_vertex, vertex_buffer, count * sizeof(PiVector3));
	_remove_duplates(vertex_buffer, vertex_count);
	for (i = 0; i < count; i++)
	{
		for (j = 0; j < *vertex_count; j++)
		{
			if (pi_vec3_is_equal(&vertex_buffer[j], &compare_vertex[i]))
			{
				index_buffer[i] = j;
			}
		}
	}
	pi_free(compare_vertex);
	compare_vertex = NULL;
}

static PiBool _rect_point_is_in_terrain_tri(PiVector3 *tri, const PiVector3 *point)
{
	PiVector3 v0, v1, v2;
	double dot00, dot01, dot02, dot11, dot12;
	double inver_deno, u, v;

	PiVector3 a = tri[0];
	PiVector3 b = tri[1];
	PiVector3 c = tri[2];
	PiVector3 p = *point;

	pi_vec3_sub(&v0, &c, &a);
	pi_vec3_sub(&v1, &b, &a);
	pi_vec3_sub(&v2, &p, &a);

	dot00 = pi_vec3_dot(&v0, &v0);
	dot01 = pi_vec3_dot(&v0, &v1);
	dot02 = pi_vec3_dot(&v0, &v2);
	dot11 = pi_vec3_dot(&v1, &v1);
	dot12 = pi_vec3_dot(&v1, &v2);
	inver_deno = 1.0f / (dot00 * dot11 - dot01 * dot01);
	u = (dot11 * dot02 - dot01 * dot12) * inver_deno;
	if (u < -FLOAT_TOLERANCE || u > 1 + FLOAT_TOLERANCE) // if u out of range, return directly
	{
		return FALSE;
	}
	v = (dot00 * dot12 - dot01 * dot02) * inver_deno;
	if (v < -FLOAT_TOLERANCE || v > 1 + FLOAT_TOLERANCE) // if v out of range, return directly
	{
		return FALSE;
	}
	return u + v <= 1 + FLOAT_TOLERANCE;
}

static PiBool _rect_point_on_segment(const PiVector3 *point1, const PiVector3 *point2, const PiVector3 *point)
{
	/*叉积是否为0，判断是否在同一直线上*/
	double max_x, min_x, max_z, min_z, value;
	value = (point1->x - point->x) * (point2->z - point->z) - (point2->x - point->x) * (point1->z - point->z);
	if (value != 0)
	{
		return FALSE;
	}
	/*判断是否在线段上*/
	max_x = point1->x > point2->x ? point1->x : point2->x;
	min_x = point1->x < point2->x ? point1->x : point2->x;
	max_z = point1->z > point2->z ? point1->z : point2->z;
	min_z = point1->z < point2->z ? point1->z : point2->z;
	if (point->x > min_x && point->x < max_x && point->z > min_z && point->z < max_z)
	{
		return TRUE;
	}
	return FALSE;
}

static void _generate_normal(TerrainDecal *decal, PiVector3 *dest_normal_buffer, PiVector3 *vertex_buffer, uint vertex_count)
{
	uint i, j, k, count;
	PiVector3 v, v0, v1, v2;
	uint point_is_same[TERRAIN_DECAL_COUNT];
	uint point_Id_list[TERRAIN_DECAL_COUNT];
	DecalTri *tri_list = decal->tri_list;
	DecalRect rect = decal->rect;
	uint tri_count = decal->tri_count;
	float len, len0, len1, len2;
	PiBool is_exist;
	for (i = 0; i < vertex_count; i++)
	{
		pi_vec3_set(&dest_normal_buffer[i], 0, 0, 0);
	}
	for (i = 0; i < vertex_count; i++)
	{
		is_exist = FALSE;
		for (j = 0; j < tri_count; j++)
		{
			if ((tri_list[j].vertex[0].x < rect.aabb_min_x && tri_list[j].vertex[1].x < rect.aabb_min_x && tri_list[j].vertex[2].x < rect.aabb_min_x) ||
				(tri_list[j].vertex[0].z < rect.aabb_min_z && tri_list[j].vertex[1].z < rect.aabb_min_z && tri_list[j].vertex[2].z < rect.aabb_min_z) ||
				(tri_list[j].vertex[0].x > rect.aabb_max_x && tri_list[j].vertex[1].x > rect.aabb_max_x && tri_list[j].vertex[2].x > rect.aabb_max_x) ||
				(tri_list[j].vertex[0].z > rect.aabb_max_z && tri_list[j].vertex[1].z > rect.aabb_max_z && tri_list[j].vertex[2].z > rect.aabb_max_z))
			{
				continue;
			}
			if (_rect_point_is_in_terrain_tri(tri_list[j].vertex, &vertex_buffer[i]))
			{
				pi_vec3_sub(&v, &vertex_buffer[i], &tri_list[j].vertex[0]);
				len0 = pi_vec3_len(&v);
				pi_vec3_sub(&v, &vertex_buffer[i], &tri_list[j].vertex[1]);
				len1 = pi_vec3_len(&v);
				pi_vec3_sub(&v, &vertex_buffer[i], &tri_list[j].vertex[2]);
				len2 = pi_vec3_len(&v);
				len = len0 + len1 + len2;
				pi_vec3_scale(&v0, &tri_list[j].normal[0], (len - len0) / len);
				pi_vec3_scale(&v1, &tri_list[j].normal[1], (len - len1) / len);
				pi_vec3_scale(&v2, &tri_list[j].normal[2], (len - len2) / len);
				pi_vec3_add(&dest_normal_buffer[i], &dest_normal_buffer[i], &v0);
				pi_vec3_add(&dest_normal_buffer[i], &dest_normal_buffer[i], &v1);
				pi_vec3_add(&dest_normal_buffer[i], &dest_normal_buffer[i], &v2);
				is_exist = TRUE;
			}
			else
			{
				for (k = 0; k < 3; k++)
				{
					if (_rect_point_on_segment(&tri_list[j].segment[k][0], &tri_list[j].segment[k][1], &vertex_buffer[i]))
					{
						pi_vec3_sub(&v, &vertex_buffer[i], &tri_list[j].segment[k][0]);
						len0 = pi_vec3_len(&v);
						pi_vec3_sub(&v, &vertex_buffer[i], &tri_list[j].segment[k][1]);
						len1 = pi_vec3_len(&v);
						len = len0 + len1;
						pi_vec3_scale(&v0, &tri_list[j].segment[k][0], (len - len0) / len);
						pi_vec3_scale(&v1, &tri_list[j].segment[k][1], (len - len1) / len);
						pi_vec3_add(&dest_normal_buffer[i], &dest_normal_buffer[i], &v0);
						pi_vec3_add(&dest_normal_buffer[i], &dest_normal_buffer[i], &v1);
						is_exist = TRUE;
					}
				}
			}
		}
	}
	for (i = 0; i < vertex_count; i++)
	{
		pi_vec3_normalise(&dest_normal_buffer[i], &dest_normal_buffer[i]);
	}
	for (i = 0; i < vertex_count; i++)
	{
		point_is_same[i] = 0;
	}
	for (i = 0; i < vertex_count; i++)
	{
		if (point_is_same[i])
		{
			continue;
		}
		is_exist = FALSE;
		count = 0;
		v1.x = v1.y = v1.z = 0;
		for (j = i + 1; j < vertex_count; j++)
		{
			if (point_is_same[j])
			{
				continue;
			}
			pi_vec3_sub(&v, &vertex_buffer[j], &vertex_buffer[i]);
			if (pi_vec3_len(&v) <= 0.3f)
			{
				if (is_exist)
				{
					pi_vec3_add(&v1, &v1, &dest_normal_buffer[j]);
					point_is_same[j] = 1;
					point_Id_list[count++] = j;
				}
				else
				{
					point_Id_list[count++] = i;
					point_Id_list[count++] = j;
					pi_vec3_add(&v1, &dest_normal_buffer[i], &dest_normal_buffer[j]);
					point_is_same[i] = point_is_same[j] = 1;
					is_exist = TRUE;
				}
			}
		}
		for (j = 0; j < count; j++)
		{
			dest_normal_buffer[point_Id_list[j]] = v1;
		}
	}
	for (i = 0; i < vertex_count; i++)
	{
		pi_vec3_normalise(&dest_normal_buffer[i], &dest_normal_buffer[i]);
	}
}

static void _generate_tangent(TerrainDecal *decal, PiVector3 *dest_tangent_buffer, PiVector3 * normal_buffer, uint vertex_count)
{
	uint i;
	PiVector3 dir;
	pi_vec3_sub(&dir, &decal->pos_a, &decal->pos_b);
	for(i = 0; i < vertex_count; i++)
	{
		/*vec3 tangent = normalize(vec3(g_Normal.y, -g_Normal.z, g_Normal.x));
        vec3 binormal = normalize(cross(g_Normal, tangent));
        tangent = normalize(cross(binormal, g_Normal));*/
		/*dest_tangent_buffer[i].x = normal_buffer[i].y;
		dest_tangent_buffer[i].y = -normal_buffer[i].z;
		dest_tangent_buffer[i].z = normal_buffer[i].x;*/
		/*pi_vec3_normalise(&dest_tangent_buffer[i], &dest_tangent_buffer[i]);

		pi_vec3_cross(&binormal, &normal_buffer[i], &dest_tangent_buffer[i]);
		pi_vec3_normalise(&binormal, &binormal);

		pi_vec3_cross(&dest_tangent_buffer[i], &binormal, &normal_buffer[i]);
		pi_vec3_normalise(&dest_tangent_buffer[i], &dest_tangent_buffer[i]);*/

		pi_vec3_cross(&dest_tangent_buffer[i], &dir, &normal_buffer[i]);
		pi_vec3_normalise(&dest_tangent_buffer[i], &dest_tangent_buffer[i]);

	}
}

static void _update_tri(TerrainDecal *decal, PiVector3 *vertex_buffer, PiVector3 *normal_buffer, uint *index_buffer, uint index_count)
{
	uint i;
	PI_ASSERT(index_count % 3 == 0, "decal 索引数目必须是3的整数倍");
	decal->tri_count = index_count / 3;
	decal->tri_list = pi_new0(DecalTri, decal->tri_count);
	for (i = 0; i < decal->tri_count; ++i)
	{
		decal->tri_list[i].vertex[0] = vertex_buffer[index_buffer[i * 3]];
		decal->tri_list[i].vertex[1] = vertex_buffer[index_buffer[i * 3 + 1]];
		decal->tri_list[i].vertex[2] = vertex_buffer[index_buffer[i * 3 + 2]];
		decal->tri_list[i].normal[0] = normal_buffer[index_buffer[i * 3]];
		decal->tri_list[i].normal[1] = normal_buffer[index_buffer[i * 3 + 1]];
		decal->tri_list[i].normal[2] = normal_buffer[index_buffer[i * 3 + 2]];
		decal->tri_list[i].segment[0][0] = decal->tri_list[i].vertex[1];
		decal->tri_list[i].segment[0][1] = decal->tri_list[i].vertex[0];
		decal->tri_list[i].segment[1][0] = decal->tri_list[i].vertex[2];
		decal->tri_list[i].segment[1][1] = decal->tri_list[i].vertex[1];
		decal->tri_list[i].segment[2][0] = decal->tri_list[i].vertex[0];
		decal->tri_list[i].segment[2][1] = decal->tri_list[i].vertex[2];
	}
}

static void _update_rect(TerrainDecal *decal, const PiVector3 *pos_a, const PiVector3 *pos_b, const PiVector3 *pos_c, const PiVector3 *pos_d)
{
	decal->pos_a = *pos_a;
	decal->pos_b = *pos_b;
	decal->pos_c = *pos_c;
	decal->pos_d = *pos_d;
	decal->rect.aabb_min_x = _min_value(pos_a->x, pos_b->x, pos_c->x, pos_d->x);
	decal->rect.aabb_min_z = _min_value(pos_a->z, pos_b->z, pos_c->z, pos_d->z);
	decal->rect.aabb_max_x = _max_value(pos_a->x, pos_b->x, pos_c->x, pos_d->x);
	decal->rect.aabb_max_z = _max_value(pos_a->z, pos_b->z, pos_c->z, pos_d->z);
	decal->rect.vertex0[0] = *pos_a;
	decal->rect.vertex0[1] = *pos_b;
	decal->rect.vertex0[2] = *pos_c;
	decal->rect.vertex1[0] = *pos_a;
	decal->rect.vertex1[1] = *pos_c;
	decal->rect.vertex1[2] = *pos_d;
	decal->rect.segment[0][0] = *pos_b;
	decal->rect.segment[0][1] = *pos_a;
	decal->rect.segment[1][0] = *pos_c;
	decal->rect.segment[1][1] = *pos_b;
	decal->rect.segment[2][0] = *pos_d;
	decal->rect.segment[2][1] = *pos_c;
	decal->rect.segment[3][0] = *pos_a;
	decal->rect.segment[3][1] = *pos_d;
}

void PI_API pi_terrain_decal_update_rect(TerrainDecal *decal)
{
	decal->rect.aabb_min_x = _min_value(decal->pos_a.x, decal->pos_b.x, decal->pos_c.x, decal->pos_d.x);
	decal->rect.aabb_min_z = _min_value(decal->pos_a.z, decal->pos_b.z, decal->pos_c.z, decal->pos_d.z);
	decal->rect.aabb_max_x = _max_value(decal->pos_a.x, decal->pos_b.x, decal->pos_c.x, decal->pos_d.x);
	decal->rect.aabb_max_z = _max_value(decal->pos_a.z, decal->pos_b.z, decal->pos_c.z, decal->pos_d.z);
	decal->rect.vertex0[0] = decal->pos_a;
	decal->rect.vertex0[1] = decal->pos_b;
	decal->rect.vertex0[2] = decal->pos_c;
	decal->rect.vertex1[0] = decal->pos_a;
	decal->rect.vertex1[1] = decal->pos_c;
	decal->rect.vertex1[2] = decal->pos_d;
	decal->rect.segment[0][0] = decal->pos_b;
	decal->rect.segment[0][1] = decal->pos_a;
	decal->rect.segment[1][0] = decal->pos_c;
	decal->rect.segment[1][1] = decal->pos_b;
	decal->rect.segment[2][0] = decal->pos_d;
	decal->rect.segment[2][1] = decal->pos_c;
	decal->rect.segment[3][0] = decal->pos_a;
	decal->rect.segment[3][1] = decal->pos_d;
}

void PI_API pi_terrain_decal_create_mesh(TerrainDecal *decal)
{
	uint i;
	uint vertex_count = 0, index_count = 0;
	PiVector3 vertex_buffer[TERRAIN_DECAL_COUNT];
	PiVector3 normal_buffer[TERRAIN_DECAL_COUNT];
	PiVector3 tangent_buffer[TERRAIN_DECAL_COUNT];
	PiVector3 show_vertex_buffer[TERRAIN_DECAL_COUNT];
	uint index_buffer[TERRAIN_DECAL_COUNT];
	float texCoord_buffer[TERRAIN_DECAL_COUNT];
	uint count = 0;/*有效顶点的个数*/
	/*预先设置足够长度的数组*/
	PiVector3 ptDest[TERRAIN_DECAL_COUNT];
	PiVector3 ptSrc[TERRAIN_DECAL_COUNT];
	DecalTri *tri = decal->tri_list;
	DecalRect rect = decal->rect;
	PiColor color;
	color.rgba[0] = 0;
	color.rgba[1] = 0;
	color.rgba[2] = 1.0f;
	color.rgba[3] = 1.0f;
	/*建立矩形的AABB框*/
	for (i = 0; i < decal->tri_count; i++)
	{
		/*如果在AABB外 不处理*/
		if ((tri[i].vertex[0].x < rect.aabb_min_x && tri[i].vertex[1].x < rect.aabb_min_x && tri[i].vertex[2].x < rect.aabb_min_x) ||
			(tri[i].vertex[0].z < rect.aabb_min_z && tri[i].vertex[1].z < rect.aabb_min_z && tri[i].vertex[2].z < rect.aabb_min_z) ||
			(tri[i].vertex[0].x > rect.aabb_max_x && tri[i].vertex[1].x > rect.aabb_max_x && tri[i].vertex[2].x > rect.aabb_max_x) ||
			(tri[i].vertex[0].z > rect.aabb_max_z && tri[i].vertex[1].z > rect.aabb_max_z && tri[i].vertex[2].z > rect.aabb_max_z))
		{
			continue;
		}
		{
			count = 0;
			_proc_point(tri[i].vertex, rect.vertex0, ptSrc, &count);
			_proc_point(tri[i].vertex, rect.vertex1, ptSrc, &count);
			_proc_segment(tri[i].segment, rect.segment, ptSrc, &count);
			/*生成顶点缓冲*/
			_create_vertex_buffer(ptDest, ptSrc, &count);
			_buffer_push_back(vertex_buffer, &vertex_count, ptDest, count);
		}
	}
	/*删除已有的数据*/
	if (decal->rect.tri_segment != NULL)
	{
		for (i = 0; i < decal->rect.tri_segment_count; i++)
		{
			pi_free(decal->rect.tri_segment[i]);
		}
		pi_free(decal->rect.tri_segment);
		decal->rect.tri_segment = NULL;
	}
	/*更新*/
	decal->rect.tri_segment_count = vertex_count;
	decal->rect.tri_segment = pi_new0(PiVector3 *, vertex_count);
	for (i = 0; i < vertex_count / 3; i++)
	{
		decal->rect.tri_segment[i * 3 + 0] = pi_new0(PiVector3, 2);
		decal->rect.tri_segment[i * 3 + 0][0] = vertex_buffer[i * 3 + 1];
		decal->rect.tri_segment[i * 3 + 0][1] = vertex_buffer[i * 3 + 0];

		decal->rect.tri_segment[i * 3 + 1] = pi_new0(PiVector3, 2);
		decal->rect.tri_segment[i * 3 + 1][0] = vertex_buffer[i * 3 + 2];
		decal->rect.tri_segment[i * 3 + 1][1] = vertex_buffer[i * 3 + 1];

		decal->rect.tri_segment[i * 3 + 2] = pi_new0(PiVector3, 2);
		decal->rect.tri_segment[i * 3 + 2][0] = vertex_buffer[i * 3 + 0];
		decal->rect.tri_segment[i * 3 + 2][1] = vertex_buffer[i * 3 + 2];
	}
	index_count = vertex_count;/*一开始顶点和所以是一一对应的, 所以索引数等于顶点数*/
	_remove_duplates_vertex(vertex_buffer, &vertex_count, index_buffer);
	_create_tex_coord(decal, texCoord_buffer, vertex_buffer, vertex_count, decal->texCoord_type);
	_generate_normal(decal, normal_buffer, vertex_buffer, vertex_count);
	_generate_tangent(decal, tangent_buffer, normal_buffer, vertex_count);
	if(decal->rect.vertex_count != 0)
	{
		pi_free(decal->rect.vertex_buffer);
		pi_free(decal->rect.normal_buffer);
		pi_free(decal->rect.tangent_buffer);
	}
	decal->rect.vertex_count = vertex_count;
	decal->rect.vertex_buffer = pi_new0(PiVector3, vertex_count);
	decal->rect.normal_buffer = pi_new0(PiVector3, vertex_count);
	decal->rect.tangent_buffer = pi_new0(PiVector3, vertex_count);
	pi_memcpy(decal->rect.vertex_buffer, vertex_buffer, vertex_count * sizeof(PiVector3));
	pi_memcpy(decal->rect.normal_buffer, normal_buffer, vertex_count * sizeof(PiVector3));
	pi_memcpy(decal->rect.tangent_buffer, tangent_buffer, vertex_count * sizeof(PiVector3));
	{
		PiMesh * mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, vertex_count, vertex_buffer, NULL, normal_buffer, texCoord_buffer, index_count, index_buffer);
		/*贴花网格*/
		/*向网格添加法线*/
		pi_renderdata_set_vertex(&mesh->data, vertex_count, TRUE, EVS_TANGENT, 3, EVT_FLOAT, EVU_STATIC_DRAW, tangent_buffer);
		decal->mesh = pi_rendermesh_new(mesh, TRUE);
	}
	/*法线网格*/
	for (i = 0; i < vertex_count; i++)
	{
		show_vertex_buffer[i * 2] = vertex_buffer[i];
		pi_vec3_add(&show_vertex_buffer[i * 2 + 1], &vertex_buffer[i], &normal_buffer[i]);
		index_buffer[i * 2] = i * 2;
		index_buffer[i * 2 + 1] = i * 2 + 1;
	}
	decal->normal_mesh = pi_rendermesh_new(pi_mesh_create(EGOT_LINE_LIST, EINDEX_32BIT, vertex_count * 2, show_vertex_buffer, NULL, NULL, NULL, vertex_count * 2, index_buffer), TRUE);
	/*切线网格*/
	for (i = 0; i < vertex_count; i++)
	{
		show_vertex_buffer[i * 2] = vertex_buffer[i];
		pi_vec3_add(&show_vertex_buffer[i * 2 + 1], &vertex_buffer[i], &tangent_buffer[i]);
	}
	decal->tangent_mesh = pi_rendermesh_new(pi_mesh_create(EGOT_LINE_LIST, EINDEX_32BIT, vertex_count * 2, show_vertex_buffer, NULL, NULL, NULL, vertex_count * 2, index_buffer), TRUE);
}

TerrainDecal *PI_API pi_terrain_decal_create(PiVector3 *pos_a, PiVector3 *pos_b, PiVector3 *pos_c, PiVector3 *pos_d, PiVector3 *vertex_buffer, PiVector3 *normal_buffer, uint *index_buffer, uint index_count)
{
	TerrainDecal *decal = pi_new0(TerrainDecal, 1);
	_update_rect(decal, pos_a, pos_b, pos_c, pos_d);
	_update_tri(decal, vertex_buffer, normal_buffer, index_buffer, index_count);
	pi_terrain_decal_create_mesh(decal);
	return decal;
}
