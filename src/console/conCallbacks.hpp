//-----------------------------------------------------------------------------
// conCallbacks.hpp - Contains definitions for callbacks used
// by console commands on invocation
//-----------------------------------------------------------------------------

#pragma once

#include <string>

// Callbacks (defined in conCallbacks.cpp)
bool conCallbackAnyBuildingPlop(std::string data);
bool conCallbackCameraInfo(std::string data);
bool conCallbackCapture(std::string data);
bool conCallbackClear(std::string data);
bool conCallbackCrash(std::string data);
bool conCallbackCursorPos(std::string data);
bool conCallbackDebug(std::string data);
bool conCallbackDifficulty(std::string data);
bool conCallbackDir(std::string data);
bool conCallbackEmergencyPowers(std::string data);
bool conCallbackEnableAmenity(std::string data);
bool conCallbackEnableFeature(std::string data);
bool conCallbackFinalDollar(std::string data);
bool conCallbackFov(std::string data);
bool conCallbackHello(std::string data);
bool conCallbackHelp(std::string data);
bool conCallbackKeyBind(std::string data);
bool conCallbackKeyUnbind(std::string data);
bool conCallbackKeyUnbindAll(std::string data);
bool conCallbackLoadObj(std::string data);
bool conCallbackLua(std::string data);
bool conCallbackMap(std::string data);
bool conCallbackMod(std::string data);
bool conCallbackPartyTime(std::string data);
bool conCallbackPropaganda(std::string data);
bool conCallbackQuit(std::string data);
bool conCallbackReadObj(std::string data);
bool conCallbackRenderObj(std::string data);
bool conCallbackRoute(std::string data);
bool conCallbackRoutePerson(std::string data);
bool conCallbackRouteTravelGroup(std::string data);
bool conCallbackRouteVehicle(std::string data);
bool conCallbackSelect(std::string data);
bool conCallbackSelectBuilding(std::string data);
bool conCallbackSelectElement(std::string data);
bool conCallbackSelectFamily(std::string data);
bool conCallbackSelectPerson(std::string data);
bool conCallbackSelectTravelGroup(std::string data);
bool conCallbackSelectVehicle(std::string data);
bool conCallbackSongEnable(std::string data);
bool conCallbackSongInfo(std::string data);
bool conCallbackSongPlay(std::string data);
bool conCallbackSongShuffle(std::string data);
bool conCallbackSteamInfo(std::string data);
bool conCallbackTestFileWrite(std::string data);
bool conCallbackTestFileRead(std::string data);
bool conCallbackTheSnap(std::string data);
bool conCallbackSetAutosave(std::string data);
bool conCallbackTutorialEnable(std::string data);
bool conCallbackTutorialInfo(std::string data);
bool conCallbackUnachiever(std::string data);
bool conCallbackUnloadObj(std::string data);
bool conCallbackVoodooEconomics(std::string data);
bool conCallbackWeather(std::string data);

