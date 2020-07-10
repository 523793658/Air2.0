#ifndef INCLUDE_PANEL_H
#define INCLUDE_PANEL_H

#include "component.h"
#include "texture.h"

/**
 * 此文件定义了用于显示图片的Panel控件
 * 此控件中pi_panel_set_translucent接口替代了pi_component_set_translucent,后者在任何时候不应被调用
 */
PI_BEGIN_DECLS

/**
 * 创建Panel控件
 * @return 控件指针
 */
PiComponent *PI_API pi_panel_new();

/**
 * 释放控件
 * 注意无需再调用component的free方法
 * @param panel 需要释放的Panel控件指针
 */
void PI_API pi_panel_delete(PiComponent *panel);

/**
 * 设置控件显示的颜色及透明度
 * 注意:此颜色在启用纹理时会以相乘的方式与纹理混合
 * @param panel 控件指针
 * @param r 颜色R通道,范围0~1,默认为1
 * @param g 颜色G通道,范围0~1,默认为1
 * @param b 颜色B通道,范围0~1,默认为1
 * @param a 颜色A通道,范围0~1,直接影响控件的最终透明度,默认为1即不透明
 */
void PI_API pi_panel_set_color(PiComponent *panel, float r, float g, float b, float a);

/**
 * 设置控件绘制的使用的纹理
 * @param panel 控件指针
 * @param texture 纹理指针,NULL表示控件使用纯颜色绘制
 */
void PI_API pi_panel_set_texture(PiComponent *panel, PiTexture *texture);

/**
 * 设置控件绘制纹理的UV动画,当u和v都为0时表示关闭uv动画
 * 注意:开启UV动画会强制关闭序列帧动画
 * @param panel 控件指针
 * @param u UV动画u分量速度
 * @param v UV动画v分量速度
 */
void PI_API pi_panel_set_texture_uv_anim(PiComponent *panel, float u, float v);

/**
 * 设置控件绘制纹理的Tile序列帧动画,当tile_x和tile_y都不大于1时,表示关闭序列帧动画
 * 注意:开启序列帧动画会强制关闭UV动画
 * @param panel 控件指针
 * @param tile_x 水平方向的动画帧数量
 * @param tile_y 竖直方向的动画帧数量
 * @param frame_time 每动画帧的持续时间,也可以用 '1 / fps' 换算
 * @param tile_count 纹理上总计播放Tile帧数量,如一个3*3Tile的纹理,参数为5,则动画只会使用前5帧动画进行循环,此参数为0时会自动计算为tile_x * tile_y
 * @param is_blend 是否启用帧间混合过渡,开启可得到更平滑的效果,但会降低性能
 */
void PI_API pi_panel_set_texture_tile_anim(PiComponent *panel, uint tile_x, uint tile_y, float frame_time, uint tile_count, PiBool is_blend);

/**
 * 设置控件是否启用半透明绘制,否则绘制时像素的Alpha将强制视为1
 * @param panel 控件指针
 * @param b 是否半透明,默认为False
 */
void PI_API pi_panel_set_translucent(PiComponent *panel, PiBool b);

PI_END_DECLS

#endif /* INCLUDE_PANEL_H */
