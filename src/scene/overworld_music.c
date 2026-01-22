#include "overworld_music.h"

#include "../audio/audio.h"
#include "../cutscene/race.h"
#include "../time/time.h"

static vector2_t settlement_loc = {
    179.4, 113.6,
};

#define SETTLEMENT_RADIUS   140.0f

static const char* music_filenames[OVERWORLD_SONG_COUNT] = {
    [OVERWORLD_SONG_AWAKENGING_TO_SILENCE] = "rom:/sounds/music/awakening_to_silence.wav64",
    [OVERWORLD_SONG_DESERT_DAYDREAMS] = "rom:/sounds/music/desert_daydreams.wav64",
    [OVERWORLD_SONG_DESERT_STRING] = "rom:/sounds/music/desert_strings.wav64",
    [OVERWORLD_SONG_RACE] = "rom:/sounds/music/race.wav64",
};

void overworld_music_init(overworld_music_t* music) {
    for (int i = 0; i < OVERWORLD_SONG_COUNT; i += 1) {
        music->songs[i] = wav64_load(music_filenames[i], NULL);
    }

    music->is_racing = race_get_state() == RACE_STATE_STARTED;
    music->did_race_start = false;
}

wav64_t* overworld_music_determine_song(overworld_music_t* music, vector3_t* player_pos, bool has_overworld) {
    if (!has_overworld) {
        return NULL;
    }

    if (music->is_racing) {
        if (update_has_layer(UPDATE_LAYER_WORLD)) {
            music->did_race_start = true;
        }

        return music->did_race_start ? music->songs[OVERWORLD_SONG_RACE] : NULL;
    }

    vector2_t offset = {
        player_pos->x - settlement_loc.x,
        player_pos->z - settlement_loc.y,
    };
    
    if (vector2MagSqr(&offset) < SETTLEMENT_RADIUS * SETTLEMENT_RADIUS) {
        return NULL;
    }

    if (offset.y < 0.0f) {
        return music->songs[OVERWORLD_SONG_DESERT_DAYDREAMS];
    }

    if (offset.x < 0.0f) {
        return music->songs[OVERWORLD_SONG_AWAKENGING_TO_SILENCE];
    }

    return music->songs[OVERWORLD_SONG_DESERT_STRING];
}

void overworld_music_update(overworld_music_t* music, vector3_t* player_pos, bool has_overworld) {
    audio_play_music(overworld_music_determine_song(music, player_pos, has_overworld));
}

void overworld_music_destroy(overworld_music_t* music) {
    audio_play_music(NULL);
     for (int i = 0; i < OVERWORLD_SONG_COUNT; i += 1) {
        wav64_close(music->songs[i]);
     }
}