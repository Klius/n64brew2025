#include "motorcycle.h"

#include "../render/tmesh.h"
#include "../resource/tmesh_cache.h"
#include "../render/render_scene.h"
#include "../collision/shapes/box.h"
#include "../collision/collision_scene.h"
#include "../time/time.h"
#include "../math/mathf.h"
#include "../config.h"
#include "../scene/scene.h"
#include "../player/inventory.h"
#include "../resource/animation_cache.h"
#include "../math/mathf.h"

#define HOVER_SAG_AMOUNT        0.25f
#define HOVER_SPRING_STRENGTH   (-GRAVITY_CONSTANT / HOVER_SAG_AMOUNT)
#define STOPPED_SPEED_THESHOLD  8.0f

#define ACCEL_RATE              20.0f
#define BOOST_ACCEL_RATE        60.0f
#define BASE_DRIVE_SPEED        25.0f
#define UPGRADED_DRIVE_SPEED    35.0f
#define BASE_BOOST_SPEED        50.0f
#define UPGRADED_BOOST_SPEED    60.0f
#define MAX_TURN_RATE           2.0f

#define MAX_BOOST_TURN_ACCEL    50.0f
#define MAX_TURN_ACCEL          30.0f
#define DRIFT_ACCEL             20.0f

#define TURN_BOOST_SLOW_THESHOLD    (MAX_BOOST_TURN_ACCEL / MAX_TURN_RATE)
#define TURN_SLOW_THRESHOLD         (MAX_TURN_ACCEL / MAX_TURN_RATE)

#define STILL_HOVER_HEIGHT      0.25f
#define RIDE_HOVER_HEIGHT       0.5f
#define FAST_HOVER_HEIGHT       1.0f
#define BOB_HEIGHT              0.1f
#define BOB_TIME                4.0f

#define BASE_BOOST_TIME         2.5f
#define UPGRADED_BOOST_TIME     3.5f

#define SELF_BOOST_COOLDOWN     10.0f

#define CAST_CENTER             0.3f

struct motorcyle_assets {
    tmesh_t* mesh;
    wav64_t* boost;
    wav64_t* idle;
};

typedef struct motorcyle_assets motorcyle_assets_t;

static motorcyle_assets_t assets;

static dynamic_object_type_t collider_type = {
    BOX_COLLIDER(0.3f, 0.4f, 0.8f),
    .max_stable_slope = 0.0f,
    .surface_type = SURFACE_TYPE_DEFAULT,
    .friction = 0.0f,
    .bounce = 0.1f,
    .center = {0.0f, 0.6f, 0.0f},
};

static vehicle_camera_target_t boost_positions[VEHICLE_CAM_COUNT] = {
    [VEHICLE_CAM_NEUTRAL] = {
        .position = {0.0f, 1.75f, -4.0f},
        .look_at = {0.0f, 1.75f, 2.0f},
    },
    [VEHICLE_CAM_U] = {
        .position = {0.0f, 12.5f, -6.0f},
        .look_at = {0.0f, 1.5f, 2.0f},
    },
    [VEHICLE_CAM_UR] = {
        .position = {-6.0f, 6.5f, -3.0f},
        .look_at = {0.0f, 1.5f, 2.0f},
    },
    [VEHICLE_CAM_R] = {
        .position = {-4.0f, 2.5f, 1.0f},
        .look_at = {0.0f, 2.0f, 1.0f},
    },
    [VEHICLE_CAM_DR] = {
        .position = {-3.0f, 1.0f, -2.0f},
        .look_at = {0.0f, 3.0f, 1.0f},
    },
    [VEHICLE_CAM_D] = {
        .position = {0.0f, 1.0f, -3.0f},
        .look_at = {0.0f, 2.5f, 2.0f},
    },
    [VEHICLE_CAM_DL] = {
        .position = {3.0f, 1.0f, -2.0f},
        .look_at = {0.0f, 3.0f, 1.0f},
    },
    [VEHICLE_CAM_L] = {
        .position = {4.0f, 2.5f, 1.0f},
        .look_at = {0.0f, 2.0f, 1.0f},
    },
    [VEHICLE_CAM_UL] = {
        .position = {6.0f, 6.5f, -3.0f},
        .look_at = {0.0f, 1.5f, 2.0f},
    },
};

