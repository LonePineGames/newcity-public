#include "aboutPanel.hpp"

#include "spdlog/spdlog.h"

#include "../icons.hpp"
#include "../name.hpp"
#include "../platform/file.hpp"
#include "../string_proxy.hpp"
#include "../util.hpp"

#include "button.hpp"
#include "hr.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "root.hpp"
#include "scrollbox.hpp"

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

const char* audioCredits[] = {
  "By KRAFTWERK2K1 :",
  "inserting_floppy_disc.wav",
  "reading_floppy_disc_2.wav",
  "\n \n",

  "By MrAuralization :",
  "Radio » FM radio tuning",
  "waterfall",
  "Windy Ambience",
  "Windy forest",
  "Rain gutter",
  "Rainy weather",
  "Highway",
  "Floppy disk drive » Floppy disk drive - read",
  "Floppy disk drive » Floppy disk drive - write",
  "Grasshoppers » Grasshoppers",
  "\n \n",

  "By soundmary :",
  "Vehicles » tractor approaching.mp3",
  "Musical box.mp3",
  "atmos - countryside traffic munching cows1.mp3",
  "flowing river.mp3",
  "windy day.mp3",
  "tube station.mp3",
  "car arrive on gravel.mp3",
  "countryside birds and traffic.mp3",
  "small gate opening.mp3",
  "opening metal door.mp3",
  "moped passing in wet 1.mp3",
  "old digital till.mp3",
  "Footsteps » waking ice and snow.mp3",
  "Footsteps » footsteps passing on gravel - birds.mp3",
  "Footsteps » footsteps on metal.mp3",
  "Footsteps » footsteps on boggy sand.mp3",
  "Vehicles » old tractor depart.mp3",
  "Vehicles » car going through ford.mp3",
  "Vehicles » tractor engine on and off.mp3",
  "Vehicles » digger working.mp3",
  "Animals » eagle owl.mp3",
  "Animals » gulls.mp3",
  "Animals » birds traffic.mp3",
  "Cars » Car - drive away.wav",
  "Sea » Sea wash - distant music and people.wav",
  "Sea » Sea wash - with people.wav",
  "Sea » Sea wash.wav",
  "Doors » tractor door slam1.mp3",
  "Doors » tractor door slam2.mp3",
  "Doors » tractor door slam3.mp3",
  "Doors » Door close.wav",
  "Hotel lobby - large open plan.wav",
  "Airport » Airport Atmos with boarding announcement.wav",
  "\n \n",

  "By klankbeeld :",
  "school » high school canteen.wav",
  "children » kids play hide and seek suburban 01.flac",
  "white noise wind » wind in tree white birch 01.wav",
  "traffic » traffic along flat LONG 130104_00.flac",
  "industrial » solid steel hammering LOOP 130903_00.wav",
  "\n \n",

  "By hargissssound :",
  "Spring Birds Loop with Low-Cut (New Jersey)",
  "Burbling River",
  "Rain Loop(unfiltered)",
  "Backhoe",
  "Suburban Park with Children and Tennis Players(binaural)",
  "Summer sounds from my walk to work",
  "\n \n",

  "By InspectorJ(www.jshaw.co.uk) of Freesound.org:",
  "Ambience, Night Wildlife, A.wav",
  "Wind, Realistic, A.wav",
  "Cat, Screaming, A.wav",
  "Exotic Creature Song, 01.wav",
  "Lorikeet Parrot Calls, Ensemble, A.wav",
  "Rain, Moderate, A.wav",
  "Door, Front, Opening, A.wav",
  "Vacuum Cleaner, On Idle Off, Close, A.wav",
  "Airplane, Boeing, Flyby, Right to Left, A.wav",
  "Ambience, Food Court, B.wav",
  "Ambience, Children Playing, Distant, A.wav",
  "\n \n",

  "Misc. Audio Credits:",
  "\"Rain and Thunder 4\" by FlatHill of Freesound.org",
  "\"Rain-Atmo.WAV\" by director89 of Freesound.org",
  "\"Nature Sounds Of Edinburgh Series » Birds In Spring (Scotland)\"",
  " by BurghRecords of Freesound.org",
  "\"Birds Singing 03.wav\" by DCPoke of Freesound.org",
  "\"A Murder of Crows\" by Frankie01234",
  "\"Crowd_cheering_football_01.wav\" by wanna73",
  "\"Crowd Noises » Bar Crowd - Logans Pub - Feb 2007.wav\" by lonemonk",
  "\"highway urban.wav\" by cognito perceptu",
  "\"Police » Police siren.mp3\" by MultiMax2121",
  "\"Police and Emergency Vehicle Sirens » police2.wav\" by guitarguy1985",
  "\"Gunshots\" by Kleeb",
  "\"summer » 20060824.forest03.wav\" by dobroide",
  "\"Industrial » Industrial ambience\" by Lewente",
  "\"Ambient Nature Soundscapes » oceanwavescrushing.wav\" by Luftrum",
  "\"industrial workshop » General Fusion industrial workshop\" by kyles",
  "\"Commercial center tour\" by Krinkron",
  "\"AMB_S_Cars_Passing_Highway_001.wav\" by conleec",
  "\"Ambience » City ambience light traffic man running by.aif\" by bennychico11",
  "\"Highway.aif\" by amszala",
  "\"School / Children » school_ambience.wav\" by Vetter Balin",
  "\"Shortwave radio » shortwave criminal history.wav\" by uair01",
  "\"Film Sounds / Movie Sounds » Waves of Hawaii\" by florianreichelt",
  "\"Wind chimes » Wind chimes 1\" by giddster",
  "\"BigDogBarking_02.wav\" by www.bonson.ca",
  "\"aNiMaLs » cat2.wav\" by NoiseCollector",
  "\"Owl.wav\" by Johnc",
  "\"flys - I.wav\" by galeku",
  "\"Chicken Single Alarm Call\" by Rudmer_Rotteveel",
  "\"Field recordings » horse snort 2.wav\" by ERH",
  "\"animals » Sheep.flac\" by Erdie",
  "\"Mowers » Lawn tractor loop\" by ursenfuns",
  "\"Thematic Pieces » Country 01.wav\" by FullMetalJedi",
  "\"Beeps & Brakes ~Construction Site Trucks\" by Bon_Vivant_Pictures",
  "\"random » door apartment buzzer unlock ext.flac\" by kyles",
  "\"Ambience » basketball_crowd_noise_01.wav\" by joedeshon",
  "\"Skyrise Apartment with open windows - Atmos\" by KenzieVaness",
  "\"Time of life in my apartment in Marseille.wav\" by GANTELMI",
  "\"Crickets » Crickets_05.wav\" by RSilveira_88",
  "\"floppy.ogg\" by ullvieib",
  "\n \nImage Credits",
  "\"Mixed Florals dress\" by https://www.flickr.com/people/30475026@N08",
  "\"Dress with circle skirt in the wind\" by Tobias ToMar Maier",
  "\"Girl Ericeira March 2013-1a\" by Alvesgaspar",

  "\0"
};

