//-----------------------------------------------------------------------------
// concmd.hpp - Contains the definitions for console commands
//-----------------------------------------------------------------------------

#pragma once

#include "../string.hpp"
#include "../string_proxy.hpp"

#include "conCallbacks.hpp"
#include "conDisplay.hpp"
#include "conInput.hpp"
#include "conMod.hpp"

#include <vector>
#include <string>

typedef bool (*CommandCallback)(std::string data);

struct ConCmd {
  const char* name;         // Command name for invocation, should be lowercase
  const char* help;         // Help text to be printed
  CommandCallback callback; // Callback with specific invocation logic
  bool hidden;              // Hides this command from lists (e.g. dev cmds)
  std::string data;         // Any associated data for invocation

  ConCmd(const char* n, const char* hlp, CommandCallback c = 0, 
    bool hde = false, std::string d = "") {
    name = n;
    help = hlp;
    callback = c;
    hidden = hde;
    data = d;
  }

  ConCmd() {
    name = 0;
    help = 0;
    callback = 0;
    hidden = false;
    data = "";
  }
};

const std::vector<ConCmd> conCmds
{
  ConCmd("bind",
  "Bind a key to an action",
  conCallbackKeyBind),

  ConCmd("camerainfo",
  "Print main camera info",
  conCallbackCameraInfo,
  true),

  ConCmd("capture",
  "Capture a framebuffer to a file",
  conCallbackCapture,
  true),

  ConCmd("clear",
  "Clear the console",
  conCallbackClear),

  /*
  ConCmd("crash",
  "Crashes the game",
  conCallbackCrash,
  true),
  */

  ConCmd("cpos",
  "Prints the cursor screen position",
  conCallbackCursorPos,
  true),
  
  ConCmd("debug",
  "Toggle debug mode",
  conCallbackDebug,
  true),

  ConCmd("difficulty",
  "Prints or sets the current difficulty",
  conCallbackDifficulty),

  ConCmd("dir",
  "Prints out the specified directory",
  conCallbackDir),

  ConCmd("enableam",
  "Enable the specified amenity",
  conCallbackEnableAmenity),

  ConCmd("enablefe",
  "Enable the specified feature",
  conCallbackEnableFeature),

  ConCmd("finaldollar",
  "Good to the last dollar",
  conCallbackFinalDollar,
  true),

  ConCmd("fov",
  "Set the camera's FOV",
  conCallbackFov),

  ConCmd("hello",
  "Prints \"Hello world!\"",
  conCallbackHello,
  true),

  ConCmd("help",
  "Displays help text for the specified command, or prints a list of commands",
  conCallbackHelp),

  ConCmd("iamhowardroark", "Place any building",
      conCallbackAnyBuildingPlop, true),

  ConCmd("loadobj",
  "Attempts to import the specified .obj file",
  conCallbackLoadObj),

  ConCmd("lua",
  "Print a Lua value, or assign a new value to a Lua variable",
  conCallbackLua),

  ConCmd("map",
    "Redraws the map",
    conCallbackMap,
    true),

  ConCmd("mod",
  "Prints mod info",
  conCallbackMod),

  ConCmd("emergencypowers",
  "Unlocks all Taxes",
  conCallbackEmergencyPowers,
  true),

  ConCmd("partytime",
    "Unlocks everything",
    conCallbackPartyTime,
    true),

  ConCmd("propaganda",
    "Forces a newspaper article by code",
    conCallbackPropaganda,
    true),

  ConCmd("quit",
    "Quits the game",
    conCallbackQuit),

  ConCmd("readobj",
    "Attempts to read the data of the specified .obj file, prints report if successful",
    conCallbackReadObj),

  ConCmd("renderobj",
    "Attempts to render the specified .obj file",
    conCallbackRenderObj),

  ConCmd("route",
    "Attempts to select and print the route for the specified object",
    conCallbackRoute),

  ConCmd("routeperson",
    "Attempts to select and print the route for the specified person",
    conCallbackRoutePerson,
    true),

  ConCmd("routegroup",
    "Attempts to select and print the route for the specified travel group",
    conCallbackRouteTravelGroup,
    true),

  ConCmd("routevehicle",
    "Attempts to select and print the route for the specified vehicle",
    conCallbackRouteVehicle,
    true),

  ConCmd("select",
    "Select an item by index using the following format - "
    "(\"b132\" = Building #132)\n"
    "(b# - building, e# - graph element, f# - family,"
    " g# - travel group, p# - person, v# - vehicle)",
    conCallbackSelect),

  ConCmd("selectbuilding",
    "Attempts to select the specified building",
    conCallbackSelectBuilding,
    true),

  ConCmd("selectelement",
    "Attempts to select the specified graph element",
    conCallbackSelectElement,
    true),

  ConCmd("selectfamily",
    "Attempts to select the specified family",
    conCallbackSelectFamily,
    true),

  ConCmd("selectperson",
    "Attempts to select the specified person",
    conCallbackSelectPerson,
    true),

  ConCmd("selectgroup",
    "Attempts to select the specified travel group",
    conCallbackSelectTravelGroup,
    true),

  ConCmd("selectvehicle",
    "Attempts to select the specified vehicle",
    conCallbackSelectVehicle,
    true),

  ConCmd("songenable",
  "Enable/disable music",
  conCallbackSongEnable),

  ConCmd("songinfo",
    "Display info on loaded songs",
    conCallbackSongInfo),

  ConCmd("songplay",
    "Play the specified song",
    conCallbackSongPlay),

  ConCmd("songshuffle",
    "Shuffle the currently playing song",
    conCallbackSongShuffle),

  ConCmd("steaminfo",
    "Prints Steam specific info, if Steam is initialized",
    conCallbackSteamInfo),

  ConCmd("testfilewrite",
    "Writes a test file with the new file format to disk",
    conCallbackTestFileWrite,
    true),

  ConCmd("testfileread",
    "Read a test file with the new file format from disk",
    conCallbackTestFileRead,
    true),

  ConCmd("the_snap",
    "Gives Thanos six Infinity Stones.",
    conCallbackTheSnap),

  ConCmd("autosave_interval",
    "Sets the autosave interval to the specified number of seconds",
    conCallbackSetAutosave),

  ConCmd("tutorialenable",
  "Enable or disable the tutorial",
  conCallbackTutorialEnable),

  ConCmd("tutorialinfo",
  "Displays tutorial state info",
  conCallbackTutorialInfo,
  true),

  ConCmd("unachiever",
  "Resets all achievements",
  conCallbackUnachiever,
  true),

  ConCmd("unbind",
    "Unbind a key from an action",
    conCallbackKeyUnbind),

  ConCmd("unbindall",
    "Unbind all bound keys from actions",
    conCallbackKeyUnbindAll),

  ConCmd("unloadobj",
  "Attempts to unload the specified .obj file",
  conCallbackUnloadObj),

  ConCmd("voodooeconomics",
  "Free Money",
  conCallbackVoodooEconomics,
  true),

  ConCmd("weather",
  "Sets the current weather",
  conCallbackWeather,
  true),
};

bool isCmd(std::string input);
bool getCmd(std::string input, ConCmd& cmd);
ConCmd getCmd(std::string input);
