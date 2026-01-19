#ifndef __CUTSCENE_CUTSCENE_STOPWATCH_H__
#define __CUTSCENE_CUTSCENE_STOPWATCH_H__

#include <stdbool.h>

void cutscene_stopwatch_set_running(bool value);
void cutscene_stopwatch_set_active(bool value);

float cutscene_last_stopwatch_time();
void cutscene_stopwatch_set(float value);
int cutscene_stopwatch_format_time(char* into, float duration);

#endif