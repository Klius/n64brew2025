#include "map_menu.h"

#include <libdragon.h>
#include "../resource/material_cache.h"
#include "../time/time.h"
#include "../time/game_mode.h"
#include "../menu/menu_rendering.h"
#include "../math/vector2s16.h"
#include "../scene/scene.h"
#include "../math/minmax.h"
#include "../math/vector4.h"
#include "../fonts/fonts.h"
#include "menu_common.h"
#include "../player/inventory.h"
#include "../render/defs.h"
#include "../render/coloru8.h"
#include "../savefile/savefile.h"
#include "../audio/audio.h"
#include <libdragon.h>

#define NEW_ITEM_ANIM_TIME      1.0f
#define OPEN_ITEM_DELAY_TIME    1.0f

#define MAP_REVEAL_TIME         1.0f

enum menu_item_type {
    MENU_ITEM_PART,
    MENU_ITEM_MAP,

    MENU_ITEM_TYPE_COUNT,
};

enum menu_icon_type {
    MENU_ICON_MOTOR_PART,
    MENU_ICON_WELL_PART,
    MENU_ICON_GEN_FAN_PART,
    MENU_ICON_GEN_STARTER_PART,
    MENU_ICON_GEN_BULB_PART,
    MENU_ICON_NANO_VIAL_PART,
    MENU_ICON_TABLET_BATTERY,
    MENU_ICON_TABLET_MEMORY,
    MENU_ICON_TABLET_SCREEN,
    MENU_ICON_TABLET,
    MENU_ICON_MAP,
    MENU_ICON_NOTE,
    MENU_ICON_IMAGE,
    MENU_ICON_NUT,
    MENU_ICON_BIKE_TELEPORT,
    MENU_ICON_SERVO,


    MENU_ICON_TYPE_COUNT,
};

enum menu_sound_effects {
    MENU_SOUND_BACK,
    MENU_SOUND_CURSOR,
    MENU_SOUND_PAUSE,
    MENU_SOUND_SELECT,
    MENU_SOUND_SAVE,

    MENU_SOUND_COUNT,
};

struct map_asssets {
    sprite_t* map;
    material_t* material;
    material_t* map_background;
    material_t* map_arrow;
    material_t* map_view;
    material_t* map_icon;
    material_t* selection_cursor;
    material_t* check_icon;

    wav64_t* sounds[MENU_SOUND_COUNT];
    wav64_t* unpause_sound;

    sprite_t* icons[MENU_ICON_TYPE_COUNT];
};

union menu_item_data {
    struct {
        const char* description;
    } part;
    struct {
        const char* image_filename;
    } map;
};

struct menu_item {
    enum menu_item_type type;
    enum menu_icon_type icon;
    enum inventory_item_type inventory_item;
    enum inventory_item_type item_complete[3];
    const char* name;
    union menu_item_data data;
    bool show_count;
    bool always_visible;
};

