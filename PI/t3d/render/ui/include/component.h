#ifndef INCLUDE_COMPONENT_H
#define INCLUDE_COMPONENT_H

#include "rectangle.h"
#include "texture.h"
#include <renderview.h>
#include <rendertarget.h>

/**
 * 本文件定义了UI系统中的原始组件
 * UI系统的核心模块,所有具体控件均由此模块扩展
 * 负责提供UI系统中坐标管理/绘制机制等除去具体绘制方法代码的基本功能
 * 需要注意的是,此模块的部分方法可能会被具体的控件模块接管,在这种情况下,随意调用此模块的
 * 部分接口将导致异常,具体参考对应控件模块内的说明
 */

#define COMPONENT_LAYER_MAX 100
#define COMPONENT_LAYER_MIM 0

typedef struct PiComponent PiComponent;
typedef void (*ComponentDrawFunc)(PiComponent *component, void *impl);
typedef void (*ComponentUpdateFunc)(PiComponent *component, float tpf);

typedef enum
{
	EWT_PANEL = 0,
	EWT_LABEL
} EWidgetsType;

struct PiComponent
{
	EWidgetsType type;

	uint width;
	uint hight;
	PiRectangle bounds;
	float scale[2];
	sint layer;

	PiBool is_visible;
	PiBool is_independent;
	PiBool is_translucent;

	PiComponent *parent;
	PiVector *children;

	PiRectangle global_bounds;
	float global_scale[2];
	PiRectangle reprint_bounds;
	float global_layer;		//实现DepthTest优化时的保留字段

	//Masks TODO:合并字段
	PiBool is_bounds_update;
	PiBool is_reprint;
	PiBool is_internal_reprint;
	PiBool is_layer_update;

	PiRenderView *independent_view_buffer;
	PiTexture *independent_view_texture;

	ComponentDrawFunc draw_func;
	ComponentUpdateFunc update_func;
	void *impl;
};

PI_BEGIN_DECLS

/**
 * 创建一个UI组件,由具体控件模块的实现负责
 * @param type 控件类型
 * @param draw_func 控件的绘制方法,为NULL时自动使用默认的空函数替代
 * @param update_func 控件的更新方法,为NULL时自动使用默认的空函数替代
 * @param impl 控件的具体实现
 * @return 组件指针
 */
PiComponent *PI_API pi_component_new(EWidgetsType type, ComponentDrawFunc draw_func, ComponentUpdateFunc update_func, void *impl);

/**
 * 释放一个UI组件,由具体控件模块的实现负责,
 * 注意:创建时的传入项不会被此函数释放
 * @param component 需要释放的组件指针
 */
void PI_API pi_component_delete(PiComponent *component);

/**
 * 将一个指定组件添加为某组件的子组件
 * 子控件的坐标/显示/变换等将以父控件为相对跟随关系
 * 默认情况下,子控件无法于父控件的绘制范围外显示(一些特定的布局管理器会根据子控件大小改变父控件,可一定程度绕过此限制)
 * 再次添加一个已经被添加为子控件的组件时将导致此组件由先前的父组件上被自动移除
 * @param parent 父组件指针
 * @param child 子组件指针
 * @param layout 布局管理参数,注意此参数会在成功remove此控件时被自动释放
 */
void PI_API pi_component_attach_child(PiComponent *parent, PiComponent *child);

/**
 * 将一个指定组件从其父组件上移除
 * 如果父组件不存在,则无任何效果
 * 此方法会自动释放add时的layout参数项
 * @param child 子组件指针
 */
void PI_API pi_component_detach_from_parent(PiComponent *child);

/**
 * 指定组件的长宽
 * @param component 组件指针
 * @param width 组件长度
 * @param height 组件宽度
 */
void PI_API pi_component_set_size(PiComponent *component, uint width, uint height);

/**
 * 指定组件的位置
 * @param component 组件指针
 * @param x 组件位置X坐标
 * @param y 组件位置Y坐标
 */
