#ifndef __PI_RANDOM_H__
#define __PI_RANDOM_H__

#include <pi_lib.h>

PI_BEGIN_DECLS

/** 
 * 随机数生成算法，带seed版本
 * 参数是随机种子，返回[0, INT_MAX]之间的值 
 */
sint PI_API pi_random_seed(sint seed);

/**
 * 带seed版本
 * 返回start到end范围的某一随机int，用Ranom1更新种子
 * 注意：如果 abs(end - start) + 1 > INT_MAX，则返回值不均匀 
 */
sint PI_API pi_random_int_seed(sint *seed, sint start, sint end);

/** 
 * 带seed版本
 * 返回start到end范围的某一随机float，用Ranom1更新种子 
 */
float PI_API pi_random_float_seed(sint *seed, float start, float end);

/** 
 * 随机数生成算法，不带seed版本
 * 参数是随机种子，返回[0, INT_MAX]之间的值 
 */
void PI_API pi_random_set_seed(sint seed);

/** 
 * 随机数生成算法，不带seed版本
 * 返回start到end范围的某一随机int，用Ranom1更新种子 
 * 注意：如果 abs(end - start) + 1 > INT_MAX，则返回值不均匀 
 */
sint PI_API pi_random_int(sint start, sint end);

/**
 * 随机数生成算法，不带seed版本
 * 返回start到end范围的某一随机float，用Ranom1更新种子 
 */
float PI_API pi_random_float(float start, float end);


PI_END_DECLS

#endif /* __PI_RANDOM_H__ */