static struct menu_item menu_items[] = {
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_NUT,
        .inventory_item = ITEM_NUT,
        .name = "Nuts",
        .data = {
            .part = {
                .description = "Valuable bits of metal used in trade",
            },
        },
        .show_count = true,
        .always_visible = true,
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_SERVO,
        .inventory_item = ITEM_SERVO,
        .name = "Servo",
        .data = {
            .part = {
                .description = "Some industrial servos the trader bot wants for a replacement screen.",
            },
        },
        .show_count = true,
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_BIKE_TELEPORT,
        .inventory_item = ITEM_BIKE_TELEPORT,
        .name = "Bike teleport",
        .data = {
            .part = {
                .description = "Use R to teleport back to your hover bike. You got this at the shop in town.",
            },
        },
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_BIKE_TELEPORT,
        .inventory_item = ITEM_FAST_TRAVEL,
        .name = "Upgraded teleport",
        .data = {
            .part = {
                .description = "Use R to teleport back to your hover bike. Use D-pad to teleport other places.",
            },
        },
    },
    {
        .type = MENU_ITEM_MAP,
        .icon = MENU_ICON_MAP,
        .inventory_item = ITEM_WELL_PUMP_PART_MAP,
        .item_complete = {ITEM_WELL_HAS_PUMP_GEAR},
        .name = "Well part map",
        .data = {
            .map = {
                .image_filename = "rom:/images/maps/well_parts_map.sprite",
            },
        },
    },
    {
        .type = MENU_ITEM_MAP,
        .icon = MENU_ICON_MAP,
        .inventory_item = ITEM_GENERATOR_PART_MAP,
        .item_complete = {ITEM_GENERATOR_PART_0, ITEM_GENERATOR_PART_1, ITEM_GENERATOR_PART_2},
        .name = "Power parts map",
        .data = {
            .map = {
                .image_filename = "rom:/images/maps/generator_parts_map.sprite",
            },
        },
    },
    {
        .type = MENU_ITEM_MAP,
        .icon = MENU_ICON_MAP,
        .inventory_item = ITEM_HEALTH_JUICE_MAP,
        .item_complete= {ITEM_HEALTH_JUICE},
        .name = "Nanite map",
        .data = {
            .map = {
                .image_filename = "rom:/images/maps/health_juice_map.sprite",
            },
        },
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_NOTE,
        .inventory_item = ITEM_TABLET_NOTE,
        .item_complete = {ITEM_TABLET_BATTERY, ITEM_TABLET_MEMORY, ITEM_TABLET_SCREEN},
        .name = "Tablet parts",
        .data = {
            .part = {
                .description = "Scrapbot found where parts could be located to repair the tablet. Check the images to see the locations.",
            },
        },
    },
    {
        .type = MENU_ITEM_MAP,
        .icon = MENU_ICON_IMAGE,
        .inventory_item = ITEM_TABLET_BATTERY_MAP,
        .item_complete = {ITEM_TABLET_BATTERY},
        .name = "Battery location",
        .data = {
            .map = {
                .image_filename = "rom:/images/maps/battery_location.sprite",
            },
        },
    },
    {
        .type = MENU_ITEM_MAP,
        .icon = MENU_ICON_IMAGE,
        .inventory_item = ITEM_TABLET_MEMORY_MAP,
        .item_complete = {ITEM_TABLET_MEMORY},
        .name = "RAM location",
        .data = {
            .map = {
                .image_filename = "rom:/images/maps/memory_location.sprite",
            },
        },
    },
    {
        .type = MENU_ITEM_MAP,
        .icon = MENU_ICON_IMAGE,
        .inventory_item = ITEM_TABLET_SCREEN_MAP,
        .item_complete = {ITEM_TABLET_SCREEN},
        .name = "Screen location",
        .data = {
            .map = {
                .image_filename = "rom:/images/maps/screen_location.sprite",
            },
        },
    },
    
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_MOTOR_PART,
        .inventory_item = ITEM_WELL_HAS_MOTOR,
        .item_complete = {ITEM_WELL_HAS_FIXED_MOTOR},
        .name = "Bike motor",
        .data = {
            .part = {
                .description = "You were lucky to find this replacement for your hover bike",
            },
        },
    },
    
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_WELL_PART,
        .inventory_item = ITEM_WELL_HAS_PUMP_GEAR,
        .item_complete = {ITEM_WELL_HAS_FIXED_PUMP},
        .name = "Well parts",
        .data = {
            .part = {
                .description = "Replacement parts needed to fix the well damaged in the raid",
            },
        },
    },
    
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_GEN_FAN_PART,
        .inventory_item = ITEM_GENERATOR_PART_0,
        .item_complete = {ITEM_GENERATOR_HAS_FIXED},
        .name = "Fan",
        .data = {
            .part = {
                .description = "A fan needed to fix the generator",
            },
        },
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_GEN_STARTER_PART,
        .inventory_item = ITEM_GENERATOR_PART_1,
        .item_complete = {ITEM_GENERATOR_HAS_FIXED},
        .name = "Starter",
        .data = {
            .part = {
                .description = "A starter needed to fix the generator",
            },
        },
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_GEN_BULB_PART,
        .inventory_item = ITEM_GENERATOR_PART_2,
        .item_complete = {ITEM_GENERATOR_HAS_FIXED},
        .name = "Bulb",
        .data = {
            .part = {
                .description = "A bulb for fixing the generator",
            },
        },
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_NANO_VIAL_PART,
        .inventory_item = ITEM_HEALTH_JUICE,
        .item_complete = {ITEM_HEALTH_MACHINE_FIXED},
        .name = "Nanites",
        .data = {
            .part = {
                .description = "Medical nano bots used to treat wounds. They quickly break down oustide of controlled environments.",
            },
        },
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_TABLET_BATTERY,
        .inventory_item = ITEM_TABLET_BATTERY,
        .item_complete = {ITEM_TABLET_REPAIRED},
        .name = "Nuclear battery",
        .data = {
            .part = {
                .description = "This battery still works years after the asteroid impact.",
            },
        },
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_TABLET_MEMORY,
        .inventory_item = ITEM_TABLET_MEMORY,
        .item_complete = {ITEM_TABLET_REPAIRED},
        .name = "DDR100 RAM",
        .data = {
            .part = {
                .description = "This memory module is hard to come by. They are mostly found in AI bots.",
            },
        },
    },
    {
        .type = MENU_ITEM_PART,
        .icon = MENU_ICON_TABLET_SCREEN,
        .inventory_item = ITEM_TABLET_SCREEN,
        .item_complete = {ITEM_TABLET_REPAIRED},
        .name = "Video screen",
        .data = {
            .part = {
                .description = "This is the same model as the broken tablet",
            },
        },
    },
    {
        .type = MENU_ITEM_MAP,
        .icon = MENU_ICON_TABLET,
        .inventory_item = ITEM_TABLET_REPAIRED,
        .item_complete = {ITEM_GIVEN_TABLET},
        .name = "Tablet",
        .data = {
            .map = {
                .image_filename = "rom:/images/menu/tablet_image.sprite",
            },
        },
    },
};

#define MENU_ITEM_COUNT      (sizeof(menu_items) / sizeof(*menu_items))

enum map_menu_state {
    MAP_MENU_LIST,
    MAP_MENU_NEW_ITEMS,
    MAP_MENU_NEW_ITEM_DETAILS,
    MAP_MENU_DETAILS_ANIMATE,
    MAP_MENU_DETAILS,
    MAP_MENU_MAP_REVEAL,
};

union map_menu_state_data {
    struct {
        float timer;
    } new_items;
    struct {
        float timer;
        sprite_t* mask;
    } map_reveal;
};

