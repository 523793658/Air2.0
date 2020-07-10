#ifndef __PI_OBB_H__
#define __PI_OBB_H__
#include <pi_vector3.h>
#include <pi_aabb.h>

typedef struct PiOBBBox
{
	PiVector3 center;	/* 中心 */
	PiVector3 axis[3];	/* OBB轴方向 */
	float extent[3];	/* 半轴长 */
}PiOBBBox;

PI_BEGIN_DECLS

void PI_API pi_obb_copy(PiOBBBox *dst, PiOBBBox *src);

void PI_API pi_obb_get_aabb(PiOBBBox *box, PiAABBBox *dst);

void PI_API pi_obb_get_points(PiOBBBox *box, PiVector3 *buffer);

void PI_API pi_obb_transform(PiOBBBox *box, PiMatrix4 *mat);

PI_END_DECLS

#endif /* __PI_OBB_H__ */
