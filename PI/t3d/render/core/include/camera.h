#ifndef INCLUDE_CAMERA_H
#define INCLUDE_CAMERA_H

#include <pi_lib.h>
#include <pi_vector3.h>
#include <pi_matrix4.h>

/**
 * 摄像机模块
 */

typedef struct  
{
	PiVector3 location;
	PiVector3 lookat;
	PiMatrix4 rotation;
	float frustum_left;
	float frustum_right;
	float frustum_bottom;
	float frustum_top;
	float frustum_near;
	float frustum_far;
	PiBool is_ortho;
	PiMatrix4 view_matrix;
	PiMatrix4 projection_matrix;
	PiMatrix4 view_projection_matrix;
}PiCamera;

PI_BEGIN_DECLS

/**
 * 创建摄像机
 * @returns 摄像机指针
 */
PiCamera* PI_API pi_camera_new();

/**
 * 释放摄像机
 * @param cam 摄相机指针
 */
void PI_API pi_camera_free(PiCamera* cam);

/**
 * 构建相机视椎体（透视投影和正交投影通用）
 * @param cam 摄相机指针
 * @param left 近裁剪面左坐标
 * @param right 近裁剪面右坐标
 * @param bottom 近裁剪面下坐标
 * @param top 近裁剪面上坐标
 * @param near 近裁剪面距离
 * @param far 远裁剪面距离
 * @param is_ortho 是否以正交投影方式构建视椎体
 */
void PI_API pi_camera_set_frustum(PiCamera* cam, float left, float right, float bottom, float top, float near, float far, PiBool is_ortho);

/**
 * 以透视方式构建视椎体
 * @param cam 摄相机指针
 * @param fovy yz平面上的视野角度（degree），必须是0°~180°之间
 * @param aspect 纵横比，宽度除以高度
 * @param near 近裁剪面距离
 * @param far 远裁剪面距离
 */
void PI_API pi_camera_set_perspective(PiCamera* cam, float fovy, float aspect, float near, float far);

/**
 * 保持近裁剪面高度不变，重置相机的纵横比
 * @param cam 摄相机指针
 * @param aspect 纵横比，宽度除以高度
 */
void PI_API pi_camera_resize(PiCamera* cam, float aspect);


/**
* 获取相机的纵横比
* @param cam 摄相机指针
*/
float PI_API pi_camera_get_aspect(PiCamera* cam);

/**
 * 设置相机位置
 * @param cam 摄相机指针
 * @param x 相机位置坐标X分量
 * @param y 相机位置坐标Y分量
 * @param z 相机位置坐标Z分量
 */
void PI_API pi_camera_set_location(PiCamera* cam, float x, float y, float z);

/**
 * 设置相机的旋转
 * @param cam 摄相机指针
 * @param w 相机旋转四元数W分量
 * @param x 相机旋转四元数X分量
 * @param y 相机旋转四元数Y分量
 * @param z 相机旋转四元数Z分量
 */
void PI_API pi_camera_set_rotation(PiCamera* cam, float w, float x, float y, float z);

/**
 * 设置相机的朝向
 * @param cam 摄相机指针
 * @param x 相机朝向的X分量
 * @param y 相机朝向的Y分量
 * @param z 相机朝向的Z分量
 */
void PI_API pi_camera_set_direction(PiCamera* cam, float x, float y, float z);

/**
 * 旋转相机使其朝向指定位置
 * @param cam 摄相机指针
 * @param x 目标位置的X分量
 * @param y 目标位置的Y分量
 * @param z 目标位置的Z分量
 */
void PI_API pi_camera_set_look_at(PiCamera* cam, float x, float y, float z);

/**
 * 设置相机的上方向向量
 * @param cam 摄相机指针
 * @param x 上方向向量的X分量
 * @param y 上方向向量的Y分量
 * @param z 上方向向量的Z分量
 */
void PI_API pi_camera_set_up(PiCamera* cam, float x, float y, float z);

/**
 * 设置自定义投影矩阵
 * 注意：此函数可能会导致相机视椎体设置信息与投影矩阵不匹配，且当投影相关函数被调用时被重新覆盖，慎重使用
 * @param cam 摄相机指针
 * @param proj_mat 投影矩阵
 */
void PI_API pi_camera_set_projection_matrix(PiCamera* cam, PiMatrix4* proj_mat);

/**
 * 获取近裁剪面左坐标
 * @param cam 摄相机指针
 * @returns 近裁剪面左坐标
 */
float PI_API pi_camera_get_frustum_left(PiCamera* cam);

/**
 * 获取近裁剪面右坐标
 * @param cam 摄相机指针
 * @returns 近裁剪面右坐标
 */
float PI_API pi_camera_get_frustum_right(PiCamera* cam);

/**
 * 获取近裁剪面底部坐标
 * @param cam 摄相机指针
 * @returns 近裁剪面底部坐标
 */
float PI_API pi_camera_get_frustum_bottom(PiCamera* cam);

