/*
 * This file is part of Vulpes, an extension of Halo Custom Edition's capabilities.
 * Copyright (C) 2019-2020 gbMichelle (Michelle van der Graaf)
 *
 * Vulpes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * Vulpes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * long with Vulpes.  If not, see <https://www.gnu.org/licenses/agpl-3.0.en.html>
 */

#include <string>

#include <lua.hpp>

#include <util/file_helpers.hpp>

#include <vulpes/hooks/map.hpp>
#include <vulpes/memory/global.hpp>
#include <vulpes/memory/types.hpp>
#include <vulpes/paths.hpp>

#include "lua.hpp"
#include "lua_console.hpp"
#include "lua_helpers.hpp"
#include "lua_open.hpp"

static lua_State* map_state = NULL;

static bool initialized = false;

static const auto LUA_MAIN_FILE = std::string("main.lua");

static void luaV_register_functions(lua_State *state, bool sandboxed) {
    luaV_openlibs(state, !sandboxed);
    luaV_register_console(state);
}

//TODO: Make this load scripts from maps.
static void luaV_load_scripts_for_map() {
    // Get path to where the lua scripts should be stored.
    auto cur_map_name = std::string(map_name());
    auto path_str = std::string(profile_path()) + LUA_MAP_PATH + "\\" + cur_map_name + "\\";

    // Check if it is actually a directory / even exists.
    // This is done so that we don't go in and create a whole lua state
    // and register all the functions just to destroy it again if there
    // isn't even a folder to load main.lua from.
    if (is_dir(path_str)) {
        // Check if there is an main.lua
        auto file_str = path_str + LUA_MAIN_FILE;

        if (is_file(file_str)) {
            // Create a new lua state.
            auto *state = luaL_newstate();

            // Register functions.
            luaV_register_functions(state, true);

            if(luaV_loadfile_as(state,
                path_str + LUA_MAIN_FILE,
                cur_map_name + ":" + LUA_MAIN_FILE)
            || lua_pcall(state, 0, 0, 0)) {
                luaV_print_error(state);
                lua_close(state);
            } else {
                // State setup was succesful, save a reference to it globally.
                map_state = state;
            }
        }
        return;
    }
}

static void luaV_unload_scripts_for_map() {
    if (map_state != NULL) {
        cprintf_info("Closing previous map-lua state.");
        lua_close(map_state);
        map_state = NULL;
    }
}

void init_lua() {
    initialized = true;

    // Create folders if they don't exist.
    make_dir(std::string(profile_path()) + VULPES_PATH);
    make_dir(std::string(profile_path()) + LUA_BASE_PATH);
    make_dir(std::string(profile_path()) + LUA_MAP_PATH);
    make_dir(std::string(profile_path()) + LUA_GLOBAL_PATH);

    // init_callbacks();
    ADD_CALLBACK(EVENT_MAP_LOAD,     luaV_load_scripts_for_map);
    ADD_CALLBACK(EVENT_PRE_MAP_LOAD, luaV_unload_scripts_for_map);
}

void destruct_lua() {
    initialized = false;
    luaV_unload_scripts_for_map();
    DEL_CALLBACK(EVENT_MAP_LOAD,     luaV_load_scripts_for_map);
    DEL_CALLBACK(EVENT_PRE_MAP_LOAD, luaV_unload_scripts_for_map);
}
