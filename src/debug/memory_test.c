
#include "../test/framework_test.h"

#include "../resource/tmesh_cache.h"

heap_stats_t test_memory_leak_start(const char* name) {
    debugf("    memory leak test %s\n", name);

    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);
    return heap_stats;
}

void test_memory_leak_end(struct test_context* t, heap_stats_t *start) {
    heap_stats_t heap_stats;
    sys_get_heap_stats(&heap_stats);
    test_eqi(t, start->used, heap_stats.used);
}

void test_memory_leaks(struct test_context* t) {
    heap_stats_t heap_stats;

    tmesh_t player_mesh;
    heap_stats = test_memory_leak_start("tmesh_load_filename");
    tmesh_load_filename(&player_mesh, "rom:/meshes/characters/NPC_scrapbot1.tmesh");
    tmesh_release(&player_mesh);
    test_memory_leak_end(t, &heap_stats);

    tmesh_cache_destroy();
    heap_stats = test_memory_leak_start("tmesh_cache_load");
    tmesh_cache_release(tmesh_cache_load("rom:/meshes/characters/NPC_scrapbot1.tmesh"));
    tmesh_cache_destroy();
    test_memory_leak_end(t, &heap_stats);
}