void PI_API pi_component_set_location(PiComponent *component, sint x, sint y);

void PI_API pi_component_get_size(PiComponent *component, uint size[2]);

void PI_API pi_component_get_location(PiComponent *component, sint location[2]);

/**
 * 指定组件的缩放
 * 注意:此接口区别于pi_component_set_size,前者是绘制时的原始大小,且改变时不影响子控件,此接口并不保证像素精度,
 * 比如过渡的放大某个控件可能导致控件出现精度不足的锯齿,而缩放父节点时子节点也会相应发生缩放保持相对的整体比例不变
 * 在独立绘制情况下,此接口速度远远快于前者
 *
 * TODO:Scale目前存在int和float转换后的精度丢失问题,会导致独立纹理化后的父控件scale后子控件运动时的绘制发生抖动和脏区域清除不净
 * 解决方法为固定独立纹理化后之后的所有控件到一个统一的坐标系(如强制缩放为1),但这样需要额外的内存数据存储和计算
 * @param component 组件指针
 * @param x 组件缩放X分量
 * @param y 组件缩放Y分量
 */
void PI_API pi_component_set_scale(PiComponent *component, float x, float y);

/**
 * 指定组件的绘制层级
 * @param component 组件指针
 * @param layer 组件绘制层级,同父组件的孩子中Layer越大的组件将被绘制到越靠前的位置,默认为0,可以为负数
 */
void PI_API pi_component_set_layer(PiComponent *component, sint layer);

/**
 * 指定组件是否可见
 * 如果父组件被隐藏,其所有子组件也会无法显示
 * @param component 组件指针
 * @param b 组件是否可见
 */
void PI_API pi_component_set_visible(PiComponent *component, PiBool b);

/**
 * 指定组件是否独立纹理化
 * 如果一个组件被指定为独立纹理化,那么其自身以及所有子控件的绘制结果将被独立的缓存起来
 * 除非内部必要的重绘操作外,其他大部分重绘请求被触发时这些组件将被视为一个单一组件,绘制时仅会产生一个DrawCall
 * 合理使用此方法可以有效节约效率,尤其是针对内部结构固定不变但作为一个整体被频繁更新的组件
 * 注意:目前实现中根组件(父亲为NULL),无需调用此接口,一律视为独立纹理化开启
 * @param component 组件指针
 * @param b 组件是否开启独立纹理化
 */
void PI_API pi_component_set_independent(PiComponent *component, PiBool b);

/**
 * 指定组件是否为半透明组件
 * TODO:暂时未实现
 * 如果组件不透明,那么绘制时会使用DepthBuffer来优化绘制速度和脏区域更新算法,可以获得更高的效率
 * 如果不开启,半透明控件的绘制将可能出现异常
 * @param component 组件指针
 * @param b 组件是否半透明,默认为False
 */
void PI_API pi_component_set_translucent(PiComponent *component, PiBool b);

/************************************************************************/
/*   以下为系统内部调用接口,除非有特殊需求,其他时候用户代码不应该调用以下接口       */
/************************************************************************/

/**
 * 强制重绘指定组件
 * 大部分时候此方法会在需要时自动调用,但如果在进行某些操作导致显示异常时,可尝试调用此方法
 * @param component 组件指针
 */
void PI_API pi_component_reprint(PiComponent *component);

/**
 * 绘制组件
 */
void PI_API pi_component_draw(PiComponent *component);

/**
 * 标记已完成的绘制
 */
void PI_API pi_component_finish_draw(PiComponent *component);

/**
 * 更新组件
 */
void PI_API pi_component_update(PiComponent *component, float tpf);

/**
 * 获取当前帧需重绘的组件
 */
void PI_API pi_component_get_reprint_components(PiComponent *component, PiVector *result_buffer, PiVector *tmp_stack, PiVector *flip_stack);

/**
 * 组件排序
 */
void PI_API pi_component_sort_layer(PiVector *components);

PI_END_DECLS

#endif /* INCLUDE_COMPONENT_H */
