#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"



int main(int argc, char* argv[]) {
    
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler
    SmallShell& smash = SmallShell::getInstance();
    //std::streambuf* backup = std::cout.rdbuf();
    //int stdout = dup(STDOUT_FILENO);
    while(true) {
        //std::cout.rdbuf(backup);
        //std::cout << "smash> ";
        *(smash.curr_fg) = getpid();
        std::cout << smash.prompt;
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}