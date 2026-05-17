#include "std_glbl.hpp"
#include <iostream>
#include <ostream>

namespace ARES::RTE::ENV {
void getErrors(const std::vector<std::string>& args)
{
    if (session_errors.empty())
    {
        std::cout << "No Errors have been reported." << std::endl;
        return;
    }

    for (size_t i = 0; i < session_errors.size(); i++)
    {
        std::cout << "[E]: '" << session_errors[i] << "'" << std::endl;
    }
}
}