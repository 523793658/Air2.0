
#include <gird_obstacle.h>

static PiBool _is_obstacle(uint8 value, PiBool onlyStatic)
{
	if(onlyStatic)
	{
		return value % 2 != 0;
	}
	else
	{
		return value != 0;
	}
}

static PiBool _is_obstacle_lazy( GirdObstacle *obs, uint32 x, uint32 y, PiBool onlyStatic)
{
	if(x < obs->width && y < obs->height)
		return _is_obstacle(obs->data[x + y * obs->width], onlyStatic);
	return TRUE;
}

static uint32 _obstacle_value( GirdObstacle *obs, uint32 x, uint32 y) 
{
	if(x < obs->width && y < obs->height)
		return obs->data[x + y * obs->width];
	return 1;
}

static void _set_obstacle(GirdObstacle *obs, uint32 x, uint32 y, uint8 value, uint8 type)
{
	if(x < obs->width && y < obs->height)
	{
		if(type == 0)
			obs->data[x + y * obs->width] = value;
		else if(type == 1)
			obs->data[x + y * obs->width] += value;
		else if(type == 2)
			obs->data[x + y * obs->width] -= value;
	}
}

static void _set_obstacle_size(GirdObstacle *obs, uint32 x, uint32 y, int32 size, uint8 value, uint8 type)
{
	int32 i, j;

	if(size == 1)
		_set_obstacle(obs, x, y, value, type );
	else
	{
		_set_obstacle(obs, x, y, value, type );
		for( i=1; i < size; ++i )
		{
			for( j = -i; j<= i; ++j)
				_set_obstacle(obs, x+j, y-i, value, type);

			for( j = -i; j<= i; ++j)
				_set_obstacle(obs, x+j, y+i, value, type);

			for( j = -i+1; j < i; ++j)
				_set_obstacle(obs, x-i, y+j, value, type);

			for( j = -i+1; j < i; ++j)
				_set_obstacle(obs, x+i, y+j, value, type);
		}
	}
}

static uint32 _get_obstacle_value( GirdObstacle *obs, uint32 x, uint32 y, int32 size ) 
{
	int32 i, j;
	uint32 result = 0;
	if (size == 1)
		return _obstacle_value(obs, x, y);

	for( i=1; i < size; ++i )
	{
		for( j = -i; j<= i; ++j)
		{
			if(_obstacle_value(obs, x+j, y-i) > result )
				result = _obstacle_value(obs, x+j, y-i);
		}

		for( j = -i; j<= i; ++j)
		{
			if(_obstacle_value(obs, x+j, y+i) > result )
				result = _obstacle_value(obs, x+j, y+i);
		}

		for( j = -i+1; j < i; ++j)
		{
			if(_obstacle_value(obs, x-i, y+j) > result )
				result = _obstacle_value(obs, x-i, y+j);
		}

		for( j = -i+1; j < i; ++j)
		{
			if(_obstacle_value(obs, x+i, y+j) > result )
				result = _obstacle_value(obs, x+i, y+j);
		}
	}

	return result;
}

uint32 PI_API s3d_get_obstacle_value (GirdObstacle *obs, uint32 x, uint32 y, int32 size)
{
    return _get_obstacle_value(obs, x, y, size);
}

static PiBool _is_obstacle_size( GirdObstacle *obs, uint32 x, uint32 y, int32 size, PiBool onlyStatic )
{
	int32 i, j;
	if(size == 1)
		return _is_obstacle_lazy(obs, x, y, onlyStatic);

	for( i=1; i < size; ++i )
	{
		for( j = -i; j<= i; ++j)
		{
			if(_is_obstacle_lazy(obs, x+j, y - i, onlyStatic) )
				return TRUE;
		}

		for( j = -i; j<= i; ++j)
		{
			if(_is_obstacle_lazy(obs,x+j, y + i, onlyStatic) )
				return TRUE;
		}

		for( j = -i+1; j < i; ++j)
		{
			if(_is_obstacle_lazy(obs,x+i, y - j, onlyStatic) )
				return TRUE;
		}

		for( j = -i+1; j < i; ++j)
		{
			if(_is_obstacle_lazy(obs,x-i, y + j, onlyStatic) )
				return TRUE;
		}
	}

	return FALSE;
}

