#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <sys/stat.h>
#include "Commands.h"
#include <map>
#include <cstring>
#include <sys/types.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>




using namespace std;
#define BUF_SIZE 1024

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  std::stringstream ss;
  bool previous_is_space = false;
    for (char c : s) {
      if (isspace(c)) {
          if (!previous_is_space) {
              ss << ' ';
          }
            previous_is_space = true;
        } else {
            ss << c;
            previous_is_space = false;
        }
    }
    return _rtrim(_ltrim(ss.str()));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}
void SmallShell::printMapList(){
  for (const auto& pair : alias_map) {
        std::cout << pair.first << "='" << pair.second <<"'"<< std::endl;
    }
  
}
void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  size_t idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  cmd_line[idx] = '\0';
  //////////ANIGAIL_Y/////////
  // //cout<<cmd_line<<endl;
  // // replace the & (background sign) with space and then remove all tailing spaces.
  // cmd_line[idx] = ' ';
  // // truncate the command line string up to the last non-space character
  // //cout<<cmd_line<<endl;
  // cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}


JobsList::JobsList(){}
JobsList::~JobsList(){
  this->removeFinishedJobs();
  //TODO: check what to do with all the others jobs.....
}
typedef enum
      {
        FILES = 0,
        DIRS = 1,
        LINKS = 2,
        TYPES_AMOUNT = 3,
      } TYPES;


// TODO: Add your implementation for classes in Commands.h 
void print_sorted(const std::vector<std::string>& vec, const std::string& label) {
        std::vector<std::string> sorted_vec = vec;
        std::sort(sorted_vec.begin(), sorted_vec.end());
        for (const auto& entry : sorted_vec) {
            std::cout << label << ": " << entry << std::endl;
        }
    };
void JobsList::removeFinishedJobs(){
  if (this->all_jobs_list.empty()){
    return;
  }
  for (auto it = this->all_jobs_list.begin(); it != this->all_jobs_list.end(); ) {
    if ((*it)->cmd && (*it)->cmd->isFinished()){
      free((*it)->string_to_delete);
      delete (*it)->cmd;
      delete (*it);
      it = all_jobs_list.erase(it);
    }else{
      ++it;
    }
  }
}
///////////////////////
//////////pipes////////
string CutpipeCommandr( std::string cmd_line1,Pipe_kind pipekind , string r_pipe)
{
    //string cut_string= (string)cmd_line;
    const char* operand;
    int plus;
    if( pipekind == COUT_PIPE){
        operand="|";
        plus = 1;
        
    }
    else if( pipekind == CERR_PIPE) {
        operand="|&";
        plus = 2;
    }
    // l_pipe = (cmd_line1).substr( 0 , (cmd_line1).find(operand));
    r_pipe = (cmd_line1).substr((cmd_line1).find(operand) + plus);
    r_pipe = _trim(r_pipe);
    return r_pipe;
}
string CutpipeCommandl( std::string cmd_line1,Pipe_kind pipekind , string l_pipe)
{
    //string cut_string= (string)cmd_line;
    const char* operand;
    
    if( pipekind == COUT_PIPE){
        operand="|";
       
        
    }
    else if( pipekind == CERR_PIPE){
        operand="|&";
    }
    l_pipe = (cmd_line1).substr( 0 , (cmd_line1).find(operand));
    // r_pipe = (cmd_line1).substr((cmd_line1).find(operand) + plus);
    // r_pipe = _trim(r_pipe);
    return l_pipe;
}


Pipe_kind is_pipe(const char* cmd_line,Pipe_kind* kind )
{
    std::string check_if_pipe = string(cmd_line);
    if( check_if_pipe.find("|&") != std::string::npos) {
        (*kind) = CERR_PIPE;
        return CERR_PIPE;
    }
    else if(check_if_pipe.find("|") != std::string::npos){
       
       (*kind) = COUT_PIPE;
        return COUT_PIPE;
    }
    
    (*kind) = NOT_PIPE;
    return NOT_PIPE;
    
}



void JobsList::addJob(ExternalCommand* cmd, char* string_to_delete){
  this->removeFinishedJobs();
  JobEntry* new_job = new JobEntry(cmd, string_to_delete);
  new_job->job_id = getMaxJobId() + 1;
  all_jobs_list.push_back(new_job);
}

JobsList::JobEntry* JobsList::getJobById(int jobId){
  for (auto& it : this->all_jobs_list){
    if (it->job_id == jobId){
      return it;
    }
  }
  return nullptr;
}

bool JobsList::isJobInTheList(int jobId){
  JobsList::JobEntry* found_job = this->getJobById(jobId);
  if (found_job == nullptr){
    return false;
  }
  return true;
}

void JobsList::removeJobById(int jobId){
  if (this->all_jobs_list.empty()){
    return;
  }
  for (auto it = this->all_jobs_list.begin(); it != this->all_jobs_list.end(); ) {
    if ((*it)->job_id == jobId){
      free((*it)->string_to_delete);
      delete (*it)->cmd;
      delete (*it);
	    it = all_jobs_list.erase(it);
      break;
    }
    ++it;
  }
}

