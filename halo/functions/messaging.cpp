/*
 * Vulpes (c) 2019 gbMichelle
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "messaging.hpp"
#include "../../hooker/hooker.hpp"
#include "../memory/gamestate/console.hpp"
#include "message_delta.hpp"
#include <cstdio>
#include <cstring>

Signature(true, sig_console_line_new,
    {0x8B, 0x15, -1, -1, -1, -1, 0x66, 0x83, 0x7A, 0x2E, 0x20, 0x75, 0x11});
Signature(false, sig_console_to_terminal_and_network,
    {0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8, 0x81, 0xEC, 0x10, 0x01, 0x00, 0x00, 0x56});

typedef uint32_t (*FpReturnsInt)();

ConsoleOutput* console_new_line(){
    ConsoleGlobals* globals = console_globals();
    static auto console_line_new =
        reinterpret_cast<FpReturnsInt>(sig_console_line_new.address());
    MemRef line;
    line.raw = console_line_new();

    if (line.raw != 0xFFFFFFFF){
        ConsoleOutput* output = &globals->output->entries[line.id.local];
        output->fade_frames = 0;
        return output;
    }else{
        return NULL;
    };
}

typedef __attribute__((regparm(1)))void (*ConsoleToTerminalAndNetworkCall)(void*);

void vcprintf(const ARGBFloat& color, const char* format, va_list args){
    static auto console_to_terminal_and_network =
        reinterpret_cast<ConsoleToTerminalAndNetworkCall>(
        sig_console_to_terminal_and_network.address());
    ConsoleGlobals* globals = console_globals();
    if (globals->initialized){
        ConsoleOutput* output = console_new_line();
        if (output){
            vsnprintf(&output->text[0], 255, format, args);
            output->color.alpha = color.alpha;
            output->color.red   = color.red;
            output->color.green = color.green;
            output->color.blue  = color.blue;
            output->tab_stops   = strstr(output->text, "|t") != NULL;
            console_to_terminal_and_network(&output->text);
        };
    };
}

void cprintf(const ARGBFloat& color, const char* format, ...){
    va_list args;
    va_start(args, format);
    vcprintf(color, format, args);
    va_end(args);
}

void cprintf(const char* format, ...){
    va_list args;
    va_start(args, format);
    vcprintf(ARGBFloat(1.0, 0.7, 0.7, 0.7), format, args);
    va_end(args);
}

void cprintf_info (const char* format, ...){
    va_list args;
    va_start(args, format);
    vcprintf(ARGBFloat(1.0, 0.0, 1.0, 0.0), format, args);
    va_end(args);
}

void cprintf_warn (const char* format, ...){
    va_list args;
    va_start(args, format);
    vcprintf(ARGBFloat(1.0, 1.0, 0.835, 0.0), format, args);
    va_end(args);
}

void cprintf_error(const char* format, ...){
    va_list args;
    va_start(args, format);
    vcprintf(ARGBFloat(1.0, 1.0, 0.0, 0.0), format, args);
    va_end(args);
}

void rprintf(int player_id, const char* format, ...){
    va_list args;
    va_start(args, format);
    char buffer[100];
    RconResponse message;
    vsnprintf(&message.text[0], 80, format, args);
    uint32_t size = mdp_encode_stateless_iterated(buffer, RCON_RESPONSE, &message);
    send_delta_message_to_player(player_id, &buffer, size, true, true, false, true, 3);
    va_end(args);
}

void rprintf_all(const char* format, ...){
    va_list args;
    va_start(args, format);
    char buffer[100];
    RconResponse message;
    vsnprintf(&message.text[0], 80, format, args);
    uint32_t size = mdp_encode_stateless_iterated(buffer, RCON_RESPONSE, &message);
    send_delta_message_to_all(&buffer, size, true, true, false, true, 3);
    va_end(args);
}