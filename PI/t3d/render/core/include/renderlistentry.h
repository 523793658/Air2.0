#ifndef INCLUDE_RENDERLISTENTRY_H
#define INCLUDE_RENDERLISTENTRY_H

#include <entity.h>

/**
 * 渲染列表中的条目
 */
typedef struct
{
	PiEntity *entity;		/* 渲染实体 */
	uint state_env;		/* 状态环境编号，和所在的渲染器的状态环境编号匹配 */
	uint shader_env;	/* shader环境编号，和所在的渲染器的shader环境编号匹配 */
	void *info;			/* 排序使用的临时信息 */
	char data[0];		/* 尾部的大小为：block字节 */
}RenderListEntry;

#endif /* INCLUDE_RENDERLISTENTRY_H */