#pragma once
#include <ostream>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <unistd.h>

#ifndef STD_GLBL_HPP
#pragma once
#define STD_GLBL_HPP

// Version information.
const std::string ARES_VERSION = "0.0.13-alpha";

#include <termios.h>

inline struct termios orig_termios;

inline void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // no echo, read char by char
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

inline void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Help Message Constant, for in case you MESS UP THE SYNTAX.
// Read the docks ffs.
const std::string HLPMSG =
    "Available commands:\n"
    "  \\@WRITE <content> [TO <file>]   - Write content to console or file\n"
    "  \\@WRITE FROM <file> [TO <file>] - Read from file and optionally write to another file\n"
    "  \\@HLT                           - Terminate the session and report errors\n"
    "  @<Binary or Binary Path>         - Executes a system binary using the built-in system shell(POSIX Shell Operand)\n"
    "-- For more information, use \\@HELP ALL --\n"
    ;
    // READ THE DOCKS!!

inline int history_index = -1;
inline std::vector<std::string> command_history;
inline void push_history(const std::string& cmd) {
    if (!cmd.empty()) {
        command_history.push_back(cmd);
        history_index = command_history.size(); // reset to end
    }
}
inline std::string get_previous_command() {
    if (command_history.empty()) return "";
    if (history_index > 0) history_index--;
    return command_history[history_index];
}

// Store variables in memory:
inline std::unordered_map<std::string, std::string> internal_vars;
// that's the pointer we store session vars into.

// Global error tracking
inline std::vector<std::string> session_errors;

inline int noposix_error_counter = 0;
inline unsigned long long* global_err_ptr = nullptr;
inline int last_error_code = 0; // initialize last error code, for future use.

const unsigned long long MEM_LIMIT = 0xFFFFFF; // Memory cap for Anti-POSIX error tracking, about 16MB... because, i know you'll try to HEAP_OVERFLOW it.

// Note: \@EXEC is added here, while prefix '@' is handled in main, why? fuck if i know, it's how i made it work
typedef std::function<void(const std::vector<std::string>&)> CommandFunc;

extern void handle_write(const std::vector<std::string>& args);
extern void handle_halt(const std::vector<std::string>& args);
extern void handle_exec(const std::vector<std::string>& args);
extern void handle_env(const std::vector<std::string>& args);
extern void handle_replace(const std::vector<std::string>& args);
extern void handle_help(const std::vector<std::string>& args);
extern void handle_kill(const std::vector<std::string>& args);
extern void handle_last_err(const std::vector<std::string>& args);
extern void handle_ldc(const std::vector<std::string>& args);
extern void handle_cwd(const std::vector<std::string>& args);
extern void handle_delete(const std::vector<std::string>& args);
extern void handle_create(const std::vector<std::string>& args);
extern void execute_Ares_Automation(const std::vector<std::string>& args);
extern void getErrors(const std::vector<std::string>& args);
extern void getVariables(const std::vector<std::string>& args);
void clearTerminalContents(const std::vector<std::string>& args){
    std::cout << "\033[2J\033[H" << std::endl;
}
// Duspatcher map, that's it, this has all the commands so DON'T TOUCH IT!.
#pragma once
std::unordered_map<std::string, CommandFunc> commands = {
    {"\\@WRITE", handle_write},
    {"\\@HALT", handle_halt},
    {"\\@HLT", handle_halt},
    {"\\@EXEC", handle_exec},
    {"\\@CREATE", handle_create},
    {"\\@DELETE", handle_delete},
    {"\\@ENV", handle_env},
    {"\\@CWD", handle_cwd},
    {"\\@LDC", handle_ldc},
    {"\\@REPLACE", handle_replace},
    {"\\@KILL", handle_kill},
    {"\\@HELP", handle_help},
    {"\\@KILALL", handle_kill}, // Mapping to same handler, because i am not rewriting this fucking logic, suck it up. - Lilly Aizawa.
    {"\\@AEX", execute_Ares_Automation},
    {"\\@CTC", clearTerminalContents},
    {"\\!?", handle_last_err},
    {"\\*?", getErrors},
    {"\\%?", getVariables},
    // Aliases for stuff we might use a lot.
    {"\\@%?", getVariables},
    {"\\@*?", getErrors},
    {"\\@!?", handle_last_err}
};
#endif