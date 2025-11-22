#include "arg_parser.h"
#include <iostream>
#include <string>
#include <cstdlib>

ProgramOptions parse_arguments(int argc, char* argv[], const char* program_name) {
    ProgramOptions opts;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-output" || arg == "--skip-csv") {
            opts.save_output = false;
        } else if (arg == "--help" || arg == "-h") {
            print_help(program_name, true);
            std::exit(0);
        } else if (arg[0] != '-') {
            opts.input_file = argv[i];
        }
    }
    
    return opts;
}

void print_help(const char* program_name, bool has_input_arg) {
    std::cout << "Usage: " << program_name << " [options]";
    if (has_input_arg) {
        std::cout << " [input_csv]";
    }
    std::cout << "\nOptions:\n"
              << "  --no-output, --skip-csv    Skip CSV output generation\n"
              << "  --help, -h                 Show this help message\n";
    if (has_input_arg) {
        std::cout << "Arguments:\n"
                  << "  input_csv                  Load initial conditions from CSV file\n";
    }
}

