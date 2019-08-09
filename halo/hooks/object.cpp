/*
 * Vulpes (c) 2019 gbMichelle
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../../hooker/hooker.hpp"
#include "../../hooker/function_pointer_safe.hpp"
#include "../memory/gamestate/object/object.hpp"
#include "../memory/behavior_definition.hpp"
#include <windows.h>
#include <cstring>

// Signatures for finding stuff.
// Currently unused. Will be used soon.

Signature(true, sig_object_update,
    {0x51, 0x8B, 0x0D, -1, -1, -1, 0x00, 0x8B, 0x51, 0x34, 0x53, 0x8B,
     0x5C, 0x24, 0x0C, 0x8B, 0xC3, 0x25, 0xFF, 0xFF, 0x00, 0x00}); // + 0 size 7

Signature(true, sig_objects_update,
    {0x00, 0x8A, -1, 0x0C, 0x83, 0xEC, 0x0C}); // -5 on client, -4 on server. size 6 on client, size 5 on server.

Signature(true, sig_object_new,
    {0x81, 0xEC, 0x1C, 0x02, 0x00, 0x00, 0x8B, 0x0D}); // size 6, do manual

Signature(true, sig_weapon_fire_call,
    {0x8B, 0x8C, 0x24, 0xAC, 0x00, 0x00, 0x00, 0x50, 0x51, 0x56,
     0xE8, -1, -1, 0x00, 0x00, 0x83, 0xC4, 0x0C}); // + 0xA size 5
Signature(true, sig_weapon_fire_object_new_call,
    {0x8D, 0x84, 0x24, 0xA4, 0x00, 0x00, 0x00, 0x50,
     0xE8, -1, -1, -1, 0x00, 0x8B, 0xF0}); // + 8 size 5

Signature(true, sig_biped_jump,
    {0x8B, 0x0D, -1, -1, -1, 0x00,
     0x8B, 0x51, 0x34, 0x83, 0xEC, 0x10, 0x53, 0x55}); // size 6 do manual

// New behavior definitions, when initialized hooking into
// one of these functions is as simple as re-assigning
// the pointer to a replacement function.

ObjectBehaviorDefinition new_obje_beh;
ObjectBehaviorDefinition new_devi_beh;
ObjectBehaviorDefinition new_item_beh;
ObjectBehaviorDefinition new_unit_beh;

ObjectBehaviorDefinition new_bipd_beh;
ObjectBehaviorDefinition new_vehi_beh;
ObjectBehaviorDefinition new_weap_beh;

ObjectBehaviorDefinition new_eqip_beh;
ObjectBehaviorDefinition new_garb_beh;
ObjectBehaviorDefinition new_proj_beh;

ObjectBehaviorDefinition new_scen_beh;
ObjectBehaviorDefinition new_mach_beh;
ObjectBehaviorDefinition new_ctrl_beh;
ObjectBehaviorDefinition new_lifi_beh;

ObjectBehaviorDefinition new_plac_beh;
ObjectBehaviorDefinition new_ssce_beh;

// Defining this as a macro since it is repeated 16 times.

#define DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(type) \
    ObjAdjustPlacement            type ## _adjust_placement              = NULL; \
    ObjCreate                     type ## _create                        = NULL; \
    ObjPlace                      type ## _place                         = NULL; \
    ObjDestroy                    type ## _destroy                       = NULL; \
    ObjUpdate                     type ## _update                        = NULL; \
    ObjExportFunctionValues       type ## _export_function_values        = NULL; \
    ObjHandleDeletedObject        type ## _handle_deleted_object         = NULL; \
    ObjHandleRegionDestroyed      type ## _handle_region_destroyed       = NULL; \
    ObjHandleParentDestroyed      type ## _handle_parent_destroyed       = NULL; \
    ObjPreprocessNodeOrientations type ## _preprocess_node_orientations  = NULL; \
    ObjPostprocessNodeMatricies   type ## _postprocess_node_matricies    = NULL; \
    ObjReset                      type ## _reset                         = NULL; \
    ObjDisconnectFromStructureBsp type ## _disconnect_from_structure_bsp = NULL; \
    ObjNotifyImpulseSound         type ## _notify_impulse_sound          = NULL; \
    ObjRenderDebug                type ## _render_debug                  = NULL; \
    ObjNewToNetwork               type ## _new_to_network                = NULL; \
    ObjUpdateBaseline             type ## _update_baseline               = NULL; \
    ObjGenerateUpdateHeader       type ## _generate_update_header        = NULL; \
    ObjHandleReceivedUpdate       type ## _handle_received_update        = NULL; \
    ObjShouldDoUpdateIncremental  type ## _should_do_update_incremental  = NULL; \
    ObjSetLastUpdateTime          type ## _set_last_update_time          = NULL; \
    // End of Macro

extern "C" { // Demangle naming for ASM.

    //////////
    // Declaring all function pointers for each object type here
    // so that the assembly can access it easily if needed.

    // Negative type indices

    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(object);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(device);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(item);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(unit);

    // Positive type indices

    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(biped);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(vehicle);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(weapon);

    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(equipment);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(garbage);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(projectile);

    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(scenery);

    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(machine);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(control);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(light_fixture);

    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(placeholder);
    DELC_OBJ_BEHAVIOR_FUNCTION_PTRS(sound_scenery);

}

// We're going to have to manually copy all of these, so we use another macro.

#define COPY_EVENT_DEF(type, def) \
    type ## _adjust_placement = def.adjust_placement; \
    type ## _create = def.create; \
    type ## _place = def.place; \
    type ## _destroy = def.destroy; \
    type ## _update = def.update; \
    type ## _export_function_values = def.export_function_values; \
    type ## _handle_deleted_object = def.handle_deleted_object; \
    type ## _handle_region_destroyed = def.handle_region_destroyed; \
    type ## _handle_parent_destroyed = def.handle_parent_destroyed; \
    type ## _preprocess_node_orientations = def.preprocess_node_orientations; \
    type ## _postprocess_node_matricies = def.postprocess_node_matricies; \
    type ## _reset = def.reset; \
    type ## _disconnect_from_structure_bsp = def.disconnect_from_structure_bsp; \
    type ## _notify_impulse_sound = def.notify_impulse_sound; \
    type ## _render_debug = def.render_debug; \
    type ## _new_to_network = def.new_to_network; \
    type ## _update_baseline = def.update_baseline; \
    type ## _generate_update_header = def.generate_update_header; \
    type ## _handle_received_update = def.handle_received_update; \
    type ## _should_do_update_incremental = def.should_do_update_incremental; \
    type ## _set_last_update_time = def.set_last_update_time; \
    // End of Macro

static ObjectBehaviorDefinition* vanilla_def_pointers_backup[POSITIVE_OBJECT_TYPES];
const size_t OBJECT_DEF_PTR_ARRAY_ABS_SIZE = sizeof(uintptr_t)*POSITIVE_OBJECT_TYPES;

void init_object_hooks(){
    auto game_defs = object_behavior_defs();

    // Copy data to our new defs.

    const size_t S = sizeof(ObjectBehaviorDefinition);

    // These top 4 are negative indices and are not in the base array,
    // but we still need them. They are parent types of other object types
    // found here though, so we get that data by looking through the parent
    // type definitions.

    memcpy(&new_obje_beh, game_defs[static_cast<int>(ObjectType::BIPED)]->parent_definitions[0], S);
    new_obje_beh.parent_definitions[0] = &new_obje_beh;

    memcpy(&new_devi_beh, game_defs[static_cast<int>(ObjectType::MACHINE)]->parent_definitions[1], S);
    new_devi_beh.parent_definitions[0] = &new_obje_beh;
    new_devi_beh.parent_definitions[1] = &new_devi_beh;

    memcpy(&new_item_beh, game_defs[static_cast<int>(ObjectType::WEAPON)]->parent_definitions[1], S);
    new_item_beh.parent_definitions[0] = &new_obje_beh;
    new_item_beh.parent_definitions[1] = &new_item_beh;

    memcpy(&new_unit_beh, game_defs[static_cast<int>(ObjectType::BIPED)]->parent_definitions[1], S);
    new_unit_beh.parent_definitions[0] = &new_obje_beh;
    new_unit_beh.parent_definitions[1] = &new_unit_beh;

    // 0 and up

    memcpy(&new_bipd_beh, game_defs[static_cast<int>(ObjectType::BIPED)], S);
    new_bipd_beh.parent_definitions[0] = &new_obje_beh;
    new_bipd_beh.parent_definitions[1] = &new_unit_beh;
    new_bipd_beh.parent_definitions[2] = &new_bipd_beh;

    memcpy(&new_vehi_beh, game_defs[static_cast<int>(ObjectType::VEHICLE)], S);
    new_vehi_beh.parent_definitions[0] = &new_obje_beh;
    new_vehi_beh.parent_definitions[1] = &new_unit_beh;
    new_vehi_beh.parent_definitions[2] = &new_vehi_beh;

    memcpy(&new_weap_beh, game_defs[static_cast<int>(ObjectType::WEAPON)], S);
    new_weap_beh.parent_definitions[0] = &new_obje_beh;
    new_weap_beh.parent_definitions[1] = &new_item_beh;
    new_weap_beh.parent_definitions[2] = &new_weap_beh;

    memcpy(&new_eqip_beh, game_defs[static_cast<int>(ObjectType::EQUIPMENT)], S);
    new_eqip_beh.parent_definitions[0] = &new_obje_beh;
    new_eqip_beh.parent_definitions[1] = &new_item_beh;
    new_eqip_beh.parent_definitions[2] = &new_eqip_beh;

    memcpy(&new_garb_beh, game_defs[static_cast<int>(ObjectType::GARBAGE)], S);
    new_garb_beh.parent_definitions[0] = &new_obje_beh;
    new_garb_beh.parent_definitions[1] = &new_item_beh;
    new_garb_beh.parent_definitions[2] = &new_garb_beh;

    memcpy(&new_proj_beh, game_defs[static_cast<int>(ObjectType::PROJECTILE)], S);
    new_proj_beh.parent_definitions[0] = &new_obje_beh;
    new_proj_beh.parent_definitions[1] = &new_proj_beh;

    memcpy(&new_scen_beh, game_defs[static_cast<int>(ObjectType::SCENERY)], S);
    new_scen_beh.parent_definitions[0] = &new_obje_beh;
    new_scen_beh.parent_definitions[1] = &new_scen_beh;

    memcpy(&new_mach_beh, game_defs[static_cast<int>(ObjectType::MACHINE)], S);
    new_mach_beh.parent_definitions[0] = &new_obje_beh;
    new_mach_beh.parent_definitions[1] = &new_devi_beh;
    new_mach_beh.parent_definitions[2] = &new_mach_beh;

    memcpy(&new_ctrl_beh, game_defs[static_cast<int>(ObjectType::CONTROL)], S);
    new_ctrl_beh.parent_definitions[0] = &new_obje_beh;
    new_ctrl_beh.parent_definitions[1] = &new_devi_beh;
    new_ctrl_beh.parent_definitions[2] = &new_ctrl_beh;

    memcpy(&new_lifi_beh, game_defs[static_cast<int>(ObjectType::LIGHT_FIXTURE)], S);
    new_lifi_beh.parent_definitions[0] = &new_obje_beh;
    new_lifi_beh.parent_definitions[1] = &new_devi_beh;
    new_lifi_beh.parent_definitions[2] = &new_lifi_beh;

    memcpy(&new_plac_beh, game_defs[static_cast<int>(ObjectType::PLACEHOLDER)], S);
    new_plac_beh.parent_definitions[0] = &new_obje_beh;
    new_plac_beh.parent_definitions[1] = &new_plac_beh;

    memcpy(&new_ssce_beh, game_defs[static_cast<int>(ObjectType::SOUND_SCENERY)], S);
    new_ssce_beh.parent_definitions[0] = &new_obje_beh;
    new_ssce_beh.parent_definitions[1] = &new_ssce_beh;

    // Backup old array of def pointers for when the DLL needs to unload.

    const size_t COPY_SIZE = OBJECT_DEF_PTR_ARRAY_ABS_SIZE;

    memcpy(&vanilla_def_pointers_backup, game_defs, COPY_SIZE);

    // Replace old array of def pointers with pointers to our new defs.

    DWORD prota, protb;
    VirtualProtect(game_defs, COPY_SIZE, PAGE_EXECUTE_READWRITE, &prota);

    game_defs[static_cast<int>(ObjectType::BIPED)]          = &new_bipd_beh;
    game_defs[static_cast<int>(ObjectType::VEHICLE)]        = &new_vehi_beh;
    game_defs[static_cast<int>(ObjectType::WEAPON)]         = &new_weap_beh;
    game_defs[static_cast<int>(ObjectType::EQUIPMENT)]      = &new_eqip_beh;
    game_defs[static_cast<int>(ObjectType::GARBAGE)]        = &new_garb_beh;
    game_defs[static_cast<int>(ObjectType::PROJECTILE)]     = &new_proj_beh;
    game_defs[static_cast<int>(ObjectType::SCENERY)]        = &new_scen_beh;
    game_defs[static_cast<int>(ObjectType::MACHINE)]        = &new_mach_beh;
    game_defs[static_cast<int>(ObjectType::CONTROL)]        = &new_ctrl_beh;
    game_defs[static_cast<int>(ObjectType::LIGHT_FIXTURE)]  = &new_lifi_beh;
    game_defs[static_cast<int>(ObjectType::PLACEHOLDER)]    = &new_plac_beh;
    game_defs[static_cast<int>(ObjectType::SOUND_SCENERY)]  = &new_ssce_beh;

    VirtualProtect(game_defs, COPY_SIZE, prota, &protb);

    // Copy the function pointers to our assembly callable vars
    // before we hook into them.

    COPY_EVENT_DEF(object,          new_obje_beh);
    COPY_EVENT_DEF(device,          new_devi_beh);
    COPY_EVENT_DEF(item,            new_item_beh);
    COPY_EVENT_DEF(unit,            new_unit_beh);

    COPY_EVENT_DEF(biped,           new_bipd_beh);
    COPY_EVENT_DEF(vehicle,         new_vehi_beh);
    COPY_EVENT_DEF(weapon,          new_weap_beh);
    COPY_EVENT_DEF(equipment,       new_eqip_beh);
    COPY_EVENT_DEF(garbage,         new_garb_beh);
    COPY_EVENT_DEF(projectile,      new_proj_beh);
    COPY_EVENT_DEF(scenery,         new_scen_beh);
    COPY_EVENT_DEF(machine,         new_mach_beh);
    COPY_EVENT_DEF(control,         new_ctrl_beh);
    COPY_EVENT_DEF(light_fixture,   new_lifi_beh);
    COPY_EVENT_DEF(placeholder,     new_plac_beh);
    COPY_EVENT_DEF(sound_scenery,   new_ssce_beh);

    // Put hooks in place.
    // Hooking into one of the above copied functions is as easy as replacing
    // its pointer with the pointer to the replacing function.
    // In most cases we will still need to execute the old one though. So,
    // when doing that make sure to call the old function. It can simply be
    // called by name because of how we've copied them.

}

void revert_object_hooks(){

    auto game_defs = object_behavior_defs();

    // Revert to the old defs.

    const size_t COPY_SIZE = OBJECT_DEF_PTR_ARRAY_ABS_SIZE;

    DWORD prota, protb;
    VirtualProtect(game_defs, COPY_SIZE, PAGE_EXECUTE_READWRITE, &prota);
    memcpy(game_defs, &vanilla_def_pointers_backup, COPY_SIZE);
    VirtualProtect(game_defs, COPY_SIZE, prota, &protb);

}
