#include <cstdint>
#include "hooker.hpp"
#include "../memory/object.hpp"

intptr_t projectile_spawn_from_weapon = 0x4C87DB;
intptr_t projectile_spawn_grenade_throw = 0x571AD6; // still attached
intptr_t object_update = 0x4FB800;
intptr_t objects_update = 0x4F87A0;


void init_object_hooks(){
}

void revert_object_hooks(){
}