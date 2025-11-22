#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <string>

struct ProgramOptions {
    bool save_output = true;
    const char* input_file = nullptr;
};

ProgramOptions parse_arguments(int argc, char* argv[], const char* program_name);
void print_help(const char* program_name, bool has_input_arg = true);

#endif // ARG_PARSER_H

