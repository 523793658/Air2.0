#include <pi_random.h>

/* 全局随机种子，给不带seed版本的随机数使用 */
static sint s_seed = 0;

/* 此模块要保证线程安全，所以需要一个锁标记 */
volatile static sint s_state = 0;

/* 最小float */
const float s_FLOAT_AM = 1.0f / MAX_INT32;

/* 初始化种子，给不带seed的函数使用 */
static void _init_seed(void)
{
	if(s_state == 0)
	{
		if(pi_atomic_test_set(&s_state, 0, 1))
		{
			s_seed = (sint)pi_time_now();
			pi_atomic_set(&s_state, 2);
		}
		else
		{
			pi_lock_wait_great(&s_seed, 1, LOCK_COUNT);
		}
	}
}

/* 随机数生成算法1，参数是随机种子，返回[0, INT_MAX]之间的值 */
sint PI_API pi_random_seed(sint seed)
{
	int k = 0;
	const int RANDOM_A= 16807;
	const int RANDOM_Q = 127773;
	// const int RANDOM_R = 2836;
	const int RANDOM_MASK = 123459876;
	const int RANDOM_AQR = MAX_INT32;
	seed ^= RANDOM_MASK;
	k = seed / RANDOM_Q;
	seed = RANDOM_A * seed - k * RANDOM_AQR;
	if(seed < 0)
		seed += MAX_INT32;
	return seed;
}

/* 随机数生成算法2，参数是随机种子，返回[0, INT_MAX]之间的值 */
/* 返回start到end范围的某一随机int，用Ranom1更新种子 */
/* 注意：如果 abs(end - start) + 1 > INT_MAX，则返回值不均匀 */
sint PI_API pi_random_int_seed(sint *seed, sint start, sint end)
{
	if(start == end)
		return start;
	if(start > end)
	{
		int temp = start; start = end; end = temp;
	}
	*seed = pi_random_seed(*seed);
	PI_ASSERT(end - start + 1 != 0, "Overflow");
	return start + (*seed) % (end - start + 1);
}

/* 返回start到end范围的某一随机float，用Ranom1更新种子 */
float PI_API pi_random_float_seed(sint *seed, float start, float end)
{
	float r = 0.0f;
	if(start == end)
		return start;
	*seed = pi_random_seed(*seed);
	r = (*seed) * s_FLOAT_AM;
	return start + r * (end - start);
}

void PI_API pi_random_set_seed(sint seed)
{
	_init_seed();
	s_seed = seed;
}

sint PI_API pi_random_int(sint start, sint end)
{
	_init_seed();

	if(start == end)
		return start;
	else if(start > end)
	{
		int temp = start; start = end; end = temp;
	}
	
	s_seed = pi_random_seed(s_seed);
	PI_ASSERT(end - start + 1 != 0, "Overflow");
	return start + s_seed % (end - start + 1);
}

float PI_API pi_random_float(float start, float end)
{
	float r = start;
	
	_init_seed();

	if(start != end)
	{
		s_seed = pi_random_seed(s_seed);
		r = s_seed * s_FLOAT_AM;
		r = start + r * (end - start);
	}
	return r;
}

