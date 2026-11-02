#pragma once
#include "std_glbl.hpp"
#include "sys/wait.h"
#include <cstring>

// REFACTOR: This file handles externals, it is recommended to have it.
//           Tho you can remove it, but you will lose all external executables in your device.
//           No, there is no fallback or way to force ARES to execute externals without this.
namespace ARES::RTE::EXTERNALS {
/**
 * Core Execution Logic
 * Wraps the standard fork/exec but tracks errors in our Anti-POSIX global
 * state.
 */
void run_external(const std::string &binary,
                  const std::vector<std::string> &args)
                  {
  pid_t pid = fork();

  if (pid == 0) { // Child Process
    std::vector<char *> c_args;
    c_args.push_back(const_cast<char *>(binary.c_str()));
    for (const auto &arg : args) {
      c_args.push_back(const_cast<char *>(arg.c_str()));
    }
    c_args.push_back(nullptr);

    execvp(binary.c_str(), c_args.data());

    // If execvp reaches here, the command wasn't found
    std::cerr << "[4083]:[BINARY_NOT_FOUND] " << binary << std::endl;
    exit(127);
  }

  if (pid > 0) { // Parent Process
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
      *global_err_ptr += 1;
      session_errors.push_back("External Operand '" + binary +
                               "' failed with status " +
                               std::to_string(WEXITSTATUS(status)));
    }
  }
}

// Same as above, but it returns the output of the command as a string.
/**
 * PD: Had to make this bullshit a fucking new definition because i am NOT fighting fucking GNU CSTD.
 *
 * @param binary Binary to invoke from path.
 * @param arguments Arguments to pass to the invoked binary.
 */
std::string execute_inline(
    const std::string &binary,
    const std::vector<std::string> &args)
{
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        session_errors.push_back(
            "Could not capture command output: " +
            std::string(std::strerror(errno)));
        ++global_err_ptr;
        last_error_code = 309;
        return {};
    }

    pid_t pid = fork();

    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);

        session_errors.push_back(
            "Could not fork external command: " +
            std::string(std::strerror(errno)));
        ++global_err_ptr;
        last_error_code = 309;
        return {};
    }

    if (pid == 0) {
        // Child process: redirect stdout and stderr to the pipe.
        close(pipefd[0]);

        if (dup2(pipefd[1], STDOUT_FILENO) == -1 ||
            dup2(pipefd[1], STDERR_FILENO) == -1) {
            _exit(126);
        }

        close(pipefd[1]);

        std::vector<char *> c_args;
        c_args.reserve(args.size() + 2);

        c_args.push_back(const_cast<char *>(binary.c_str()));

        for (const auto &arg : args) {
            c_args.push_back(const_cast<char *>(arg.c_str()));
        }

        c_args.push_back(nullptr);

        execvp(binary.c_str(), c_args.data());

        // execvp failed. Avoid using iostreams after fork.
        static constexpr char msg[] =
            "[4083]:[BINARY_NOT_FOUND] execvp failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);

        _exit(errno == ENOENT ? 127 : 126);
    }

    // Parent process: capture output until the child closes the pipe.
    close(pipefd[1]);

    std::string output;
    char buffer[4096];

    for (;;) {
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer));

        if (bytes_read > 0) {
            output.append(buffer, static_cast<std::size_t>(bytes_read));
        } else if (bytes_read == 0) {
            break; // EOF
        } else if (errno == EINTR) {
            continue; // Interrupted; retry.
        } else {
            session_errors.push_back(
                "Could not read command output: " +
                std::string(std::strerror(errno)));
            ++global_err_ptr;
            last_error_code = 309;
            break;
        }
    }

    close(pipefd[0]);

    int status = 0;
    pid_t waited;

    do {
        waited = waitpid(pid, &status, 0);
    } while (waited == -1 && errno == EINTR);

    if (waited == -1) {
        session_errors.push_back(
            "Could not wait for external command: " +
            std::string(std::strerror(errno)));
        ++global_err_ptr;
        last_error_code = 309;
        return output;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        ++global_err_ptr;

        session_errors.push_back(
            "External Operand '" + binary +
            "' failed with status " +
            std::to_string(WEXITSTATUS(status)));
    } else if (WIFSIGNALED(status)) {
        ++global_err_ptr;

        session_errors.push_back(
            "External Operand '" + binary +
            "' terminated by signal " +
            std::to_string(WTERMSIG(status)));
    }

    return output;
}


// Handler for \@EXEC <command>, same as @. they do the same thing, just
// different prefixes.
void handle_exec(const std::vector<std::string> &args)
{
  if (args.size() < 2) {
    session_errors.push_back(
        "[EXEC] : [Syntax Error] : [\\@EXEC :: requires a target binary]");
    return;
  }

  // Extract everything after \@EXEC
  std::vector<std::string> pass_through(args.begin() + 2, args.end());
  ARES::RTE::EXTERNALS::run_external(args[1], pass_through);
}
} // namespace ARES::RTE::EXTERNALS