struct map_menu {
    enum map_menu_state state;
    union map_menu_state_data state_data;
    enum inventory_item_type selected_item;
    vector2s16_t last_position;
    bool can_unpause;

    bool has_prev[MENU_ITEM_COUNT];

    sprite_t* details_image;
};

#define MAP_TILE_SIZE           32
#define MAP_SIZE                128

#define MAP_X                   30
#define MAP_Y                   46

#define MENU_X                  (SCREEN_WD - MAP_X - MAP_SIZE)

#define BRUSH_HALF_SIZE         11
#define BLUR_RADIUS             5

#define ICON_SIZE               32

static uint8_t __attribute__((aligned(16))) map_revealed[MAP_SIZE * MAP_SIZE];
static uint8_t reveal_brush[BRUSH_HALF_SIZE * BRUSH_HALF_SIZE];
static struct map_asssets assets;
static struct map_menu map_menu;

static const char* icon_files[MENU_ICON_TYPE_COUNT] = {
    [MENU_ICON_MOTOR_PART] = "rom:/images/parts/motor.sprite",
    [MENU_ICON_WELL_PART] = "rom:/images/parts/water_pump_gear.sprite",
    [MENU_ICON_GEN_FAN_PART] = "rom:/images/parts/gen_fan.sprite",
    [MENU_ICON_GEN_STARTER_PART] = "rom:/images/parts/gen_starter.sprite",
    [MENU_ICON_GEN_BULB_PART] = "rom:/images/parts/gen_bulb.sprite",
    [MENU_ICON_NANO_VIAL_PART] = "rom:/images/parts/nano_vial.sprite",
    [MENU_ICON_TABLET_BATTERY] = "rom:/images/parts/tablet_battery.sprite",
    [MENU_ICON_TABLET_MEMORY] = "rom:/images/parts/tablet_memory.sprite",
    [MENU_ICON_TABLET_SCREEN] = "rom:/images/parts/tablet_screen.sprite",
    [MENU_ICON_TABLET] = "rom:/images/parts/tablet.sprite",
    [MENU_ICON_MAP] = "rom:/images/maps/dot_matrix_map_icon.sprite",
    [MENU_ICON_NOTE] = "rom:/images/maps/note.sprite",
    [MENU_ICON_IMAGE] = "rom:/images/maps/image_icon.sprite",
    [MENU_ICON_NUT] = "rom:/images/maps/nut.sprite",
    [MENU_ICON_BIKE_TELEPORT] = "rom:/images/parts/bike_teleport.sprite",
    [MENU_ICON_SERVO] = "rom:/images/parts/servo_motor_icon.sprite",
};

static vector2_t player_cursor_points[3] = {
    {0.0f, 5.0f},
    {3.0f, -5.0f},
    {-3.0f, -5.0f},
};

#define TAN_HORZ 0.93361004861225457857f
#define VIEW_DEPTH  14.0f

static vector2_t camera_cursor_points[3] = {
    {0.0f, 0.0f},
    {-TAN_HORZ * VIEW_DEPTH, VIEW_DEPTH},
    {TAN_HORZ * VIEW_DEPTH, VIEW_DEPTH},
};

vector2_t default_min = {-3072.0f, 3072.0f};
vector2_t default_max = {3072.0f, -3072.0f};

void map_get_position(vector3_t* world_pos, vector2_t* map_pos) {
    vector3_t final_pos;
    vector2_t rotation;
    vector2ComplexFromAngle(current_scene->minimap_rotation, &rotation);

    vector3RotateWith2(world_pos, &rotation, &final_pos);
    final_pos.x += current_scene->minimap_location.x;
    final_pos.z += current_scene->minimap_location.y;

    vector2_t* min = current_scene->overworld ? &current_scene->minimap_min : &default_min;
    vector2_t* max = current_scene->overworld ? &current_scene->minimap_max : &default_max;

    float width = max->x - min->x;
    float height = max->y - min->y;

    map_pos->x = fabsf(width) < 0.001f ? 0.0f : (MAP_SIZE * (final_pos.x - min->x) / width);
    map_pos->y = fabsf(height) < 0.001f ? 0.0f : (MAP_SIZE * (1.0f - (final_pos.z - min->y) / height));

}