static vehicle_definiton_t vehicle_def = {
    .local_player_position = {
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f,
    },
    .exit_position = {-1.0f, 0.0f, 0.0f},
    .camera_positions = {
        [VEHICLE_CAM_NEUTRAL] = {
            .position = {0.0f, 2.0f, -4.0f},
            .look_at = {0.0f, 2.0f, 2.0f},
        },
        [VEHICLE_CAM_U] = {
            .position = {0.0f, 12.5f, -6.0f},
            .look_at = {0.0f, 1.5f, 2.0f},
        },
        [VEHICLE_CAM_UR] = {
            .position = {-6.0f, 6.5f, -3.0f},
            .look_at = {0.0f, 1.5f, 2.0f},
        },
        [VEHICLE_CAM_R] = {
            .position = {-4.0f, 2.5f, 1.0f},
            .look_at = {0.0f, 2.0f, 1.0f},
        },
        [VEHICLE_CAM_DR] = {
            .position = {-3.0f, 1.0f, -2.0f},
            .look_at = {0.0f, 3.0f, 1.0f},
        },
        [VEHICLE_CAM_D] = {
            .position = {0.0f, 1.0f, -3.0f},
            .look_at = {0.0f, 2.5f, 2.0f},
        },
        [VEHICLE_CAM_DL] = {
            .position = {3.0f, 1.0f, -2.0f},
            .look_at = {0.0f, 3.0f, 1.0f},
        },
        [VEHICLE_CAM_L] = {
            .position = {4.0f, 2.5f, 1.0f},
            .look_at = {0.0f, 2.0f, 1.0f},
        },
        [VEHICLE_CAM_UL] = {
            .position = {6.0f, 6.5f, -3.0f},
            .look_at = {0.0f, 1.5f, 2.0f},
        },
    },
    .boost_camera_positions = boost_positions,
};

static vector3_t local_cast_points[CAST_POINT_COUNT] = {
    {0.0f, CAST_CENTER, 0.0f},
    {0.31f, CAST_CENTER, 1.5f},
    {0.31f, CAST_CENTER, -1.5f},
    {-0.31f, CAST_CENTER, 1.5f},
    {-0.31f, CAST_CENTER, -1.5f},
};

void motorcycle_common_init() {
    assets.mesh = tmesh_cache_load("rom:/meshes/vehicles/bike.tmesh");
    assets.boost = wav64_load("rom:/sounds/vehicle/boost_pad.wav64", NULL);
    assets.idle = wav64_load("rom:/sounds/vehicle/cycle_idle.wav64", NULL);
}

void motorcycle_common_destroy() {
    tmesh_cache_release(assets.mesh);
    wav64_close(assets.boost);
    wav64_close(assets.idle);
    assets.mesh = NULL;
    assets.boost = NULL;
    assets.idle = NULL;
}

void motorcycle_ride(struct interactable* interactable, entity_id from) {

}

float motorcycle_hover_height(motorcycle_t* motorcycle, float speed) {
    float bob_height = BOB_HEIGHT * cosf(game_time * 2.0f * PI_F * (1.0f / BOB_TIME));

    if (!motorcycle->vehicle.driver) {
        return STILL_HOVER_HEIGHT + bob_height;
    }
    
    float lerp = speed * (1.0f / UPGRADED_DRIVE_SPEED);

    if (lerp > 1.0f) {
        return FAST_HOVER_HEIGHT;
    }

    return mathfLerp(RIDE_HOVER_HEIGHT + bob_height, FAST_HOVER_HEIGHT, lerp);
}

float motorcycle_target_speed(motorcycle_t* motorcycle, joypad_inputs_t input) {
    if (input.btn.a || motorcycle->vehicle.is_boosting) {
        if (motorcycle->boost_timer > 0.0f) {
            if (inventory_has_item(ITEM_UPGRADE_FASTER)) {
                return UPGRADED_BOOST_SPEED;
            }

            return BASE_BOOST_SPEED;
        }

        if (inventory_has_item(ITEM_UPGRADE_FASTER)) {
            return UPGRADED_DRIVE_SPEED;
        }

        return BASE_DRIVE_SPEED;
    } else if (input.btn.b) {
        return 0.0f;
    } else {
        return 0.99f * sqrtf(vector3MagSqrd2D(&motorcycle->collider.velocity));
    }
}

