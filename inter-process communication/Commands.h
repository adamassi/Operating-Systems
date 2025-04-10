#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <list>
#include <regex>
#include <queue>
#include <deque>




#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using namespace std;

typedef enum {
  BuiltIn,
  External} Command_type;

  enum Pipe_kind{
    COUT_PIPE,
    CERR_PIPE,
    NOT_PIPE
};

class Command {
// TODO: Add your data members
 public:
  Command(const char* cmd_line, int new_dest, int old_dest);
  const char* cmd_line;
  int n_args;
  char** args;
  string instruct;
  int new_dest;
  int old_dest;
  virtual ~Command();
  //virtual void execute();
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed

};
//void Command::execute(){
//  cout << "BuiltInCommand::execute()" << endl;
//}


class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line, int new_dest,int old_dest);
  virtual ~BuiltInCommand() {}
  //void execute() override;
};
//void BuiltInCommand::execute(){
//  cout << "BuiltInCommand::execute()" << endl;
//}


class ExternalCommand : public Command {
 public:
  pid_t pid;
  string original_trim_cmd_line;
  ExternalCommand(const char* cmd_line, const char* original_trim_cmd_line,int new_dest, int old_dest );
  bool isFinished();
  virtual ~ExternalCommand() {}
  //void execute() override;
};

//void ExternalCommand::execute(){
//  cout << "BuiltInCommand::execute()" << endl;
//}

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  //void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  //void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  //void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  //void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  //void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  //void execute() override;
};




class JobsList {
 public:
  class JobEntry {
    public:
      ExternalCommand* cmd;
      char* string_to_delete;
      //bool isStopped = false;
      int job_id; //
   // TODO: Add your data members
    JobEntry(ExternalCommand* cmd, char* string_to_delete) : cmd(cmd), string_to_delete(string_to_delete)  {}
  };
 // TODO: Add your data members
 public:
  list<JobEntry*> all_jobs_list;
  JobsList();
  ~JobsList();
  void addJob(ExternalCommand* cmd,char* string_to_delete);
  void printJobsList();
  void killAllJobs(bool send_signal_and_print);
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  bool isJobInTheList(int jobId);
  void removeJobById(int jobId);
  int getMaxJobId();
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  //void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  //void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  //void execute() override;
};

class ChmodCommand : public BuiltInCommand {
 public:
  ChmodCommand(const char* cmd_line);
  virtual ~ChmodCommand() {}
  //void execute() override;
};


class SmallShell {
 private:
  
  // TODO: Add your data members
  SmallShell();
 public:
  string prompt;
  string last_work_dir;
  bool alias_com=false;
  string to_ins;
  //vector<string> alias_vec;
  const pid_t* smash_pid;
  pid_t* curr_fg; //PID
  JobsList* jobs;
  //std::queue<string> alias_que;
  std::map<string,string> alias_map;
  //vector<map<string,string>> alias_vec;
  
  //vector<int> al;
  //list<int> ll;


  

  Command *CreateCommand(const char* cmd_line, int new_dest, int old_dest);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  void printMapList();

  ~SmallShell();
  void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
  void deleteSpecificItem(string s);
  void printQueue(std::queue<string> q);
    
};
// struct linux_dirent {
//     unsigned long  d_ino;     // Inode number
//     unsigned long  d_off;     // Offset to next linux_dirent
//     unsigned short d_reclen;  // Length of this linux_dirent
//     char           d_name;  // Filename (null-terminated)
// };
// struct Entry {
//     std::string type;
//     std::string name;
//     std::string target;  // For symbolic links
// };

//SmallShell::SmallShell() {}
//SmallShell::~SmallShell() {}

#endif //SMASH_COMMAND_H_
