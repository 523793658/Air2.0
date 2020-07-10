#include "flood_controller.h"

static const int weight[PRIORITY_COUNT] = {1000, 100, 10, 1};

//对于闪烁，交换开始、结束亮度和开始、结束颜色
static void _swap_flash(FloodParam *param)
{
    float scale;
    PiVector3 handle_vector;
    scale = param->start_scale;
    param->start_scale = param->end_scale;
    param->end_scale = scale;
    pi_vec3_copy(&handle_vector, &param->start_color);
    pi_vec3_copy(&param->start_color, &param->end_color);
    pi_vec3_copy(&param->end_color, &handle_vector);
}

//对开始时间的处理，保证显示同步
static void _start_play_handle(FloodParam *param)
{
    int count;
    if (param->curr_time == 0)
    {
        return;
    }
    count = (int)(param->curr_time / param->duration);
    param->count = param->is_loop ? param->count : param->count - count;
    param->curr_time -= param->duration * count;
    if (param->is_flash && count % 2 == 1)
    {
        _swap_flash(param);
    }
}

//时间到时的处理
static void _end_play_handle(PiController *c, FloodPriority priority)
{
    Flood *impl = (Flood *)(c->impl);
    FloodParam *param = impl->params[priority];
    if (param == NULL || param->curr_time < 0)
    {
        return;
    }
    param->count--;
    if (!param->is_loop && param->count <= 0)
    {
        pi_flood_stop(c, priority);
    }
    else
    {
        param->curr_time = 0;
        if (param->is_flash) 
        {
            _swap_flash(param);
        }
    }
}

//计算每层对控制器颜色和亮度的影响值
static PiBool _cal_param(PiController *c, float tpf, FloodPriority priority)
{
    Flood *impl = (Flood *)(c->impl);
    float frac, scale, weightScale;
    PiVector3 handle_vector;
    FloodParam *param = impl->params[priority];
    if (param == NULL || param->curr_time < 0)
    {
        return FALSE;
    }
    param->curr_time += tpf;
    if (param->curr_time >= param->duration)
    {
        param->curr_time = param->duration;
    }
    frac = param->curr_time / param->duration;
    weightScale = (float)weight[priority] / (float)impl->weight;
    
    scale = pi_lerp_float(param->start_scale, param->end_scale, frac);
    impl->flood_scale += weightScale * scale;//MAX(impl->flood_scale, scale);

    pi_vec3_lerp(&handle_vector, &param->start_color, &param->end_color, frac);
    pi_vec3_scale(&handle_vector, &handle_vector, weightScale);
    pi_vec3_add(&impl->flood_color, &impl->flood_color, &handle_vector);
    
    if (param->curr_time >= param->duration)
    {
        _end_play_handle(c, priority);
    }
    return TRUE;
}

static void _update_weight(PiController *c)
{
    int i;
    Flood *impl = (Flood *)(c->impl);
    impl->weight = 0;
    for (i = 0; i < PRIORITY_COUNT; i++)
    {
        if (impl->params[i] != NULL && impl->params[i]->curr_time >= 0)
        {
            impl->weight += weight[i];
        }
    }
}

static PiBool _update(PiController *c, float tpf)
{
    int i;
    PiBool show_flag = FALSE;
    Flood *impl = (Flood *)(c->impl);
    impl->flood_scale = 0.0f;
    pi_vec3_set(&impl->flood_color, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < PRIORITY_COUNT; i++)
    {
        show_flag = _cal_param(c, tpf, (FloodPriority)i) || show_flag;
    }
    impl->show_flag = show_flag;
	return !show_flag;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
    Flood *impl = (Flood *)(c->impl);
    PiEntity* entity = (PiEntity*)obj;
    PiMaterial *material = entity->material;
    PI_ASSERT(type == CAT_ENTITY, "The flood controller can only support entity!");
    pi_material_set_def(material, impl->FLOOD, impl->show_flag);
	pi_material_set_uniform(material, impl->U_FloodScale, UT_FLOAT, 1, &impl->flood_scale, TRUE);
    pi_material_set_uniform(material, impl->U_FloodColor, UT_VEC3, 1, &impl->flood_color, TRUE);
    if (impl->flood_map && impl->flood_tex != NULL)
    {
        pi_material_set_def(material, impl->FLOOD_TEX, impl->flood_map);
		pi_sampler_set_texture(&impl->sampler, impl->flood_tex);
		pi_material_set_uniform(material, impl->U_FloodMap, UT_SAMPLER_2D, 1, &impl->sampler, TRUE);
        pi_material_set_uniform(material, impl->U_FLoodUVVector, UT_VEC2, 1, &impl->uv, TRUE);
    }
    return TRUE;
}

