
#ifndef COMMAND_RUNNER_H_INCLUDED
#define COMMAND_RUNNER_H_INCLUDED

#include "INTERFACE/TextBar.h"

struct CommandResult;

class CommandRunner : public TextBar {
public:
    CommandRunner();
    CommandResult run(void *adeptide);
};

struct CommandResult {
    bool successful;
    std::string message;

    CommandResult(bool successful, const std::string& message);
};

#endif
