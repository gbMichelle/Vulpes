#pragma once

#include <cstdint>

enum MessageDeltaType : int8_t {
    OBJECT_DELETION                    = 0,
    PROJECTILE_UPDATE                  = 1,
    EQUIPMENT_UPDATE                   = 2,
    WEAPON_UPDATE                      = 3,
    BIPED_UPDATE                       = 4,
    VEHICLE_UPDATE                     = 5,
    HUD_ADD_ITEM                       = 6,
    PLAYER_CREATE                      = 7,
    PLAYER_SPAWN                       = 8,
    PLAYER_EXIT_VEHICLE                = 9,
    PLAYER_SET_ACTION_RESULT           = 0xA,
    PLAYER_EFFECT_START                = 0xB,
    UNIT_KILL                          = 0xC,
    CLIENT_GAME_UPDATE                 = 0xD,
    PLAYER_HANDLE_POWERUP              = 0xE,
    HUD_CHAT                           = 0xF,
    SLAYER_UPDATE                      = 0x10,
    CTF_UPDATE                         = 0x11,
    ODDBALL_UPDATE                     = 0x12,
    KING_UPDATE                        = 0x13,
    RACE_UPDATE                        = 0x14,
    PLAYER_SCORE_UPDATE                = 0x15,
    GAME_ENGINE_CHANGE_MODE            = 0x16,
    GAME_ENGINE_MAP_RESET              = 0x17,
    MULTIPLAYER_HUD_MESSAGE            = 0x18,
    MULTIPLAYER_SOUND                  = 0x19,
    TEAM_CHANGE                        = 0x1A,
    UNIT_DROP_CURRENT_WEAPON           = 0x1B,
    VEHICLE_NEW                        = 0x1C,
    BIPED_NEW                          = 0x1D,
    PROJECTILE_NEW                     = 0x1E,
    EQUIPMENT_NEW                      = 0x1F,
    WEAPON_NEW                         = 0x20,
    GAME_SETTINGS_UPDATE               = 0x21,
    PARAMETERS_PROTOCOL                = 0x22,
    LOCAL_PLAYER_UPDATE                = 0x23,
    LOCAL_PLAYER_VEHICLE_UPDATE        = 0x24,
    REMOTE_PLAYER_ACTION_UPDATE        = 0x25,
    SUPER_REMOTE_PLAYERS_ACTION_UPDATE = 0x26,
    REMOTE_PLAYER_POSITION_UPDATE      = 0x27,
    REMOTE_PLAYER_VEHICLE_UPDATE       = 0x28,
    REMOTE_PLAYER_TOTAL_UPDATE_BIPED   = 0x29,
    REMOTE_PLAYER_TOTAL_UPDATE_VEHICLE = 0x2A,
    WEAPON_START_RELOAD                = 0x2B,
    WEAPON_AMMO_PICKUP_MID_RELOAD      = 0x2C,
    WEAPON_FINISH_RELOAD               = 0x2D,
    WEAPON_CANCEL_RELOAD               = 0x2E,
    NETGAME_EQUIPMENT_NEW              = 0x2F,
    PROJECTILE_DETONATE                = 0x30,
    ITEM_ACCELERATE                    = 0x31,
    DAMAGE_DEALT                       = 0x32,
    PROJECTILE_ATTACH                  = 0x33,
    CLIENT_TO_SERVER_PONG              = 0x34,
    SUPER_PING_UPDATE                  = 0x35,
    SV_MOTD                            = 0x36,
    RCON_REQUEST                       = 0x37,
    RCON_RESPONSE                      = 0x38,

    DEFAULT_MESSAGE_TYPE_COUNT = 0x39,

    VULPES_MSG = 0x39,

    NEW_MESSAGE_TYPE_COUNT = 0x3A
};

enum class MessageDeltaMode : int32_t {
    STATELESS = 0,
    INCREMENTAL = 1
};

enum class HudChatType : int8_t {
    NONE    = -1,
    ALL     = 0,
    TEAM    = 1,
    VEHICLE = 2,
    SERVER  = 3, // Phasor?
    PRIVATE = 4, // Phasor?
    HAC     = 5, // Sapp uses this channel to communicate with hac2.
    HCN     = 6, // Another mod uses this channel to communicate with hac2.
    VULPES  = 17  // The channel we use to communicate.
};
