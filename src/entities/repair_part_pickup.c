#include "repair_part_pickup.h"

#include "../render/render_scene.h"
#include "../collision/shapes/box.h"
#include "../collision/collision_scene.h"
#include "../cutscene/expression_evaluate.h"
#include "../entity/entity_spawner.h"
#include "../scene/scene_definition.h"
#include "../time/time.h"
#include "../audio/audio.h"
#include "../scene/scene.h"
#include "../math/mathf.h"
#include "../player/inventory.h"
#include "../menu/hud.h"

struct repair_part_type_def {
    union {
        const char* mesh_name;
        const char* particle_material;
    };
    float particle_radius;
    uint8_t particle_frame_max_x;
    uint8_t particle_frame_step;
    dynamic_object_type_t collider;
};

struct repair_part_pickup_assets {
    wav64_t* beacon_beep;
};

typedef struct repair_part_type_def repair_part_type_def_t;

static entity_id tracking_item;
static float tacking_item_distnace;

static repair_part_type_def_t types[REPAIR_PART_COUNT] = {
    [REPAIR_PART_MOTOR] = {
        .mesh_name = "rom:/meshes/parts/motor.tmesh",
        .collider = {
            BOX_COLLIDER(0.2f, 0.2f, 0.3f),
            .center = {0.0f, 0.2f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        },
    },
    [REPAIR_PART_WATER_PUMP_GEAR] = {
        .mesh_name = "rom:/meshes/parts/water_pump_gear.tmesh",
        .collider = {
            BOX_COLLIDER(0.5f, 0.1f, 0.5f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_GEN_FAN] = {
        .mesh_name = "rom:/meshes/parts/gen_fan.tmesh",
        .collider = {
            BOX_COLLIDER(0.4f, 0.4f, 0.4f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_GEN_STARTER] = {
        .mesh_name = "rom:/meshes/parts/gen_starter.tmesh",
        .collider = {
            BOX_COLLIDER(0.3f, 0.3f, 0.3f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_GEN_BULB] = {
        .mesh_name = "rom:/meshes/parts/gen_bulb.tmesh",
        .collider = {
            BOX_COLLIDER(0.3f, 0.5f, 0.3f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_NOTE] = {
        .mesh_name = "rom:/meshes/parts/note.tmesh",
        .collider = {
            BOX_COLLIDER(0.2f, 0.2f, 0.2f),
            .center = {0.0f, 0.2f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_TABLET_BATTERY] = {
        .mesh_name = "rom:/meshes/parts/tablet_battery.tmesh",
        .collider = {
            BOX_COLLIDER(0.1f, 0.1f, 0.2f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_TABLET_MEMORY] = {
        .mesh_name = "rom:/meshes/parts/tablet_memory.tmesh",
        .collider = {
            BOX_COLLIDER(0.2f, 0.1f, 0.2f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_TABLET_SCREEN] = {
        .mesh_name = "rom:/meshes/parts/tablet_screen.tmesh",
        .collider = {
            BOX_COLLIDER(0.3f, 0.1f, 0.3f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    
    [REPAIR_PART_TABLET] = {
        .mesh_name = "rom:/meshes/parts/tablet.tmesh",
        .collider = {
            BOX_COLLIDER(0.3f, 0.1f, 0.3f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_MONEY] = {
        .particle_material = "rom:/materials/parts/nut_particle.mat",
        .particle_radius = 0.2f,
        .particle_frame_max_x = 255,
        .particle_frame_step = 32,
        .collider = {
            BOX_COLLIDER(0.1f, 0.2f, 0.1f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    [REPAIR_PART_BOAT_SWITCH] = {
        .mesh_name = "rom:/meshes/parts/boat_switch.tmesh",
        .collider = {
            BOX_COLLIDER(0.3f, 0.1f, 0.3f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
    
    [REPAIR_PART_TABLET_SCREEN] = {
        .mesh_name = "rom:/meshes/parts/tablet_screen.tmesh",
        .collider = {
            BOX_COLLIDER(0.3f, 0.1f, 0.3f),
            .center = {0.0f, 0.0f, 0.0f},
            .friction = 0.5,
            .max_stable_slope = 0.219131191f,
        }
    },
};

static struct repair_part_pickup_assets assets;

void repair_part_pickup_common_init() {
    assets.beacon_beep = wav64_load("rom:/sounds/parts/tracker_ping.wav64", NULL);
}

void repair_part_pickup_common_destroy() {
    wav64_close(assets.beacon_beep);
}

void repair_part_interact(struct interactable* interactable, entity_id from) {
    repair_part_pickup_t* part = (repair_part_pickup_t*)interactable->data;
    expression_set_bool(part->has_part, true);
    entity_despawn(interactable->id);

    if (part->count != VARIABLE_DISCONNECTED) {
        expression_set_integer(part->count, expression_get_integer(part->count) + 1);
    }
}

#define CLOSE_BEEP_INTERVAL     0.2f
#define FAR_BEEP_INTERVAL       1.5f
#define MAX_BEEP_DISTANCE       90.0f

#define CLOSE_FREQ              1.3f
#define FAR_FREQ                0.7f

void repair_part_pickup_update(void* data) {
    repair_part_pickup_t* part = (repair_part_pickup_t*)data;
    float distance = sqrtf(vector3DistSqrd(&part->transform.position, player_get_position(&current_scene->player)));

    if (!tracking_item || distance < tacking_item_distnace) {
        tracking_item = part->entity_id;
    }

    if (part->entity_id == tracking_item) {
        tacking_item_distnace = distance;
    } else {
        return;
    }
    
    if (distance > MAX_BEEP_DISTANCE) {
        part->beep_timer = FAR_BEEP_INTERVAL;
        return;
    }

    float lerp = distance * (1.0f / MAX_BEEP_DISTANCE);


    float beep_threshold = mathfLerp(CLOSE_BEEP_INTERVAL, FAR_BEEP_INTERVAL, lerp);

    if (part->beep_timer >= beep_threshold) {
        audio_play_2d(assets.beacon_beep, 1.0f, 0.0f, mathfLerp(CLOSE_FREQ, FAR_FREQ, lerp), 0);
        hud_flash_tracker();
        part->beep_timer = 0.0f;
    } else {
        part->beep_timer += scaled_time_step;
    }
}

void repair_part_pickup_init(repair_part_pickup_t* part, struct repair_part_pickup_definition* definition, entity_id entity_id) {
    transformSaInit(&part->transform, &definition->position, &definition->rotation, 1.0f);

    if (expression_get_bool(definition->has_part)) {
        part->is_active = false;
        return;
    }
    part->is_active = true;
    part->has_part = definition->has_part;
    part->has_tracker = definition->has_tracker;
    part->beep_timer = FAR_BEEP_INTERVAL;
    part->count = definition->count;
    part->entity_id = entity_id;

    if (inventory_has_item(ITEM_DETECT_NUTS) && definition->part_type == REPAIR_PART_MONEY) {
        part->has_tracker = true;
    }
    
    if (inventory_has_item(ITEM_DETECT_SERVO) && definition->part_type == REPAIR_PART_SERVO_MOTOR) {
        part->has_tracker = true;
    }

    repair_part_type_def_t* def = &types[definition->part_type];

    part->is_particle = def->particle_radius > 0.0f;

    if (part->is_particle) {
        renderable_init_point(&part->renderable, &part->transform.position, def->particle_radius, def->particle_material);
        part->renderable.point_render.frame_max_x = def->particle_frame_max_x;
        part->renderable.point_render.frame_step = def->particle_frame_step;
    } else {
        renderable_single_axis_init(&part->renderable, &part->transform, def->mesh_name);
    } 
    render_scene_add_renderable(&part->renderable, 1.0f);

    dynamic_object_init(entity_id, &part->collider, &def->collider, COLLISION_LAYER_INTERACT_ONLY, &part->transform.position, &part->transform.rotation);
    collision_scene_add(&part->collider);

    interactable_init(&part->interactable, entity_id, INTERACT_TYPE_TAKE, repair_part_interact, part);

    if (part->has_tracker) {
        update_add(part, repair_part_pickup_update, UPDATE_PRIORITY_EFFECTS, UPDATE_LAYER_WORLD);
    }
}

void repair_part_pickup_destroy(repair_part_pickup_t* part) {
    if (!part->is_active) {
        return;
    }
    render_scene_remove(&part->renderable);
    if (part->is_particle) {
        renderable_destroy_point(&part->renderable);
    } else {
        renderable_destroy(&part->renderable);
    }
    collision_scene_remove(&part->collider);
    interactable_destroy(&part->interactable);

    if (part->has_tracker) {
        update_remove(part);
    }

    if (part->entity_id == tracking_item) {
        tracking_item = 0;
    }
}