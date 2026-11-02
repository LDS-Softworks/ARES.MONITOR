#pragma once

// Logic for @ENV <VARNAME> <Value>
#include <algorithm>
#include <any>
#include <cstddef>
#include <iostream>
#include <ostream>
namespace ARES {
namespace IO {
void handle_replace(const std::vector<std::string> &args) {
  // Check for @REPLACE <Word> <File> WITH <NewWord> (args size 5)
  if (args.size() < 5 || args[3] != "WITH") {
    session_errors.push_back(
        "[REPLACE] : [Syntax Error: @REPLACE <Find> <File> WITH <Replace>]");
    return;
  }

  std::string find_word = args[1];
  std::string file_path = args[2];
  std::string replace_word = args[4];

  std::ifstream in(file_path);
  if (!in) {
    session_errors.push_back("[REPLACE]:[Error]:[Cannot open entry: \"" +
                             file_path + "\" ]");
    return;
  }

  // Read whole file
  std::string content((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
  in.close();

  size_t pos = content.find(find_word);

  // @REPLACE (First occurrence only)
  if (pos != std::string::npos) {
    content.replace(pos, find_word.length(), replace_word);
  }

  std::ofstream out(file_path);
  out << content;
}
} // namespace IO
namespace RTE::ENV {
void handle_env(const std::vector<std::string> &args) {
  if (args.size() < 3) {
    session_errors.push_back("[ENV]:[Syntax Error]:: requires NAME and VALUE.");
    ++(*global_err_ptr);
    return;
  }

  const std::string &name = args[1];
  std::string value;

  if (args[2] == "\\&") {
    // Command substitution: ENV NAME \& BINARY ARG...
    if (args.size() < 4) {
      session_errors.push_back(
          "[ENV]:[Syntax Error]:: expected a binary after \\&.");
      ++(*global_err_ptr);
      return;
    }

    const std::string &binary = args[3];

    std::vector<std::string> subArgs(args.begin() + 4, args.end());

    value = ARES::RTE::EXTERNALS::execute_inline(binary, subArgs);
  } else {
    // Literal assignment: ENV NAME VALUE
    value = args[2];
  }

  // Set the process environment to the resolved value.
  if (setenv(name.c_str(), value.c_str(), 1) != 0) {
    session_errors.push_back(
        "[ENV]:[Error]:: Could not set environment variable '" + name + "'.");
    ++(*global_err_ptr);
    return;
  }

  // Store the resolved value in ARES's internal environment.
  ARES::RTE::ENV::internal_vars[name] = value;

  std::cout << "[ENV]: Set " << name << " to " << value << std::endl;
}

// this one is the same as @\033NV, but better, because it doesn't need the
// @\ prefix.

void expand_variables(std::vector<std::string> &tokens) {
  for (auto &token : tokens) {
    size_t pos = 0;

    while ((pos = token.find('%', pos)) != std::string::npos) {
      size_t start = pos + 1;
      size_t end = start;

      // variable name = letters/numbers/underscore
      while (end < token.size() && (isalnum(token[end]) || token[end] == '_')) {
        end++;
      }

      std::string var_name = token.substr(start, end - start);

      if (ARES::RTE::ENV::internal_vars.count(var_name)) {
        std::string value = ARES::RTE::ENV::internal_vars[var_name];
        token.replace(pos, end - pos, value);
        pos += value.size(); // continue after inserted text
      } else {
        pos = end; // skip unknown var
      }
    }
  }
}

// Find a specific variable by name
std::string find_var(const std::string &name) {
  auto it = ARES::RTE::ENV::internal_vars.find(name);
  if (it != ARES::RTE::ENV::internal_vars.end()) {
    return it->second; // found, return value
  }
  return ""; // not found
}

void getVariables(const std::vector<std::string> &args) {
  bool prettyPrint = args.size() >= 2 && args[1] == "PRETTY";

  if (args.size() < 2 || prettyPrint) {
    // Dump all variables
    bool first = true;
    for (const auto &pair : ARES::RTE::ENV::internal_vars) {
      if (prettyPrint) {
        if (first)
          std::cout << "[ENVIRONMENT:ALL]" << std::endl;
        if (!first)
          std::cout << ":";
        std::cout << "'" << pair.first << "'='" << pair.second << "'";
        first = false;
      } else {
        std::cout << "[ENV]: '" << pair.first << "' = '" << pair.second << "'"
                  << std::endl;
      }
    }
    if (prettyPrint)
      std::cout << std::endl;
  } else {
    // Specific variable requested
    std::string val = find_var(args[1]);
    if (val.empty()) {
      std::cout << "[ENV]: Variable '" << args[1] << "' not found."
                << std::endl;
    } else {
      std::cout << "[ENV:VAR]: '" << args[1] << "' = '" << val << "'"
                << std::endl;
    }
  }
}
} // namespace RTE::ENV
} // namespace ARES