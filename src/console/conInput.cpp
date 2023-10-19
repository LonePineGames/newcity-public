#include "conInput.hpp"

const int conMaxInputMem = 32;

const char* conErrCmd = "Invalid command";
const char* conErrArg = "Invalid argument to command";
const char* conErrCb = "Command has no associated callback";
const char* conErrPtr = "Pointer was null";
const char* conErrUnkMod = "Unknown mod";
const char* conErrInvMod = "Invalid mod";
const char* conErrDir = "Error reading directory";
const char* conErrFile = "Error reading file";
const char* conErrObjLoad = "Error loading .obj file";
const char* conErrLuaVar = "Unrecognized Lua variable";
const char* conErrLuaVal = "Invalid Lua value";
const char* conErrLuaAll = "Lua variable did not exist in all states";
const char* conErrUnk = "Unknown error";

static uint32_t inputMemIndex = 0;
static std::vector<std::string> inputMem;


void addToInputMem(std::string input) {
  inputMem.push_back(input);
  if(inputMem.size() > conMaxInputMem) {
    inputMem.erase(inputMem.begin());
  }
  inputMemIndex = inputMem.size();
}

uint32_t consoleGetInputMemIndex() {
  return inputMemIndex;
}

bool consoleTrySetInputMemIndex(uint32_t i) {
  if(i < 0 || i >= inputMem.size()) {
    return false;
  }

  inputMemIndex = i;
  return true;
}

std::string* consoleGetInputMemArr() {
  return inputMem.data();
}

std::string* consoleGetInputMemArr(int& size) {
  size = inputMem.size();
  return inputMem.data();
}

std::string consoleGetInputMemAtIndex() {
  return inputMem[inputMemIndex];
}

const char* consoleGetError(ConStatus e) {
  switch(e) {
    case ConStatus::INVALID_CMD:
      return conErrCmd;
    case ConStatus::INVALID_ARG:
      return conErrArg;
    case ConStatus::NO_CALLBACK:
      return conErrCb;
    case ConStatus::NULL_PTR:
      return conErrPtr;
    case ConStatus::UNKNOWN_MOD:
      return conErrUnkMod;
    case ConStatus::INVALID_MOD:
      return conErrInvMod;
    case ConStatus::DIR_READ_ERROR:
      return conErrDir;
    case ConStatus::FILE_READ_ERROR:
      return conErrFile;
    case ConStatus::OBJ_LOAD_ERROR:
      return conErrObjLoad;
    case ConStatus::LUA_VAR_ERROR:
      return conErrLuaVar;
    case ConStatus::LUA_VAL_ERROR:
      return conErrLuaVal;
    case ConStatus::LUA_ALL_ERROR:
      return conErrLuaAll;
    default:
      return conErrUnk;
  }
}

int consoleHandleLua(std::string input) {
  // Seperate input into variable and data, if present
  std::string inVar = "";
  std::string inData = "";
  size_t split = input.find(" ");
  if(split == std::string::npos) {
    // No data found
    inVar = input;
  } else {
    inVar = input.substr(0,split);
    inData = input.substr(split+1,std::string::npos);
    // Parse data string to lowercase
    for(int i = 0; i < inData.length(); i++) {
      inData[i] = tolower(inData[i]);
    }
  }

  const char* varcstr = inVar.c_str();
  if(luaDoesValueExistAll(varcstr)) {
    if(inData.length() > 0) {
      //try {
        float newValue = stof(inData);
        if(!luaSetValueAll(varcstr, newValue)) {
          consolePrintLine(sprintf_o("No Lua states could be set "
                "with new number value (%f)", newValue));
        } else {
          // Successfully set one or more values; reload from Lua
          loadConstantsFromLua();
        }
      //} catch(int e) {
        //consolePrintLine(sprintf_o("(Exception %d raised)", e));
        //return ConStatus::LUA_VAR_ERROR;
      //}
    } else {
      float val = getValueFromLua(varcstr);
      consolePrintLine(sprintf_o("%s: %f",varcstr,val));
    }

  } else if(luaDoesBoolExistAll(varcstr)) {
    if(inData.length() > 0) {
      // Validate input
      bool validBool = true;
      bool newBool;
      if(inData[0] == '0' || inData[0] == 'f') {
        newBool = false;
      } else if (inData[0] == '1' || inData[0] == 't'){
        newBool = true;
      } else {
        validBool = false;
      }

      // Handle input, and attempt to set and reload Lua vars if valid
      if(!validBool) {
        consolePrintLine(sprintf_o("Invalid boolean value passed (%s)", varcstr));
      } else if(!luaSetBoolAll(varcstr, newBool)) {
        consolePrintLine(sprintf_o("No Lua states could be set with new boolean value (%f)", newBool));
      } else {
        // Successfully set one or more values; reload from Lua
        loadConstantsFromLua();
      }
    } else {
      bool val = getBoolFromLua(varcstr);
      consolePrintLine(sprintf_o("%s: %s",varcstr,val ? "true" : "false"));
    }
  } else {
    return ConStatus::LUA_ALL_ERROR;
  }

  return ConStatus::SUCCESS;
}

int handleMod(ConMod mod, std::string input) {
  if(mod.type == ConModType::INVALID) {
    return ConStatus::INVALID_MOD;
  }
  if(mod.type == ConModType::LUA) {
    return consoleHandleLua(input);
  }

  // Get the substring starting at pos 1 to end of string
  std::string cmdTxt = input.substr(1,std::string::npos);
  ConCmd cmd;
  if(!getCmd(cmdTxt,cmd)) {
    return ConStatus::INVALID_CMD;
  }

  switch(mod.type) {
    case ConModType::HELP:
      consolePrintLine(cmd.help);
      return ConStatus::SUCCESS;
    case ConModType::LUA:
      // print or modify Lua value
      return ConStatus::SUCCESS;
    default:
      return ConStatus::UNKNOWN_MOD;
  }
}

int handleCmd(std::string input) {
  // Seperate input into command and data, if present
  std::string inCmd = "";
  std::string inData = "";
  size_t split = input.find(" ");
  if(split == std::string::npos) {
    // No data found
    inCmd = input;
  } else {
    inCmd = input.substr(0,split);
    inData = input.substr(split+1,std::string::npos);
  }

  ConCmd cmd;
  if(!getCmd(inCmd,cmd)) {
    return ConStatus::INVALID_CMD;
  }
  
  if(cmd.callback == 0) {
    return ConStatus::NO_CALLBACK;
  }

  cmd.callback(inData);
  return ConStatus::SUCCESS;
}

int consoleParseInput(std::string input) {
  if(input.length() == 0) {
    return ConStatus::INVALID_CMD;
  }

  if(input[0] == '\0') {
    return ConStatus::INVALID_CMD;
  }

  addToInputMem(input);
  consoleEchoInput(input);

  // Check if a mod was used, and handle accordingly
  ConMod mod;
  if(consoleGetModifier(input[0],mod)) {
    return handleMod(mod, input);
  }

  // If no mod, handle it as a command
  return handleCmd(input);
}
