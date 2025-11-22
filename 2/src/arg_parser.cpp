#include "arg_parser.h"
#include <iostream>
#include <string>
#include <cstdlib>
#ifdef USE_MPI
#include <mpi.h>
#endif

ProgramOptions parse_arguments(int argc, char* argv[], const char* program_name, bool is_mpi) {
    ProgramOptions opts;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--no-output" || arg == "--skip-csv") {
            opts.save_output = false;
        } else if (arg == "--help" || arg == "-h") {
            print_help(program_name, is_mpi);
            if (is_mpi) {
#ifdef USE_MPI
                MPI_Finalize();
#endif
            }
            std::exit(0);
        } else if (arg[0] != '-') {
            opts.input_file = argv[i];
        }
    }
    
    return opts;
}

void print_help(const char* program_name, bool is_mpi) {
    if (is_mpi) {
        std::cout << "Usage: mpirun -np <procs> " << program_name;
    } else {
        std::cout << "Usage: " << program_name;
    }
    std::cout << " [options] [input_csv]\n"
              << "Options:\n"
              << "  --no-output, --skip-csv    Skip CSV output generation\n"
              << "  --help, -h                 Show this help message\n"
              << "Arguments:\n"
              << "  input_csv                  Load initial conditions from CSV file\n";
}

