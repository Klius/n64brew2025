#include "checkpoint.h"    
    
#include "../render/render_scene.h"
#include "../collision/collision_scene.h"
#include "../cutscene/expression_evaluate.h"
#include "../time/time.h"
#include "../audio/audio.h"

struct checkpoint_assets
{
    wav64_t checkpoint_sound;
};

static struct checkpoint_assets assets;

static spatial_trigger_type_t trigger_type = {
    .type = SPATIAL_TRIGGER_BOX,
    .data = {
        .box = {
            .half_size = {5.0f, 3.0f, 0.5f},
        }
    },
    .center = {0.0f, 2.0f, 0.0f},
};

static const char* checkpoint_meshes[CHECKPOINT_TYPE_COUNT] = {
    [CHECKPOINT_FINISH] = "rom:/meshes/objects/race_start+finish.tmesh",
    [CHECKPOINT_MIDDLE] = "rom:/meshes/objects/race_checkpoint.tmesh",
};

void checkpoint_update(void* data) {
    checkpoint_t* checkpoint = (checkpoint_t*)data;

    if (contacts_are_touching(checkpoint->trigger.active_contacts, ENTITY_ID_MOTORCYLE)) {
        int last_checkpoint = expression_get_integer(checkpoint->race_progress);
        if (last_checkpoint + 1 == checkpoint->checkpoint_index) {
            expression_set_integer(checkpoint->race_progress, last_checkpoint + 1);
            audio_play_2d(&assets.checkpoint_sound, 1.0f, 0.0f, 1.0f, 1);
        }
    }
}

void checkpoint_init(checkpoint_t* checkpoint, struct checkpoint_definition* definition, entity_id entity_id) {
    transformSaInit(&checkpoint->transform, &definition->position, &definition->rotation, 1.0f);
    
    spatial_trigger_init(&checkpoint->trigger, &checkpoint->transform, &trigger_type, COLLISION_LAYER_TANGIBLE, entity_id);
    collision_scene_add_trigger(&checkpoint->trigger);

    renderable_single_axis_init(&checkpoint->renderable, &checkpoint->transform, checkpoint_meshes[definition->checkpoint_type]);
    render_scene_add_renderable(&checkpoint->renderable, 7.0f);
    update_add(checkpoint, checkpoint_update, UPDATE_PRIORITY_EFFECTS, UPDATE_LAYER_WORLD);

    checkpoint->race_progress = definition->race_progress;
    checkpoint->checkpoint_index = definition->checkpoint_index;
}

void checkpoint_destroy(checkpoint_t* checkpoint, struct checkpoint_definition* definition) {
    collision_scene_remove_trigger(&checkpoint->trigger);

    render_scene_remove(&checkpoint->renderable);
    renderable_destroy(&checkpoint->renderable);
    update_remove(checkpoint);
}

void checkpoint_common_init() {
    wav64_open(&assets.checkpoint_sound, "rom:/sounds/race/checkpoint.wav");
}

void checkpoint_common_destroy() {
    wav64_close(&assets.checkpoint_sound);
}
