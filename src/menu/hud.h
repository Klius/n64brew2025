#ifndef __MENU_HUD_H__
#define __MENU_HUD_H__

#include "../render/material.h"
#include "../player/player.h"
#include "../render/material.h"
#include "../render/camera.h"

struct hud_assets {
    material_t* overlay_material;
    material_t* icon_material;
    material_t* compass_arrow;
    material_t* saving_icon;
    material_t* tracker_icon;
    material_t* nut_icon;
    material_t* b_button;
    material_t* r_button;
    sprite_t* compass_border;
    sprite_t* boost_indicator;
};

typedef struct hud_assets hud_assets_t;

struct hud {
    struct player* player;
    camera_t* camera;
    hud_assets_t assets;
    float track_flash_timer;
    float nut_show_timer;
    uint16_t prev_nut_count;
};

typedef struct hud hud_t;

void hud_init(struct hud* hud, struct player* player, camera_t* camera);
void hud_destroy(struct hud* hud);
void hud_flash_tracker();
void hud_show_nuts(struct hud* hud);

#endif