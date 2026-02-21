#include "../test/framework_test.h"

#include "resource_cache.h"

#define RESOURCE_A              (void*)16
#define RESOURCE_COLLISION      (void*)(16 + 16 * 32)

void test_resource_cache(struct test_context* t) {
    resource_cache_t cache;
    memset(&cache, 0, sizeof(resource_cache_t));

    resource_cache_entry_t* entry = resource_cache_use(&cache, "a");
    
    test_eqi(t, 0, (int)entry->resource);
    resource_cache_set_resource(&cache, entry, RESOURCE_A);
    test_eqi(t, 1, entry->reference_count);

    entry = resource_cache_use(&cache, "a");
    test_eqi(t, (int)RESOURCE_A, (int)entry->resource);
    test_eqi(t, 2, entry->reference_count);

    bool should_free = resource_cache_free(&cache, RESOURCE_A);
    test_eqi(t, false, should_free);

    should_free = resource_cache_free(&cache, RESOURCE_A);
    test_eqi(t, true, should_free);

    resource_cache_destroy(&cache);

    entry = resource_cache_use(&cache, "a");
    resource_cache_set_resource(&cache, entry, RESOURCE_A);
    entry = resource_cache_use(&cache, "b");
    resource_cache_set_resource(&cache, entry, RESOURCE_COLLISION);
    
    should_free = resource_cache_free(&cache, RESOURCE_A);
    test_eqi(t, true, should_free);
    should_free = resource_cache_free(&cache, RESOURCE_COLLISION);
    test_eqi(t, true, should_free);
    
    resource_cache_destroy(&cache);
}