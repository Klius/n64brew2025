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

void cutscene_stopwatch_render(void* data) {
    char time[20];

    int sub_seconds = (int)ceilf(timer.current_time * 100.0f);
    
    int seconds = sub_seconds / 100;
    sub_seconds = sub_seconds % 100;

    int minutes = seconds / 60;
    seconds = seconds % 60;

    sprintf(time, "%02d:%02d.%02d", minutes, seconds, sub_seconds);
    
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
        strlen(time)
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
    timer.current_time = 0.0f;
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