void map_render_minimap(int map_x, int map_y) {
    if (!current_scene) {
        return;
    }

    material_apply(assets.map_background);

    rdpq_texture_rectangle(
        TILE0,
        map_x, map_y,
        map_x + MAP_SIZE,
        map_y + MAP_SIZE,
        13, 13
    );

    surface_t surf = sprite_get_pixels(assets.map);

    surf.width = MAP_TILE_SIZE;
    surf.height = MAP_TILE_SIZE;

    rdpq_texparms_t tex_params = {
        .palette = 0,
        .tmem_addr = 0,
    };
    
    surface_t mask_surface = {
        .buffer = map_revealed,
        .flags = 0x11,
        .stride = 0x80,
        .width = MAP_TILE_SIZE,
        .height = MAP_TILE_SIZE,
    };
    rdpq_texparms_t mask_parms = {
        .palette = 0,
        .tmem_addr = 2048,
    };

    material_apply(assets.material);

    for (int y = 0; y < MAP_SIZE; y += MAP_TILE_SIZE) {
        for (int x = 0; x < MAP_SIZE; x += MAP_TILE_SIZE) {
            int pixel_index = x + y * MAP_SIZE;

            surf.buffer = (void*)((uint16_t*)assets.map->data + pixel_index);
            mask_surface.buffer = (void*)(map_revealed + pixel_index);

            rdpq_tex_upload(TILE1, &mask_surface, &mask_parms);
            rdpq_tex_upload(TILE0, &surf, &tex_params);

            rdpq_texture_rectangle(
                TILE0, 
                x + map_x, y + map_y, 
                x + map_x + MAP_TILE_SIZE, y + map_y + MAP_TILE_SIZE, 
                0, 0
            );
        }
    }
    
    
    material_apply(assets.map_view);

    vector2_t screen_pos;
    map_get_position(player_get_position(&current_scene->player), &screen_pos);

    
    vector2_t scene_rotation;
    if (current_scene->minimap_rotation != 0.0f) {
        vector2ComplexFromAngle(current_scene->minimap_rotation, &scene_rotation);
    } else {
        scene_rotation = (vector2_t){1.0f, 0.0f};
    }

    vector3_t forward;
    quatMultVector(&current_scene->camera.transform.rotation, &gForward, &forward);
    vector2_t cam_rot;
    vector2LookDir(&cam_rot, &forward);
    vector2Negate(&cam_rot, &cam_rot);

    vector2ComplexMul(&cam_rot, &scene_rotation, &cam_rot);
    
    struct view_vertex cursor_points[3];
    for (int i = 0; i < 3; i += 1) {
        menu_transform_point(&camera_cursor_points[i], &cam_rot, &screen_pos, &cursor_points[i].pos);
        cursor_points[i].pos.x += map_x;
        cursor_points[i].pos.y += map_y;
        cursor_points[i].col = (vector4_t){
            1.0f, 1.0f, 0.0f,
            i == 0 ? 0.5f : 0.0f
        };
    }
    rdpq_triangle(
        &TRIFMT_SHADE, 
        (float*)&cursor_points[0], 
        (float*)&cursor_points[1], 
        (float*)&cursor_points[2]
    );
    
    material_apply(assets.map_arrow);

    vector2_t* rot = player_get_rotation(&current_scene->player);
    vector2_t player_rot;
    vector2ComplexMul(rot, &scene_rotation, &player_rot);
    for (int i = 0; i < 3; i += 1) {
        menu_transform_point(&player_cursor_points[i], &player_rot, &screen_pos, &cursor_points[i].pos);
        cursor_points[i].pos.x += map_x;
        cursor_points[i].pos.y += map_y;
    }
    rdpq_triangle(
        &TRIFMT_FILL, 
        (float*)&cursor_points[0], 
        (float*)&cursor_points[1], 
        (float*)&cursor_points[2]
    );
}

void map_render_title(struct menu_item* item) {
    if (!item) {
        return;
    }

    rdpq_text_printn(&(rdpq_textparms_t){
            // .line_spacing = -3,
            .align = ALIGN_RIGHT,
            .valign = VALIGN_BOTTOM,
            .width = 128,
            .height = 0,
            .wrap = WRAP_NONE,
        }, 
        FONT_DIALOG, 
        MENU_X, MAP_Y - 4, 
        item->name,
        strlen(item->name)
    );
}

void map_render_details(struct menu_item* item) {
    switch (item->type) {
        case MENU_ITEM_PART:
            material_apply(assets.map_icon);

            rdpq_sprite_blit(assets.icons[item->icon], MENU_X, MAP_Y, NULL);

            rdpq_text_printn(&(rdpq_textparms_t){
                    // .line_spacing = -3,
                    .align = ALIGN_LEFT,
                    .valign = VALIGN_TOP,
                    .width = 128,
                    .height = 128,
                    .wrap = WRAP_WORD,
                }, 
                FONT_DIALOG, 
                MENU_X, MAP_Y + ICON_SIZE, 
                item->data.part.description,
                strlen(item->data.part.description)
            );
            break;
        case MENU_ITEM_MAP:
            if (map_menu.details_image) {
                material_apply(assets.map_icon);

                int x = (MAP_SIZE - map_menu.details_image->width) >> 1;
                int y = (MAP_SIZE - map_menu.details_image->height) >> 1;
                rdpq_sprite_blit(map_menu.details_image, MENU_X + x, MAP_Y + y, NULL);
            }
            break;
        default:
            break;
    }
}

bool map_should_show_item(struct menu_item* item) {
    return item->always_visible || inventory_has_item(item->inventory_item);
}

#define FADE_IN_RATIO   0.3f

static inline void map_next_tile_location(int* x, int* y) {
    *x += ICON_SIZE;

    if (*x >= MAP_SIZE) {
        *x = 0;
        *y += ICON_SIZE;
    }
}

bool map_is_item_complete(struct menu_item* item) {
    bool has_check = false;

    for (int completion_index = 0; completion_index < 3; completion_index += 1) {
        if (!item->item_complete[completion_index]) {
            break;
        }

        has_check = inventory_has_item(item->item_complete[completion_index]);

        if (!has_check) {
            return false;
        }
    }

    return has_check;
}

enum inventory_item_type map_render_get_item_at_pass(int* xPtr, int* yPtr, int xAt, int yAt, bool finished) {
    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        struct menu_item* item = &menu_items[i];

        if (!map_should_show_item(&menu_items[i]) || map_is_item_complete(item) != finished) {
            continue;
        }
        
