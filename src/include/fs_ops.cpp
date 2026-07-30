
// for FS stuff.
#include "std_glbl.hpp"
#include <filesystem>
#include <iomanip> // For pretty alignment
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#if defined (__android__) || defined(__ANDROID__)
#include <sstream>
#endif

namespace fs = std::filesystem;

namespace ARES {
// Flagging.
bool has_flag(const std::vector<std::string> &args, const std::string &flag) {
    return std::find(args.begin(), args.end(), flag) != args.end();
}
std::string current_path = "";
namespace IO {
void handle_displayCurrentWorkingDirectory(
    const std::vector<std::string> &args) {
  std::cout << "Current Path: " << ARES::current_path << std::endl;
}
namespace FileOperations {
// MARK: - Create File/Directory Logic
// Logic for @CREATE
void handle_create(const std::vector<std::string> &args) {
  if (args.size() < 3)
    return;
  // @CREATE FILE <Path> [WITH <Contents>]
  if (args[1] == "FILE") {
    std::ofstream file(args[2]);
    if (args.size() >= 5 && args[3] == "WITH") {
      file << args[4];
    }
    return;
  }

  // @CREATE DIR <Path>
  if (args[1] == "DIR" && args.size() == 3) {
    try {
      fs::create_directory(args[2]);
    } catch (fs::filesystem_error e) {
      std::string emsg = "[\\@CREATE DIR \\ E] [" + std::string(e.what()) + "]";
      session_errors.push_back(emsg);
    }
    return;
  }

  // @CREATE DIR STRUCTURE <Path>
  if (args[1] == "DIR" && args[2] == "STRUCTURE" && args.size() >= 4) {
    try {
      fs::create_directories(args[3]);
    } catch (fs::filesystem_error e) {
      std::string emsg =
          "[\\@CREATE DIR STRUCTURE \\ E] [" + std::string(e.what()) + "]";
      session_errors.push_back(emsg);
    }
    return;
  }
}

// MARK: - Delete File/Directory Logic
// Logic for @DELETE FILE <Path> | @DELETE DIR <Path> | @DELETE DIR RECURSIVELY
// <Path>
void handle_delete(const std::vector<std::string> &args) {
  if (args.size() < 3) {
    session_errors.push_back(
        "[DELETE]:[Syntax Error]:[Missing TYPE and PATH Sub-Operators]");
    return;
  }

  std::string type = args[1];

  // Case: @DELETE FILE <Path>
  if (type == "FILE") {
    if (!fs::exists(args[2])) {
      std::cerr << "[40433]:[FILE_NOT_FOUND] " << args[2] << std::endl;
      return;
    }
    // WIP: Deleting all files with a specific extension.
    // I KNOW that this is documented in the help message, i am still working on
    // it.
    if (args[3] == "WITHEXT") {
      // Syntax: @DELETE FILE <Dir> WITHEXT <.ext|*>
      if (args.size() < 5) {
        session_errors.push_back(
            "[DELETE]:[Syntax Error]:[WITHEXT requires an extension argument]");
        return;
      }

      std::string ext = args[4]; // e.g. ".png" or "*"
      fs::path target_dir = args[2];

      // Check if the target isn't a directory.
      if (!fs::is_directory(target_dir)) {
        std::cerr << "[4043]:[NOT_A_DIRECTORY] " << args[2] << std::endl;
        session_errors.push_back(
            "[DELETE]:[WITHEXT]:[Target path is not a directory]");
        return;
      }

      for (const auto &entry : fs::directory_iterator(
               target_dir, fs::directory_options::skip_permission_denied)) {
        try {
          if (!entry.is_regular_file())
            continue;

          // Wildcards and matching. for "WITHEXT".
          bool should_delete = (ext == "ANY" || ext == "EVERYTHING") ||
                               (entry.path().extension() == ext);

          // If the boolean is true, nuke it.
          if (should_delete) {
            try {
              fs::remove(entry.path());
            } catch (fs::filesystem_error e) {
              std::cout
                  << "An error occurred when removing the specified entry: '"
                  << std::string(e.what()) << "'" << std::endl;
              session_errors.push_back(
                  "[DELETE]:[WITHEXT]:[Could Not Delete Entry][SystemMessage:" +
                  std::string(e.what()) + "]");
            }
          }
        } catch (const fs::filesystem_error &e) {
          session_errors.push_back(
              "[DELETE]:[WITHEXT]:[Could Not Delete Entry]:[SystemMessage:" +
              std::string(e.what()) + "]");
        }
      }
      return;
    } else {
      try {
        fs::remove(args[2]);
      } catch (fs::filesystem_error e) {
        std::cout << "An error occurred deleting file " << args[2].c_str()
                  << " E:'" << e.what() << "'" << std::endl;
        session_errors.push_back("[DELETE]:[FILE]:[Error Deleting File]:[SystemMessage" +
                                 std::string(e.what()) + "]");
      }
    }
    return;
    // Finally added a proper close brace which for whatever reason i had NOT
    // seen until today. Still do not know how this went by me.
  }
  // Case: @DELETE DIR <Path> or @DELETE DIR RECURSIVELY <Path>
  if (type == "DIR") {
    // Subcase: RECURSIVELY
    if (args[2] == "RECURSIVELY" && args.size() >= 4) {
      try {
        fs::remove_all(args[3]);
      } catch (fs::filesystem_error e) {
        std::cout << "[DELETE:ERROR]:[E:" << std::string(e.what()) << "]"
                  << std::endl;
        session_errors.push_back("[Error Deleting.TYPE::" + type +
                                 "]:[E:" + std::string(e.what()) + "]");
      }
      return;
    }
    // Standard DIR delete (must be empty)
    try {
      fs::remove(args[2]);
    } catch (fs::filesystem_error e) {
      std::cout << "[DELETE]:[DIR]:[SystemMessage:" << std::string(e.what()) << "]"
                << std::endl;
      session_errors.push_back("[DELETE]:[DIR]:[SystemMessage:" + std::string(e.what()) + "]");
    }
    return;
  }
}
// NS:: IO::FileOperations::*
static bool path_exists(const std::string &path) {
  std::ifstream f(path);
  // std::cout << "[DEBUG path_exists] '" << path << "' = " << f.good()
  //           << std::endl;
  return f.good();
}

// MARK: - Change Directory Logic
// Logic for \\@CWD <path>
void handle_cwd(const std::vector<std::string> &args) {
  // Syntax Check
  if (args.size() < 2) {
    session_errors.push_back(
        "[CWD]:[Syntax Error]:[Missing PATH Sub-Operator]");
    ARES::CORE::HELP::handle_help({"", "\\@CWD"});
    *global_err_ptr += 1;
    return;
  }

  fs::path new_path = args[1];

  // Validation using C++17 Filesystem
  if (!fs::exists(new_path)) {
    std::cerr << "[4083]:[PATH_NOT_FOUND] " << args[1] << std::endl;
    session_errors.push_back("[CWD]:[No Such Entry]:[\"" + args[1] + "\"]");
    *global_err_ptr += 1;
    return;
  }

  if (!fs::is_directory(new_path)) {
    std::cerr << "[4043]:[NOT_A_DIRECTORY] " << args[1] << std::endl;
    session_errors.push_back("[CWD]:[TRYING TO TREAT FILE ENTRY AS DIRECTORY IS NOT VALID BEHAVIOR]:[\"" + args[1] + "\"]");
    *global_err_ptr += 1;
    return;
  }

  // Perform the move
  try {
    fs::current_path(new_path);
    ARES::current_path = fs::current_path();
    std::cout << "New Working Directory:" << current_path << std::endl;
  } catch (const fs::filesystem_error &e) {
    std::cerr << "[Change Working Directory:Error]:[" << std::string(e.what())
              << "]" << std::endl;
    session_errors.push_back(
        "[CWD]:[SECINT]:[SystemMessage:" + std::string(e.what()) + "]");
    *global_err_ptr += 1;
  }
}

// MARK: - List Directory Contents Logic
// Logic for @LDC [Path]
void handle_ldc(const std::vector<std::string> &args) {
  // If args[1] exists and isn't the EVERYTHING flag, use it as path
  fs::path target_path =
      (args.size() > 1 && args[1] != "EVERYTHING") ? fs::path(args[1]) : fs::current_path();

  bool show_all = ARES::has_flag(args, "EVERYTHING");

  if (!fs::exists(target_path)) {
    std::cerr << "[LDC]:[PATH_NOT_FOUND] " << target_path << std::endl;
    session_errors.push_back("[LDC]:[ERROR]:[Target entry does NOT exist or is restricted]");
    return;
  }
 
  std::cout << "[LISTING]: " << fs::absolute(target_path) << "\n";
  std::cout << "------------------------------------------\n";
  for (const auto &entry : fs::directory_iterator(
           target_path,
           fs::directory_options::skip_permission_denied)) {
    std::string filename = entry.path().filename().string();

    // Skip dotfiles unless EVERYTHING is specified
    if (!show_all && !filename.empty() && filename[0] == '.') continue;

    std::string type_label = entry.is_directory() ? "[D] " : "[F] ";
    std::string size_info = "<DIR>";

    try {
      // ONLY call file_size if it is NOT a directory
      if (!entry.is_directory()) {
        const long long KB = 1024;
        const long long MB = KB * 1024;
        const long long GB = MB * 1024;
        const long long TB = GB * 1024;

        std::string size_measure = " B";
        double final_size = static_cast<double>(entry.file_size());

        if (entry.file_size() >= TB) {
          final_size /= TB;
          size_measure = " TB"; // We didn't really needed this... but... LDS pushed me to add it anyways. who tf has files reading fucking terabytes? fucking Government institutions? and why the fuck would they use ARES?
        } else if (entry.file_size() >= GB) {
          final_size /= GB;
          size_measure = " GB";
        } else if (entry.file_size() >= MB) {
          final_size /= MB;
          size_measure = " MB";
        } else if (entry.file_size() >= KB) {
          final_size /= KB;
          size_measure = " KB";
        }

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << final_size << size_measure;
        size_info = ss.str();
      }

      std::cout << type_label << std::left << std::setw(20)
                << filename << " | " << size_info << "\n";
    } catch (const std::exception &e) {
      session_errors.push_back(
          "[LDC]:[Could Not Read Entry]:[SystemMessage:" +
          std::string(e.what()) +
          "]"
      );
    }
  }
  std::cout << "------------------------------------------" << std::endl;
}
} // namespace FileOperations
} // namespace IO
} // namespace ARES