bool compareByJobID(const JobsList::JobEntry* a, const JobsList::JobEntry* b) {
    return a->job_id < b->job_id;
}

int JobsList::getMaxJobId(){
  if (this->all_jobs_list.empty()){
    return 0;
  }
  this->all_jobs_list.sort(compareByJobID);
  return this->all_jobs_list.back()->job_id;
}

void JobsList::killAllJobs(bool send_signal_and_print){
  for (auto it = this->all_jobs_list.begin(); it != this->all_jobs_list.end(); ) {
    if (send_signal_and_print){
        cout << (*it)->cmd->pid << ": " << (*it)->cmd->original_trim_cmd_line << endl;
        if (kill((*it)->cmd->pid, SIGKILL) == -1){
           perror("smash error: kill failed");
        }
    }

    //cout<<'h'<< endl;
    free((*it)->string_to_delete);
    delete (*it)->cmd;
    delete (*it);
    it = all_jobs_list.erase(it);
  }
}

void JobsList::printJobsList(){
  this->removeFinishedJobs();
  this->all_jobs_list.sort(compareByJobID);
  for (auto& it : this->all_jobs_list){
    cout << "[" << it->job_id << "] " << it->cmd->original_trim_cmd_line << endl;
  }
}

SmallShell::SmallShell() : prompt("smash> "), last_work_dir(""){
  smash_pid = new pid_t(getpid());
  curr_fg = new pid_t((getpid()));
  jobs = new JobsList();
  //alias_que = std::queue<string>();
  //alias_map=std::map<string,string>();
  //alias_que = //   alias_que.push("default_value1");
//   alias_que.push("default_value2");
}