/**
 * 获取近裁剪面顶部坐标
 * @param cam 摄相机指针
 * @returns 近裁剪面顶部坐标
 */
float PI_API pi_camera_get_frustum_top(PiCamera* cam);

/**
 * 获取近裁剪面距离
 * @param cam 摄相机指针
 * @returns 近裁剪面距离
 */
float PI_API pi_camera_get_frustum_near(PiCamera* cam);

/**
 * 获取远裁剪面距离
 * @param cam 摄相机指针
 * @returns 远裁剪面距离
 */
float PI_API pi_camera_get_frustum_far(PiCamera* cam);

/**
 * 获取相机的当前位置
 * @param cam 摄相机指针
 * @returns 相机位置
 */
PiVector3* PI_API pi_camera_get_location(PiCamera* cam);

/**
 * 获取相机的旋转
 * @param cam 摄相机指针
 * @param result 结果缓存
 */
void PI_API pi_camera_get_rotation(PiCamera* cam, PiQuaternion* result);

/**
 * 获取相机朝向
 * @param cam 摄相机指针
 * @param result 结果缓存
 */
void PI_API pi_camera_get_direction(PiCamera* cam, PiVector3* result);

/**
 * 获取相机的向上向量
 * @param cam 摄相机指针
 * @param result 结果缓存
 */
void PI_API pi_camera_get_up(PiCamera* cam, PiVector3* result);

/**
 * 获取视图矩阵
 * @param cam 摄相机指针
 * @returns 获取视图矩阵
 */
PiMatrix4* PI_API pi_camera_get_view_matrix(PiCamera* cam);

/**
 * 获取投影矩阵
 * @param cam 摄相机指针
 * @returns 获取投影矩阵
 */
PiMatrix4* PI_API pi_camera_get_projection_matrix(PiCamera* cam);

/**
 * 获取相机的视图投影矩阵
 * @param cam 摄相机指针
 * @returns 视图投影矩阵
 */
PiMatrix4* PI_API pi_camera_get_view_projection_matrix(PiCamera* cam);

/**
 * 获取当前相机是否为正投影
 * @param cam 摄相机指针
 * @returns 是否为正投影
 */
PiBool PI_API pi_camera_is_ortho(PiCamera* cam);

/**
 * 将世界空间坐标转换到投影空间
 * @param cam 摄相机指针
 * @param x 世界空间坐标X分量
 * @param y 世界空间坐标Y分量
 * @param z 世界空间坐标Z分量
 * @param result 结果缓存
 */
void PI_API pi_camera_world2projection(PiCamera* cam, float x, float y, float z, PiVector3* result);

/**
 * 将投影空间坐标转换到世界空间
 * @param cam 摄相机指针
 * @param x 投影空间坐标X分量
 * @param y 投影空间坐标Y分量
 * @param z 投影空间坐标Z分量
 * @param result 结果缓存
 */
void PI_API pi_camera_projection2world(PiCamera* cam, float x, float y, float z, PiVector3* result);

/**
 * 将屏幕空间坐标转换到投影空间
 * @param screen_width 屏幕宽度
 * @param screen_height 屏幕高度
 * @param x 屏幕空间坐标X分量
 * @param y 屏幕空间坐标Y分量
 * @param result 结果缓存，注意Z分量无意义
 */
void PI_API pi_camera_screen2projection(uint screen_width, uint screen_height, float x, float y, PiVector3* result);

/**
 * 将投影空间坐标转换到屏幕空间
 * @param screen_width 屏幕宽度
 * @param screen_height 屏幕高度
 * @param x 投影空间坐标X分量
 * @param y 投影空间坐标Y分量
 * @param result 结果缓存，注意Z分量无意义
 */
void PI_API pi_camera_projection2screen(uint screen_width, uint screen_height, float x, float y, PiVector3* result);

/**
 * 将世界空间坐标转换到屏幕空间
 * @param cam 摄相机指针
 * @param screen_width 屏幕宽度
 * @param screen_height 屏幕高度
 * @param x 世界空间坐标X分量
 * @param y 世界空间坐标Y分量
 * @param z 世界空间坐标Z分量
 * @param result 结果缓存，注意Z分量无意义
 */
void PI_API pi_camera_world2screen(PiCamera* cam, uint screen_width, uint screen_height, float x, float y, float z, PiVector3* result);

/**
 * 将屏幕空间坐标转换到世界空间
 * @param cam 摄相机指针
 * @param screen_width 屏幕宽度
 * @param screen_height 屏幕高度
 * @param x 屏幕空间坐标X分量
 * @param y 屏幕空间坐标Y分量
 * @param z 投影空间深度值，通常的-1表示近裁剪面，1表示远裁剪面
 * @param result 结果缓存
 */
void PI_API pi_camera_screen2world(PiCamera* cam, uint screen_width, uint screen_height, float x, float y, float z, PiVector3* result);


/**
* 克隆相机
*/
PiCamera* PI_API pi_camera_clone(PiCamera* src);
PI_END_DECLS


#endif /* INCLUDE_CAMERA_H */