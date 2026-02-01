#ifndef __ENTITIES_MAP_PICKUP_H__
#define __ENTITIES_MAP_PICKUP_H__

#include "../math/vector3.h"
#include "../entity/entity_id.h"
#include "../scene/scene_definition.h"
#include "../math/transform_single_axis.h"
#include "../render/renderable.h"
#include "../collision/dynamic_object.h"
#include "../entity/interactable.h"

struct map_pickup {
    transform_sa_t transform;
    renderable_t renderable;
    dynamic_object_t collider;
    interactable_t interactable;
    enum inventory_item_type map_type;
};

typedef struct map_pickup map_pickup_t;

void map_pickup_init(map_pickup_t* map_pickup, struct map_pickup_definition* definition, entity_id entity_id);
void map_pickup_destroy(map_pickup_t* map_pickup, struct map_pickup_definition* definition);
void map_pickup_common_init();
void map_pickup_common_destroy();

#endif