#include <singular/Singular/libsingular.h>
#include <iostream>
#include <fstream>
#include "singular_functions.hpp"

int main() {
    // Initialize Singular
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

    // Open the file using ssi_open_for_read
    si_link file = ssi_open_for_read("waas_54549_1738847576961148731");
    if (file == nullptr) {
        std::cerr << "Failed to open file for reading!" << std::endl;
        return -1;
    }

    return 0;
}
