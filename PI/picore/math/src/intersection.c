#include <pi_intersection.h>

/**
 * 测试三角形t在边<p, dir>的两侧还是同侧
 * 根据投影计算t：p + t * dir
 * 所有的t > 0，返回1；外面
 * 所有的t < 0, 返回-1； 里面
 * 否则，返回0，表示这条边在三角形的两侧
 */
static sint in_which_tri_size(PiVector3 *t, PiVector3 *p, PiVector3 *dir)
{
	sint i, positive = 0, negative = 0, zero = 0;

	for (i = 0; i < 3; ++i)
	{
		float num;
		PiVector3 tmp;
		
		pi_vec3_sub(&tmp, &t[i], p);
		num = pi_vec3_dot(dir, &tmp);
		if (num > 0.0f)
			++positive;
		else if (num < 0.0f)
			++negative;
		else
			++zero;

		if (positive > 0 && negative > 0)
			return 0;
	}
	return (zero == 0 ? (positive > 0 ? 1 : -1) : 0);
}

/* 2D三角形的相交性检测，方法：分离轴测试 */
PiBool PI_API is_2d_tri_tri_overlapped(PiVector3 *t1, PiVector3 *t2)
{
	sint i1, i2;
	PiVector3 dir;

	/* 用t1的每条边作为分离轴进行测试 */
	for (i1 = 0, i2 = 2; i1 < 3; i2 = i1++)
	{
		/* 测试边 t1[i2] + t * perp(t1[i1] - t1[i2]), perp(x,y) = (y, -x). */
		pi_vec3_set(&dir, t1[i1].y - t1[i2].y, t1[i2].x - t1[i1].x, 0.0f);
		
		if (in_which_tri_size(t2, &t1[i2], &dir) > 0)
		{
			/* t2完全在t1某条边的外面 */
			return FALSE;
		}
	}

	// 用t2的每条边作为分离轴进行测试
	for (i1 = 0, i2 = 2; i1 < 3; i2 = i1++)
	{
		/* 测试边 t2[i2] + t * perp(t2[i1] - t2[i2]), perp(x,y) = (y, -x) */
		pi_vec3_set(&dir, t2[i1].y - t2[i2].y, t2[i2].x - t2[i1].x, 0.0f);
		
		if (in_which_tri_size(t1, &t2[i2], &dir) > 0)
		{
			/* t1完全在t2某条边的外面 */
			return FALSE;
		}
	}
	return TRUE;
}