const char* credits[] = {
  "GLFW",
  "GLEW",
  "GLM",
  "OpenAL Soft",
  "stb_image & stb_vorbis",
  "CMake",
  "GCC",
  "GDB",
  "Valgrind",
  "Callgrind",
  "KCachegrind",
  "RenderDoc",
  "Git",
  "NeoVim",
  "MuseScore",
  "FluidSynth",
  "Ogg Vorbis",
  "Miniz",
  "Linux",
};
const int numCredits = 19;

const char* mitCredits[] = {
  "OBJ-Loader (c) 2016 Robert Smith",
  "Full text - https://github.com/Bly7/OBJ-Loader/blob/master/LICENSE.txt",
  "\n \n",
  "\0",
};

const static float abtPad = 0.25;
const static float aWidth = 36;
const static float aHeight = 27;
const static vec2 abtSize = vec2(aWidth+abtPad*2, aHeight+abtPad*2);

ScrollState creditsScroll;
ScrollState audioScrollState;

void addContributors(const char* tierFile, const char* tierName,
    Part* credScroll, float* credYValue) {
  FILE *file;
  char* fn = sprintf_o("contributors/%s.txt", tierFile);
  int len = 255;
  int num = 0;

  if (file = fopen(fn, "r")) {

    float column = (aWidth-abtPad*2-1)*.5f;
    float scale = 0.8;
    char* headline = sprintf_o("%s Contributors", tierName);
    r(credScroll, label(vec2(abtPad, *credYValue), scale*1.5f, headline));
    *credYValue += scale*2.5f;

    char* line = (char*) malloc(sizeof(char)*len);
    while(fgets(line, len, file) != NULL) {
      char* trimmedLine = trimWhiteSpace(line);
      if (trimmedLine[0] == 0) continue;

      float x = num % 2 == 0 ? abtPad*2 : column;
      r(credScroll, label(vec2(x, *credYValue), scale,
            strdup_s(trimmedLine)));

      if (num % 2 == 1) *credYValue += scale;
      num ++;
    }

    if (num % 2 == 1) *credYValue += scale;
    *credYValue += scale;

    fclose(file);
  }
  free(fn);
}