        if (*xPtr == xAt && *yPtr == yAt) {
            return item->inventory_item;
        }

        map_next_tile_location(xPtr, yPtr);
    }

    return ITEM_TYPE_NONE;
}

enum inventory_item_type map_render_get_item_at(int xAt, int yAt) {
    int x = 0;
    int y = 0;

    enum inventory_item_type active = map_render_get_item_at_pass(&x, &y, xAt, yAt, false);

    if (active != ITEM_TYPE_NONE) {
        return active;
    }

    return map_render_get_item_at_pass(&x, &y, xAt, yAt, true);
}

bool map_render_get_position_pass(enum menu_item_type type, int* xPtr, int* yPtr, int* xOut, int* yOut, bool finished) {
    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        struct menu_item* item = &menu_items[i];

        if (!map_should_show_item(&menu_items[i]) || map_is_item_complete(item) != finished) {
            continue;
        }
        
        if (item->inventory_item == map_menu.selected_item) {
            *xOut = *xPtr;
            *yOut = *yPtr;
            return true;
        }
        
        map_next_tile_location(xPtr, yPtr);
    }

    return false;
}

bool map_render_get_position(enum menu_item_type type, int* xOut, int* yOut) {
    int x = 0;
    int y = 0;
    return map_render_get_position_pass(type, &x, &y, xOut, yOut, false) || map_render_get_position_pass(type, &x, &y, xOut, yOut, true);
}

void map_items_render_pass(int* xPtr, int* yPtr, bool finished, float lerp_amount) {
    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        int x = *xPtr;
        int y = *yPtr;
        struct menu_item* item = &menu_items[i];

        bool is_complete = map_is_item_complete(item);
        
        if (!map_should_show_item(&menu_items[i]) || is_complete != finished) {
            continue;
        }

        if (item->inventory_item == map_menu.selected_item) {
            material_apply(assets.selection_cursor);
            rdpq_texture_rectangle(TILE0, x + MENU_X, y + MAP_Y, x + MENU_X + ICON_SIZE, y + MAP_Y + ICON_SIZE, 0, 0);
        }

        material_apply(assets.map_icon);

        color_t prim_color;
        if (lerp_amount >= 1.0f || map_menu.has_prev[i]) {
            if (is_complete) {
                prim_color = (color_t){64, 64, 64, 196};
            } else {
                prim_color = (color_t){0, 0, 0, 255};
            }
        } else if (lerp_amount < FADE_IN_RATIO) {
            prim_color = coloru8_lerp(&(color_t){
                255, 255, 255, 0,
            }, &(color_t){
                255, 255, 255, 255,
            }, lerp_amount * (1.0f / FADE_IN_RATIO));
        } else {
            prim_color = coloru8_lerp(&(color_t){
                255, 255, 255, 255,
            }, &(color_t){
                0, 0, 0, 255,
            }, (lerp_amount - FADE_IN_RATIO) * (1.0f / (1.0f - FADE_IN_RATIO)));
        }

        rdpq_set_prim_color(prim_color);

        rdpq_sprite_blit(assets.icons[item->icon], x + MENU_X, y + MAP_Y, NULL);

        if (item->show_count) {
            char count[8];
            int len = sprintf(count, "%d", inventory_get_count(item->inventory_item));
            
            rdpq_text_printn(&(rdpq_textparms_t){
                    // .line_spacing = -3,
                    .align = ALIGN_RIGHT,
                    .valign = VALIGN_BOTTOM,
                    .width = 32,
                    .height = 32,
                    .wrap = WRAP_NONE,
                }, 
                FONT_DIALOG, 
                x + MENU_X, y + MAP_Y, 
                count,
                len
            );
        }

        map_next_tile_location(xPtr, yPtr);
    }
}

void map_render_check_icons() {
    int x = 0;
    int y = 0;
    
    material_apply(assets.check_icon);

    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        struct menu_item* item = &menu_items[i];
        
        if (!map_should_show_item(&menu_items[i])) {
            continue;
        }

        if (!map_is_item_complete(item)) {
            map_next_tile_location(&x, &y);
        }
    }
    
    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        struct menu_item* item = &menu_items[i];
        
        if (!map_should_show_item(&menu_items[i])) {
            continue;
        }

        if (!map_is_item_complete(item)) {
            continue;
        }

        rdpq_texture_rectangle(
            TILE0, 
            x + MENU_X + 16, 
            y + MAP_Y + 16, 
            x + MENU_X + ICON_SIZE, 
            y + MAP_Y + ICON_SIZE, 
            0, 0
        );
        
        map_next_tile_location(&x, &y);
    }
}

void map_render_items(float lerp_amount) {
    int x = 0;
    int y = 0;
    
    map_items_render_pass(&x, &y, false, lerp_amount);
    map_items_render_pass(&x, &y, true, 1.0f);
    map_render_check_icons();
}

enum inventory_item_type map_get_default_selection() {
    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        if (!map_should_show_item(&menu_items[i])) {
            continue;
        }
        return menu_items[i].inventory_item;
    }

    return ITEM_TYPE_NONE;
}

struct menu_item* map_find_selected_item() {
    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        if (!map_should_show_item(&menu_items[i])) {
            continue;
        }
        if (menu_items[i].inventory_item == map_menu.selected_item) {
            return &menu_items[i];
        }
    }

    return NULL;
}