PiBool PI_API line2aabb(PiAABBBox* aabb, PiLineSegment* line) 
{
	float x = line->start.x; // start x
	float y = line->start.y; // start y
	float z = line->start.z; // start z
	float x2 = line->end.x; // end x
	float y2 = line->end.y; // end y
	float z2 = line->end.z; // end z
	float dx, dy, dz;
	float t, hx, hy, hz;
	PiVector3 startPt, endPt;
	PiVector3* min = &aabb->minPt;
	PiVector3* max = &aabb->maxPt;

	pi_vec3_set(&startPt, x, y, z);
	pi_vec3_set(&endPt, x2, y2, z2);

	// 快速判断线段是否完全在 aabb 外
	if ((x < min->x && x2 < min->x) || 
		(x > max->x && x2 > max->x)	|| 
		(y < min->y && y2 < min->y)	||
		(y > max->y && y2 > max->y)	|| 
		(z < min->z && z2 < min->z)	|| 
		(z > max->z && z2 > max->z))
		return FALSE;

	// 快速判断线段是否至少一端在 aabb 内
	if (pi_aabb_is_contian_point(aabb, &startPt) || 
		pi_aabb_is_contian_point(aabb, &endPt))
		return TRUE;

	// 完整判断思路：
	// 依次测试线段和三对面每一对中最近的那个面是否相交，共判断三次
	dx = x2 - x;
	dy = y2 - y;
	dz = z2 - z;

	// yz 面
	if (x < min->x && dx > 0)
	{
		t = (min->x - x) / dx;
		hy = t * dy + y;
		hz = t * dz + z;
		if (hy >= min->y - FLOAT_TOLERANCE && hy <= max->y + FLOAT_TOLERANCE && hz >= min->z - FLOAT_TOLERANCE && hz <= max->z + FLOAT_TOLERANCE)
			return TRUE;
	}
	else if (x > max->x && dx < 0)
	{
		t = (max->x - x) / dx;
		hy = t * dy + y;
		hz = t * dz + z;
		if (hy >= min->y - FLOAT_TOLERANCE && hy <= max->y + FLOAT_TOLERANCE && hz >= min->z - FLOAT_TOLERANCE && hz <= max->z + FLOAT_TOLERANCE)
			return TRUE;
	}

	// xz 面
	if (y < min->y && dy > 0)
	{
		t = (min->y - y) / dy;
		hx = t * dx + x;
		hz = t * dz + z;
		if (hx >= min->x - FLOAT_TOLERANCE && hx <= max->x + FLOAT_TOLERANCE && hz >= min->z - FLOAT_TOLERANCE && hz <= max->z + FLOAT_TOLERANCE)
			return TRUE;
	}
	else if (y > max->y && dy < 0)
	{
		t = (max->y - y) / dy;
		hx = t * dx + x;
		hz = t * dz + z;
		if (hx >= min->x - FLOAT_TOLERANCE && hx <= max->x + FLOAT_TOLERANCE && hz >= min->z - FLOAT_TOLERANCE && hz <= max->z + FLOAT_TOLERANCE)
			return TRUE;
	}

	// xy 面
	if (z < min->z && dz > 0)
	{
		t = (min->z - z) / dz;
		hx = t * dx + x;
		hy = t * dy + y;
		if (hx >= min->x - FLOAT_TOLERANCE && hx <= max->x + FLOAT_TOLERANCE && hy >= min->y - FLOAT_TOLERANCE && hy <= max->y + FLOAT_TOLERANCE)
			return TRUE;
	}
	else if (z > max->z && dz < 0)
	{
		t = (max->z - z) / dz;
		hx = t * dx + x;
		hy = t * dy + y;
		if (hx >= min->x - FLOAT_TOLERANCE && hx <= max->x + FLOAT_TOLERANCE && hy >= min->y - FLOAT_TOLERANCE && hy <= max->y + FLOAT_TOLERANCE)
			return TRUE;
	}

	return FALSE;
}

