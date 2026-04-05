#include "std_glbl.hpp"
#include <iostream>
#include <ostream>

void getErrors(const std::vector<std::string>& args) {
    int error_size = session_errors.size();
    if (error_size <= 0)
    {
        std::cout << "No Errors have been reported." << std::endl;
        return;
    }
    for (int i = 0; i < error_size ; i++)
    {
        std::cout << "[E]: '" << session_errors[i] << "'" << std::endl;
    }
    return;
}