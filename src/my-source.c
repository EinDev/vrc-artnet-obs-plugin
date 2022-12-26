#include <obs-module.h>

struct vrc_source {
	struct vec4 color_white;
	struct vec4 color_black;

	uint32_t width;
	uint32_t height;

	obs_source_t *src;
};

static const char *my_source_name(void *unused) {
    UNUSED_PARAMETER(unused);
    return obs_module_text("SourceName");
}

static void *my_source_create(obs_data_t *settings, obs_source_t *source) {
    struct vrc_source *context = bzalloc(sizeof(struct vrc_source));
}

static void my_source_destroy(void *data)
{
	bfree(data);
}

static void my_source_update(void *data, obs_data_t *settings)
{
	struct vrc_source *context = data;
	uint32_t width = (uint32_t)obs_data_get_int(settings, "width");
	uint32_t height = (uint32_t)obs_data_get_int(settings, "height");

	vec4_from_rgba(&context->color_white, 0x000000FF);
	vec4_from_rgba_srgb(&context->color_black, 0xFFFFFFFF);
	context->width = width;
	context->height = height;
}

static void my_source_render_helper(struct color_source *context)
{
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *color = gs_effect_get_param_by_name(solid, "color");
	gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

	gs_effect_set_vec4(color, context->color_white);

	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);
	
	gs_draw_sprite(0, 0, 8, 8);
	gs_draw_sprite(8, 8, 8, 8);
	gs_draw_sprite(16, 16, 8, 8);

	gs_technique_end_pass(tech);
	gs_technique_end(tech);
}

static void my_source_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);

	struct vrc_source *context = data;

	/* need linear path for correct alpha blending */
	const bool linear_srgb = gs_get_linear_srgb() ||
				 (context->color.w < 1.0f);

	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(linear_srgb);

	if (linear_srgb)
		my_source_render_helper(context, &context->color_srgb);
	else
		my_source_render_helper(context, &context->color);

	gs_enable_framebuffer_srgb(previous);
}

static uint32_t my_source_width(void *data)
{
	struct vrc_source *context = data;
	return context->width;
}

static uint32_t my_source_height(void *data)
{
	struct vrc_source *context = data;
	return context->height;
}

static void my_source_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "width", 1920);
	obs_data_set_default_int(settings, "height", 1080);
}

static obs_properties_t *my_source_properties(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();

	obs_properties_add_int(props, "width",
			       obs_module_text("MySource.Width"), 0, 4096,
			       1);

	obs_properties_add_int(props, "height",
			       obs_module_text("MySource.Height"), 0, 4096,
			       1);

	return props;
}

struct obs_source_info my_source {
        .id           = "vrc_artnet_plugin",
        .type         = OBS_SOURCE_TYPE_INPUT,
        .output_flags = OBS_SOURCE_VIDEO,
        .get_name     = my_source_name,
        .create       = my_source_create,
        .destroy      = my_source_destroy,
        .update       = my_source_update,
        .video_render = my_source_render,
        .get_width    = my_source_width,
        .get_height   = my_source_height,
		.get_defaults = my_source_defaults,
		.get_properties = my_source_properties
};
