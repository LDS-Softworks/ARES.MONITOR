#pragma once
#include <string>
#include <unistd.h>
#include <limits.h>

namespace ARES {
    namespace CORE {
        // There at some point will be more than just this. for now, this is simply to define stuff and not have weird bugs.

        namespace UTILS {
            /**
             * @brief Gets the absolute path of the currently running executable(ARES Monitor... expected.)
             * @return std::string Absolute path to the executable, or "ares" as a fallback if it fails.
             * @note This function uses the /proc/self/exe symlink, which is a Linux-specific feature. It may not work on non-Linux systems.
             */
            std::string get_self_path()
            {
                char buffer[PATH_MAX];
                ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
                if (len != -1) {
                    buffer[len] = '\0';
                    return std::string(buffer);
                }
                return "ares"; // fallback if something goes wrong
            }
        }
    }
}