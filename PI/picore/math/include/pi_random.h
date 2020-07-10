#ifndef __PI_RANDOM_H__
#define __PI_RANDOM_H__

#include <pi_lib.h>

PI_BEGIN_DECLS

/** 
 * ����������㷨����seed�汾
 * ������������ӣ�����[0, INT_MAX]֮���ֵ 
 */
sint PI_API pi_random_seed(sint seed);

/**
 * ��seed�汾
 * ����start��end��Χ��ĳһ���int����Ranom1��������
 * ע�⣺��� abs(end - start) + 1 > INT_MAX���򷵻�ֵ������ 
 */
sint PI_API pi_random_int_seed(sint *seed, sint start, sint end);

/** 
 * ��seed�汾
 * ����start��end��Χ��ĳһ���float����Ranom1�������� 
 */
float PI_API pi_random_float_seed(sint *seed, float start, float end);

/** 
 * ����������㷨������seed�汾
 * ������������ӣ�����[0, INT_MAX]֮���ֵ 
 */
void PI_API pi_random_set_seed(sint seed);

/** 
 * ����������㷨������seed�汾
 * ����start��end��Χ��ĳһ���int����Ranom1�������� 
 * ע�⣺��� abs(end - start) + 1 > INT_MAX���򷵻�ֵ������ 
 */
sint PI_API pi_random_int(sint start, sint end);

/**
 * ����������㷨������seed�汾
 * ����start��end��Χ��ĳһ���float����Ranom1�������� 
 */
float PI_API pi_random_float(float start, float end);


PI_END_DECLS

#endif /* __PI_RANDOM_H__ */

