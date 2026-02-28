#ifndef __PROFILE_PROFILE_H__
#define __PROFILE_PROFILE_H__

#define ENABLE_PROFILE_main     0
#define ENABLE_PROFILE_update   0
#define ENABLE_PROFILE_scene    0

void profile_start();
void profile_end(const char* name);

#define SC_PROFILE_ENABLED(group) ENABLE_PROFILE_##group
#define SC_PROFILE_START(group)   if (SC_PROFILE_ENABLED(group)) profile_start()
#define SC_PROFILE_END(group, label)   if (SC_PROFILE_ENABLED(group)) profile_end(#group " " #label)

#endif