PiBool PI_API s3d_is_obstacle( GirdObstacle *obs, uint32 x, uint32 y, int32 size )
{
    return _is_obstacle_size(obs, x, y, size, FALSE);
}

PiBool PI_API s3d_is_static_obstacle( GirdObstacle *obs, uint32 x, uint32 y, int32 size ){
	return _is_obstacle_size(obs, x, y, size, TRUE);
}

// 交换整数 a 、b 的值
static void _swap_int(int32 *a, int32 *b) {
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

// Bresenham's line algorithm
static PiBool _node_visible_test(GirdObstacle *obs, uint32 srcIdx, uint32 dstIdx, int32 *outX, int32 *outY, PiBool onlyStatic, int32 *obsX, int32 *obsY)
{
	int32 x1 = srcIdx % obs->width;
	int32 y1 = srcIdx / obs->width;
	int32 x2 = dstIdx % obs->width;
	int32 y2 = dstIdx / obs->width;

	return node_visible_test(obs, x1, y1, x2, y2, outX, outY, FALSE, obsX, obsY);
}

PiBool node_visible_test_func(GirdObstacle *obs, uint32 srcIdx, uint32 dstIdx, PiBool onlyStatic)
{
	return _node_visible_test(obs, srcIdx, dstIdx, NULL, NULL, onlyStatic, NULL, NULL);
}

PiBool PI_API node_visible_test(GirdObstacle *obs, int32 x1, int32 y1, int32 x2, int32 y2, int32 *outX, int32* outY, PiBool onlyStatic, int32 *obsX, int32 *obsY)
{
	PiBool is_obstacle1, is_obstacle2, y_flag;

	int32 dx = ABS(x2 - x1),
		dy = ABS(y2 - y1),
		yy = 0;
	int32 ix, iy, cx, cy, n2dy, n2dydx, d;

	if (dx < dy) {
		yy = 1;
		_swap_int(&x1, &y1);
		_swap_int(&x2, &y2);
		_swap_int(&dx, &dy);
	}

	ix = (x2 - x1) > 0 ? 1 : -1;
	iy = (y2 - y1) > 0 ? 1 : -1;
	cx = x1;
	cy = y1;
	n2dy = dy * 2;
	n2dydx = (dy - dx) * 2;
	d = dy * 2 - dx; //误差初值
	if(obs->curr_size == 1){
		if (yy) { // 如果直线与 x 轴的夹角大于 45 度
			while (cx != x2) {
				is_obstacle1 = _is_obstacle_lazy(obs, cy + iy, cx + ix, onlyStatic);
				is_obstacle2 = _is_obstacle_lazy(obs, cy, cx + ix, onlyStatic);
				if (d < 0) {
					d += n2dy;
                    y_flag = TRUE;
				} else {
					cy += iy;
					d += n2dydx;
                    y_flag = FALSE;
				}
				if(cx == x1 && cy == y1 && !is_obstacle1 && !is_obstacle2){
					cx += ix;
					continue;
				}
				if(!_is_obstacle_lazy(obs, cy, cx, onlyStatic) && (ABS(cy + iy - y1) > dy || !is_obstacle1) && !is_obstacle2)
					cx += ix;
				else
				{
					if(outX != NULL)
						*outX = y_flag ? cy : cy - iy;
					if(outY != NULL)
						*outY = cx == x1 ? cx : cx - ix;
					if(obsX != NULL && obsY != NULL)
					{
						
						if(is_obstacle1){
							*obsX = cy + iy;
							*obsY = cx + ix;
						}
						else if(is_obstacle2){
							*obsX = cy;
							*obsY = cx + ix;
						}
						else
						{
							*obsX = cy;
							*obsY = cx;
						}
					}
					return FALSE;
				}
				//putpixel(img, cy, cx, c);
			}
		} else { // 如果直线与 x 轴的夹角小于 45 度
			while (cx != x2) {
				//todo  切线方向上的障碍为什么为True？
				is_obstacle1 = _is_obstacle_lazy(obs, cx + ix, cy + iy, onlyStatic);
				is_obstacle2 = _is_obstacle_lazy(obs, cx + ix, cy, onlyStatic);
				if (d < 0) {
					d += n2dy;
                    y_flag = TRUE;
				} else {
					cy += iy;
					d += n2dydx;
                    y_flag = FALSE;
				}
				if(cx == x1 && cy == y1 && !is_obstacle1 && !is_obstacle2){
					cx += ix;
					continue;
				}
				if(!_is_obstacle_lazy(obs, cx, cy, onlyStatic) && (ABS(cy + iy - y1) > dy || !is_obstacle1) && !is_obstacle2)
					cx += ix;
				else
				{
					if(outX != NULL)
						*outX = cx == x1 ? cx : cx - ix;
					if(outY != NULL)
						*outY = y_flag ? cy : cy - iy;
					if(obsX != NULL && obsY != NULL)
					{
						if(is_obstacle1){
							*obsX = cx + ix;
							*obsY = cy + iy;
						}
						else if(is_obstacle2){
							*obsX = cx + ix;
							*obsY = cy;
						}
						else
						{
							*obsX = cy;
							*obsY = cx;
						}
					}
					return FALSE;
				}
				//putpixel(img, cx, cy, c);
			}
		}
	}else{
		if (yy) { 
            y_flag = TRUE;
			while (cx != x2) {
				int32 temp_x = ABS(cx - x1);
				int32 temp_y = ABS(cy - y1);
				if (temp_x  < obs->curr_size && temp_y< obs->curr_size)
				{
					if (d < 0) {
						d += n2dy;
                        y_flag = TRUE;
					} else {
						cy += iy;
						d += n2dydx;
                        y_flag = FALSE;
					}
					cx += ix;
				}else if (temp_x < 2 * obs->curr_size && temp_y < 2 * obs->curr_size)
				{
					//rect axis-x
					int32 i, j;
					for(i = obs->curr_size - temp_x; i < obs->curr_size; i++){
						for (j = 1 - obs->curr_size; j < obs->curr_size; ++j)
						{
							if (_is_obstacle_lazy(obs, cy + j, cx + i, onlyStatic))
							{
								if(outX != NULL)
									*outX = y_flag ? cy : cy - iy;
								if(obsX != NULL)
									*obsX = cy;
								if(outY != NULL)
									*outY = cx == x1 ? cx : cx - ix;
								if(obsY != NULL)
									*obsY = cx;
								return FALSE;
							}
						}	
					}
					//rect axis-y
					for(j = obs->curr_size - temp_x; j < obs->curr_size; j++){
						for (i = 1 - obs->curr_size; i < obs->curr_size; ++i)
						{
							if (i > obs->curr_size + x1 - 1)
							{
								continue;
							}
							if (_is_obstacle_lazy(obs,  cy + j, cx + i, onlyStatic))
							{
								if(outX != NULL)
									*outX = y_flag ? cy : cy - iy;
								if(obsX != NULL)
									*obsX = cy;
								if(outY != NULL)
									*outY = cx == x1 ? cx : cx - ix;
								if(obsY != NULL)
									*obsY = cx;
								return FALSE;
							}
						}	
					}
					if (d < 0) {
						d += n2dy;
                        y_flag = TRUE;
					} else {
						cy += iy;
						d += n2dydx;
                        y_flag = FALSE;
					}
					cx += ix;
				}else{
					is_obstacle1 = _is_obstacle_size(obs, cy + iy, cx + ix, obs->curr_size, onlyStatic);
					is_obstacle2 = _is_obstacle_size(obs, cy, cx + ix, obs->curr_size, onlyStatic);
					if (d < 0) {
						d += n2dy;
                        y_flag = TRUE;
					} else {
						cy += iy;
						d += n2dydx;
                        y_flag = FALSE;
					}
					if(!_is_obstacle_size(obs, cy, cx, obs->curr_size, onlyStatic) && (ABS(cy + iy - y1) > dy || !is_obstacle1) && !is_obstacle2)
						cx += ix;
					else
					{
						if(outX != NULL)
							*outX = y_flag ? cy : cy - iy;
						if(obsX != NULL)
							*obsX = cy;
						if(outY != NULL)
							*outY = cx == x1 ? cx : cx - ix;
						if(obsY != NULL)
							*obsY = cx;
						return FALSE;
					}
				}
				//putpixel(img, cy, cx, c);
			}
		} else { 
            y_flag = TRUE;
			while (cx != x2) {
				int32 temp_x = ABS(cx - x1);
				int32 temp_y = ABS(cy - y1);
				if (temp_x  < obs->curr_size && temp_y< obs->curr_size)
				{
					if (d < 0) {
						d += n2dy;
                        y_flag = TRUE;
					} else {
						cy += iy;
						d += n2dydx;
                        y_flag = FALSE;
					}
					cx += ix;
				}else if (temp_x < 2 * obs->curr_size && temp_y < 2 * obs->curr_size)
				{
					//rect axis-x
					int32 i, j;
					for(i = obs->curr_size - temp_x; i < obs->curr_size; i++){
						for (j = 1 - obs->curr_size; j < obs->curr_size; ++j)
						{
							if (_is_obstacle_lazy(obs, cx + i, cy + j, onlyStatic))
							{
								if(outX != NULL)
									*outX = y_flag ? cy : cy - iy;
								if(obsX != NULL)
									*obsX = cy;
								if(outY != NULL)
									*outY = cx == x1 ? cx : cx - ix;
								if(obsY != NULL)
									*obsY = cx;
								return FALSE;
							}
						}	
					}
					//rect axis-y
					for(j = obs->curr_size - temp_x; j < obs->curr_size; j++){
						for (i = 1 - obs->curr_size; i < obs->curr_size; ++i)
						{
							if (i > obs->curr_size + x1 - 1)
							{
								continue;
							}
							if (_is_obstacle_lazy(obs, cx + i, cy + j, onlyStatic))
							{
								if(outX != NULL)
									*outX = y_flag ? cy : cy - iy;
								if(obsX != NULL)
									*obsX = cy;
								if(outY != NULL)
									*outY = cx == x1 ? cx : cx - ix;
								if(obsY != NULL)
									*obsY = cx;
								return FALSE;
							}
						}	
					}
					if (d < 0) {
						d += n2dy;
                        y_flag = TRUE;
					} else {
						cy += iy;
						d += n2dydx;
                        y_flag = FALSE;
					}
					cx += ix;
				}
				else{
					is_obstacle1 = _is_obstacle_lazy(obs, cx + ix, cy + iy, onlyStatic);
					is_obstacle2 = _is_obstacle_lazy(obs, cx + ix, cy, onlyStatic);
					if (d < 0) {
						d += n2dy;
                        y_flag = TRUE;
					} else {
						cy += iy;
						d += n2dydx;
                        y_flag = FALSE;
					}
					if(cx == x1 && cy == y1 && !is_obstacle1 && !is_obstacle2){
						cx += ix;
						continue;
					}
					if(!_is_obstacle_size(obs, cx, cy, obs->curr_size, onlyStatic) && (ABS(cy + iy - y1) > dy || !is_obstacle1) && !is_obstacle2)
						cx += ix;
					else
					{
						if(outX != NULL)
							*outX = cx == x1 ? cx : cx - ix;
						if(obsX != NULL)
							*obsX = cx;
						if(outY != NULL)
							*outY = y_flag ? cy : cy - iy;
						if(obsY != NULL)
							*obsY = cy;
						return FALSE;
					} 
					//putpixel(img, cx, cy, c);
				}
			}
		}
	}

	return TRUE;
}

void fix_node_to_reachable(GirdObstacle *obs, uint32 *idx, uint32 *ref_idx)
{
	uint32 x = *idx % obs->width;
	uint32 y = *idx / obs->width;
	int32 i,j;

	if(!_is_obstacle_size(obs, x, y, obs->curr_size, FALSE))
		return;

	//一圈圈外扩找
	for( i = obs->curr_size; i < 5 * obs->curr_size; i += 1)
	{
		for(j = -i; j <= i; ++j){
			if( !_is_obstacle_size(obs, x - i, y + j, obs->curr_size, FALSE) )
			{
				*idx = x-i + (y + j) * obs->width;
				return;
			}
		}

		for(j = -i; j <= i; ++j){
			if( !_is_obstacle_size(obs, x + i, y + j, obs->curr_size, FALSE) )
			{
				*idx = x + i + (y + j) * obs->width;
				return;
			}
		}

		for(j = -i+1; j < i; ++j){
			if( !_is_obstacle_size(obs, x + j, y - i, obs->curr_size, FALSE) )
			{
				*idx = x+j + (y-i) * obs->width;
				return;
			}
		}

		for(j = -i+1; j < i; ++j){
			if( !_is_obstacle_size(obs, x + j, y + i, obs->curr_size, FALSE) )
			{
				*idx = x+j + (y+i) * obs->width;
				return;
			}
		}
	}

	//外扩找不到可达点,则找与ref_node直连的第一个障碍的前一个点
	_node_visible_test(obs, *ref_idx, *idx, (int32*)&x, (int32*)&y, FALSE, NULL, NULL);
	*idx = x + y * obs->width;
}

GirdObstacle* PI_API s3d_obstacle_create(uint8 *data, uint32 width, uint32 height)
{
	GirdObstacle *obs = pi_new0(GirdObstacle, 1);
	obs->data = pi_new0(uint8, width * height);
	pi_memcpy(obs->data, data, sizeof(uint8) * width * height);
	obs->width = width;
	obs->height = height;
    obs->curr_size = 1;
	return obs;
}

void PI_API s3d_obstacle_free(GirdObstacle* obs)
{
	pi_free(obs->data);
	pi_free(obs);
}

void PI_API s3d_obstacle_modify_data_bat(GirdObstacle* obs, PiDvector *modify_data)
{
    uint32 num = pi_dvector_size(modify_data);
	uint32 i;
	int32 size;
	uint32 x, y,value,type;

	PI_ASSERT(num % 5 == 0, "gird modify data should be a number that can bi divisible by 4");
	num /= 5;
	for( i=0; i < num; ++i)
	{
		x = *( (uint32*)pi_dvector_get(modify_data, i*5) );
		y = *( (uint32*)pi_dvector_get(modify_data, i*5+1) );
		size = *( (uint32*)pi_dvector_get(modify_data, i*5+2) );
		value = *( (uint32*)pi_dvector_get(modify_data, i*5+3) );
		type = *( (uint32*)pi_dvector_get(modify_data, i*5+4) );

		_set_obstacle_size(obs, x, y, size, (uint8)value, (uint8)type);
	}
}

void PI_API s3d_obstacle_modify_data(GirdObstacle* obs, uint32 x, uint32 y, uint32 size, uint8 value, uint8 type)
{
	_set_obstacle_size(obs, x, y, size, value, type);
}

PiBool PI_API s3d_obstacle_visible_test(GirdObstacle *obs, uint32 src[2], uint32 des[2], int32 result[4])
{
	PiBool value;
	value = _node_visible_test(obs, src[1] * obs->width + src[0], des[1] * obs->width + des[0], &result[0], &result[1], FALSE, &result[2], &result[3]);
	return value;
}

PiBool PI_API s3d_obstacle_modify_data_with_rotate(GirdObstacle *obs, float x, float y, float scaleX, float scaleY, float dirX, float dirY, uint8 value, uint8 type, PiBool modify, uint testPointX, uint testPointY)
{
	 float points[4][2], srcPoints[4][2], dx[4], dy[4], currentX, currentY, valueY[4];
	 int leftPoint = 0, rightPoint, next, first;
	 int32 i, j;
	 float len, sinV, cosV;
	 PiBool isNormal = FALSE;
	 len = pi_math_sqrt(dirX*dirX + dirY*dirY);
	 sinV = dirX/len;
	 cosV = dirY/len;
	 srcPoints[0][0] = -scaleX/2;
	 srcPoints[0][1] = -scaleY/2;
	 srcPoints[1][0] = scaleX/2;
	 srcPoints[1][1] = -scaleY/2;
	 srcPoints[2][0] = scaleX/2;
	 srcPoints[2][1] = scaleY/2;
	 srcPoints[3][0] = -scaleX/2;
	 srcPoints[3][1] = scaleY/2;

	 for(i = 0 ; i < 4; i++)
	 {
		 points[i][0] = srcPoints[i][0]*cosV + srcPoints[i][1] *sinV +x;
		 points[i][1] = srcPoints[i][1]*cosV - srcPoints[i][0] * sinV + y;
		 if(points[i][0] < points[leftPoint][0])
		 {
			 leftPoint = i;
		 }

	 }

	 rightPoint = (leftPoint + 2) % 4;
	 first = leftPoint;
	 for(i = 0 ; i < 4; i++)
	 {
		 next = (first+1)%4;
		 dx[i] = points[next][0] - points[first][0];
		 dy[i] = points[next][1] - points[first][1];
		 if(dx[i] == 0 || dy[i] ==0)
		 {
			 isNormal = TRUE;
			 break;
		 }
		 first = (1+first)%4;
	 }
	 for(currentX = points[leftPoint][0]; currentX <= points[rightPoint][0]; currentX +=1)
	 {
		 float maxY = -1000000, minY = 10000000;
		 int max = 0, min = 0;
		 if(isNormal)
		 {
			 if(points[(leftPoint+1)%4][1] > points[(leftPoint+3)%4][1])
			 {
				 maxY = points[(leftPoint+1)%4][1];
				 minY = points[(leftPoint+3)%4][1];
			 }
			 else
			 {
				 minY = points[(leftPoint+1)%4][1];
				 maxY = points[(leftPoint+3)%4][1];
			 }
		 }
		 else
		 {
			 for(j = 0; j < 4; j++)
			 {
				 first = (leftPoint+j)%4;
				 valueY[j] = (currentX - points[first][0])*dy[j]/dx[j] +points[first][1];
				 max = valueY[max] > valueY[j] ? max : j;
				 min = valueY[min] < valueY[j] ? min : j;
			 }
			 for(j = 0 ; j < 4; j++)
			 {
				 if(j != max && j != min)
				 {
					 if(valueY[j] > maxY)
					 {
						 maxY = valueY[j];
					 }
					 if(valueY[j] < minY)
					 {
						 minY = valueY[j];
					 }
				 }
			 }
		 }
		 for(currentY = minY; currentY <= maxY +0.5; currentY+=1)
		 {
				if(modify)
				{
					_set_obstacle(obs,(int)(currentX), (int)(currentY), value, type);
				}
				else if(((uint)(currentX)) == testPointX && ((uint)(currentY)) == testPointY)
				{
					return TRUE;
				}
		 }
	 }
	 return FALSE;
}

PiBool PI_API is_obstacle_size(GirdObstacle *obs, uint32 x, uint32 y, int32 size, PiBool only_static)
{
	return _is_obstacle_size(obs, x, y, size, only_static);
}

