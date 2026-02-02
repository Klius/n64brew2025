#include "hud.h"

#include "menu_rendering.h"
#include "../resource/material_cache.h"
#include "../cutscene/cutscene_runner.h"
#include "../render/coloru8.h"
#include "../resource/material_cache.h"
#include "../collision/collision_scene.h"
#include "../render/screen_coords.h"
#include "../time/time.h"
#include "../render/defs.h"
#include "../fonts/fonts.h"
#include "../resource/material_cache.h"
#include "../player/inventory.h"
#include "../scene/scene.h"
#include "../math/vector2.h"
#include "../render/defs.h"
#include "../savefile/savefile.h"
#include "../entities/motorcycle.h"
#include "menu_common.h"
#include <string.h>
#include "../config.h"

#define SCREEN_EDGE_MARGIN      20.0f
#define TEXT_PADDING            2
#define BOX_HEIGHT              10
#define AUTO_SAVE_MARGIN        25
#define AUTO_SAVE_SIZE          16
#define TRACKER_FLASHING_TIME   0.25f
#define TRACKER_FULL_FLASH_TIME 0.25f

static sprite_t* map_test;
static material_t* map_render;

int measure_text(enum font_type font, const char* message) {
    const char* curr = message;

    int result = 0;

    while (*curr) {
        rdpq_font_gmetrics_t metrics;
        bool was_found = rdpq_font_get_glyph_metrics(font_get(font), *curr, &metrics);
        ++curr;

        if (was_found) {
            result += metrics.xadvance;
        }
    }
    
    return result;
}

void hud_render_interaction_preview(struct hud* hud) {
    if (!hud->player->hover_interaction || !update_has_layer(UPDATE_LAYER_WORLD)) {
        return;
    }

    interactable_t *target = interactable_get(hud->player->hover_interaction);

    if (!target) {
        return;
    }

    const char* interaction_name = interact_type_to_name(target->interact_type);

    if (!interaction_name) {
        return;
    }

    dynamic_object_t *obj = collision_scene_find_object(hud->player->hover_interaction);

    if (!obj) {
        return;
    }

    vector2_t screen_pos;
    vector3_t pos = *obj->position;
    pos.y = obj->bounding_box.max.y;
    camera_screen_from_position(hud->camera, &pos, &screen_pos);

    if (screen_pos.x < -SCREEN_EDGE_MARGIN || screen_pos.y < -SCREEN_EDGE_MARGIN ||
        screen_pos.x > SCREEN_WD + SCREEN_EDGE_MARGIN || screen_pos.y > SCREEN_HT + SCREEN_EDGE_MARGIN) {
        return;
    }

    screen_pos.x = floorf(screen_pos.x);
    screen_pos.y = floorf(screen_pos.y);

    int box_width = measure_text(FONT_DIALOG, interaction_name);

    rdpq_sync_pipe();
    material_apply(hud->assets.overlay_material);
    rdpq_set_prim_color((color_t){0, 0, 0, 128});
    rdpq_texture_rectangle(
        TILE0, 
        screen_pos.x - TEXT_PADDING, screen_pos.y - BOX_HEIGHT - TEXT_PADDING, 
        screen_pos.x + box_width + TEXT_PADDING, screen_pos.y + TEXT_PADDING,
        0, 0
    );
    rdpq_sync_pipe();

    rdpq_text_printn(&(rdpq_textparms_t){
            // .line_spacing = -3,
            .align = ALIGN_LEFT,
            .valign = VALIGN_BOTTOM,
            .width = 260,
            .height = 0,
            .wrap = WRAP_NONE,
        }, 
        FONT_DIALOG, 
        screen_pos.x, screen_pos.y, 
        interaction_name,
        strlen(interaction_name)
    );
}

static color_t compass_color = {0x5d, 0x56, 0x9f, 0xff};

static vector2_t arrow_vertices[] = {
    {0.0f, 7.0f},
    {-3.0f, -5.0f},
    {0.0f, -3.0f},
    {3.0f, -5.0f},
};

