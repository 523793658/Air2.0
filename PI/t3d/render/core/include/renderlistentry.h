#ifndef INCLUDE_RENDERLISTENTRY_H
#define INCLUDE_RENDERLISTENTRY_H

#include <entity.h>

/**
 * ��Ⱦ�б��е���Ŀ
 */
typedef struct
{
	PiEntity *entity;		/* ��Ⱦʵ�� */
	uint state_env;		/* ״̬������ţ������ڵ���Ⱦ����״̬�������ƥ�� */
	uint shader_env;	/* shader������ţ������ڵ���Ⱦ����shader�������ƥ�� */
	void *info;			/* ����ʹ�õ���ʱ��Ϣ */
	char data[0];		/* β���Ĵ�СΪ��block�ֽ� */
}RenderListEntry;

#endif /* INCLUDE_RENDERLISTENTRY_H */