void map_render(void* data) {
    menu_common_render_background(26, 26, 268, 188);

    rdpq_text_printn(&(rdpq_textparms_t){
            // .line_spacing = -3,
            .align = ALIGN_RIGHT,
            .valign = VALIGN_BOTTOM,
            .width = 128,
            .height = 0,
            .wrap = WRAP_NONE,
        }, 
        FONT_DIALOG, 
        MAP_X, MAP_Y - 4, 
        "Map",
        3
    );

    map_render_minimap(MAP_X, MAP_Y);

    struct menu_item* selected_item = map_find_selected_item();
    map_render_title(selected_item);

    switch (map_menu.state) {
        case MAP_MENU_LIST:
            map_render_items(1.0f);
            break;
        case MAP_MENU_NEW_ITEMS:
        case MAP_MENU_NEW_ITEM_DETAILS:
        case MAP_MENU_MAP_REVEAL:
            map_render_items(map_menu.state_data.new_items.timer * (1.0f / NEW_ITEM_ANIM_TIME));
            break;
        case MAP_MENU_DETAILS_ANIMATE:
        case MAP_MENU_DETAILS:
            map_render_details(selected_item);
            break;
    }
}

void map_menu_init() {
    uint8_t* pixel = reveal_brush;

    for (int y = 0; y < BRUSH_HALF_SIZE; y += 1) {
        for (int x = 0; x < BRUSH_HALF_SIZE; x += 1) {
            float distance = sqrtf(x * x + y * y);
            if (distance < BRUSH_HALF_SIZE - BLUR_RADIUS) {
                *pixel = 255;
            } else if (distance > BRUSH_HALF_SIZE) {
                *pixel = 0;
            } else {
                *pixel = 255 - (uint8_t)(255.0f * (distance - (BRUSH_HALF_SIZE - BLUR_RADIUS)) * (1.0f / BLUR_RADIUS));
            }

            pixel += 1;
        }
    }

    map_menu.selected_item = 0;
}

void map_menu_update_has_prev() {
    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        map_menu.has_prev[i] = map_should_show_item(&menu_items[i]);
    }
}

void map_menu_destroy() {

}

void map_menu_show_details() {
    struct menu_item* selected = map_find_selected_item();

    if (!selected) {
        return;
    }

    if (map_menu.state == MAP_MENU_NEW_ITEMS) {
        map_menu.state = MAP_MENU_NEW_ITEM_DETAILS;
    } else {
        map_menu.state = MAP_MENU_DETAILS;
        if (selected->type == MENU_ITEM_MAP) {
            map_menu.details_image = sprite_load(selected->data.map.image_filename);
        }
    }
}

void map_menu_hide_details() {
    if (map_menu.details_image) {
        sprite_free(map_menu.details_image);
        map_menu.details_image = NULL;
    }

    map_menu.state = MAP_MENU_LIST;
}

void map_menu_animate_new_items(bool show_details) {
    map_menu.state = show_details ? MAP_MENU_NEW_ITEM_DETAILS : MAP_MENU_NEW_ITEMS;
    map_menu.state_data = (union map_menu_state_data){
        .new_items = {
            .timer = 0.0f,
        },
    };
}

void map_check_direction() {
    static int prev_x;
    static int prev_y;

    joypad_inputs_t inputs = joypad_get_inputs(0);

    int dx = 0;
    int dy = 0;

    if (inputs.stick_x > 40 && prev_x <= 40) {
        dx = ICON_SIZE;
    } else if (inputs.stick_x < -40 && prev_x >= -40) {
        dx = -ICON_SIZE;
    }

    if (inputs.stick_y > 40 && prev_y <= 40) {
        dy = -ICON_SIZE;
    } else if (inputs.stick_y < -40 && prev_y >= -40) {
        dy = ICON_SIZE;
    }

    prev_x = inputs.stick_x;
    prev_y = inputs.stick_y;

    if (dx == 0 && dy == 0) {
        return;
    }

    int x, y;
    map_render_get_position(map_menu.selected_item, &x, &y);

    enum inventory_item_type new_selection = map_render_get_item_at(x + dx, y + dy);

    if (new_selection == ITEM_TYPE_NONE) {
        return;
    }

    audio_play_2d(assets.sounds[MENU_SOUND_CURSOR], 1.0f, 0.0f, 1.0f, 1);
    map_menu.selected_item = new_selection;
}

void map_menu_reveal_update() {
    map_menu.state_data.map_reveal.timer += fixed_time_step;

    int scalar = (int)(map_menu.state_data.map_reveal.timer * (256.0f / MAP_REVEAL_TIME));

    if (scalar > 256) {
        scalar = 256;
    }

    uint8_t* mask = (uint8_t*)map_menu.state_data.map_reveal.mask->data;
    uint8_t* end = map_revealed + MAP_SIZE * MAP_SIZE;

    for (uint8_t* revealed = map_revealed; revealed < end; ++revealed, ++mask) {
        uint8_t mask_value = (int8_t)((scalar * *mask) >> 8);

        if (mask_value > *revealed) {
            *revealed = mask_value;
        }
    }

    if (map_menu.state_data.map_reveal.timer >= MAP_REVEAL_TIME) {
        map_menu.state = MAP_MENU_LIST;
        sprite_free(map_menu.state_data.map_reveal.mask);
    }
}