#define COMPASS_MARGIN      20
#define COMPASS_BORDER_W    28
#define COMPASS_BORDER_H    38

#define BOOST_OFFSET_X      3
#define BOOST_OFFSET_Y      27

static vector2_t compass_center = {SCREEN_WD - 20 - COMPASS_BORDER_W * 0.5 , 34.0f};

void hud_render_compass(struct hud* hud) {
    if (!current_scene || !current_scene->overworld) {
        return;
    }
    material_apply(hud->assets.icon_material);

    rdpq_sprite_blit(hud->assets.compass_border, SCREEN_WD - COMPASS_MARGIN - COMPASS_BORDER_W, COMPASS_MARGIN, NULL);

    int boost_width = (int)(22.0f * motorcycle_get_boost_charge(motorcycle_get()));

    if (boost_width) {
        rdpq_sprite_blit(
            hud->assets.boost_indicator,
            SCREEN_WD - COMPASS_MARGIN - COMPASS_BORDER_W + BOOST_OFFSET_X, COMPASS_MARGIN + BOOST_OFFSET_Y,
            &(rdpq_blitparms_t){
                .width = boost_width,
                .height = 8,
                .t0 = boost_width == 22 ? 8 : 0,
            }
        );
    }

    material_apply(hud->assets.compass_arrow);
    rdpq_set_prim_color(compass_color);

    vector2_t transformed[4];

    vector2_t* rot = player_get_rotation(&current_scene->player);
    for (int i = 0; i < 4; i += 1) {
        menu_transform_point(&arrow_vertices[i], rot, &compass_center, &transformed[i]);
    }
    rdpq_triangle(
        &TRIFMT_FILL, 
        (float*)&transformed[0], 
        (float*)&transformed[1], 
        (float*)&transformed[2]
    );
    rdpq_triangle(
        &TRIFMT_FILL, 
        (float*)&transformed[0], 
        (float*)&transformed[2], 
        (float*)&transformed[3]
    );
}

void hud_render_autosave(struct hud* hud) {
    if (!savefile_is_autosaving()) {
        return;
    }

    material_apply(hud->assets.saving_icon);

    rdpq_set_prim_color((color_t){255, 255, 255, (uint8_t)(cosf(game_time * 4.0f) * 120 + 128)});

    rdpq_texture_rectangle(
        TILE0, 
        SCREEN_WD - AUTO_SAVE_MARGIN - AUTO_SAVE_SIZE, SCREEN_HT - AUTO_SAVE_MARGIN - AUTO_SAVE_SIZE, 
        SCREEN_WD - AUTO_SAVE_MARGIN, SCREEN_HT - AUTO_SAVE_MARGIN,
        0, 0
    );
}

#define TRACKER_ICON_SIZE   24

void hud_render_tracker_icon(struct hud* hud) {
    if (hud->track_flash_timer <= 0.0f) {
        return;
    }

    material_apply(hud->assets.tracker_icon); 
    if (hud->track_flash_timer < TRACKER_FLASHING_TIME) {
        rdpq_set_prim_color((color_t){110, 232, 4, (uint8_t)(255.0f / TRACKER_FLASHING_TIME) * hud->track_flash_timer});
    }
    rdpq_texture_rectangle(
        TILE0,
        SCREEN_WD - COMPASS_MARGIN - COMPASS_BORDER_W - TRACKER_ICON_SIZE, COMPASS_MARGIN,
        SCREEN_WD - COMPASS_MARGIN - COMPASS_BORDER_W, COMPASS_MARGIN + TRACKER_ICON_SIZE,
        0, 0
    );

    hud->track_flash_timer -= fixed_time_step;
}

#define MEM_PADDING 20
#define MEM_HEIGHT  8
#define MEM_WIDTH   100