float motorycle_get_ground_height(motorcycle_t* motorcycle, float target_height, vector3_t* ground_normal) {
    float min_height_offset = target_height;

    for (int i = 0; i < CAST_POINT_COUNT; i += 1) {
        cast_point_t* cast_point = &motorcycle->cast_points[i];

        if (cast_point->surface_type != SURFACE_TYPE_NONE) {
            float actual_height = cast_point->pos.y - cast_point->y - CAST_CENTER;

            if (actual_height < min_height_offset) {
                min_height_offset = actual_height;
            }

            if (actual_height < target_height) {
                vector3Add(ground_normal, &cast_point->normal, ground_normal);
            }
        }

        vector3_t pos;
        transformSaTransformPoint(&motorcycle->transform, &local_cast_points[i], &pos);
        cast_point_set_pos(cast_point, &pos);
    }

    return min_height_offset;
}

#define MOTORCYCLE_CULL_DISTANCE    70.0f

bool motorcycle_check_active(motorcycle_t* motorcycle) {
    bool is_active = vector3DistSqrd(&motorcycle->transform.position, player_get_position(&current_scene->player)) < MOTORCYCLE_CULL_DISTANCE * MOTORCYCLE_CULL_DISTANCE;

    if (is_active && !motorcycle->is_active) {
        render_scene_init_add_renderable(&motorcycle->renderable, &motorcycle->transform, assets.mesh, 2.0f);
        collision_scene_add(&motorcycle->collider);
        motorcycle->drop_shadow.enabled = false;
    } else if (!is_active && motorcycle->is_active) {
        render_scene_remove_renderable(&motorcycle->renderable);
        collision_scene_remove(&motorcycle->collider);
        motorcycle->drop_shadow.enabled = true;
    }

    motorcycle->is_active = is_active;

    return is_active;
}

void motorcycle_check_for_mount(motorcycle_t* motorcycle) {
    if (inventory_has_item(ITEM_RIDING_MOTORCYCLE) && current_scene->player.state != PLAYER_IN_VEHICLE) {
        player_enter_vehicle(&current_scene->player, ENTITY_ID_MOTORCYLE);
    }
}

void motorcycle_update_sound(motorcycle_t* motorcycle, float speed) {
    if (motorcycle->vehicle.driver && !motorcycle->idle_sound) {
        motorcycle->idle_sound = audio_play_3d(assets.idle, 0.3f, &motorcycle->transform.position, &motorcycle->collider.velocity, 1.0f, 2);
    } else if (!motorcycle->vehicle.driver && motorcycle->idle_sound) {
        audio_stop(motorcycle->idle_sound);
        motorcycle->idle_sound = 0;
    }

    if (motorcycle->idle_sound) {
        float idle_lerp = speed * (1.0f / UPGRADED_DRIVE_SPEED);

        if (idle_lerp > 1.0f) {
            idle_lerp = 1.0f;
        }

        audio_update_position(motorcycle->idle_sound, &motorcycle->transform.position, &motorcycle->collider.velocity);
        audio_update_volume(motorcycle->idle_sound, mathfLerp(0.3f, 0.5f, idle_lerp));
        audio_update_pitch(motorcycle->idle_sound, mathfLerp(0.9f, 1.5f, idle_lerp));
    }
}

