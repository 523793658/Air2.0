
static void color_set(PiColor *color, float r, float g, float b, float a)
{
	color->rgba[0] = r;
	color->rgba[1] = g;
	color->rgba[2] = b;
	color->rgba[3] = a;
}

static void color_set_byte(PiColor *color, byte r, byte g, byte b, byte a)
{
	color->rgba[0] = r / 255.0f;
	color->rgba[1] = g / 255.0f;
	color->rgba[2] = b / 255.0f;
	color->rgba[3] = a / 255.0f;
}

// TODO: 大端上有问题
static void color_from_int(PiColor *color, uint32 rgba)
{
	byte *c = (byte *)&rgba;
	color_set_byte(color, c[3], c[2], c[1], c[0]);
}

static PiBool color_is_equal(const PiColor *c1, const PiColor *c2)
{
	return pi_memcmp_inline(c1->rgba, c2->rgba, sizeof(c1->rgba)) == PI_COMP_EQUAL;
}

static void color_copy(PiColor *dst, const PiColor *src)
{
	pi_memcpy_inline(dst, src, sizeof(PiColor));
}