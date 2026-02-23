#include "menu_common.h"

#include "../resource/material_cache.h"
#include "../math/vector2.h"

static struct material* menu_background_material;
static struct material* menu_border_material;

struct material* menu_icons_material;
struct material* solid_primitive_material;
struct material* sprite_blit;

void menu_common_init() {
    menu_background_material = material_cache_load("rom:/materials/menu/menu_corner.mat");
    menu_border_material = material_cache_load("rom:/materials/menu/menu_border.mat");
    menu_icons_material = material_cache_load("rom:/materials/menu/menu_icons.mat");
    solid_primitive_material = material_cache_load("rom:/materials/menu/solid_primitive.mat");
    sprite_blit = material_cache_load("rom:/materials/menu/sprite_blit.mat");
}

void menu_common_destroy() {
    if (!menu_background_material) {
        return;
    }

    material_cache_release(menu_background_material);
    menu_background_material = NULL;
    material_cache_release(menu_border_material);
    menu_border_material = NULL;
    material_cache_release(menu_icons_material);
    menu_icons_material = NULL;
    material_cache_release(solid_primitive_material);
    solid_primitive_material = NULL;
    material_cache_release(sprite_blit);
    sprite_blit = NULL;
}

#define BORDER_MARGIN       5
#define CORNER_SIZE         13
#define BORDER_IMAGE_SIZE   32

#define INSET_SIZE          (CORNER_SIZE - BORDER_MARGIN)

void menu_common_render_background(int x, int y, int w, int h) {
    material_apply(menu_border_material);

    rdpq_texture_rectangle(
        TILE0,
        x - BORDER_MARGIN, y - BORDER_MARGIN,
        x + INSET_SIZE, y + INSET_SIZE, 
        0, 0
    );

    rdpq_texture_rectangle_scaled(
        TILE0,
        x + INSET_SIZE, y - BORDER_MARGIN,
        x + w - INSET_SIZE, y + INSET_SIZE, 
        CORNER_SIZE, 0,
        BORDER_IMAGE_SIZE - CORNER_SIZE, CORNER_SIZE
    );

    rdpq_texture_rectangle(
        TILE0,
        x + w - INSET_SIZE, y - BORDER_MARGIN,
        x + w + BORDER_MARGIN, y + INSET_SIZE, 
        BORDER_IMAGE_SIZE - CORNER_SIZE, 0
    );

    rdpq_texture_rectangle_scaled(
        TILE0,
        x - BORDER_MARGIN, y + INSET_SIZE,
        x + INSET_SIZE, y + h - INSET_SIZE, 
        0, CORNER_SIZE,
        CORNER_SIZE, BORDER_IMAGE_SIZE - CORNER_SIZE
    );

    rdpq_texture_rectangle(
        TILE0,
        x - BORDER_MARGIN, y + h - INSET_SIZE,
        x + INSET_SIZE, y + h + BORDER_MARGIN, 
        0, BORDER_IMAGE_SIZE - CORNER_SIZE
    );

    rdpq_texture_rectangle_scaled(
        TILE0,
        x + INSET_SIZE, y + h - INSET_SIZE,
        x + w - INSET_SIZE, y + h + BORDER_MARGIN, 
        CORNER_SIZE, BORDER_IMAGE_SIZE - CORNER_SIZE,
        BORDER_IMAGE_SIZE - CORNER_SIZE, BORDER_IMAGE_SIZE
    );

    rdpq_texture_rectangle(
        TILE0,
        x + w - INSET_SIZE, y + h - INSET_SIZE,
        x + w + BORDER_MARGIN, y + h + BORDER_MARGIN, 
        BORDER_IMAGE_SIZE - CORNER_SIZE, BORDER_IMAGE_SIZE - CORNER_SIZE
    );

    rdpq_texture_rectangle_scaled(
        TILE0,
        x + w - INSET_SIZE, y + INSET_SIZE,
        x + w + BORDER_MARGIN, y + h - INSET_SIZE, 
        BORDER_IMAGE_SIZE - CORNER_SIZE, CORNER_SIZE,
        BORDER_IMAGE_SIZE, BORDER_IMAGE_SIZE - CORNER_SIZE
    );

    rdpq_texture_rectangle_scaled(
        TILE0,
        x + INSET_SIZE, y + INSET_SIZE,
        x + w - INSET_SIZE, y + h - INSET_SIZE,
        CORNER_SIZE, CORNER_SIZE,
        BORDER_IMAGE_SIZE - CORNER_SIZE, BORDER_IMAGE_SIZE - CORNER_SIZE
    );
}

void menu_transform_point(vector2_t* input, vector2_t* rotation, vector2_t* screen_pos, vector2_t* output) {
    vector2ComplexMul(input, rotation, output);
    vector2Add(output, screen_pos, output);
}