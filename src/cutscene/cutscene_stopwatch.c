#include "cutscene_stopwatch.h"

#include "../time/time.h"
#include "../menu/menu_rendering.h"
#include "../fonts/fonts.h"

struct cutscene_stopwatch {
    bool is_active;
    bool is_running;
    float current_time;
};

static struct cutscene_stopwatch timer;

int cutscene_stopwatch_format_time(char* into, float duration) {
    int sub_seconds = (int)ceilf(duration * 100.0f);
    
    int seconds = sub_seconds / 100;
    sub_seconds = sub_seconds % 100;

    int minutes = seconds / 60;
    seconds = seconds % 60;

    return sprintf(into, "%02d:%02d.%02d", minutes, seconds, sub_seconds);
}

void cutscene_stopwatch_render(void* data) {
    char time[20];
    int len = cutscene_stopwatch_format_time(time, timer.current_time);
    
    rdpq_text_printn(&(rdpq_textparms_t){
            // .line_spacing = -3,
            .align = ALIGN_LEFT,
            .valign = VALIGN_TOP,
            .width = 260,
            .height = 60,
            .wrap = WRAP_NONE,
        }, 
        FONT_DIALOG, 
        30, 30, 
        time,
        len
    );
}

void cutscene_stopwatch_update(void* data) {
    if (timer.is_running) {
        timer.current_time += scaled_time_step;
    }
}

void cutscene_stopwatch_set_running(bool value) {
    timer.is_running = value;
}

void cutscene_stopwatch_set_active(bool value) {
    if (value) {
        timer.current_time = 0.0f;
    }
    timer.is_running = false;

    if (timer.is_active == value) {
        return;
    }

    timer.is_active = value;

    if (value) {
        update_add(&timer, cutscene_stopwatch_update, UPDATE_PRIORITY_EFFECTS, UPDATE_LAYER_WORLD);
        menu_add_callback(cutscene_stopwatch_render, &timer, MENU_PRIORITY_HUD);
        font_type_use(FONT_DIALOG);
    } else {
        update_remove(&timer);
        menu_remove_callback(&timer);
        font_type_release(FONT_DIALOG);
    }
}

float cutscene_last_stopwatch_time() {
    return timer.current_time;
}

void cutscene_stopwatch_set(float value) {
    timer.current_time = value;
}