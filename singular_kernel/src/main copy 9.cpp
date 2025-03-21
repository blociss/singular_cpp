#include <singular/Singular/libsingular.h>
#include <iostream>
#include <fstream>
#include "singular_functions.hpp"

int main() {
    // Initialize Singular
    siInit((char *)"/home/atraore/Singular4/lib/libSingular.so");

    // Open the file using ssi_open_for_read
    si_link file = ssi_open_for_read("temp/waas_54549_1738847576961148731");
    if (file == nullptr) {
        std::cerr << "Failed to open file for reading!" << std::endl;
        return -1;
    }

    try {
        // Read the structure from the file (adjust field names as needed)
        lists m = ssi_read_newstruct(file, "generators");  // Accessing "generators" first
        if (!m) {
            std::cerr << "Failed to read 'generators' from file!" << std::endl;
            ssi_close_and_remove(file);
            return -1;
        }

        // Process the "generators" data
        std::cout << "Successfully read 'generators' data from file: " << std::endl;
        std::cout << m << std::endl;  // Adjust how you process this field

        // Now, access the "intersection_gpi" field
        lists intersection = ssi_read_newstruct(file, "intersection_gpi");
        if (!intersection) {
            std::cerr << "Failed to read 'intersection_gpi' from file!" << std::endl;
            ssi_close_and_remove(file);
            return -1;
        }

        // Process the "intersection_gpi" data
        std::cout << "Successfully read 'intersection_gpi' data from file: " << std::endl;
        std::cout << intersection << std::endl;  // Adjust how you process this field

        // Clean up
        ssi_close_and_remove(file);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        ssi_close_and_remove(file);
        return -1;
    }

    return 0;
}
