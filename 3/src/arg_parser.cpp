#include "arg_parser.h"
#include <iostream>
#include <string>
#include <cstdlib>

ProgramOptions parse_arguments(int argc, char* argv[], const char* program_name, bool is_mpi) {
    ProgramOptions opts;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-output" || arg == "--skip-csv") {
            opts.save_output = false;
        } else if (arg == "--help" || arg == "-h") {
            print_help(program_name, is_mpi);
            std::exit(0);
        }
    }
    
    return opts;
}

void print_help(const char* program_name, bool /* is_mpi */) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  --no-output, --skip-csv    Skip CSV output generation\n"
              << "  --help, -h                 Show this help message\n";
}