void hud_render_memory_usage(struct hud* hud) {
    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);

    material_apply(hud->assets.compass_arrow);

    rdpq_set_prim_color((color_t){0, 0, 0, 255});

    rdpq_texture_rectangle(
        TILE0,
        MEM_PADDING, MEM_PADDING,
        MEM_PADDING + MEM_WIDTH / 2, MEM_PADDING + MEM_HEIGHT,
        0, 0
    );

    rdpq_sync_pipe();
    rdpq_set_prim_color((color_t){128, 0, 0, 255});
    
    rdpq_texture_rectangle(
        TILE0,
        MEM_PADDING + MEM_WIDTH / 2, MEM_PADDING,
        MEM_PADDING + MEM_WIDTH, MEM_PADDING + MEM_HEIGHT,
        0, 0
    );

    rdpq_sync_pipe();
    rdpq_set_prim_color((color_t){0, 255, 0, 255});
    
    rdpq_texture_rectangle(
        TILE0,
        MEM_PADDING, MEM_PADDING,
        MEM_PADDING + MEM_WIDTH * heap_stats.used / heap_stats.total, MEM_PADDING + MEM_HEIGHT,
        0, 0
    );
}

void hud_render_nuts(struct hud* hud) {
    int nut_count = inventory_get_count(ITEM_NUT);

    if (nut_count != hud->prev_nut_count) {
        hud->nut_show_timer = 4.0f;
    }

    if (hud->nut_show_timer <= 0.0f) {
        return;
    }

    if (update_has_layer(UPDATE_LAYER_WORLD)) {
        hud->nut_show_timer -= scaled_time_step;
    }

    material_apply(hud->assets.nut_icon);
    
    int frame = (int)(total_time * 8) % 8;

    int nut_y = current_scene && current_scene->overworld ? 20 + COMPASS_BORDER_H + 4 : 20;

    rdpq_texture_rectangle_scaled(
        TILE0,
        SCREEN_WD - 20 - 16, nut_y,
        SCREEN_WD - 20, nut_y + 16,
        frame * 8, 0,
        frame * 8 + 8, 8
    );
    
    char text[16];
    int len = sprintf(text, "%d", nut_count);

    rdpq_text_printn(&(rdpq_textparms_t){
            // .line_spacing = -3,
            .align = ALIGN_RIGHT,
            .valign = VALIGN_TOP,
            .width = 100,
            .height = 20,
            .wrap = WRAP_NONE,
        }, 
        FONT_DIALOG, 
        SCREEN_WD - 100 - 20 - 18, nut_y + 1, 
        text,
        len
    );

    hud->prev_nut_count = nut_count;
}

bool hud_render_dismount(struct hud* hud) {
    if (!current_scene || 
        !current_scene->overworld || 
        !inventory_has_item(ITEM_RIDING_MOTORCYCLE) || 
        inventory_has_item(ITEM_HAS_DISMOUNTED)
    ) {
        return false;
    }

    material_apply(hud->assets.overlay_material);
    rdpq_set_prim_color((color_t){0, 0, 0, 128});

    int x = (SCREEN_WD - 54) / 2;
    int y = 190;

    rdpq_texture_rectangle(
        TILE0, 
        x - TEXT_PADDING, y - TEXT_PADDING, 
        x + 54 + TEXT_PADDING, y + TEXT_PADDING + 25,
        0, 0
    );

    material_apply(hud->assets.b_button);
    rdpq_texture_rectangle(TILE0, x, y, x + 24, y + 24, 0, 0);
    
    rdpq_text_printn(&(rdpq_textparms_t){
            .align = ALIGN_LEFT,
            .valign = VALIGN_TOP,
            .width = 100,
            .height = 20,
            .wrap = WRAP_NONE,
        }, 
        FONT_DIALOG, 
        x + 28, y + 4, 
        "Exit",
        4
    );

    return true;
}

float drift_prompt_at = 0.0f;

