#include "conCmd.hpp"

// Is the specified string a recognized command?
bool isCmd(std::string input) {
  const char* input_c = input.c_str();
  for(int i = 0; i < conCmds.size(); i++) {
    const char* cmd = conCmds[i].name;
    if(streql(input_c,cmd)) {
      return true;
    }
  }
  return false;
}

// Same as basic isCmd
// Passes the cmd's index out via the reference if found and returns success 
bool isCmd(std::string input, int& index) {
  const char* input_c = input.c_str();
  for(int i = 0; i < conCmds.size(); i++) {
    const char* cmd = conCmds[i].name;
    if(streql(input_c,cmd)) {
      index = i;
      return true;
    }
  }
  return false;
}

// Attempts to get a command that matches the input string
// Passes the command out via the reference if found and returns success
bool getCmd(std::string input, ConCmd& cmd) {
  int index = -1;
  if(isCmd(input,index)) {
    cmd = conCmds[index];
    return true;
  }
  return false;
}

// Attempts to get a command that matches the input string
ConCmd getCmd(std::string input) {
  int index = -1;
  ConCmd cmd;
  if(isCmd(input,index)) {
    cmd = conCmds[index];
  }
  return cmd;
}