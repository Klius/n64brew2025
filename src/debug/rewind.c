#include "rewind.h"

#include "../config.h"
#include "../math/vector3.h"
#include "../math/vector2.h"
#include "../entities/motorcycle.h"
#include "../scene/scene.h"

#if ENABLE_REWIND

#define MAX_ENTRIES     128

struct rewind_entry {
    vector3_t pos;
    vector2_t rot;
    vector3_t vel;
};

typedef struct rewind_entry rewind_entry_t;

#define REWIND_HEADER   0x52574E44
#define SRAM_ADDRESS    0x08000000

struct rewind_data {
    uint32_t header;
    uint16_t write_head;
    uint16_t entry_count;
    uint16_t playback_frames;
    rewind_entry_t entries[MAX_ENTRIES];
};

static struct rewind_data data;

void player_teleport_to_cycle(void* data);

int rewind_get_read_head() {
    int read_head = (int)data.write_head - (int)data.playback_frames;

    if (read_head < 0) {
        read_head += MAX_ENTRIES;
    }
    
    return read_head;
}

bool rewind_read_frame(motorcycle_t* motorcycle) {
    --data.playback_frames;

    if (!data.playback_frames) {
        return false;
    }

    int read_head = rewind_get_read_head();

    rewind_entry_t* entry = &data.entries[read_head];
    motorcycle->transform.rotation = entry->rot;
    motorcycle->collider.velocity = entry->vel;

    return true;
}

void rewind_write_frame(motorcycle_t* motorcycle) {
    data.entries[data.write_head] = (rewind_entry_t){
        .pos = motorcycle->transform.position,
        .rot = motorcycle->transform.rotation,
        .vel = motorcycle->collider.velocity,
    };

    ++data.write_head;

    if (data.entry_count < MAX_ENTRIES) {
        ++data.entry_count;
    }

    if (data.write_head == MAX_ENTRIES) {
        data.write_head = 0;
    }
}

void rewind_update() {
    motorcycle_t* motorcycle = motorcycle_get();

    if (!motorcycle) {
        return;
    }

    if (data.header != REWIND_HEADER) {
        data_cache_hit_writeback_invalidate(&data, sizeof(data));
        dma_read_async(&data, SRAM_ADDRESS, sizeof(data));
        dma_wait();

        if (data.header != REWIND_HEADER) {
            debugf("not saved\n");
            data.header = REWIND_HEADER;
            data.write_head = 0;
            data.entry_count = 0;
            data.playback_frames = 0;
        } else if (data.playback_frames) {
            debugf("loaded %d\n", data.playback_frames);
            int read_head = rewind_get_read_head();
            rewind_entry_t* entry = &data.entries[read_head];
            motorcycle->transform.position = entry->pos;
            motorcycle->transform.rotation = entry->rot;
            motorcycle->collider.velocity = entry->vel;
            player_teleport_to_cycle(&current_scene->player);
        }
    }

    joypad_buttons_t down = joypad_get_buttons_held(0);
    joypad_buttons_t up = joypad_get_buttons_released(0);

    if (down.d_down && data.entry_count > 0) {
        if (data.entry_count > data.playback_frames) {
            ++data.playback_frames;
        }

        int read_head = rewind_get_read_head();
        rewind_entry_t* entry = &data.entries[read_head];
        motorcycle->transform.position = entry->pos;
        motorcycle->transform.rotation = entry->rot;
        motorcycle->collider.velocity = entry->vel;
        return;
    } else if (up.d_down) {
        debugf("saving\n");
        data_cache_hit_writeback_invalidate(&data, sizeof(data));
        dma_write_raw_async(&data, SRAM_ADDRESS, sizeof(data));
        dma_wait();
    }

    if (data.playback_frames) {
        if (rewind_read_frame(motorcycle)) {
            return;
        }
    }

    rewind_write_frame(motorcycle);
}

#else

void rewind_update() {
    // nop
}

#endif