bool hud_render_drift(struct hud* hud) {
    if (!current_scene || !current_scene->overworld || inventory_has_item(ITEM_HAS_DRIFTED)) {
        return false;
    }

    if (inventory_has_item(ITEM_RIDING_MOTORCYCLE)) {
        if (drift_prompt_at == 0.0f) {
            drift_prompt_at = game_time + 20.0f;
        }
    } else {
        drift_prompt_at = 0.0f;
        return false;
    }

    if (game_time < drift_prompt_at) {
        return false;
    }

    material_apply(hud->assets.overlay_material);
    rdpq_set_prim_color((color_t){0, 0, 0, 128});

    int x = (SCREEN_WD - 54) / 2;
    int y = 190;

    rdpq_texture_rectangle(
        TILE0, 
        x - TEXT_PADDING, y - TEXT_PADDING, 
        x + 62 + TEXT_PADDING, y + TEXT_PADDING + 25,
        0, 0
    );

    material_apply(hud->assets.r_button);
    rdpq_texture_rectangle(TILE0, x, y + 6, x + 24, y + 18, 0, 0);
    
    rdpq_text_printn(&(rdpq_textparms_t){
            .align = ALIGN_LEFT,
            .valign = VALIGN_TOP,
            .width = 100,
            .height = 20,
            .wrap = WRAP_NONE,
        }, 
        FONT_DIALOG, 
        x + 28, y + 4, 
        "Drift",
        5
    );

    return true;
}

void hud_render(void *data) {
#if ENABLE_CHEATS
    hud_render_memory_usage(data);
#endif

    hud_render_nuts(data);

    if (!update_has_layer(UPDATE_LAYER_WORLD)) {
        return;
    }

    hud_render_interaction_preview(data);
    hud_render_compass(data);
    hud_render_autosave(data);
    hud_render_tracker_icon(data);
    if (!hud_render_dismount(data)) {
        hud_render_drift(data);
    }
}

void hud_init(struct hud* hud, struct player* player, camera_t* camera) {
    menu_add_callback(hud_render, hud, MENU_PRIORITY_HUD);
    font_type_use(FONT_DIALOG);
    hud->player = player;
    hud->camera = camera;
    hud->track_flash_timer = 0.0f;
    hud->nut_show_timer = 0.0f;

    hud->assets.overlay_material = material_cache_load("rom:/materials/menu/solid_primitive.mat");
    hud->assets.compass_border = sprite_load("rom:/images/menu/compass_border.sprite");
    hud->assets.boost_indicator = sprite_load("rom:/images/menu/boost_indicator.sprite");
    hud->assets.icon_material = material_cache_load("rom:/materials/menu/map_icon.mat");
    hud->assets.compass_arrow = material_cache_load("rom:/materials/menu/map_arrow.mat");
    hud->assets.saving_icon = material_cache_load("rom:/materials/menu/nut_icon.mat");
    hud->assets.tracker_icon = material_cache_load("rom:/materials/menu/tracker_icon.mat");   
    hud->assets.nut_icon = material_cache_load("rom:/materials/parts/nut_particle.mat");
    hud->assets.r_button = material_cache_load("rom:/materials/menu/r_button.mat");

    if (!inventory_has_item(ITEM_HAS_DISMOUNTED)) {
        hud->assets.b_button = material_cache_load("rom:/materials/menu/b_button.mat");   
    } else {
        hud->assets.b_button = NULL;
    }

    hud->prev_nut_count = inventory_get_count(ITEM_NUT);
}

void hud_destroy(struct hud* hud) {
    menu_remove_callback(hud);
    font_type_release(FONT_DIALOG);
    material_cache_release(hud->assets.overlay_material);
    material_cache_release(hud->assets.icon_material);
    material_cache_release(hud->assets.compass_arrow);
    material_cache_release(hud->assets.saving_icon);
    material_cache_release(hud->assets.tracker_icon);
    material_cache_release(hud->assets.nut_icon);
    material_cache_release(hud->assets.r_button);
    if (hud->assets.b_button) {
        material_cache_release(hud->assets.b_button);
    }
    sprite_free(hud->assets.compass_border);
    sprite_free(hud->assets.boost_indicator);
}

void hud_flash_tracker() {
    if (!current_scene) {
        return;
    }

    current_scene->hud.track_flash_timer = TRACKER_FLASHING_TIME + TRACKER_FULL_FLASH_TIME;
}

void hud_show_nuts(struct hud* hud) {
    hud->nut_show_timer = 4.0f;
}