void map_menu_update(void* data) {
    joypad_buttons_t pressed = joypad_get_buttons_pressed(0);
    joypad_inputs_t input = joypad_get_inputs(0);
    
    if (!input.btn.start) {
        map_menu.can_unpause = true;
    }

    if (pressed.c_right) {
        if (savefile_save()) {
            audio_play_2d(assets.sounds[MENU_SOUND_SAVE], 1.0f, 0.0f, 1.0f, 1);
        }
    }

    switch (map_menu.state) {
        case MAP_MENU_LIST:
            if ((pressed.start && map_menu.can_unpause) || pressed.b) {
                map_menu_hide();
                audio_play_2d(assets.unpause_sound, 1.0f, 0.0f, 1.0f, 1);
                return;
            }
            map_check_direction();
            if (pressed.a) {
                audio_play_2d(assets.sounds[MENU_SOUND_SELECT], 1.0f, 0.0f, 1.0f, 1);
                map_menu_show_details();
            }
            break;
        case MAP_MENU_NEW_ITEMS:
            map_menu.state_data.new_items.timer += fixed_time_step;
            if (map_menu.state_data.new_items.timer >= NEW_ITEM_ANIM_TIME) {
                map_menu.state = MAP_MENU_LIST;
            }
            break;
        case MAP_MENU_NEW_ITEM_DETAILS:
            map_menu.state_data.new_items.timer += fixed_time_step;
            if (map_menu.state_data.new_items.timer >= NEW_ITEM_ANIM_TIME + OPEN_ITEM_DELAY_TIME) {
                map_menu.state = MAP_MENU_LIST;
                map_menu_show_details();
            }
            break;
        case MAP_MENU_DETAILS_ANIMATE:
            map_menu.state = MAP_MENU_DETAILS;
            break;
        case MAP_MENU_DETAILS:
            if (pressed.start && map_menu.can_unpause) {
                map_menu_hide();
                audio_play_2d(assets.unpause_sound, 1.0f, 0.0f, 1.0f, 1);
                return;
            }
            if (pressed.b) {
                audio_play_2d(assets.sounds[MENU_SOUND_BACK], 1.0f, 0.0f, 1.0f, 1);
                map_menu_hide_details();
            }
            break;
        case MAP_MENU_MAP_REVEAL:
            map_menu_reveal_update();
            break;
    }
}

const char* map_menu_check_for_map_item(enum inventory_item_type item) {
    switch (item) {
        case ITEM_MAP_RIVER:
            return "rom:/images/maps/river_map.sprite";
        case ITEM_MAP_DUNES:
            return "rom:/images/maps/dunes_map.sprite";
        case ITEM_MAP_CANYON:
            return "rom:/images/maps/canyons_map.sprite";
        default:
            return NULL;
    }
}

static const char* menu_sound_files[MENU_SOUND_COUNT] = {
    [MENU_SOUND_BACK] = "rom:/sounds/menu/back.wav64",
    [MENU_SOUND_CURSOR] = "rom:/sounds/menu/cursor.wav64",
    [MENU_SOUND_PAUSE] = "rom:/sounds/menu/pause.wav64",
    [MENU_SOUND_SELECT] = "rom:/sounds/menu/select.wav64",
    [MENU_SOUND_SAVE] = "rom:/sounds/race/checkpoint.wav64",
};

void map_menu_show_with_item(enum inventory_item_type item) {
    if (current_scene) {
        current_scene->can_pause = false;
    }

    assets.map = sprite_load("rom:/images/menu/map.sprite");
    assets.material = material_cache_load("rom:/materials/menu/map.mat");
    assets.map_background = material_cache_load("rom:/materials/menu/map_grid.mat");
    assets.map_arrow = material_cache_load("rom:/materials/menu/map_arrow.mat");
    assets.map_view = material_cache_load("rom:/materials/menu/map_view.mat");
    assets.map_icon = material_cache_load("rom:/materials/menu/map_icon.mat");
    assets.selection_cursor = material_cache_load("rom:/materials/menu/selection_cursor.mat");
    assets.check_icon = material_cache_load("rom:/materials/menu/check.mat");

    for (int i = 0; i < MENU_SOUND_COUNT; i += 1) {
        assets.sounds[i] = wav64_load(menu_sound_files[i], NULL);
    }
    if (!assets.unpause_sound) {
        assets.unpause_sound = wav64_load("rom:/sounds/menu/unpause.wav64", NULL);
    }
    
    map_menu.details_image = NULL;

    bool has_new = false;
    bool should_show_details = false;

    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        bool should_show = map_should_show_item(&menu_items[i]);

        if (should_show && !map_menu.has_prev[i]) {
            if (menu_items[i].inventory_item == item && menu_items[i].type == MENU_ITEM_MAP) {
                should_show_details = true;
            }            

            has_new = true;
        }
    }

    if (item != ITEM_TYPE_NONE) {
        map_menu.state = MAP_MENU_LIST;
    } else if (map_menu.state == MAP_MENU_DETAILS) {
        struct menu_item* selected = map_find_selected_item();

        if (selected && selected->type == MENU_ITEM_MAP) {
            map_menu.details_image = sprite_load(selected->data.map.image_filename);
        }
    }

    if (has_new) {
        map_menu_animate_new_items(should_show_details);
    }

    for (int i = 0; i < MENU_ICON_TYPE_COUNT; i += 1) {
        assets.icons[i] = sprite_load(icon_files[i]);
    }

    update_pause_layers(UPDATE_LAYER_WORLD | UPDATE_LAYER_CUTSCENE);
    update_unpause_layers(UPDATE_LAYER_PAUSE_MENU);
    game_mode_enter_menu();
    menu_add_callback(map_render, &map_menu, MENU_PRIORITY_OVERLAY);
    update_add(&map_menu, map_menu_update, UPDATE_PRIORITY_PLAYER, UPDATE_LAYER_PAUSE_MENU);
    font_type_use(FONT_DIALOG);
    map_menu.can_unpause = false;

    const char* map_mask = map_menu_check_for_map_item(item);

    if (map_mask) {
        map_menu.state = MAP_MENU_MAP_REVEAL;
        map_menu.state_data.map_reveal.timer = 0.0f;
        map_menu.state_data.map_reveal.mask = sprite_load(map_mask);
    } else if (item != ITEM_TYPE_NONE) {
        map_menu.selected_item = item;
    } else if (map_menu.selected_item == ITEM_TYPE_NONE || !map_find_selected_item()) {
        map_menu.selected_item = map_get_default_selection();
    }
    
    audio_pause_all();
    audio_play_2d(assets.sounds[MENU_SOUND_PAUSE], 1.0f, 0.0f, 1.0f, 1);
}

