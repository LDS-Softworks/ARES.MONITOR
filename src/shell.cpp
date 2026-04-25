// Include for ALL definitions that are NOT inside the int main() context.
// The reason this exist? to shut up the warning about #pragma once.
#include "../shell.hpp"

// ovbious main entry point. we might or might not need this... just saying, i
// is very self-explanatory.
int main() {
  init_system();
  std::cout << HLPMSG << std::endl;
  std::string line;
  bool condition_active = false; // are we inside an #IF block
  bool condition_met = false;    // did the condition pass

  while (true) {
    std::cout << "ARES &> ";
    if (!std::getline(std::cin, line))
      break;
    if (line.empty())
      continue;
    push_history(line);
    auto tokens = smart_tokenize(line);
    std::string cmd = tokens[0];

    // --- #IF / #ELSE BLOCK ---
    if (cmd == "\\#IF") {
      condition_active = true;
      if (line.find("RETURNS !0") != std::string::npos)
        condition_met = (last_error_code != 0);
      else
        condition_met = (last_error_code == 0);
      continue;
    }
    if (cmd == "\\#ELSE") {
      condition_met = !condition_met;
      continue;
    }
    if (cmd == "\\#ENDIF") {
      condition_active = false;
      condition_met = false;
      continue;
    }
    if (condition_active && !condition_met)
      continue; // skip line

    // --- VARIABLE ASSIGNMENT ---
    if (tokens.size() == 2 && cmd[0] != '\\' && cmd[0] != '@') {
      internal_vars[cmd] = tokens[1];
      continue;
    }

    expand_variables(tokens);

    if (commands.count(tokens[0])) {
      commands[tokens[0]](tokens);
    } else if (tokens[0][0] == '@') {
      run_external(tokens[0].substr(1), {tokens.begin() + 1, tokens.end()});
    } else {
      handle_syntax_punishment();
    }
  }
  return 0;
}