void motorcycle_update(void* data) {
    motorcycle_t* motorcycle = (motorcycle_t*)data;

    if (!motorcycle_check_active(motorcycle)) {
        return;
    }

    armature_t* armature = renderable_get_armature(&motorcycle->renderable);
    animator_update(&motorcycle->animator, armature, scaled_time_step);

    vector3_t ground_normal = (vector3_t){};
    float min_height_offset = motorycle_get_ground_height(motorcycle, FAST_HOVER_HEIGHT + HOVER_SAG_AMOUNT, &ground_normal);
    vector3Normalize(&ground_normal, &ground_normal);
    
    vector3_t ground_velocity;
    vector3ProjectPlane(&motorcycle->collider.velocity, &ground_normal, &ground_velocity);

    float current_speed = sqrtf(vector3MagSqrd2D(&ground_velocity));

    float target_height = motorcycle_hover_height(motorcycle, current_speed) + HOVER_SAG_AMOUNT;

    vector3_t forward;

    vector2ToLookDir(&motorcycle->transform.rotation, &forward);

    joypad_inputs_t input = joypad_get_inputs(0);
    joypad_buttons_t pressed = joypad_get_buttons_pressed(0);

    bool activate_boost = motorcycle->vehicle.hit_boost_pad;

    motorcycle->vehicle.is_boosting = activate_boost;
    
    if (pressed.z && motorcycle->self_boost_cooldown <= 0.0f && inventory_has_item(ITEM_BOOST_ANYWHERE)) {
        motorcycle->self_boost_cooldown = SELF_BOOST_COOLDOWN;
        activate_boost = true;
    }
    
#if ENABLE_CHEATS
    if (pressed.z) {
        activate_boost = true;
    }
#endif

    if (motorcycle->self_boost_cooldown > 0.0f) {
        motorcycle->self_boost_cooldown -= fixed_time_step;
    }

    if (activate_boost) {
        if (inventory_has_item(ITEM_LONGER_BOOST)) {
            motorcycle->boost_timer = UPGRADED_BOOST_TIME;
        } else {
            motorcycle->boost_timer = BASE_BOOST_TIME;
        }

        motorcycle->vehicle.hit_boost_pad = false;
        
        if (!motorcycle->boost_sound) {
            motorcycle->boost_sound = audio_play_2d(assets.boost, 0.3f, 0.0f, 1.0f, 1);
        }
    } else if (motorcycle->boost_sound) {
        motorcycle->boost_sound = 0;
    }

    if (motorcycle->boost_timer > 0.0f) {
        motorcycle->boost_timer -= fixed_time_step;
        motorcycle->vehicle.is_boosting = input.btn.a || activate_boost;

        if (!input.btn.a) {
            motorcycle->boost_timer = 0.0f;
        }
    }

    bool are_brakes_on = true;
    
    float target_speed = current_speed;

    motorcycle_update_sound(motorcycle, current_speed);

    if (motorcycle->vehicle.driver && update_has_layer(UPDATE_LAYER_WORLD)) {
        float accel = motorcycle->vehicle.is_boosting ? BOOST_ACCEL_RATE : ACCEL_RATE;

        float target_max_speed = motorcycle_target_speed(motorcycle, input);
        are_brakes_on = target_max_speed == 0.0f;
        target_speed = mathfMoveTowards(target_speed, target_max_speed, fixed_time_step * ACCEL_RATE);

        if (target_speed > target_max_speed) {
            target_speed = target_max_speed;
        }

        float turn_rate = MAX_TURN_RATE;

        float slow_threshold = motorcycle->vehicle.is_boosting ? TURN_BOOST_SLOW_THESHOLD : TURN_SLOW_THRESHOLD;
        float turn_accel = motorcycle->vehicle.is_boosting ? MAX_BOOST_TURN_ACCEL : MAX_TURN_ACCEL;

        if (target_speed > slow_threshold) {
            turn_rate = turn_accel / target_speed;
        }

        vector2_t new_rot;
        vector2_t rotation_amount;

        vector2ComplexFromAngle(fixed_time_step * turn_rate * input.stick_x * (1.0f / 80.0f), &rotation_amount);
        vector2ComplexMul(&motorcycle->transform.rotation, &rotation_amount, &new_rot);
        
        vector2ToLookDir(&new_rot, &forward);
        motorcycle->transform.rotation = new_rot;
    } else {
        target_speed = mathfMoveTowards(target_speed, 0.0f, fixed_time_step * ACCEL_RATE);
    }

    motorcycle->vehicle.is_stopped = vector3MagSqrd2D(&motorcycle->collider.velocity) < STOPPED_SPEED_THESHOLD * STOPPED_SPEED_THESHOLD;

    if (motorcycle->collider.hit_kill_plane) {
        motorcycle->collider.velocity = gZeroVec;
        motorcycle->transform.position = motorcycle->last_ground_location;
        motorcycle->collider.hit_kill_plane = 0;
        motorcycle_check_for_mount(motorcycle);
    }

    if (min_height_offset < target_height) {
        vector3_t* vel = &motorcycle->collider.velocity;
        float prev_y = vel->y;

        float is_going_up = vector3Dot(vel, &ground_normal) > 0.0f;

        vector3_t target_vel;
        vector2ToLookDir(&motorcycle->transform.rotation, &target_vel);
        vector3ProjectPlane(&target_vel, &ground_normal, &target_vel);
        vector3Normalize(&target_vel, &target_vel);;
        vector3Normalize(&ground_velocity, &ground_velocity);

        float speed_in_target_direction = vector3Dot(&target_vel, &ground_velocity);

        if (speed_in_target_direction > 0) {
            float transferred_speed = 0.95f * current_speed * speed_in_target_direction;

            if (transferred_speed > target_speed) {
                target_speed = transferred_speed;
            }
        }

        vector3Scale(&target_vel, &target_vel, target_speed);
        motorcycle->last_ground_location = motorcycle->transform.position;

        motorcycle->last_ground_location.y += 5.0f;

        float max_accel = motorcycle->has_traction ? MAX_TURN_ACCEL : DRIFT_ACCEL;

        float target_offset = target_height - min_height_offset;
        float spring_accel = target_offset * HOVER_SPRING_STRENGTH;

        if (prev_y > 0.0f) {
            float vel_threshold = -target_offset * 2.0f * GRAVITY_CONSTANT;

            if (prev_y * prev_y > vel_threshold) {
                spring_accel = 0.0f;
            }
        }

        if (are_brakes_on) {
            target_vel.y += spring_accel * fixed_time_step;
        } else {
            vector3AddScaled(&target_vel, &ground_normal, spring_accel * fixed_time_step, &target_vel);
        }

        motorcycle->has_traction = vector3MoveTowards(vel, &target_vel, 2.0f * max_accel * scaled_time_step, vel);
    }

    if (motorcycle->collider.active_contacts && motorcycle->vehicle.driver) {
        // this sucks
        vector3AddScaled(&motorcycle->transform.position, &motorcycle->collider.active_contacts->normal, 0.5f, &motorcycle->transform.position);
    }
}

