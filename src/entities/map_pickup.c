#include "map_pickup.h"    
    
#include "../render/render_scene.h"
#include "../collision/collision_scene.h"
#include "../collision/shapes/box.h"
#include "../player/inventory.h"
#include "../menu/map_menu.h"

static dynamic_object_type_t collider = {
    BOX_COLLIDER(0.7f, 0.7f, 0.2f),
    .center = {0.0f, 0.0f, 0.0f},
    .friction = 0.5,
    .max_stable_slope = 0.219131191f,
};

void map_pickup_iteract(struct interactable* interactable, entity_id from) {
    map_pickup_t* map = (map_pickup_t*)interactable->data;

    if (inventory_has_item(map->map_type)) {
        return;
    }

    inventory_set_has_item(map->map_type, true);
    map_menu_show_with_item(map->map_type);
    interactable->interact_type = INTERACT_TYPE_NONE;
}

void map_pickup_init(map_pickup_t* map_pickup, struct map_pickup_definition* definition, entity_id entity_id) {
    transformSaInit(&map_pickup->transform, &definition->position, &definition->rotation, 1.0f);

    map_pickup->map_type = definition->map_type;

    renderable_single_axis_init(&map_pickup->renderable, &map_pickup->transform, "rom:/meshes/parts/map_pickup.tmesh");
    render_scene_add_renderable(&map_pickup->renderable, 1.0f);

    dynamic_object_init(
        entity_id,
        &map_pickup->collider,
        &collider,
        COLLISION_LAYER_TANGIBLE,
        &map_pickup->transform.position,
        &map_pickup->transform.rotation
    );

    map_pickup->collider.is_fixed = true;
    map_pickup->collider.weight_class = WEIGHT_CLASS_SUPER_HEAVY;

    collision_scene_add(&map_pickup->collider);
    
    interactable_init(&map_pickup->interactable, entity_id, inventory_has_item(map_pickup->map_type) ? INTERACT_TYPE_NONE : INTERACT_TYPE_CHECK, map_pickup_iteract, map_pickup);
}

void map_pickup_destroy(map_pickup_t* map_pickup, struct map_pickup_definition* definition) {
    render_scene_remove(&map_pickup->renderable);
    renderable_destroy(&map_pickup->renderable);
    collision_scene_remove(&map_pickup->collider);
    interactable_destroy(&map_pickup->interactable);
}

void map_pickup_common_init() {

}

void map_pickup_common_destroy() {

}