PiBool PI_API sphere2aabb(PiAABBBox* aabb, PiSphere* sphere)
{
	PiVector3 nearPt;
	PiVector3 centerPt;
	float nearX;
	float nearY;
	float nearZ;
	float centerX = sphere->pos.x;
	float centerY = sphere->pos.y;
	float centerZ = sphere->pos.z;

	PiVector3* min = &aabb->minPt;
	PiVector3* max = &aabb->maxPt;
	//查找距离圆心最近的点
	if (centerX < min->x)
	{
		nearX = min->x;
	}
	else if (centerX > max->x)
	{
		nearX = max->x;
	}
	else
	{
		nearX = centerX;
	}

	if (centerY < min->y)
	{
		nearY = min->y;
	}
	else if (centerY > max->y)
	{
		nearY = max->y;
	}
	else
	{
		nearY = centerY;
	}

	if (centerZ < min->z)
	{
		nearZ = min->z;
	}
	else if (centerZ > max->z)
	{
		nearZ = max->z;
	}
	else
	{
		nearZ = centerZ;
	}

	pi_vec3_set(&nearPt, nearX, nearY, nearZ);
	pi_vec3_set(&centerPt, centerX, centerY, centerZ);

	if (pi_vec3_distance_square(&nearPt, &centerPt) <= sphere->radius*sphere->radius)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

PiBool PI_API aabb2aabb(PiAABBBox* aabb_p, PiAABBBox* aabb_i)
{
	const PiVector3* minCube = &aabb_i->minPt;
	const PiVector3* maxCube = &aabb_i->maxPt;
	const PiVector3* minBox = &aabb_p->minPt;
	const PiVector3* maxBox = &aabb_p->maxPt;

	if (minCube->x > maxBox->x || maxCube->x < minBox->x)
		return FALSE;
	if (minCube->y > maxBox->y || maxCube->y < minBox->y)
		return FALSE;
	if (minCube->z > maxBox->z || maxCube->z < minBox->z)
		return FALSE;

	return TRUE;
}

PiBool PI_API frustum2aabb(PiAABBBox* aabb, PiFrustum* frustum)
{
	return pi_frustum_is_aabb_visible(frustum, aabb);
}

PiBool PI_API line2obb(PiOBBBox* obb, PiLineSegment* line)
{
	float AWdU[3], ADdU[3], AWxDdU[3], RHS, line_length;
	PiVector3 WxD, diff, line_center, line_direction;

	pi_vec3_add(&line_center, &line->start, &line->end);
	line_center.x /= 2.0f;
	line_center.y /= 2.0f;
	line_center.z /= 2.0f;
	pi_vec3_sub(&line_direction, &line->end, &line->start);
	line_length = pi_vec3_len(&line_direction);
	line_direction.x /= line_length;
	line_direction.y /= line_length;
	line_direction.z /= line_length;
	pi_vec3_sub(&diff, &line_center, &obb->center);

	AWdU[0] = pi_math_abs(pi_vec3_dot(&line_direction, &obb->axis[0]));
	ADdU[0] = pi_math_abs(pi_vec3_dot(&diff, &obb->axis[0]));
	RHS = obb->extent[0] + line_length * AWdU[0];
	if (ADdU[0] > RHS)
	{
		return FALSE;
	}

	AWdU[1] = pi_math_abs(pi_vec3_dot(&line_direction, &obb->axis[1]));
	ADdU[1] = pi_math_abs(pi_vec3_dot(&diff, &obb->axis[1]));
	RHS = obb->extent[1] + line_length * AWdU[1];
	if (ADdU[1] > RHS)
	{
		return FALSE;
	}

	AWdU[2] = pi_math_abs(pi_vec3_dot(&line_direction, &obb->axis[2]));
	ADdU[2] = pi_math_abs(pi_vec3_dot(&diff, &obb->axis[2]));
	RHS = obb->extent[2] + line_length * AWdU[2];
	if (ADdU[2] > RHS)
	{
		return FALSE;
	}

	pi_vec3_cross(&WxD, &line_direction, &diff);

	AWxDdU[0] = pi_math_abs(pi_vec3_dot(&WxD, &obb->axis[0]));
	RHS = obb->extent[1] * AWdU[2] + obb->extent[2] * AWdU[1];
	if (AWxDdU[0] > RHS)
	{
		return FALSE;
	}

	AWxDdU[1] = pi_math_abs(pi_vec3_dot(&WxD, &obb->axis[1]));
	RHS = obb->extent[0] * AWdU[2] + obb->extent[2] * AWdU[0];
	if (AWxDdU[1] > RHS)
	{
		return FALSE;
	}

	AWxDdU[2] = pi_math_abs(pi_vec3_dot(&WxD, &obb->axis[2]));
	RHS = obb->extent[0] * AWdU[1] + obb->extent[1] * AWdU[0];
	if (AWxDdU[2] > RHS)
	{
		return FALSE;
	}

	return TRUE;
}

PiBool PI_API sphere2obb(PiOBBBox* obb, PiSphere* sphere)
{
	PiVector3 cdiff;
	float ax, ay, az, dx, dy, dz;
	pi_vec3_sub(&cdiff, &sphere->pos, &obb->center);

	ax = pi_math_abs(pi_vec3_dot(&cdiff, &obb->axis[0]));
	ay = pi_math_abs(pi_vec3_dot(&cdiff, &obb->axis[1]));
	az = pi_math_abs(pi_vec3_dot(&cdiff, &obb->axis[2]));
	dx = ax - obb->extent[0];
	dy = ay - obb->extent[1];
	dz = az - obb->extent[2];

	if (ax <= obb->extent[0])
	{
		if (ay <= obb->extent[1])
		{
			if (az <= obb->extent[2])
			{
				// Sphere center inside box.
				return TRUE;
			}
			else
			{
				// Potential sphere-face intersection with face z.
				return dz <= sphere->radius;
			}
		}
		else
		{
			if (az <= obb->extent[2])
			{
				// Potential sphere-face intersection with face y.
				return dy <= sphere->radius;
			}
			else
			{
				// Potential sphere-edge intersection with edge formed
				// by faces y and z.
				return dy * dy + dz * dz <= sphere->radius * sphere->radius;
			}
		}
	}
	else
	{
		if (ay <= obb->extent[1])
		{
			if (az <= obb->extent[2])
			{
				// Potential sphere-face intersection with face x.
				return dx <= sphere->radius;
			}
			else
			{
				// Potential sphere-edge intersection with edge formed
				// by faces x and z.
				return dx * dx + dz * dz <= sphere->radius * sphere->radius;
			}
		}
		else
		{
			if (az <= obb->extent[2])
			{
				// Potential sphere-edge intersection with edge formed
				// by faces x and y.
				return dx * dx + dy * dy <= sphere->radius * sphere->radius;
			}
			else
			{
				// Potential sphere-vertex intersection at corner formed
				// by faces x,y,z.
				return dx * dx + dy * dy + dz * dz <= sphere->radius * sphere->radius;
			}
		}
	}
}

PiBool PI_API obb2obb(PiOBBBox* obb_p, PiOBBBox* obb_i)
{
	const float cutoff = 1.0f - (float)FLOAT_TOLERANCE;
	PiBool existsParallelPair = FALSE;
	int i;

	// Convenience variables.
	const PiVector3* A = obb_p->axis;
	const PiVector3* B = obb_i->axis;
	const float* EA = obb_p->extent;
	const float* EB = obb_i->extent;

	float C[3][3];     // matrix C = A^T B, c_{ij} = Dot(A_i,B_j)
	float AbsC[3][3];  // |c_{ij}|
	float AD[3];       // Dot(A_i,D)
	float r0, r1, r;   // interval radii and distance between centers
	float r01;         // = R0 + R1

	// Compute difference of box centers, D = C1-C0.
	PiVector3 D;
	pi_vec3_sub(&D, &obb_i->center, &obb_p->center);

	// axis C0+t*A0
	for (i = 0; i < 3; ++i)
	{
		C[0][i] = pi_vec3_dot(&A[0], &B[i]);
		AbsC[0][i] = pi_math_abs(C[0][i]);
		if (AbsC[0][i] > cutoff)
		{
			existsParallelPair = TRUE;
		}
	}
	AD[0] = pi_vec3_dot(&A[0], &D);
	r = pi_math_abs(AD[0]);
	r1 = EB[0]*AbsC[0][0] + EB[1]*AbsC[0][1] + EB[2]*AbsC[0][2];
	r01 = EA[0] + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A1
	for (i = 0; i < 3; ++i)
	{
		C[1][i] = pi_vec3_dot(&A[1], &B[i]);
		AbsC[1][i] = pi_math_abs(C[1][i]);
		if (AbsC[1][i] > cutoff)
		{
			existsParallelPair = TRUE;
		}
	}
	AD[1] = pi_vec3_dot(&A[1], &D);
	r = pi_math_abs(AD[1]);
	r1 = EB[0]*AbsC[1][0] + EB[1]*AbsC[1][1] + EB[2]*AbsC[1][2];
	r01 = EA[1] + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A2
	for (i = 0; i < 3; ++i)
	{
		C[2][i] = pi_vec3_dot(&A[2], &B[i]);
		AbsC[2][i] = pi_math_abs(C[2][i]);
		if (AbsC[2][i] > cutoff)
		{
			existsParallelPair = TRUE;
		}
	}
	AD[2] = pi_vec3_dot(&A[2], &D);
	r = pi_math_abs(AD[2]);
	r1 = EB[0]*AbsC[2][0] + EB[1]*AbsC[2][1] + EB[2]*AbsC[2][2];
	r01 = EA[2] + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*B0
	r = pi_math_abs(pi_vec3_dot(&B[0], &D));
	r0 = EA[0]*AbsC[0][0] + EA[1]*AbsC[1][0] + EA[2]*AbsC[2][0];
	r01 = r0 + EB[0];
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*B1
	r = pi_math_abs(pi_vec3_dot(&B[1], &D));
	r0 = EA[0]*AbsC[0][1] + EA[1]*AbsC[1][1] + EA[2]*AbsC[2][1];
	r01 = r0 + EB[1];
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*B2
	r = pi_math_abs(pi_vec3_dot(&B[2], &D));
	r0 = EA[0]*AbsC[0][2] + EA[1]*AbsC[1][2] + EA[2]*AbsC[2][2];
	r01 = r0 + EB[2];
	if (r > r01)
	{
		return FALSE;
	}

	// At least one pair of box axes was parallel, so the separation is
	// effectively in 2D where checking the "edge" normals is sufficient for
	// the separation of the boxes.
	if (existsParallelPair)
	{
		return TRUE;
	}

	// axis C0+t*A0xB0
	r = pi_math_abs(AD[2]*C[1][0] - AD[1]*C[2][0]);
	r0 = EA[1]*AbsC[2][0] + EA[2]*AbsC[1][0];
	r1 = EB[1]*AbsC[0][2] + EB[2]*AbsC[0][1];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A0xB1
	r = pi_math_abs(AD[2]*C[1][1] - AD[1]*C[2][1]);
	r0 = EA[1]*AbsC[2][1] + EA[2]*AbsC[1][1];
	r1 = EB[0]*AbsC[0][2] + EB[2]*AbsC[0][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A0xB2
	r = pi_math_abs(AD[2]*C[1][2] - AD[1]*C[2][2]);
	r0 = EA[1]*AbsC[2][2] + EA[2]*AbsC[1][2];
	r1 = EB[0]*AbsC[0][1] + EB[1]*AbsC[0][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A1xB0
	r = pi_math_abs(AD[0]*C[2][0] - AD[2]*C[0][0]);
	r0 = EA[0]*AbsC[2][0] + EA[2]*AbsC[0][0];
	r1 = EB[1]*AbsC[1][2] + EB[2]*AbsC[1][1];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A1xB1
	r = pi_math_abs(AD[0]*C[2][1] - AD[2]*C[0][1]);
	r0 = EA[0]*AbsC[2][1] + EA[2]*AbsC[0][1];
	r1 = EB[0]*AbsC[1][2] + EB[2]*AbsC[1][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A1xB2
	r = pi_math_abs(AD[0]*C[2][2] - AD[2]*C[0][2]);
	r0 = EA[0]*AbsC[2][2] + EA[2]*AbsC[0][2];
	r1 = EB[0]*AbsC[1][1] + EB[1]*AbsC[1][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A2xB0
	r = pi_math_abs(AD[1]*C[0][0] - AD[0]*C[1][0]);
	r0 = EA[0]*AbsC[1][0] + EA[1]*AbsC[0][0];
	r1 = EB[1]*AbsC[2][2] + EB[2]*AbsC[2][1];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A2xB1
	r = pi_math_abs(AD[1]*C[0][1] - AD[0]*C[1][1]);
	r0 = EA[0]*AbsC[1][1] + EA[1]*AbsC[0][1];
	r1 = EB[0]*AbsC[2][2] + EB[2]*AbsC[2][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	// axis C0+t*A2xB2
	r = pi_math_abs(AD[1]*C[0][2] - AD[0]*C[1][2]);
	r0 = EA[0]*AbsC[1][2] + EA[1]*AbsC[0][2];
	r1 = EB[0]*AbsC[2][1] + EB[1]*AbsC[2][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return FALSE;
	}

	return TRUE;
}

PiBool PI_API aabb2obb(PiOBBBox* obb, PiAABBBox* aabb)
{
	PiOBBBox cube_obb;
	cube_obb.axis[0].x = 1;
	cube_obb.axis[0].y = 0;
	cube_obb.axis[0].z = 0;
	cube_obb.axis[1].x = 0;
	cube_obb.axis[1].y = 1;
	cube_obb.axis[1].z = 0;
	cube_obb.axis[2].x = 0;
	cube_obb.axis[2].y = 0;
	cube_obb.axis[2].z = 1;
	cube_obb.center.x = (aabb->minPt.x + aabb->maxPt.x) / 2.0f;
	cube_obb.center.y = (aabb->minPt.y + aabb->maxPt.y) / 2.0f;
	cube_obb.center.z = (aabb->minPt.z + aabb->maxPt.z) / 2.0f;
	cube_obb.extent[0] = aabb->maxPt.x - cube_obb.center.x;
	cube_obb.extent[1] = aabb->maxPt.y - cube_obb.center.y;
	cube_obb.extent[2] = aabb->maxPt.z - cube_obb.center.z;
	return obb2obb(obb, &cube_obb);
}

PiBool PI_API frustum2obb(PiOBBBox* obb, PiFrustum* frustum)
{
	int i, j;
	float d;
	PiBool flag = FALSE;
	PiVector3 points[8];
	pi_obb_get_points(obb, points);
	for(i = 0; i < EFP_COUNT; ++i)
	{
		flag = FALSE;
		for(j = 0; j < 8; j++) {
			d = pi_plane_dot(&frustum->plane[i], points[j].x, points[j].y, points[j].z);
			if(0.0f <= d) {
				flag = TRUE;
				break;
			}
		}
		if(!flag) {
			return FALSE;	/* 包围盒的8个点都在同一个面以外，不可见 */
		}
	}
	return TRUE;
}

PiBool PI_API line2point(PiVector3* point, PiLineSegment* line)
{
	PiVector3 vec0;
	PiVector3 vec1;
	float max;
	float min;
	float length;
	if(pi_vec3_is_equal(point, &line->start))
	{
		return TRUE;
	}
	pi_vec3_sub(&vec0, point, &line->end); 
	pi_vec3_sub(&vec1, point, &line->start);
	length = pi_vec3_len(&vec0);
	vec0.x /= length;
	vec0.y /= length;
	vec0.z /= length;
	length = pi_vec3_len(&vec1);
	vec1.x /= length;
	vec1.y /= length;
	vec1.z /= length;
	if(1.0f - pi_math_abs(pi_vec3_dot(&vec0, &vec1)) < FLOAT_TOLERANCE) {
		max = MAX(line->start.x, line->end.x);
		min = MIN(line->start.x, line->end.x);
		if(max < point->x)
			return FALSE;
		if(min >= point->x)
			return FALSE;
		max = MAX(line->start.y, line->end.y);
		min = MIN(line->start.y, line->end.y);
		if(max < point->y)
			return FALSE;
		if(min >= point->y)
			return FALSE;
		max = MAX(line->start.z, line->end.z);
		min = MIN(line->start.z, line->end.z);
		if(max < point->z)
			return FALSE;
		if(min >= point->z)
			return FALSE;
		return TRUE;	
	}	

	return FALSE;

}

PiBool PI_API sphere2point(PiVector3* point, PiSphere* sphere)
{
	float distance = pi_vec3_distance_square(point, &sphere->pos);
	return distance < sphere->radius * sphere->radius;
}

PiBool PI_API aabb2point(PiVector3* point, PiAABBBox* aabb)
{
	if(aabb->maxPt.x < point->x)
		return FALSE;
	if(aabb->maxPt.y < point->y)
		return FALSE;
	if(aabb->maxPt.z < point->z)
		return FALSE;
	if(aabb->minPt.x >= point->x)
		return FALSE;
	if(aabb->minPt.y >= point->y)
		return FALSE;
	if(aabb->minPt.z >= point->z)
		return FALSE;
	return TRUE;
}

PiBool PI_API obb2point(PiVector3* point, PiOBBBox* obb)
{
	PiMatrix4 rotation;
	PiVector3 tmp;
	pi_vec3_sub(&tmp, point, &obb->center);
	rotation.m[0][0] = obb->axis[0].x;
	rotation.m[0][1] = obb->axis[0].y;
	rotation.m[0][2] = obb->axis[0].z;
	rotation.m[0][3] = 0;
	rotation.m[1][0] = obb->axis[1].x;
	rotation.m[1][1] = obb->axis[1].y;
	rotation.m[1][2] = obb->axis[1].z;
	rotation.m[1][3] = 0;
	rotation.m[2][0] = obb->axis[2].x;
	rotation.m[2][1] = obb->axis[2].y;
	rotation.m[2][2] = obb->axis[2].z;
	rotation.m[2][3] = 0;
	rotation.m[3][0] = 0;
	rotation.m[3][1] = 0;
	rotation.m[3][2] = 0;
	rotation.m[3][3] = 1;

	pi_mat4_apply_point(&tmp, &tmp, &rotation);

	if(obb->extent[0] < tmp.x)
		return FALSE;
	if(obb->extent[1] < tmp.y)
		return FALSE;
	if(obb->extent[2] < tmp.z)
		return FALSE;
	if(-obb->extent[0] >= tmp.x)
		return FALSE;
	if(-obb->extent[1] >= tmp.y)
		return FALSE;
	if(-obb->extent[2] >= tmp.z)
		return FALSE;
	return TRUE;
}


PiBool PI_API frustum2point(PiVector3* point, PiFrustum* frustum)
{
	return pi_frustum_is_point_visible(frustum, point);
}

EIntersectState PI_API line2aabb_i(PiAABBBox* aabb, PiLineSegment* line)
{
	return line2aabb(aabb, line) ? EIS_INTERSECTS : EIS_OUTSIDE;
}

EIntersectState PI_API sphere2aabb_i(PiAABBBox* aabb, PiSphere* sphere)
{
	PiVector3 far_point, aabb_center;
	pi_aabb_get_center(aabb, &aabb_center);	
	if (aabb_center.x >= sphere->pos.x)
	{
		far_point.x = aabb->maxPt.x;
	}
	else 
	{
		far_point.x = aabb->minPt.x;
	}

	if (aabb_center.y >= sphere->pos.y)
	{
		far_point.y = aabb->maxPt.y;
	}
	else 
	{
		far_point.y = aabb->minPt.y;
	}

	if (aabb_center.z >= sphere->pos.z)
	{
		far_point.z = aabb->maxPt.z;
	}
	else 
	{
		far_point.z = aabb->minPt.z;
	}

	if (pi_vec3_distance_square(&far_point, &sphere->pos) < sphere->radius * sphere->radius)
	{
		return EIS_INSIDE;
	}
	else 
	{
		return sphere2aabb(aabb, sphere) ? EIS_INTERSECTS : EIS_OUTSIDE;
	}

}

EIntersectState PI_API aabb2aabb_i(PiAABBBox* aabb_p, PiAABBBox* aabb_i)
{
	const PiVector3* minCube = &aabb_i->minPt;
	const PiVector3* maxCube = &aabb_i->maxPt;
	const PiVector3* minBox = &aabb_p->minPt;
	const PiVector3* maxBox = &aabb_p->maxPt;

	if ((minCube->x < minBox->x && maxCube->x > maxBox->x) 
		&& (minCube->y < minBox->y && maxCube->y > maxBox->y)
		&& (minCube->z < minBox->z && maxCube->z > maxBox->z))
	{
		return EIS_INSIDE;	
	}

	return aabb2aabb(aabb_p, aabb_i) ? EIS_INTERSECTS : EIS_OUTSIDE; 
}

EIntersectState PI_API obb2aabb_i(PiAABBBox* aabb, PiOBBBox* obb)
{
	uint i; 
	PiVector3 points[8];
	points[0].x = aabb->maxPt.x;
	points[0].y = aabb->maxPt.y;
	points[0].z = aabb->maxPt.z;
	points[1].x = aabb->minPt.x;
	points[1].y = aabb->minPt.y;
	points[1].z = aabb->minPt.z;
	points[2].x = aabb->minPt.x;
	points[2].y = aabb->maxPt.y;
	points[2].z = aabb->maxPt.z;
	points[3].x = aabb->maxPt.x;
	points[3].y = aabb->minPt.y;
	points[3].z = aabb->maxPt.z;
	points[4].x = aabb->maxPt.x;
	points[4].y = aabb->maxPt.y;
	points[4].z = aabb->minPt.z;
	points[5].x = aabb->minPt.x;
	points[5].y = aabb->minPt.y;
	points[5].z = aabb->maxPt.z;
	points[6].x = aabb->minPt.x;
	points[6].y = aabb->maxPt.y;
	points[6].z = aabb->minPt.z;
	points[7].x = aabb->maxPt.x;
	points[7].y = aabb->minPt.y;
	points[7].z = aabb->minPt.z;

	for(i = 0; i < 8; i++) {
		if(!obb2point(&points[i], obb)) {
			return aabb2obb(obb, aabb) ? EIS_INTERSECTS : EIS_OUTSIDE;
		}
	}
	return EIS_INSIDE;
}

EIntersectState PI_API frustum2aabb_i(PiAABBBox* aabb, PiFrustum* frustum)
{
	int i, j;
	float d;
	PiBool in;
	PiBool out;
	EIntersectState result = EIS_INSIDE;
	PiVector3 points[8];
	points[0].x = aabb->maxPt.x;
	points[0].y = aabb->maxPt.y;
	points[0].z = aabb->maxPt.z;
	points[1].x = aabb->minPt.x;
	points[1].y = aabb->minPt.y;
	points[1].z = aabb->minPt.z;
	points[2].x = aabb->minPt.x;
	points[2].y = aabb->maxPt.y;
	points[2].z = aabb->maxPt.z;
	points[3].x = aabb->maxPt.x;
	points[3].y = aabb->minPt.y;
	points[3].z = aabb->maxPt.z;
	points[4].x = aabb->maxPt.x;
	points[4].y = aabb->maxPt.y;
	points[4].z = aabb->minPt.z;
	points[5].x = aabb->minPt.x;
	points[5].y = aabb->minPt.y;
	points[5].z = aabb->maxPt.z;
	points[6].x = aabb->minPt.x;
	points[6].y = aabb->maxPt.y;
	points[6].z = aabb->minPt.z;
	points[7].x = aabb->maxPt.x;
	points[7].y = aabb->minPt.y;
	points[7].z = aabb->minPt.z;
	for(i = 0; i < EFP_COUNT; ++i)
	{
		in = FALSE;
		out = FALSE;
		for(j = 0; j < 8; j++) {
			d = pi_plane_dot(&frustum->plane[i], points[j].x, points[j].y, points[j].z);
			if(0.0f <= d) {
				in = TRUE;
			}
			else
			{
				out = TRUE;
			}
			if(in && out) {
				break;
			}
		}
		if(!in && out) {
			return EIS_OUTSIDE;
		}
		else if(in && out)
		{
			result = EIS_INTERSECTS;
		}
	}
	return result;
}