static motorcycle_t* current_instance;

void motorcycle_init(motorcycle_t* motorcycle, struct motorcycle_definition* definition, entity_id entity_id) {
    transformSaInit(&motorcycle->transform, &definition->position, &definition->rotation, 1.0f);
    render_scene_init_add_renderable(&motorcycle->renderable, &motorcycle->transform, assets.mesh, 2.0f);
    dynamic_object_init(entity_id, &motorcycle->collider, &collider_type, COLLISION_LAYER_TANGIBLE, &motorcycle->transform.position, &motorcycle->transform.rotation);
    
    armature_t* armature = renderable_get_armature(&motorcycle->renderable);
    animator_init(&motorcycle->animator, armature->bone_count);
    motorcycle->animations = animation_cache_load("rom:/meshes/vehicles/bike.anim");
    vehicle_init(&motorcycle->vehicle, &motorcycle->transform, &vehicle_def, entity_id);
    collision_scene_add(&motorcycle->collider);
    update_add(motorcycle, motorcycle_update, UPDATE_PRIORITY_PHYICS, UPDATE_LAYER_WORLD | UPDATE_LAYER_CUTSCENE);
    
    interactable_init(&motorcycle->interactable, entity_id, INTERACT_TYPE_RIDE, motorcycle_ride, motorcycle);
    
    motorcycle->has_traction = true;
    motorcycle->is_active = true;
    motorcycle->boost_timer = 0.0f;
    motorcycle->last_ground_location = definition->position;
    motorcycle->self_boost_cooldown = SELF_BOOST_COOLDOWN;
    motorcycle->boost_sound = 0;
    motorcycle->idle_sound = 0;

    for (int i = 0; i < CAST_POINT_COUNT; i += 1) {
        vector3_t cast_point;
        transformSaTransformPoint(&motorcycle->transform, &local_cast_points[i], &cast_point);
        collision_scene_add_cast_point(&motorcycle->cast_points[i], &cast_point);
    }

    motorcycle_check_for_mount(motorcycle);
    animator_run_clip(&motorcycle->animator, animation_set_find_clip(motorcycle->animations, "idle"), 0.0f, true);
    drop_shadow_init(&motorcycle->drop_shadow, &motorcycle->collider, "rom:/meshes/effects/drop-shadow.tmesh");

    current_instance = motorcycle;
}

void motorcycle_destroy(motorcycle_t* motorcycle) {
    if (motorcycle->is_active) {
        render_scene_remove_renderable(&motorcycle->renderable);
        collision_scene_remove(&motorcycle->collider);
    }
    interactable_destroy(&motorcycle->interactable);
    update_remove(motorcycle);
    vehicle_destroy(&motorcycle->vehicle);
    animator_destroy(&motorcycle->animator);
    animation_cache_release(motorcycle->animations);
    drop_shadow_destroy(&motorcycle->drop_shadow);
    
    for (int i = 0; i < CAST_POINT_COUNT; i += 1) {
        collision_scene_remove_cast_point(&motorcycle->cast_points[i]);
    }

    current_instance = NULL;
}

motorcycle_t* motorcycle_get() {
    return current_instance;
}