void map_menu_show() {
    map_menu_show_with_item(ITEM_TYPE_NONE);
}

void map_menu_hide() {
    sprite_free(assets.map);
    material_cache_release(assets.material);
    material_cache_release(assets.map_background);
    material_cache_release(assets.map_arrow);
    material_cache_release(assets.map_view);
    material_cache_release(assets.map_icon);
    material_cache_release(assets.selection_cursor);
    material_cache_release(assets.check_icon);
    for (int i = 0; i < MENU_SOUND_COUNT; i += 1) {
        wav64_close(assets.sounds[i]);
    }
    assets.map = NULL;
    assets.material = NULL;
    assets.map_background = NULL;
    assets.map_arrow = NULL;
    assets.map_view = NULL;
    assets.map_icon = NULL;
    assets.selection_cursor = NULL;

    if (map_menu.details_image) {
        sprite_free(map_menu.details_image);
        map_menu.details_image = NULL;
    }

    for (int i = 0; i < MENU_ICON_TYPE_COUNT; i += 1) {
        sprite_free(assets.icons[i]);
        assets.icons[i] = NULL;
    }
    
    for (int i = 0; i < MENU_ITEM_COUNT; i += 1) {
        map_menu.has_prev[i] = map_should_show_item(&menu_items[i]);
    }

    update_unpause_layers(UPDATE_LAYER_WORLD | UPDATE_LAYER_CUTSCENE);
    update_pause_layers(UPDATE_LAYER_PAUSE_MENU);
    game_mode_exit_menu();
    menu_remove_callback(&map_menu);
    update_remove(&map_menu);
    font_type_release(FONT_DIALOG);

    audio_unpause_all();
}

void map_mark_revealed(struct Vector3* pos) {
    if (!current_scene || (current_scene->overworld == NULL && current_scene->minimap_location.x == 0.0f && current_scene->minimap_location.y == 0.0f)) {
        return;
    }

    vector2_t screen_pos;
    map_get_position(pos, &screen_pos);

    int center_x = (int)screen_pos.x;
    int center_y = (int)screen_pos.y;

    if (center_x == map_menu.last_position.x && center_y == map_menu.last_position.y) {
        return;
    }

    map_menu.last_position.x = center_x;
    map_menu.last_position.y = center_y;

    for (int dy = -BRUSH_HALF_SIZE + 1; dy < BRUSH_HALF_SIZE; dy += 1) {
        int y = center_y + dy;

        if (y < 0 || y >= MAP_SIZE) {
            continue;
        }

        uint8_t* target_row = &map_revealed[y * MAP_SIZE];
        uint8_t* src_row = &reveal_brush[abs(dy) * BRUSH_HALF_SIZE];

        for (int dx = -BRUSH_HALF_SIZE + 1; dx < BRUSH_HALF_SIZE; dx += 1) {
            int x = center_x + dx;

            if (x < 0 || x >= MAP_SIZE) {
                continue;
            }

            uint8_t* pixel = &target_row[x];

            *pixel = MAX(*pixel, src_row[abs(dx)]);
        }

        uint8_t* chunk_a = (uint8_t*)ALIGN_16((int)target_row + center_x - BRUSH_HALF_SIZE + 1);
        uint8_t* chunk_b = (uint8_t*)ALIGN_16((int)target_row + center_x + BRUSH_HALF_SIZE - 1);

        data_cache_hit_writeback(chunk_a, 16);
        if (chunk_a != chunk_b) {
            data_cache_hit_writeback(chunk_b, 16);
        }
    }
}

uint8_t* map_get_revealed() {
    return map_revealed;
}

bool map_menu_has_revealed(vector3_t* pos) {
    if (!current_scene) {
        return false;
    }

    vector2_t screen_pos;
    map_get_position(pos, &screen_pos);

    int x = (int)screen_pos.x;
    int y = (int)screen_pos.y;

    if (x < 0 || y < 0 || x >= MAP_SIZE || y >= MAP_SIZE) {
        return false;
    }

    return map_revealed[x + y * MAP_SIZE] != 0;
}