bool linkCallback(Part* part, InputEvent event) {
  if(part == 0) {
    return false;
  }

  std::string url;

  switch((AboutLink)part->itemData) {
    case AboutLink::LP_WEB:
      url = "http://lonepine.io/";
      break;
    case AboutLink::LP_REDDIT:
      url = "https://www.reddit.com/r/NewCity/";
      break;
    case AboutLink::LP_DISCORD:
      url = "http://discord.gg/cz6t4J5";
      break;
    case AboutLink::LP_STEAM:
      url = "https://store.steampowered.com/app/1067860/NewCity/";
      break;
    case AboutLink::LP_TWITTER:
      url = "https://twitter.com/lone_pine_games";
      break;
    case AboutLink::LP_YOUTUBE:
      url = "https://www.youtube.com/channel/UCRc9Dfz_cZsyymCLIIdaJsg";
      break;
    case AboutLink::LP_TWITCH:
      url = "https://www.twitch.tv/lonepinegames";
      break;
    case AboutLink::LP_FACEBOOK:
      url = "https://www.facebook.com/BuildProsperity";
      break;
    case AboutLink::LP_EMAIL:
      url = "mailto:info@lonepine.io";
      break;
    case AboutLink::OBJLOAD_LICENSE:
      url = "https://github.com/Bly7/OBJ-Loader/blob/master/LICENSE.txt";
      break;
    default:
      SPDLOG_INFO("Invalid About Link value {}", part->itemData);
      return true;
  }

  // TODO: Add OSX
  #ifdef WIN32
    ShellExecuteA(0, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
  #else
    system(sprintf_o("xdg-open %s", url.c_str()));
  #endif

  return true;
}

Part* aboutPanel(float aspectRatio) {
  float uiX = uiGridSizeX * aspectRatio;
  Part* result = panel(vec2(uiX,uiGridSizeY)*0.5f - abtSize*0.5f, abtSize);
  result->padding = abtPad;

  float mlHeight = 0.0f;
  float y = 0;
  float startYOffset = 0;
  float scale = 1;

  r(result, label(vec2(0,0), 1.5f, strdup_s("About NewCity")));
  r(result, button(vec2(aWidth-1,0), iconX, openMainMenu));
  y += 1.5f;
  //r(result, hr(vec2(0,y), aWidth));
  //y += 0.25;
  startYOffset = y;

  float scrollY = aHeight - y;
  float scrollX = aWidth;
  float credYValue = 0.0f;
  float column = (scrollX-abtPad*2-scale)*.5f;
  Part* credScroll = scrollbox(vec2(0,0), vec2(scrollX,(numCredits/2)*scale));
  // Update scale for credits
  scale = 0.8;

  Part* lpWeb = button(vec2(column, credYValue + (scale*1.0f)),
    vec2(13.0f, scale), strdup_s("Follow development at https://lonepine.io"), linkCallback);
  lpWeb->itemData = AboutLink::LP_WEB;
  Part* lpRed = button(vec2(column, credYValue + (scale*2.0f)),
    vec2(13.0f, scale), strdup_s("Reddit - r/NewCity"), linkCallback);
  lpRed->itemData = AboutLink::LP_REDDIT;
  Part* lpDis = button(vec2(column, credYValue + (scale*3.0f)),
    vec2(13.0f, scale), strdup_s("Join the discussion on Discord"), linkCallback);
  lpDis->itemData = AboutLink::LP_DISCORD;
  Part* lpSte = button(vec2(column, credYValue + (scale*4.0f)),
    vec2(13.0f, scale), strdup_s("Check out NewCity's Steam Store page"), linkCallback);
  lpSte->itemData = AboutLink::LP_STEAM;
  Part* lpTwi = button(vec2(column, credYValue + (scale*5.0f)),
    vec2(13.0f, scale), strdup_s("Twitter - @lone_pine_games"), linkCallback);
  lpTwi->itemData = AboutLink::LP_TWITTER;
  Part* lpYou = button(vec2(column, credYValue + (scale*6.0f)),
    vec2(13.0f, scale), strdup_s("Follow Lone Pine Games on YouTube"), linkCallback);
  lpYou->itemData = AboutLink::LP_YOUTUBE;
  Part* lpTwc = button(vec2(column, credYValue + (scale*7.0f)),
    vec2(13.0f, scale), strdup_s("Follow Lone Pine Games on Twitch"), linkCallback);
  lpTwc->itemData = AboutLink::LP_TWITCH;
  Part* lpFB = button(vec2(column, credYValue + (scale*8.0f)),
    vec2(13.0f, scale), strdup_s("Like NewCity on Facebook"), linkCallback);
  lpFB->itemData = AboutLink::LP_FACEBOOK;
  Part* lpEm = button(vec2(column, credYValue + (scale*9.0f)),
    vec2(13.0f, scale), strdup_s("Support - lonepinegames@gmail.com"), linkCallback);
  lpEm->itemData = AboutLink::LP_EMAIL;

  r(credScroll, lpWeb);
  r(credScroll, lpRed);
  r(credScroll, lpDis);
  r(credScroll, lpSte);
  r(credScroll, lpTwi);
  r(credScroll, lpYou);
  r(credScroll, lpTwc);
  r(credScroll, lpFB);
  r(credScroll, lpEm);

  // Lone Pine Games credits
  r(credScroll, label(vec2(abtPad, credYValue), scale*1.5f,
    strdup_s("Lone Pine Games:")));
  credYValue += scale*1.5f+scale;

  r(credScroll, label(vec2(abtPad*2, credYValue), scale*1.1f,
    strdup_s("Creator, Lead Developer\n")));
  credYValue += scale*1.1f;
  r(credScroll, label(vec2(abtPad*2, credYValue), scale,
    strdup_s("Conor \"Lone Pine\" Sullivan\n")));
  credYValue += scale*2;

  r(credScroll, label(vec2(abtPad*2, credYValue), scale*1.1f,
    strdup_s("Developer, Designer, Community Manager\n")));
  credYValue += scale * 1.1f;
  r(credScroll, label(vec2(abtPad*2, credYValue), scale,
    strdup_s("Mitch \"supersoup\" Gentry\n")));
  credYValue += scale*2;

  r(credScroll, label(vec2(abtPad*2, credYValue), scale*1.1f,
    strdup_s("2D/3D Artist\n")));
  credYValue += scale * 1.1f;
  r(credScroll, label(vec2(abtPad*2, credYValue), scale,
    strdup_s("Mateus \"Gainos\" Schwaab\n")));
  credYValue += scale*2;

  r(credScroll, label(vec2(abtPad*2, credYValue), scale*1.1f,
    strdup_s("Marketing and Business Development\n")));
  credYValue += scale * 1.1f;
  r(credScroll, label(vec2(abtPad*2, credYValue), scale,
    strdup_s("David \"Swordless Mimetown\" Teraoka\n")));
  credYValue += scale*2;

  r(credScroll, label(vec2(abtPad*2, credYValue), scale*1.1f,
    strdup_s("Executive Producer, Business Advisor\n")));
  credYValue += scale * 1.1f;
  r(credScroll, label(vec2(abtPad*2, credYValue), scale,
    strdup_s("Kevin \"Galahad\" Sullivan\n")));
  credYValue += scale * 2;

  r(credScroll, hr(vec2(abtPad, credYValue), scrollX-(abtPad*6)));
  credYValue += scale;

  // Contributors credits
  addContributors("skyscraper", "Skyscraper", credScroll, &credYValue);
  addContributors("office-complex", "Office Complex", credScroll, &credYValue);
  addContributors("factory", "Factory", credScroll, &credYValue);
  addContributors("apartment-complex", "Apartment Complex",
      credScroll, &credYValue);
  addContributors("house", "House", credScroll, &credYValue);
  r(credScroll, hr(vec2(abtPad, credYValue), scrollX-(abtPad*6)));
  credYValue += scale;

  // Tech credits
  r(credScroll, label(vec2(abtPad, credYValue), scale*1.5f,
    strdup_s("Technology:")));
  r(credScroll, multiline(vec2(column, credYValue), vec2(column, scale),
    strdup_s(
      "NewCity would not be possible without free and "
      "open source software. Many programmers worked for "
      "free to create the software you use today. They did "
      "it because they believed in technology."), &mlHeight));
  credYValue += scale + mlHeight;

  // License specific credits
  /*
  r(credScroll, label(vec2(abtPad, credYValue), scale*1.5f,
    strdup_s("License-specific Credits:")));
  credYValue += scale * 1.5f + scale;
  */

  // MIT credits
  r(credScroll, label(vec2(abtPad*2, credYValue), scale,
    strdup_s("OBJ-Loader (c) 2016 Robert Smith - MIT License")));
  credYValue += scale;

  Part* objLicButt = button(vec2(abtPad*2, credYValue), vec2(23.0f, scale),
    strdup_s("Full text - "
      "https://github.com/Bly7/OBJ-Loader/blob/master/LICENSE.txt"), linkCallback);
  objLicButt->itemData = AboutLink::OBJLOAD_LICENSE;

  r(credScroll, objLicButt);
  credYValue += scale*2;

  /*
  for(int m = 0; mitCredits[m][0] != '\0'; m++) {
    r(credScroll, label(vec2(abtPad, credYValue), scale,
      strdup_s(mitCredits[m])));
    credYValue += scale;
  }
  */

  for (int i = 0; i < numCredits; i++) {
    r(credScroll, label(vec2(abtPad*2 + (i%2)*column, credYValue+(i/2)*scale),
          scale, strdup_s(credits[i])));
  }
  credYValue += ((numCredits+1)/2)*scale;
  r(credScroll, label(vec2(1+((numCredits-1)%2)*column, credYValue),
        scale*.75f, strdup_s("Fine, Richard, GNU/Linux")));
  credYValue += scale*2;

  r(credScroll, hr(vec2(abtPad,credYValue),scrollX-(abtPad*6)));
  credYValue += scale;

  // Audio credits
  r(credScroll, label(vec2(abtPad, credYValue), scale*1.5f,
    strdup_s("Audio Credits:")));
  credYValue += scale*1.5f+scale;

  for(int a = 0; audioCredits[a][0] != '\0'; a++) {
    r(credScroll, label(vec2(abtPad*2, credYValue), scale,
          strdup_s(audioCredits[a])));
    credYValue += scale;
  }

  credYValue += scale;
  r(credScroll, hr(vec2(abtPad, credYValue), scrollX-(abtPad*6)));
  credYValue += scale;
  r(credScroll, multiline(vec2(abtPad*2, credYValue), vec2(aWidth-scale, scale),
    strdup_s(
      "NewCity, and all art and music therein "
      "Copyright Lone Pine Games 2021. "
      "All rights reserved.\n \n"), &mlHeight));
  credYValue += scale + mlHeight;

  r(result, scrollboxFrame(vec2(0,y), vec2(scrollX,scrollY),
        &creditsScroll, credScroll));

  return result;
}