SmallShell::~SmallShell() {
  delete smash_pid;
  delete curr_fg;
  delete jobs;
  //delete alias_que=delete;
  // while (!alias_que.empty())
  // {
  //   alias_que.pop();
  //   /* code */
  // }
  
 //TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/

Command::Command(const char* cmd_line, int new_dest, int old_dest){
      this->cmd_line = cmd_line;
      this->new_dest = new_dest;
      this-> old_dest = old_dest;
      args = (char**)malloc(COMMAND_MAX_ARGS*sizeof(char*));
      n_args =_parseCommandLine(cmd_line, args) - 1;
      instruct = string(args[0]);
}

Command::~Command(){
  if (this->new_dest != -1){
    if (close(new_dest) == -1) {
      perror("smash error: close failed");
    }
    if (this->old_dest != -1){
      if (dup2(old_dest, STDOUT_FILENO) == -1) {
        perror("smash error: dup2 failed");
      }
    }
  }
  for(int i = 0; i < n_args+1; i++){
    free(args[i]);
  }
  free(args);
}

BuiltInCommand::BuiltInCommand(const char* cmd_line, int new_dest, int old_dest) : Command(cmd_line,new_dest,old_dest){}
//BuiltInCommand::~BuiltInCommand(){}

ExternalCommand::ExternalCommand(const char* cmd_line, const char* original_trim_cmd_line_, int new_dest, int old_dest) : 
Command(cmd_line, new_dest, old_dest), original_trim_cmd_line(original_trim_cmd_line_){}

bool ExternalCommand::isFinished(){
  int status;
  int result = waitpid(this->pid, &status, WNOHANG);
  if (result == -1){
    // perror("smash error: waitpid failed");
    // cout << "ERROR CHECK WHAT HAPPEN 1" << endl; //TODO: remove this!!!!!
    return true; //TODO: check if this is correct
  }
  else if (result == 0){
    return false;
  }
  else{
    if (this->pid == result){
      return true;
    }
    else{
      // cout << "ERROR CHECK WHAT HAPPEN" << endl; //TODO: remove this!!!!!
      return false;
    }
  }
}

Command * SmallShell::CreateCommand(const char* cmd_line, int new_dest, int old_dest) {
    Command*  new_cmd= new Command(cmd_line,new_dest,old_dest);
    return new_cmd;
}

bool isInteger(const string& str) {
  if ((str[0] != '-') && (!isdigit(str[0]))){
    return false;
  }
  for (size_t i = 1; i < str.size(); ++i) {
        if (!isdigit(str[i])) {
            return false; // Found a non-digit character
        }
  }
  try {
      int num = std::stoi(str);
      float num_float = std::stof(str);
      return (num_float ==  static_cast<int>(num_float));
    } catch (const std::invalid_argument&) {
        return false; 
    } catch (const std::out_of_range&) {
        return false; 
    } catch(...) {
        return false;
    }
}

bool isNegativeInteger(const string& str) {
  if (str[0] != '-'){
    return false;
  }
  return isInteger(str.substr(1));
  /*
    for (size_t i = 1; i < str.size(); ++i) {
        if (!isdigit(str[i])) {
            return false; // Found a non-digit character
        }
  }
  try {
      int num = std::stoi(str);
      if (num >= 0) {
        return false;
      }
      float num_float = std::stof(str);
      return (num_float ==  static_cast<int>(num_float));
  } catch (const std::invalid_argument&) {
        return false; 
  } catch (const std::out_of_range&) {
        return false; 
  } catch(...) {
        return false;
  }
  */
}

int cd_check_errors(int num_arg, char* first_arg,string last_work_dir){
  if (num_arg > 1){
    
    cerr << "smash error: cd: too many arguments" << endl;
    return -1;
  }
  if (num_arg == 1 && string(first_arg) == "-" && last_work_dir == ""){
    cerr << "smash error: cd: OLDPWD not set" << endl;
    return -1;
  }
  
  return 0;

}

int fg_check_errors(int num_arg, char* first_arg, bool job_list_is_empty, JobsList* jobs){
  if ((num_arg ==0) && (job_list_is_empty)){
    cerr << "smash error: fg: jobs list is empty" << endl;
    return -1;
  } 
  if (num_arg>0){
    const string first_arg_ = string(first_arg);
    if (isInteger(first_arg_)&&num_arg<2){
      int job_id = std::stoi(string(first_arg_));
      if (!(jobs->isJobInTheList(job_id))){
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            return -1;
      }
    }
    else{
      cerr << "smash error: fg: invalid arguments" << endl;
      return -1;
    }
    if (num_arg > 1 ){
      cerr << "smash error: fg: invalid arguments" << endl;
      return -1;
    }
  }
  return 0;
}

int kill_check_errors(int num_arg, char* first_arg, char* second_arg, JobsList* jobs){
  if (second_arg != nullptr){
    const string second_arg_ = string(second_arg);
    if (isInteger(second_arg_) == true){
      int job_id = std::stoi(string(second_arg_));
          if(num_arg>2|| !isNegativeInteger(string(first_arg))){
            cerr << "smash error: kill: invalid arguments" << endl;
            return -1;
          }
          if (!(jobs->isJobInTheList(job_id))){
            cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
            return -1;
          }
          else if (num_arg == 2 && isNegativeInteger(string(first_arg))){
            return 0;
          }
          else {
            cerr << "smash error: kill: invalid arguments" << endl;
            return -1;
          }

    }
    else{
      cerr << "smash error: kill: invalid arguments" << endl;
      return -1;
    }
  }
  else{
    cerr << "smash error: kill: invalid arguments" << endl;
    return -1;
  }

  }
  
bool is_built_in(string instruct){
  return (instruct == "chprompt" )|| (instruct == "showpid") ||( instruct == "pwd") ||
         (instruct == "cd") || (instruct == "jobs") || (instruct == "fg") ||
         (instruct == "quit") || (instruct == "kill") || (instruct == "chprompt&" )||
         (instruct == "showpid&") ||( instruct == "pwd&") || (instruct == "cd&") ||
         (instruct == "jobs&") || (instruct == "fg&") || (instruct == "quit&") ||
         (instruct == "kill&")|| (instruct == "alias")|| (instruct == "alias&")|| (instruct == "unalias")|| (instruct == "unalias&");
}

bool is_special_command(string instruct){
  return (instruct == "listdir" )|| (instruct == "listdir&")|| (instruct == "getuser")|| (instruct == "getuser&") ;
}

bool is_valid_octal(const char *str) {
  int i=0;
    for ( i = 0; str[i] != '\0'; ++i) {
        // Check if the character is a digit and if it's between '0' and '7'
        if (str[i] < '0' || str[i] > '7') {
            return false;
        }
    }
    if (i<5){
      return true;
    }
    
    return false;
}

int is_output_redirection(string cmd_trim){
  if (cmd_trim.find(">>") != string::npos){
    return 2;
  }

  else if(cmd_trim.find(">") != string::npos){
    return 1;
  }
  else{
    return -1;
  }
}
bool isResevedKeyword(string n){
  //if(n=="quit"||n=="quit&"||n=="lisdir"||n=="lisdir&") return true;
  return is_special_command(n)||is_built_in(n);
  return false;
}

bool containsWildcard(const std::string& str) {
    for (char c : str) {
        if (c == '?' || c == '*') {
            return true; 
        }
    }
    return false; 
}


void fork_and_execute(ExternalCommand* cmd, bool is_background, bool is_wildcard, SmallShell* smash ){            
  bool* child_failed = new bool();
  *child_failed = false;
  int fork_pid = fork();
  if(fork_pid == -1){
    perror("smash error: fork failed");
  } 
  else if (fork_pid == 0){ //child
    setpgrp();
    // const char* cmd_line = is_background? cmd->cmd_line :(cmd->original_trim_cmd_line).c_str();
    
    const char* cmd_line = cmd->cmd_line;

    if (is_wildcard){
      if (execlp("bash", "bash", "-c",cmd_line, nullptr) == -1){
        delete cmd;
        perror("smash error: execlp failed");
        *child_failed = true;
        exit(0);
      }
    }
    if (execvp((cmd->args[0]), cmd->args) == -1){
      //free((char*)(cmd->cmd_line));
      delete cmd;
      perror("smash error: execvp failed");
      *child_failed = true;
      exit(0);
    }
  }
  else{ //parent
    int status;
    if(!is_background){
      *(smash->curr_fg) = fork_pid;
      int result = waitpid(fork_pid, &status, 0);
      if (!(*child_failed)){
        if (result == -1){
            perror("smash error: waitpid failed");
        }
      }
      *(smash->curr_fg) = *(smash->smash_pid);
      
    }
    else{
      cmd->pid = fork_pid;
    }
    delete child_failed;
  }
  return;
}
std::queue<string> alias_que;
void SmallShell::deleteSpecificItem(string s){
  queue<string> tempQ;
  while (!alias_que.empty()){
    /* code */
    if (alias_que.front()!=s){
      /* code */
      tempQ.push(alias_que.front());
    }
    alias_que.pop();
  }
  alias_que=move(tempQ);
  
}
void SmallShell::printQueue(std::queue<string> q) { // Pass by value to preserve the original queue
    string key;
    while (!q.empty()) {
      key=q.front();
        std::cout << key <<"='"<<alias_map[key]<<"'"<<endl;
        q.pop();
    }
    //std::cout << std::endl;
}

void SmallShell::executeCommand(const char *cmd_line) {
  //cout<<"aaaaaaaaaaaaaaaaaa1";
  Pipe_kind pipekind; 
  //_removeBackgroundSign(cmd_line)
  string alias_cmd_line=string(cmd_line);
  // unsigned int idx = alias_cmd_line.find_last_not_of(WHITESPACE);
  // if (idx != string::npos) {
  //   if (alias_cmd_line[idx] == '&') {
  //         alias_cmd_line[idx] = '\0';
  //   }
  // }
  //_removeBackgroundSign(alias_cmd_line.c_str());
  string cmd_trim = _trim(string(cmd_line));
  
  if (cmd_trim.empty()){
    return;
  }
  // string to_find=string(cmd->args[0]);
  //  if (alias_map.find(to_find)!=alias_map.end()){
  //   cout<<endl<<"alaias    "<<to_find<<endl;
  //   cout<<endl<<"alaias[ll]=    "<<alias_map[to_find]<<endl;
  //  }

  //Command *cmd = CreateCommand(cmd_trim.c_str(),-1,-1);
   Command* cmd = CreateCommand(cmd_trim.c_str(),-1,-1);
   string to_find=string(cmd->args[0]);
   //Command* cmd;
   if (alias_map.find(to_find)!=alias_map.end()){
        delete cmd;
        string s3=cmd_trim;
        to_ins=alias_cmd_line;
        s3.erase(0,to_find.length());
        alias_com=true;
        executeCommand((alias_map[to_find]+s3).c_str());
                alias_com=false;

        return;

  }
  // else{
  //    cmd = CreateCommand(cmd_trim.c_str(),-1,-1);
  // }
  //delete cmd1;

  /////////////////////////////////////////
  //TODO
  // string strtemp1 = fidinlistalias(cmd->cmd_line);
  // if(strtemp1)
  // {
  //   cmd.
  // }
  // string to_find=string(cmd->args[0]);
  //  if (alias_map.find(to_find)!=alias_map.end()){
  //   cout<<endl<<"alaias    "<<to_find<<endl;
  //   cout<<endl<<"alaias[ll]=    "<<alias_map[to_find]<<endl;
  //  }
  bool can_be_pip=false;
  if (cmd->instruct=="alias")
  {
    /* code */
  
  // for (int i = 0; i < cmd->n_args+1; ++i) {
  //       if (strcmp(cmd->args[i], "|")==0) {
  //           can_be_pip=true;
  //       }
  //       if (strcmp(cmd->args[i], "|&")==0) {
  //           can_be_pip=true;
  //       }
  //   }
  string s=string(cmd_line);
    regex regex_p(R"(^alias ([a-zA-Z0-9_]+)='([^']*)'$)") ;
    smatch match; 
    if (regex_match(s, match, regex_p)) {
      can_be_pip=false;
    }
  }
  else{
    can_be_pip=true;
  }
  if (cmd->instruct=="unalias")
  {
    if ((alias_map.find("|")!=alias_map.end())||(alias_map.find("|&")!=alias_map.end())){
      can_be_pip= false;
    }
  }
  
  // else{
  //   can_be_pip=true;
  // }
  if(is_pipe(cmd->cmd_line,&pipekind)!=NOT_PIPE&&can_be_pip ){

    string l_pipe; //left_section_pipe
    string r_pipe; //right_section_pipe
    l_pipe= CutpipeCommandl(string(cmd_line),pipekind , l_pipe);
    r_pipe= CutpipeCommandr(string(cmd_line),pipekind , r_pipe);
     // cout<< l_pipe << endl ;
    int my_pipe[2];// my_pipe[0] to read my_pipe[1] to write
    if(pipe(my_pipe) == -1){
        perror("smash error: pipe failed");
        return;
    } 
    pid_t pid1 = fork();
    if (pid1 == -1){
        perror("smash error: fork failed");
    }
    //son1
    if(pid1==0){ 
      setpgrp();
        if(pipekind == COUT_PIPE)
        {
            dup2(my_pipe[1], 1);
        }
        if(pipekind == CERR_PIPE)
        {
            dup2(my_pipe[1], 2);
        }

        close(my_pipe[0]);
        close(my_pipe[1]);
        executeCommand(l_pipe.c_str());
        exit(0);
    }
    pid_t pid2 = fork();
    if (pid2 == -1)
    {
        perror("smash error: fork failed");
    }

    //second child
    if(pid2 == 0){
        setpgrp();
        dup2(my_pipe[0], STDIN_FILENO);
        close(my_pipe[0]);
        close(my_pipe[1]);
        executeCommand(r_pipe.c_str());
        exit(0);
    }

    close(my_pipe[0]);
    close(my_pipe[1]);

    if(waitpid(pid1, nullptr, WUNTRACED) == -1){
            perror("smash error: waitpid failed");
            return;
        }
        if(waitpid(pid2, nullptr, WUNTRACED) == -1){
            perror("smash error: waitpid failed");
            return;
        }
        delete cmd;
    return;

  }
  int new_dest = -1;
  int old_dest = -1;
      //&&cmd->instruct!="unalias"&&cmd->instruct!="unalias&"
  
      bool can_be_ = false;
  if (cmd->instruct=="alias" ){
    //cout<< "fuck3"<<endl;
    //cout<<cmd->args[1];
    // for (int i = 0; i < cmd->n_args+1; ++i) {
    //     if (strcmp(cmd->args[i], ">")==0) {
    //         can_be_ = true;
    //     }
    //     if (strcmp(cmd->args[i], ">>")==0) {
    //         can_be_ = true;
    //     }
    // }
    // string s=string(cmd_line);
    // std::size_t pos = s.find('>');

    // // If '>' is found, erase everything from '>' to the end
    // if (pos != std::string::npos) {
    //     s.erase(pos);
    // }
    // else{
    //   std::size_t pos2 = s.find('>>');
    //   if (pos2 != std::string::npos) {
    //     s.erase(pos2);
    // }
    // }
    string s=string(cmd_line);
    regex regex_p(R"(^alias ([a-zA-Z0-9_]+)='([^']*)'$)") ;
    smatch match; 
    if (regex_match(s, match, regex_p)) {
      can_be_=false;
    }
  }
  else{
    can_be_=true;
    //cout<< "fuck2";
  }
  if (cmd->instruct=="unalias")
  {
    if ((alias_map.find(">")!=alias_map.end())||(alias_map.find(">>")!=alias_map.end())){
      can_be_= false;
    }
  }
  
  if (can_be_)
  {
    /* code */
    //cout<< "fuck";
      
      int append = is_output_redirection(cmd_trim);
      //cout<<"aaaaaa";
      if (append != -1){ 
        //cmd_trim.find(">")
          if (append == 1){
            std::string comand = _trim(cmd_trim.substr(0, cmd_trim.find_last_of(">")));
            std::string dest = _trim(cmd_trim.substr(cmd_trim.find_last_of(">") + 1));
            
            size_t idx = dest.find_last_not_of(WHITESPACE);
            if (idx != string::npos) {
                          //cout<<dest<<endl;

                if (dest[idx] == '&') {
                 // cout<<dest<<endl;
                      //alias_cmd_line[idx] = '\0';
                      dest.erase(idx);
                }
              }
            cmd_trim = comand;
            new_dest =  open(dest.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
          }
          // cmd_trim.find(">>")
          else if (append == 2){
            std::string comand = _trim(cmd_trim.substr(0, cmd_trim.find_last_of(">>")-1));
            std::string dest = _trim(cmd_trim.substr(cmd_trim.find_last_of(">>") + 2));
            //cout<<comand<<endl;
            cmd_trim = comand;
            new_dest = open(dest.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0666);
          }
          
          

          if (new_dest == -1){
              perror("smash error: open failed");
              return;
          }
          else{
            old_dest = dup(STDOUT_FILENO);
            if (old_dest == -1) {
                perror("smash error: dup failed");
                close(new_dest);
                return;
            }
            else {
              if (dup2(new_dest, STDOUT_FILENO) == -1) {
                  perror("smash error: dup2 failed");
                  close(new_dest);
                  return;
              }
            }
          }
      }
}

  if (is_built_in(cmd->instruct)){
      delete cmd;
      char* new_cmd = (char*)malloc(100*sizeof(char));
      new_cmd = strcpy(new_cmd, cmd_trim.c_str());
          //cout<< "fuck6";


      _removeBackgroundSign(new_cmd);
             string cmd_trim1 = _trim(string(new_cmd));
             //cout << cmd_trim1<<"fuck"<<endl;
      //cout<<new_cmd;
      BuiltInCommand* cmd = new BuiltInCommand(cmd_trim1.c_str(), new_dest, old_dest);

      if (cmd->instruct == "chprompt"){
        this->jobs->removeFinishedJobs();
        this->prompt = cmd->n_args > 0 ? string(cmd->args[1]) + "> " : "smash> ";
        delete cmd;
        free(new_cmd);
        return;
      }
      
      if (cmd->instruct == "showpid"){
        this->jobs->removeFinishedJobs();
        cout << "smash pid is " << *(this->smash_pid) << endl;
        delete cmd;
        free(new_cmd);
        return;
      }
      
      if (cmd->instruct == "pwd"){
        this->jobs->removeFinishedJobs();
        char * work_dir = getcwd(NULL, 0);
          if (work_dir == NULL){
            perror("smash error: getcwd failed");
          }
          else{ cout << work_dir << endl; }
          
          delete cmd;
          free(new_cmd);
          return;
        }

      if (cmd->instruct == "cd"){
        this->jobs->removeFinishedJobs();
          if (cd_check_errors(cmd->n_args, cmd->args[1],this->last_work_dir) == 0){
            string new_dir = string(cmd->args[1]) == "-" ? this->last_work_dir : cmd->args[1];
            char * curr= getcwd(NULL,0);
            if (curr == NULL){
                perror("smash error: getcwd failed");
            }
            else{
                if (chdir(new_dir.c_str()) == 0){
                this->last_work_dir=string(curr);
                }
                else {
                  perror("smash error: chdir failed");
                }
            }
          }  
          delete cmd;
          free(new_cmd);
          return;
      }

      if (cmd->instruct == "jobs"){
      /////    this->jobs->removeFinishedJobs();
          this->jobs->printJobsList();
          delete cmd;
          free(new_cmd);
          return;
      }

      if(cmd->instruct == "fg"){
        this->jobs->removeFinishedJobs();
        if (fg_check_errors(cmd->n_args, cmd->args[1], this->jobs->all_jobs_list.empty(),this->jobs) == 0) {
          int job_id = cmd->n_args == 0 ? this->jobs->getMaxJobId() : std::stoi(string(cmd->args[1]));
          if (!(this->jobs->isJobInTheList(job_id))){
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
          }
          else{
            JobsList::JobEntry* job_in_the_list = this->jobs->getJobById(job_id);
            cout << job_in_the_list->cmd->original_trim_cmd_line << " " << job_in_the_list->cmd->pid << endl;
            *(this ->curr_fg) = job_in_the_list->cmd->pid;
            int result = waitpid(job_in_the_list->cmd->pid, nullptr,0);
            if (result == -1){
                perror("smash error: waitpid failed");
              }
              else{
                this->jobs->removeJobById(job_id);
              }
            *(this ->curr_fg) = *(this->smash_pid);
          }
        }
        delete cmd;
        free(new_cmd);
        return;
      }   
            
      if(cmd->instruct == "quit"){
        //TODO: check what happen if we specified 'kill' not as the first argument.
        // for example: quit 888 hhhe ckcks kill jdckv &
        // TODO: check if the word kill is case sensitive ( KILL ect..)
        this->jobs->removeFinishedJobs();
        bool send_signal_and_print = (((cmd->n_args) > 0) && (string(cmd->args[1]) == "kill"));
        if (send_signal_and_print){
          cout << "smash: sending SIGKILL signal to " << this->jobs->all_jobs_list.size() << " jobs:" << endl;
        }
        this->jobs->killAllJobs(send_signal_and_print);
        delete cmd;
        free(new_cmd);
        exit(0);
      }

      if (cmd->instruct == "kill"){
        this->jobs->removeFinishedJobs();
        if (cmd->n_args >=1){
          int result = kill_check_errors(cmd->n_args, cmd->args[1], cmd->args[2], this->jobs);
          if (result == 0){
            pid_t pid = jobs->getJobById(std::stoi(string(cmd->args[2])))->cmd->pid;
            int signal = std::stoi((string(cmd->args[1])).substr(1));
            if (kill(pid, signal) == -1){
              cout << "signal number " << signal << " was sent to pid " << pid << endl;
              perror("smash error: kill failed");
            }
            else {
              cout << "signal number " << signal << " was sent to pid " << pid << endl;
            }
          }
        }
        else{
          cerr << "smash error: kill: invalid arguments" << endl;
        }
        delete cmd;
        free(new_cmd);
        return;
     } 
      if (cmd->instruct == "alias")
        {
          if(cmd->n_args==0){
            //printMapList();
            printQueue(alias_que);
            return;
          }
          //cout<<cmd->cmd_line;
          //parse alias
          //const string regex_d =_trim(string(cmd->cmd_line));            // regex definition
          const string regex_d =cmd->cmd_line; 
          //cout<<regex_d<<"1"<<endl;
          //cout<<"2";
          // regex regex_p("^alias ([a-zA-Z0-9_]+)=('[^']*'$)") ; //regex pattern
          regex regex_p(R"(^alias ([a-zA-Z0-9_]+)='([^']*)'$)") ; //regex pattern
          //regex pattern1("[a-zA-Z0-9_]+");
          //regex pattern2("'[^']*'");
          smatch match;
          // printf("1 %s\n",cmd->cmd_line);

          if (regex_match(regex_d, match, regex_p)) {
            string key_n=match[1].str(); //the name 
            string data_c=match[2].str(); //the command

            if(alias_map.find(key_n)!=alias_map.end()||isResevedKeyword(key_n)){
              cerr<<"smash error: alias: " <<key_n<< " already exists or is a reserved command"<<endl;
              free(cmd);
              return;
            }
            size_t idx = alias_cmd_line.find_last_not_of(WHITESPACE);
              if (idx != string::npos) {
                if (alias_cmd_line[idx] == '&') {
                      //alias_cmd_line[idx] = '\0';
                      alias_cmd_line.erase(idx);
                }
              }
               idx = alias_cmd_line.find_last_not_of(WHITESPACE);
              if (idx != string::npos) {
                if (alias_cmd_line[idx] == '\'') {
                      //alias_cmd_line[idx] = '\0';
                      alias_cmd_line.erase(idx);
                }
              }
            size_t pos = alias_cmd_line.find("='");
            data_c=alias_cmd_line.substr(pos + 2, alias_cmd_line.length() - pos - 2);
            //  printf("2  %s\n",key_n);
            //  printf("3  %s\n",data_c);
            // std::cout << "Full match: " << match[0] << std::endl;
            // std::cout << "First group: " << match[1] << std::endl;
            //  std::cout << "seconde group: " << match[2] << std::endl;
            //  std::cout << "seconde group: " << key_n << std::endl;
            alias_map[key_n]=data_c;
            //std::cout << "seconde group: " << match[2] << std::endl;
            //std::cout << "seconde (data): " << data_c << std::endl;
            //string key2=match[1].str(); //the name 
            //alias_que.push("aaa");
            alias_que.push(key_n);
            return;
            

          }
          cerr<<"smash error: alias: invalid alias format"<<endl;
          free(cmd);
          return;

        }
        if (cmd->instruct == "unalias"){
          if (cmd->n_args==0)
          {
            cerr<<"smash error: unalias: not enough arguments"<<endl;
            free(cmd);
            return;
          }
          string key;
          //string key2;
          for (int i = 0; i < cmd->n_args; i++)
          {
            key=cmd->args[i+1];
            if (alias_map.count(key) > 0) {
              continue;
            }
            
            cerr<<"smash error: unalias: "<<key<<" alias does not exist"<<endl;
            free(cmd);
            return;
            /* code */
          }
          for (int i = 0; i < cmd->n_args; i++)
          {

            key=cmd->args[i+1];
            //key2=cmd->args[i+1];
            alias_map.erase(key);
            deleteSpecificItem(key);
            //alias_vec.erase(std::remove(alias_vec.begin(), alias_vec.end(), key), alias_vec.end());

          }
          free(cmd);
            return;
        }
        
            
    }

  else if(is_special_command(cmd->instruct)){
    this->jobs->removeFinishedJobs();
    if(cmd->instruct=="listdir"){
      // cout<< cmd->n_args << endl;
      if(cmd->n_args >= 2){
        std::cerr << "smash error: listdir: too many arguments" << std::endl;
        return;
      }
      std::vector<std::string> files_v, directories_v, links_v;
      const char* path = (cmd->n_args == 0) ? "." : cmd->args[1];
      // DIR* dir = opendir(path);
      //   if (!dir) {
      //       perror("smash error: opendir failed");
      //       delete cmd;
      //       return;
      //   }
      char* buf=(char *)malloc(BUF_SIZE * sizeof(char));
      int fd;
      ssize_t nread;
      //nread = getdents(fd, buf, BUF_SIZE);
      // if (nread == -1) {
      //     perror("getdents");
      //     ////////////////////////
      //     ///////////////////////
      //     //////////////////////
      //     //TODO
      //     return ;
      // }

      //string to_open=string(path);
      //int fd = open(to_open.c_str(), O_RDONLY | O_DIRECTORY);
      for (int  t= 0; t < TYPES::TYPES_AMOUNT; t++)
      {
        /* code */
            fd = open(path, O_RDONLY | O_DIRECTORY);
            if (fd == -1) {
              perror("smash error: open failed");
              delete cmd;
              return ;
            }
            //char* buf=(char *)malloc(BUF_SIZE * sizeof(char));
            //std::vector<Entry> entries;

                for (;;) {
                        nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
                        if (nread == -1) {
                          perror("smash error: getdents64 failed");
                          close(fd);
                          delete cmd;
                          return;
                        }

                        if (nread == 0) break;
                        
                        for (int bpos = 0; bpos < nread;) {
                            struct dirent64* d = (struct dirent64*)(buf + bpos);
                            string  d_name(d->d_name);
                            if(d_name=="."||d_name==".."){
                              bpos += d->d_reclen;
                              continue;
                            }
                            // std::cout << d->d_name<<endl ;
                            // std::cout << (d->d_type==DT_REG)<<endl ;
                            // std::cout << DT_REG<<endl ;
                            // std::cout << d->d_type<<endl ;
                            // std::cout << "type:"<< t<<endl ;
                            if (t==TYPES::FILES && d->d_type == DT_REG){
                              /* code */
                              //cout<<"file: "<<d_name<<endl;
                              files_v.push_back(d_name);
                            }
                            else if (t==TYPES::DIRS && d->d_type == DT_DIR){
                              //cout<<"directory: "<<d_name<<endl;
                              directories_v.push_back(d_name);
                            }
                            else if (t==TYPES::LINKS && d->d_type == DT_LNK){
                              /* code */
                              //cout<<"link: "<<d_name<<endl;
                              links_v.push_back(d_name);
                            }

                            
                            bpos += d->d_reclen;
                          }

                  } // the close of rhe inf loop 
                  close(fd);
      }
      for (int t = 0; t < TYPES::TYPES_AMOUNT; t++) {
        if (t == TYPES::FILES) {
            print_sorted(files_v, "file");
        } else if (t == TYPES::DIRS) {
            print_sorted(directories_v, "directory");
        } else if (t == TYPES::LINKS) {
            print_sorted(links_v, "link");
        }
    }

    
      
    }// the end of the listdir 
    if(cmd->instruct=="getuser") {
        if (cmd->n_args >= 2) {
            std::cerr << "smash error: getuser: too many arguments" << std::endl;
            return;
        }
        std::string user = "NULL";
        std::string group = "NULL";
        pid_t pid = std::stoi(cmd->args[1]);;
        std::stringstream ss;

        ss << "/proc/" << pid << "/status";

        FILE* statusFile = fopen(ss.str().c_str(), "r");
        if (!statusFile) {
            std::cerr << "smash error: getuser: process " << pid << " does not exist" << std::endl;
            return ;
        }

        char line[256];
        uid_t Uid = -1;
        gid_t Gid = -1;

        while (fgets(line, sizeof(line), statusFile)) {
            if (strncmp(line, "Uid:", 4) == 0) {
                sscanf(line + 4, "%u", &Uid);
            }
            if (strncmp(line, "Gid:", 4) == 0) {
                sscanf(line + 4, "%u", &Gid);
            }
        }

        fclose(statusFile);
        struct passwd *p = getpwuid(Uid);
        struct group *g = getgrgid(Gid);

        if (p != nullptr) {
            user = p->pw_name;
        }
        if (g != nullptr) {
            group = g->gr_name;
        }
        std::cout << "User: " << user << std::endl;
        std::cout << "Group: " << group << std::endl;
        return;
    }

  }  // the end of the is_special_command
  else if (cmd_trim[0] != '&'){ //external command
      this->jobs->removeFinishedJobs();
      char* new_cmd = (char*)malloc(80*sizeof(char));
      new_cmd = strcpy(new_cmd, cmd_trim.c_str());
      bool is_background = _isBackgroundComamnd(new_cmd);
      bool is_wildcard = containsWildcard(string(new_cmd));
      delete cmd;
      ExternalCommand * cmd;
      _removeBackgroundSign(new_cmd);
      if (alias_com){
               cmd = new ExternalCommand(new_cmd, to_ins.c_str(), new_dest, old_dest);

      }
      else{
               cmd = new ExternalCommand(new_cmd, cmd_line, new_dest, old_dest);

      }
      //ExternalCommand::ExternalCommand(const char* cmd_line, const char* original_trim_cmd_line_, int new_dest, int old_dest) : 
      //Command(cmd_line, new_dest, old_dest), original_trim_cmd_line(original_trim_cmd_line_){}

      if (is_background){
        fork_and_execute(cmd, is_background, is_wildcard,this);
        this->jobs->addJob(cmd, new_cmd);
        //b7aol atba3 original_trim_cmd_line_ be alias 
        // if (alias_com)
        // {
        //    char* new_cmd1 = (char*)malloc(80*sizeof(char));
        //    new_cmd1 = strcpy(new_cmd1, to_ins.c_str());
        //     bool is_background1 = _isBackgroundComamnd(new_cmd1);
        //    bool is_wildcard1 = containsWildcard(string(new_cmd1));
        //   ExternalCommand * cmd1 = new ExternalCommand(new_cmd1, to_ins.c_str(), new_dest, old_dest);
        //   this->jobs->addJob(cmd1, new_cmd1);
        // }
        // else{
        //   this->jobs->addJob(cmd, new_cmd);
        // }
        
        
      }
      else{ // foreground
          fork_and_execute(cmd, is_background,is_wildcard,this);
          free(new_cmd);
          delete cmd;
      }
      return;

      //BuiltInCommand* cmd = new BuiltInCommand(new_cmd);

  }
  delete cmd;
  


       
 
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)

  //delete cmd;
}