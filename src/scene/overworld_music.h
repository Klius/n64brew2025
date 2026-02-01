#ifndef __OVERWORLD_MUSIC_H__
#define __OVERWORLD_MUSIC_H__

#include <libdragon.h>
#include <stdbool.h>
#include "../math/vector3.h"

enum overworld_songs {
    OVERWORLD_SONG_AWAKENGING_TO_SILENCE,
    OVERWORLD_SONG_DESERT_DAYDREAMS,
    OVERWORLD_SONG_DESERT_STRING,
    OVERWORLD_SONG_RACE,
    OVERWORLD_SONG_MEMORIES_INDOOR,
    OVERWORLD_SONG_MEMORIES_OUTDOOR,
    OVERWORLD_SONG_INDOOR_AMBIENCE,

    OVERWORLD_SONG_COUNT,
};

struct overworld_music {
    wav64_t* songs[OVERWORLD_SONG_COUNT];
    bool is_racing;
    bool did_race_start;
};

typedef struct overworld_music overworld_music_t;

void overworld_music_init(overworld_music_t* music);
void overworld_music_update(overworld_music_t* music, vector3_t* player_pos, bool has_overworld);
void overworld_music_destroy(overworld_music_t* music);

#endif