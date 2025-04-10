#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
  cout << "smash: got ctrl-C" << endl;
  SmallShell& smash = SmallShell::getInstance();
  if (*(smash.curr_fg) == *(smash.smash_pid)){
      return;
  }
  if(*(smash.curr_fg) != -1) {
      //smash.jobs->removeJobById(smash.jobs->);
      if (kill(*(smash.curr_fg) , SIGKILL) == -1){
          perror("smash error: kill failed");
      }
      cout << "smash: process " << *(smash.curr_fg) << " was killed" << endl;
      // *(smash.curr_fg) = *(smash.smash_pid);
      return;
  }
  return;
}

//void alarmHandler(int sig_num) {
  // TODO: Add your implementation
//}

