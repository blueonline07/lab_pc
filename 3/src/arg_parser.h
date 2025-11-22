#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <string>

struct ProgramOptions {
    bool save_output = true;
    const char* input_file = nullptr;
};

ProgramOptions parse_arguments(int argc, char* argv[], const char* program_name, bool is_mpi = false);
void print_help(const char* program_name, bool is_mpi = false);

#endif // ARG_PARSER_H

