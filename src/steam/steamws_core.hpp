//-----------------------------------------------------------------------------
// steamws_core - Contains all the NewCity specific API hooks to 
// interact with the Steam Workshop and UGC
//-----------------------------------------------------------------------------

#pragma once

#include "steamws.hpp"
#include "../game/game.hpp"

// The steamws_core API should have everything needed for NewCity Steam Workshop
// The only publicly exposed functions should be the steamws_core API functions
// The steamws_core class and associated methods shouldn't ever need to be directly called from outside

enum steamws_pathEnum {
  PathRoot = 0,
  PathData,
  PathDesigns,
  PathDocs,
  PathFonts,
  PathLocale,
  PathModelsRoot,
  PathModelsDecorations,
  PathModelsVehicles,
  PathModpacks,
  PathNewspaper,
  PathShaders,
  PathSoundRoot,
  PathSoundEnvironment,
  PathSoundMusic,
  PathTexturesRoot,
  PathTexturesBuildings,
  PathTexturesVehicles,
  PathStaging,
  Num_Paths
};


//------------------------------------------------------------------------------
// steamws_core API function definitions
//------------------------------------------------------------------------------

void steamws_initWorkshop();
void steamws_ensureDirs();
void steamws_clearAllLocal();
steamws_state_struct* steamws_GetStateStructPtr();
bool steamws_IsBusy();
void steamws_OpenOverlayToWorkshop();
void steamws_tick();
void steamws_shutdown();

//------------------------------------------------------------------------------
// steamws_core API utility function definitions
//------------------------------------------------------------------------------
steamws_pathEnum steamws_getLocalPathEnumFromTag(steamws_itemTag tag);
std::string steamws_getLocalRelativePath(steamws_pathEnum path);
std::string steamws_getRelativeDirForTag(steamws_itemTag tag);
steamws_itemTag steamws_getTagForExt(std::string ext);