PiController *PI_API pi_flood_new()
{
    int i;
    Flood *impl = pi_new0(Flood, 1);
    PiController *c = pi_controller_new((ControllerType)CONTROLLERER_TYPE, _apply, _update, impl);
    impl->flood_scale = 0.0f;
    impl->weight = 0;
    impl->show_flag = FALSE;
    impl->flood_map = FALSE;
    pi_renderstate_set_default_sampler(&impl->sampler);
	pi_sampler_set_addr_mode(&impl->sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->sampler, TFO_MIN_MAG_POINT);
    pi_vec3_set(&impl->flood_color, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < PRIORITY_COUNT; i++)
    {
        impl->params[i] = NULL;
    }

    impl->FLOOD = pi_conststr("FLOOD");
    impl->FLOOD_TEX = pi_conststr("FLOOD_TEX");
    impl->U_FloodScale = pi_conststr("u_FloodScale");
    impl->U_FloodColor = pi_conststr("u_FloodColor");
    impl->U_FLoodUVVector = pi_conststr("u_FloodUVVector");
    impl->U_FloodMap = pi_conststr("u_FloodMap");

    return c;
}

void PI_API pi_flood_free(PiController *c)
{
    int i;
    Flood *impl = (Flood *)(c->impl);
    for (i = 0; i < PRIORITY_COUNT; i++)
    {
        if (impl->params[i] != NULL) 
        {
            pi_free(impl->params[i]);
            impl->params[i] = NULL;
        }
    }
    pi_free(impl);
	pi_controller_free(c);
}

PiBool PI_API pi_flood_set_map(PiController *c, PiTexture* flood_tex, float u, float v)
{
    Flood *impl = (Flood *)(c->impl);
    impl->flood_map = TRUE;
    impl->flood_tex = flood_tex;
    impl->uv[0] = u;
    impl->uv[1] = v;
    return TRUE;
}

PiBool PI_API pi_flood_set_play(PiController *c, float start_x, float start_y, float start_z, float end_x, float end_y,
                                float end_z, float start_scale, float end_scale, float duration, float start_time, 
                                int count, PiBool loop, PiBool flash, FloodPriority priority)
{
    Flood *impl = (Flood *)(c->impl);
    PI_ASSERT(priority < PRIORITY_COUNT, "The flood params priority error!");
    if (impl->params[priority] == NULL)
    {
        impl->params[priority] = pi_new0(FloodParam, 1);
    }
    pi_vec3_set(&(impl->params[priority]->start_color), start_x, start_y, start_z);
    pi_vec3_set(&(impl->params[priority]->end_color), end_x, end_y, end_z);
    impl->params[priority]->start_scale = start_scale;
    impl->params[priority]->end_scale = end_scale;
    impl->params[priority]->curr_time = start_time;
    impl->params[priority]->is_flash = flash;
    impl->params[priority]->is_loop = loop;
    impl->params[priority]->duration = flash ? duration / 2 : duration;
    impl->params[priority]->count = flash ? count * 2 : count;
    _update_weight(c);
    _start_play_handle(impl->params[priority]);
    return TRUE;
}

PiBool PI_API pi_flood_stop(PiController *c, FloodPriority priority)
{
    Flood *impl = (Flood *)(c->impl);
    PI_ASSERT(priority < PRIORITY_COUNT, "The flood params priority error!");
    if (impl->params[priority] == NULL)
    {
        return FALSE;
    }
    impl->params[priority]->count = 0;
    impl->params[priority]->curr_time = -1;
    _update_weight(c);
    return TRUE;
}