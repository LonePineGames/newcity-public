#pragma once

struct ChangelogData {
  const char* version;
  const char* text;

  ChangelogData(const char* v, const char* t) {
    version = v;
    text = t;
  }
};

const ChangelogData changelogs[] = {
  ChangelogData(
    "Version 0.0.58.17 ALPHA",

    "Fixed a bug where roads and subways would randomly get cut up and deleted.\n \n"
    "Reduced Memory Usage.\n \n"
    "Some UI elements are now white (related to the budget) or gold (related to achievements and progress in the game).\n \n"
    "The Money Bar now tells you how much in the hole you are.\n \n"
    "The Tourism Rating is now harder.\n \n"
    "You can now edit building design stats (such as number of families and businesses inside) without leaving the game.\n \n"
    "There is now an achievement for beating the game on Very Hard.\n \n"
    "Buildings with negative stats are now shown in the relevant heatmaps.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.16 ALPHA",

    "Features:\n \n"
    "There is now an animation when winning the game.\n \n"
    "For winning the game, a skyscraper is now defined as a building with Density 9 or 10 (CSkyscraperMinDensity) and a minimum height of 100 meters (CSkyscraperMinHeight).\n \n"
    "The Citipedia has been doubled in size, and now contains information about Amenity Effect Scores, Budget Lines, Homelessness, Unemployment and Transportation.\n \n"
    "There are now links to the Citipedia from the Heatmaps Tool, the Zone Tool, the Amenity Tool and the Budget Panel.\n \n"
    "There is now an achievement for getting a second City Center, defined as a skyscraper at least 5km away from the first skyscraper (CCityDistance*2)\n \n"
    "There is now a chart for number of Skyscrapers and number of City Centers.\n \n"
    "The Charts Panel now has a search feature.\n \n"
    "Information about all Amenity Effect Scores can now be pinned to the sidebar.\n \n"
    "The Pollution Heatmap is now visible on water.\n \n"
    "Added advertisements to fill empty space in the newspaper.\n \n"
    "Added some visual effects to the newspaper.\n \n"
    "Force a newspaper article to appear by typing \"propaganda <newspaper article code>\" into the in-game console.\n \n"
    "Steam Workshop upload now works with new building design package format.\n \n"
    "Added buttons to the Building Designer and Design Organizer to quickly upload a building design to the Steam Workshop.\n \n"
    "Added a button to clear all Steam Workshop files, just in case.\n \n"
    "Added Charter School and College Prep Amenities. (We now have 5 different types of schools: School, Opportunity School, Magnet School, Charter School and College Prep. We likely will remove or combine 2 of these. Let us know which ones you love and which ones you hate!)\n \n"

    "Changes:\n \n"
    "There is a new Citipedia icon.\n \n"
    "Amenity Tool: buildings and roads in the way can be demolished by holding shift.\n \n"
    "Bulldozer Tool: Box delete now neatly trims roads.\n \n"
    "Cars has less negative effect on Value.\n \n"
    "Easy Difficulty: Value and Health are easier.\n \n"
    "The number of PhDs per Education Score point has been increased from 5 to 20.\n \n"
    "Community now reduces crime citywide; this effect was previously given by Health.\n \n"
    "Business Score now grants 5 (CBizPerBizPoint) extra Office Businesses per point. Employment has been rebalanced slightly."
    "The Newspaper now has four articles per day. Added some new articles.\n \n"
    "Some changes to Building Designs.\n \n"
    "Framerate is capped at 400fps, even if the FPS Cap is set to None. (We don't want to fry your GPU.)\n \n"
    "Tax rates, Loan Terms, Budget Controls on Mandatory Expenses can now be modified using LUA.\n \n"
    "Schools now educate more people.\n \n"
    "No matter how much education you build, you cannot educate more than 90% of your population to High School level and 75% to Bachelor's. The UI now makes this more clear.\n \n"

    "Fixes:\n \n"
    "A couple of small, old wooden houses (D0V0) had 9 families crammed inside! We've given them more space.\n \n"
    "Fixed some bugs in charts associated with budget lines.\n \n"
    "The Education heatmap now better represents the actual education level of citizens in the area.\n \n"
    "Transit Bias: Fixed a bug where transit was much harder than it should be (CTransitBias). Easy and Medium now have a 2x bias (buff) applied to their travel times; Hard now has no bias, and Very Hard has a 20% penalty (nerf) for transit travel time.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.15c ALPHA",

    "1. The Art Update:\n \n"
    "Transit View and Traffic Visor are now easier to read.\n \n"
    "2. Building Designer Changes:\n \n"
    "Saving a design with a new name now copies over the textures too.\n \n"
    "Delete a design from the Design Organizer; Improved the positioning of buildings in the design organizer.\n \n"

    "3. Gameplay Changes:\n \n"
    "Public and Social Housing: Two new unlockable amenities allow you to reduce the number of homeless in your city.\n \n"
    "There is now a default set of official blueprints. This only applies for new players or players that delete blueprints.txt\n \n"
    "Traffic is easier: Vehicles are now removed if they are trapped in traffic for more than their expected trip time times 3 (= CTripLimitMultiplier)\n \n"
    "Average Trip Time calculation improved: Trips longer than 4 hours (= CMaxCommute) will not contribute to Average Trip Time.\n \n"

    "Health is easier: Pollution has less effect on health; Health has a bigger radius of effect.\n \n"
    "Prosperity is easier: Employed people give prosperity in their homes.\n \n"
    "Fixing a bug where neighboring cities could unlock achievements.\n \n"
    "Police Stand amenity now available from the start of the game.\n \n"
    "Various changes to amenity stats\n \n"

    "4. Engine Changes:\n \n"
    "Fixed some memory leaks.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.15 ALPHA",

    "1. The Art Update:\n \n"
    "- Building designs have been overhauled to present an all-new art style.\n \n"
    "- Designs have been reworked as \"Design Packages.\" Editing designs, assigning textures, and designs distribution are now more convenient and intuitive. (Additional changes to the Steam Workshop integration to support this are forthcoming.)\n \n"
    "- Textures (albedo and illumination) can now be assigned to designs via an intuitive texture picker UI, and will \"stick\" to the design.\n \n"
    "- The UI has been revamped for ease-of-access to tools and information.\n \n"
    "- Load Menu now has options for \"New Game,\" \"New Scenario,\" and \"New Design.\"\n \n"
    "- Changed core game palette and mod decos.\n \n"
    "- Changed terrain trees to be textured billboards (planes that always face the camera).\n \n"
    "- Terrain trees are now visible from a longer distance.\n \n"
    "- Polluted water is no longer shiny. Instead, it\'s as opaque and murky as 2020-2022.\n \n"
    "- Several bugfixes and improvements to the tutorial system.\n \n"

    "2. Building Designer Changes:\n \n"
    "- The Config Panel now displays units.\n \n"
    "- Corrected building height in the Design Organizer.\n \n"
    "- Design Organizer now uses the pointOnLand function to place designs.\n \n"
    "- Searching with the Deco tool has been improved.\n \n"

    "3. Gameplay Changes:\n \n"
    "- Some amenities have slight changes to their costs and effects\n \n"
    "- Earthworks has been reworked to default to \"Raise/Lower\" mode. Hold LMB or Alt+LMB to Raise/Lower terrain respectively. You can still toggle back to \"Level\" mode via a button.\n \n"
    "- Earthworks cost per tick has been greatly reduced (from 200 to 2).\n \n"
    "- Hard mode is now easier. You start with more money, and there is less crime and traffic.\n \n"
    "- Scrolling a list in the UI is now a bit slower. It is a user-modifiable variable (CScrollSpeed in data/constants.lua).\n \n"
    "- Clicking on an achievement message now selects the responsible amenity.\n \n"
    "- Traffic now provides less pollution by almost half, and pollution has half as much of an impact on health.\n \n"
    "- Density now degrades Community\n \n"
    "- All NewCitizens are now speedwalkers. Their walking speed has been buffed from 5 to 20m/s, which will positively impact the distance at which transit stops are viable.\n \n"
    "- A small random factor has been added to routing to help break bad routes loose.\n \n"

    "4. Engine Changes:\n \n"
    "- Performance has been improved in situations where there are lots of visible vehicles.\n \n"
    "- Various bug fixes and stability improvements.\n \n"

    "5. Known issues:\n \n"
    "- The initial load time for the game is longer in this version. It\'s due to a loading step for design textures. Improvements to this process are already in the pipeline.\n \n"
    "- Due to the top-to-bottom change in designs and amenities, it will likely break compatibility with existing cities. YOU MAY EXPERIENCE A CRASH WHEN FIRST LAUNCHING THIS VERSION AS A RESULT. If this happens, select \"Start a New Game\" during the recovery launch. We advise making a new city for v58.15, and will offer a beta branch (pre-art-update) which will remain at v58.14 for backward compatibility.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.14 ALPHA",

    "1. Overhauled the Tutorial Experience:\n \n"
    "- All New Tutorials covering basic gameplay, transit, expressways, economy, education, heatmaps, blueprints, and winning the game.\n \n"
    "- Streamlined new player experience.\n \n"
    "- Several bugfixes and improvements to the tutorial system.\n \n"

    "2. Reworked the toolbar position and layout:\n \n"
    "- The toolbar is now positioned along the left edge of the screen, with labels for each tool, making it easier to navigate.\n \n"

    "3. Changed road and rail elevation behavior:\n \n"
    "- Roads and rails are now always built at a height relative to the terrain.\n \n"
    "- This may prevent certain bugs where roads didn't quite snap to one another, and were offset vertically.\n \n"

    "4. Changes to Heatmaps:\n \n"
    "- Pollution spreads further.\n \n"
    "- Increased Community's protective effect on Health\n \n"
    "- Neighboring cities will no longer affect the Density and Health heatmaps. Neighbors create less Crime and Pollution.\n \n"

    "5. Misc. changes:\n \n"
    "- Mixed Use zones can now spawn Residential, Retail, and Office buildings, in addition to Mixed Use buildings. Mixed Use zones cannot spawn Farms or Factories.\n \n"
    "- Added the Summer Spectacular winner names to the list used for random citizen generation.\n \n"
    "- Added many newspaper articles focused on you as the Mayor, funny characters in your city, and economics.\n \n"
    "- Added a Design Organizer to help organize building designs by zone, density and value. The Design Organizer can be accessed from the Building Designer.\n \n"
    "- Search in Controls Options.\n \n"
    "- Markdown can now display charts and animated images.\n \n"
    "- Various improvements to performance and stability.\n \n"

    "6. Plugged a number of memory leaks:\n \n"
    "- Fixed a memory leak when reading blueprints.\n \n"
    "- Fixed a memory leak during building illumination texture cleanup when resetting textures.\n \n"
    "- Fixed a memory leak when rendering multiline text in the UI.\n \n"
    "- Fixed a memory leak when rendering span UI elements.\n \n"
    "- Fixed a memory leak when caching building decoration information.\n \n"
    "- Fixed a memory leak when reading the Designs directory.\n \n"
    "- Fixed a memory leak when drawing the cursor.\n \n"
    "- Fixed a memory leak during tree cleanup when resetting terrain.\n \n"
    "- Fixed a memory leak for family members when resetting people.\n \n"
    "- Fixed a memory leak when reading the RenumTable.\n \n"
    "- Fixed a memory leak when finishing a RouteRequest.\n \n"

    "7. Improvements to stability and preventative measures for certain crashes:\n \n"
    "- Fixed a bug where reading binary data from the input file would result in reading zero-length or invalid length data when copying to cache.\n \n"
    "- Changed how Steam API functionality is shutdown to prevent race conditions and fully clean up state.\n \n"
    "- Fixed a potential crash when attempting to load a preview image for a city or design that doesn't exist.\n \n"
    "- Added some Windows specific error handling in certain situations to prevent CTDs (Crashes to desktop) unless absolutely necessary.\n \n"
    "- Added better error checking to the isEntityActive check.\n \n"
    "- Changed the frontBuffer flag to an atomic boolean to prevent certain bugs related to multithreading.\n \n"

    "8. Various other bug fixes:\n \n"
    "- Fixed a bug where the Route Inspector said citizens value their time at \"$0.00 per hour.\"\n \n"
    "- Fixed a bug in Planner Mode when building expressways that caused a \"Bad Angle\" error.\n \n"
    "- Fixed the Traffic Infoview, which was not displaying correctly.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.13 ALPHA",

    "And we're back! We've got a light update in terms of changelog items for v58.13, but expect to see that grow in the updates yet to come on the road to v1.0.\n \n"

    "Improvements:\n \n"
    "Overhauled our input management system to use KeyBinds composed of Actions and Keys. These are gathered into a KeyMap which allows players to now customize binds to suit their playstyle.\n \n"
    "UI buttons with an associated action now show the key assigned to their bind.\n \n"
    "Implmented a new binary file format for reading and writing data; it will be field tested first with the keymap and other input data.\n \n"

    "Fixes:\n \n"
    "Repositioned key labels to better suit new system.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.12 ALPHA",

    "Starting next week, we'll be starting the NewCity Summer Spectacular! Pay attention to our social media to see how to participate and what prizes you can win!\n \n"

    "Improvements:\n \n"
    "The terrain grid has been updated. We're going back to dots for the smallest grid size, and there is a compass rose pointing North. The grid can easily be modified by editing or replacing textures/land.png\n \n"
    "Small cities can now support more density.\n \n"

    "Fixes:\n \n"
    "Autosave is now enabled by default.\n \n"
    "When generating terrain for a new game, if hills are turned off, the maximum height is now CSuperflatHeight. This fixes a visual glitch some players noticed.\n \n"
    "Fixed a crash-on-launch.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.11 ALPHA",

    "Development news:\n \n"
    "We've made significant improvements to the long standing issue with early game zone demand. The early game should now be much less frustrating and tedious.\n \n"

    "Improvements:\n \n"
    "Density now grows faster up to Density 3. Density 4-10 is unchanged.\n \n"
    "Workers now get jobs immediately when moving to the city. Workers also get a new job when they get laid off, and 90% of the time when losing a job due to a long commute. This dramatically reduces unemployment, and now unemployment matches the Target Unemployment Rate. More information on this will be detailed on the blog at lonepine.io.\n \n"
    "Community, Health, Prosperity and Crime now influence population growth.\n \n"
    "Factories have zero demand until the city has 1,000 people.\n \n"
    "Offices have zero demand until the city has 10,000 people.\n \n"
    "Mixed Use has zero demand until the city has 20,000 people.\n \n"

    "Fixes:\n \n"
    "Fixed edge scrolling in all video modes.\n \n"
    "Fixed several issues that could cause population growth to stall in the early game.\n \n"
    "Fixed two situations which would cause a crash.\n \n"
    "Fixed a bug where statistics which count people by education level could be negative or inaccurate.\n \n"
    "Fixed an issue where houses turned gray when zooming out.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.10 ALPHA",

    "Development news:\n \n"
    "You asked for it, and you got it: edge scrolling is now available!\n \n"

    "Features:\n \n"
    "Added toggles for edge scrolling and locking the mouse to the window in Options->General.\n \n"
    "Added new decorations (by group): pools and building components (structure), trees on sidewalk (trees), and garbage truck (vehicles). \n \n"
    "Added and modified a variety of designs.\n \n"
    "Farm demand variables have been slightly tuned. (CFarmNoEduPositions from 15 to 25, CFarmHSPositions from 0 to 5)\n \n"
    "Pollution now spreads farther.\n \n"

    "Changes:\n \n"
    "Reduced the size of the save file buffer. (Possible fix for long/infinite save issue)\n \n"
    "Zone demand is now updated more often.\n \n"
    "Payments from the Casino and Biffco Plant are now more consistent.\n \n"

    "Fixes:\n \n"
    "Fixed a bug that made Density 10 inordinately difficult to accomplish.\n \n"
    "Fixed a bug where starting a new game at a different difficulty within the same session didn't actually change the difficulty.\n \n"

    "Known issues:\n \n"
    "Mid-late game mass transit can be buggy and isn't where it should be.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.9 ALPHA",

    "Development news:\n \n"
    "Check out our new trailer for the Tourist Update; link on lonepine.io! \n \n"
    
    "Features:\n \n"
    "Added the ability to add newline characters in text boxes by using Shift+Enter.\n \n"
    "Added and modified many Retail, Government, Industrial, Agricultural, and Residential designs.\n \n"

    "Changes:\n \n"
    "Changed the appearance of sidewalks. Thanks to qv on Discord for the suggestion!\n \n"
    "The transit stop radius has been buffed (From 30 to 40).\n \n"

    "Fixes:\n \n"
    "Fix for shader error \"\'return\' with wrong type int\".\n \n"
    "Fixed building texture assignment for police buildings.\n \n"
    
    "Known issues:\n \n"
    "Zone/demand balance tuning. \n \n"
  ),

  ChangelogData(
    "Version 0.0.58.8 ALPHA",

    "The Tourist Update!\n \n"
    "Development news:\n \n"
    "A new trailer is coming soon.\n \n"
    "Want to make your own maps? Test Mode is now the Scenario Editor, and terrain editing (Bulldozer->Earthworks) is free! We're looking forward to seeing what you build.\n \n"
    "Features:\n \n"
    "Added a terrain seam hider to prevent certain situations where terrain seams were visible.\n \n"
    "Changed Test Mode to Scenario Editor, and made Earthworks free in the Scenario Editor to facilitate creating your own unique terrains. The Scenario Editor is a work-in-progress, and subject to changes and bugs as we go. The Amenity tool is currently disabled in the Scenario Editor.\n \n"
    "Changes:\n \n"
    "Added/updated many designs including the new Gas Station, fixing shrubs in certain designs, and more.\n \n"
    "Reduced the spawn rate of some wanderers.\n \n"
    "Added a queued satmap refresh shortly after loading a save, as well as satmap refreshes on using the Bulldozer tool.\n \n"
    "Transportation Tool: Link Mode is now on by default.\n \n"
    "Nerfed tourism (CTouristsPerPoint to 200 down from 500).\n \n"
    "Reduced penalty for pedestrian walking time to get to transit. (CWalkingSpeed to 5 up from 2.5).\n \n"
    "Lots reposition and reparent themselves more intelligently.\n \n"
    "Buffed density cap from health (CDensityCapHealth to 0.5 up from 0.25).\n \n"
    "Brightened the satmap.\n \n"
    "Moved daylight options to the Visual tab in the Options menu.\n \n"

    "Fixes:\n \n"
    "Fixed SetThreadDescription exception when running the game with Windows 7.\n \n"
    "Fixed a situation where transit stops would not properly reposition themselves.\n \n"
    "When rain and snow is disabled in the options menu, the seasonal palette is disabled too. (It's locked to the summer palette.)\n \n"
    "Fixed appearance of leafless trees in the winter.\n \n"
    "Fixed vehicle illumination textures.\n \n"
    "Fixed transparent UI pixels in the palette.\n \n"
    "Fixed certain situations where lots would be created too close to the road.\n \n"
    "Console: Prevented a possible infinite loop when running the 'capture' command.\n \n"
    "Fixed some bugs in wanderer movement and vehicle interpolation.\n \n"
    "Fixed certain repeating designs.\n \n"
    "Fixed pillars culling at the edge of the screen.\n \n"
    "Removed road shine, to prevent visual glitches on suspension bridges.\n \n"
    "Ensured that the Zone tool only zones along roads.\n \n"
    "Orphaned buildings now clean themselves up if too far from a road.\n \n"
    "Removed transit stop debug logging left from development.\n \n"
    "Known issues:\n \n"
    "Occasionally saves or autosaves can take an extraordinary amount of time to complete. *Under active investigation*\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.7 ALPHA",

    "Development news:\n \n"
    "The new versioning system (Major.Minor.Build.Patch) is being delayed to minimize breaking changes.\n \n"
    "We're going to be updating the main/default branch to this patch at the same time the test patch goes live, to assist with ensuring we're working with up-to-date feedback and info on bugs.\n \n"
    "Tourist Update and new trailer coming soon!\n \n"
    "Features:\n \n"
    "Added the first flight of our new Tutorial system! With \"Enable Tutorial\" checked in Options, the Tutorial will initiate shortly after loading into an old or new city. It covers the basics for getting started in NewCity in an interactive way.\n \n"
    "Added the song \"Tourist Trap\" to the soundtrack.\n \n"
    "Changes:\n \n"
    "Improved shader error checking and logging for future debugging efforts.\n \n"
    "Improved recovery from startOggStream errors and issues loading songs.\n \n"
    "Slightly increased the number of routes processed in a batch to prevent some from being \"forgotten.\""
    "Reduced the number of factories in the Quarry Crusher design.\n \n"
    "Tweaked the global and land fragment shaders to add more definition to objects in shadow and minimize flat shading.\n \n"
    "Fixes:\n \n"
    "Fixed an issue where transit passengers would skip their ride on buses and trains.\n \n"
    "Fixed a visible seam on flat-roof buildings.\n \n"
    "Known issues:\n \n"
    "Occasionally saves or autosaves can take an extraordinary amount of time to complete.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.6 ALPHA",

    "Development news:\n \n"
    "We're going to start including a \"Known issues\" section in the changelog, to better communicate issues that we're aware of and actively working on fixes for.\n \n"
    "NewCity will be transitioning to a new versioning system in the near future, likely as soon as the next patch. - Major.Minor.Build.Patch (e.g. 0.1.1.0)\n \n"
    "Features:\n \n"
    "Added additional animated decorations for the amusement park and beach (kites, flags).\n \n"
    "Added eagle, shark, and whale wanderers.\n \n"
    "Added structure, farm, park, industrial, and vehicle group decorations.\n \n"
    "Changes:\n \n"
    "Changed Planner mode to discard all plans on exiting the mode.\n \n"
    "Changed how activity probability is represented when querying a person.\n \n"
    "Changed factory and office demand to be enabled for low population cities.\n \n"
    "Fixes:\n \n"
    "Fixed an issue with the router that could cause softlocks in low population cities.\n \n"
    "Fixed an issue where amenity mandatory expenses were treated as amenity income.\n \n"
    "Fixed certain situations where roads in constructed blueprints would not connect properly.\n \n"
    "Fixed improper icon for activities when querying a person.\n \n"
    "Fixed situations where the NoteChart and NoteObject features could be set disabled when they should be enabled.\n \n"
    "Fixed a segfault when switching modpacks.\n \n"
    "Fixed two improperly named textures, which prevented them from being properly applied to designs.\n \n"
    "Possible fix for \"procedure entry point SetThreadDescription could not be located\" error on Windows.\n \n"
    "Fixed a large number of missing decorations which would cause console spam.\n \n"
    "Removed debug logging for rendering buoys.\n \n"
    "Known issues:\n \n"
    "In certain situations, UI text can render as splotchy or broken.\n \n"
    "Deleting a rail for a train transit system does not delete the train lines.\n \n"
    "Adding new stops to existing mass transit lines deletes all later stops.\n \n"
    "Large numbers of passengers may be left waiting at transit stations.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.5 ALPHA",

    "Features:\n \n"
    "Added ability for amenities to provide positive cash flow.\n \n"
    "Added two positive cash flow amenities: the Casino and the Toxic Waste Plant.\n \n"
    "Added floor, industrial, pillar, park, residential, sign, and structure decorations; including decos specific to the Casino, Toxic Waste Plant, Zoo, and Museum.\n \n"
    "Added new cost widget in the Building Designer configuration panel.\n \n"
    "Improvements:\n \n"
    "Major balancing was done for designs across the board, which will have a significant impact on gameplay. Be prepared to start a new save.\n \n"
    "Changes:\n \n"
    "Changed the budget line item for police income to amenity income.\n \n"
    "Changed Test Mode/Building Designer to prevent city growth.\n \n"
    "Performance Improvements:\n \n"
    "Made changes to the compiler flags used for Windows-based builds, which may marginally improve performance.\n \n"
    "Fixes:\n \n"
    "Fixed the autosave infinite loop bug.\n \n"
    "Fixed an issue which cause amenity maintanence costs to be four times higher than they should have been.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.4 ALPHA",

    "Features\n \n"
    "There is now a slider to control how often the game autosaves. Drag the slider all the way to the right to disable autosaving entirely.\n \n"
    "Added an option to start the game paused.\n \n"
    "Link Mode - When building roads, click on the link button to build a chain of roads.\n \n"
    "When querying a building, there is now a button to open that building design in the designer.\n \n"
    "Modding: It is now possible to read and modify buildings and building designs in Lua. Also added Lua API calls to get the name of the city's newspaper. See the Lua API section of the Citipedia for more information.\n \n"
    "Improvements\n \n"
    "Roads are slightly less shiny.\n \n"
    "Water is now completely flat, to improve the visuals of boats.\n \n"
    "Historical buildings are now shown in the roadmap, the transit view, and the underground view.\n \n"
    "Performance Improvements\n \n"
    "Fixes\n \n"
    "Fixed issues with flickering or reappearing UI elements.\n \n"
    "Linux: Changed the launch script (newcity-linux.sh) to improve compatibility across distros.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.3 ALPHA",

    "Features\n \n"
    "Wanderers - The game now features birds, fish, boats, helicopters, balloons, and blimps. They all move about the map dynamically, and they are animated. Some buildings and amenities spawn wanderers.\n \n"
    "Modding: It is possible to make custom wanderers, and to control their movement in Lua. See the Lua API section of the Citipedia for more information.\n \n"
    "Added sound effects for trains, buses, aircraft, boats, and birds.\n \n"
    "Modding: Vehicles and wanderers can now have sound effects.\n \n"
    "FPS Cap: In the video options menu, the FPS can be capped to reduce CPU usage.\n \n"
    "When the game autosaves, there is now an animated message box.\n \n"
    "Improvements\n \n"
    "The game has a new color scheme! Colors are now brighter and more consistent. Colors change with season.\n \n"
    "The skybox has a new texture. The texture of the skybox clouds can be modified with texture/clouds.png\n \n"
    "To test the appearance of clouds, run the command \"weather 4\".\n \n"
    "The stadium amenity has been redesigned.\n \n"
    "When the UI is hidden, the selection cone is now hidden too. This makes it easier to make screenshots and videos while following vehicles and wanderers.\n \n"
    "UI: It is now easier to set sliders all the way to the left or the right.\n \n"
    "Aquatic buildings are now worth more and are more likely to spawn. Added some aquatic buildings.\n \n"
    "Vehicles and Wanderers are now included in the automatically generated images for cities and building designs.\n \n"
    "Natural beaches are smaller, to improve the spawn rate of aquatic buildings. This also fixes a visual glitch.\n \n"
    "Bug Fixes\n \n"
    "Building Designer: The land and water now renders properly for aquatic buildings.\n \n"
    "Fixed a bug where the convention center was shown as abandoned.\n \n"
    "Fixed a bug where hotel buildings sometimes would not have a hotel business inside.\n \n"
    "Fixed a memory leak. Fixed some crashes and glitches.\n \n"
    "Reduced visual glitches (far plane clipping) when the camera is high above ground.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.2 ALPHA",

    "Features:\n \n"
    "New Game Panel: Added a button to regenerate the current map\n \n"
    "Improvements:\n \n"
    "Hotels will not appear if there is no tourism.\n \n"
    "Small hotels have more rooms.\n \n"
    "Tourists go shopping more often.\n \n"
    "Bug Fixes:\n \n"
    "Fixed a massive memory leak.\n \n"
    "Fixed an issue where chunks of terrain were invisible.\n \n"
    "Fixed a bug where there was massive unemployment in pre-simulation.\n \n"
    "Heatmap values are now more consistent between the query tool, the building panel, and the map.\n \n"
    "Fixed a bug in task data logging.\n \n"
    "Fixed statistics for Homeless and Households.\n \n"
    "Fixed bad snapping when placing roads.\n \n"
  ),

  ChangelogData(
    "Version 0.0.58.1 ALPHA",

    "Features:\n \n"
    "Tourism - NewCity now features a Tourism system. Once you build an amenity with Tourist points, tourists will start to visit your city. Every Tourist point gives +500 tourists per year. The city will be given a Tourism rating, based on tourists' experiences. The higher your rating is, the longer tourists will stay -- and the more money they spend!\n \n"
    "Tourists will only visit amenities which have at least one effect point in either Tourism or Prestige.\n \n"
    "Added new building designs, including hotels, motels and resorts. There are retail and mixed use zoned buildings. They will spawn when there is demand for hotel rooms, regardless of the retail zone demand.\n \n"
    "New Amenity: Police Operations Center\n \n"
    "New Amenity: Theater\n \n"
    "LUA - added timeNow(), which returns the time of day.\n \n"
    "When placing roads, there are now icons showing intersection points and upgrade plans.\n \n"
    "New Newspaper articles\n \n"
    "Improvements:\n \n"
    "Performance has been improved. You can see data about performance by setting CLogUpdateTime = true.\n \n"
    "Many Amenities now give Tourism points. Some other amenity effect points have been updated.\n \n"
    "The Public Beach Amenity is now always unlocked. The Ice Rink now only unlocks when the weather is cold enough. (Sorry, desert city players.)\n \n"
    "The Convention Center Amenity now contains hotel rooms.\n \n"
    "The total number of retail businesses in your city is now driven primarily by the total value of retail transactions per hour. More shopping => more retail.\n \n"
    "Hospitals are no longer \"Limit one per city\"\n \n"
    "Buildings now have an effect on the Value heatmap.\n \n"
    "Less traffic in small cities.\n \n"
    "Cars will now despawn when lost rather than rerouting.\n \n"
    "The Police Headquarters Amenity now gives more Order, less Law.\n \n"
    "In-game Amenity Info now covers all information about the amenity.\n \n"
    "Visual:\n \n"
    "Shadows are now sharper. Fixed shadow acne and peter-panning.\n \n"
    "Light color is now sampled from the palette, in the same way that skybox color is. Light color is the first color patch of the three.\n \n"
    "Updated the appearance of water.\n \n"
    "Animations are now smoother.\n \n"
    "Off map roads are hidden again, except in the road map, traffic view and transit view. Fading roads heading off map are now longer and easier to see.\n \n"
    "Land Grain has been removed. It can be brought back by replacing textures/land-grain.png with textures/land-grain-alt.png\n \n"
    "Trees on boulevards are removed for now. They will be brought back when we improve road rendering in a future release.\n \n"
    "Added statues to university buildings.\n \n"
    "Known issue: there are some visual glitches such as UI elements reappearing after being hidden. To fix this, set CCullingAlgo = 0. Performance will be degraded slightly.\n \n"
    "Building Designer:\n \n"
    "Designs can now have up to 2 billion families and businesses of each type.\n \n"
    "You can now disable spawning on particular designs, by unchecking the \"Spawnable\" box. Use this for in-progress, prototype and scratch designs to prevent them from appearing in game.\n \n"
    "Designs now have a default end year of 9999. Designs which previously had an end year of 2100 (the previous default) will now have an end year of 9999.\n \n"
    "Known issue: Playing NewCity for 8049 years will result in buildings not spawning, and is not recommended for your health.\n \n"
    "Bug Fixes:\n \n"
    "Fixed several bugs related to families and family membership.\n \n"
    "Fixed flickering when placing a road.\n \n"
    "Fixed a bug when rotating or flipping blueprints.\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.8 ALPHA",

    "Features:\n \n"
    "Neighboring cities have been revamped.\n \n"
    "There are now large and small neighboring cities.\n \n"
    "The game now populates neighboring cities when generating a new map, and it simulates five years of activity before the start.\n \n"
    "Neighboring cities now have NewTrack stations, allowing you to connect your train lines to your neighbors.\n \n"
    "These changes only apply to newly generated maps.\n \n"
    "Improvements:\n \n"
    "Increased population growth.\n \n"
    "Freight deliveries are now more frequent.\n \n"
    "Improved performance when placing blueprints.\n \n"
    "Trains and buses now unload their passengers faster. The rate is controlled by CAlightingRate. This only applies to unloading; Passenger loading rate is unchanged.\n \n"
    "The statistic \"National Stock Index\" (National Economy > Macros) is now \"National Stock Market Index\".\n \n"
    "Neighboring cities now have more educated people.\n \n"
    "Bug Fixes:\n \n"
    "Lua API: Fixed an issue in getStatisticAvg() where it would return an incorrect value.\n \n"
    "Building Designer: Fixing some overlapping icons in the load menu preview image.\n \n"
    "Fixed a bug where routes without a possible mass transit solution would be recomputed rather than cached.\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.7 ALPHA",

    "Features\n \n"
    "It is now possible to pin charts from the National Economy or neighboring cities.\n \n"
    "Added some growable seaside buildings\n \n"
    "Improvements\n \n"
    "Transit routes now have a bonus no matter how long they are. Previously, only transit routes shorter than 2 hours got the bonus.\n \n"
    "On Very Hard difficulty, Value is harder and more dependent on prosperity.\n \n"
    "Bug Fixes\n \n"
    "Fixing an issue that prevented upload to steam, related to image resolutions.\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.6 ALPHA",

    "Schedule Update: We will be moving to a 2-week release cycle, starting next week. There will not be a patch on December 24th, and every other week thereafter. There will be a patch on 17th and the 31st.\n \n"

    "Improvements:\n \n"
    "Removed the density requirement for the Public Pool, Social Services, Central Library, Courthouse, Hospital, and Big Park.\n \n"
    "Building Designer: Added a line of buoys to represent low tide for aquatic buildings. Also adding some text explaining the purpose of low tide and high tide.\n \n"
    "When showing a price of $0, the game now says \"Free\" instead of \"$0.00 cents.\"\n \n"
    "When budget items are pinned to the right, negative values are highlighted in red.\n \n"
    "Bugfixes:\n \n"
    "Fixed a crash related to message board achievement messages.\n \n"
    "Fixed an issue where always available designs were not available on new games (not loaded from a save.)\n \n"
    "Fixed a bug where water didn't quite connect with land in the building designer.\n \n"
    "Fixed a visual glitch on the water.\n \n"
    "Fixed a bug where trees would shift around as buildings were built. Terrain trees are no longer randomly rotated.\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.5 ALPHA",

    "Features:\n \n"
    "Adding Tennis Court, Basketball Court, and Ice Rink amenities.\n \n"
    "Added new statistics: Deaths and Average Age at Death\n \n"
    "Added powerful new cheat code: the_snap\n \n"
    "Improvements:\n \n"
    "Text and icons now have a shadow, improving legibility on bright backgrounds.\n \n"
    "The Amenity Tool UI has been changed.\n \n"
    "Reduced the rate of death.\n \n"
    "Accelerated Start is now the default for new cities.\n \n"
    "There is now more traffic in small cities. CTrafficRate0 defines the traffic in small villages and CTrafficRate1M defines the traffic in a city of 1 million people.\n \n"
    "Roads and Expressways now have a lighter color. Expressways are now lighter than roads.\n \n"
    "When loading and saving cities or design files, the menu now keeps it's scroll position better.\n \n"
    "Building design images now have an aspect ratio of 4:3, to better match the Steam Workshop.\n \n"
    "Amenities which are not unlocked are now shown in the Amenity Tool (4), but they are grayed out and cannot be built.\n \n"
    "Fixes:\n \n"
    "To prevent flicking and visual glitches, the new culling algorithm has been disabled by default for now.\n \n"
    "Fixed a crash on load.\n \n"
    "Fixed a bug where zoned lots would be visible when the game first loaded, even if the \"Show Zoned Lots\" option was unchecked.\n \n"
    "Fixed an issue in road building where a sloped road was \"Too Steep\" due to bad splitting.\n \n"
    "Fixing a bug where dead people remained members of their household. Households are now removed when all members are dead.\n \n"
    "Fixed a bug where a person didn't quit their job when they retired.\n \n"
    "The console has been moved to the bottom right.\n \n"
    "Fixed a bug where highlighting of roads, expressways and railroads was not visible when using the Query Tool (1>Q) or Transportation Tool (2).\n \n"
    "Fixed a bug where the beach had grass in the building designer.\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.4 ALPHA",
    "Features:\n \n"
    "People now die based on age and health. This will cause a slight temporary reduction in population in existing cities.\n \n"
    "Retirement: people now retire when they reach age 65 (CRetirementAge).\n \n"
    "Always Night option. Daylight options moved to the Video tab in Options Menu.\n \n"
    "Toggle gridlines option.\n \n"
    "Added new building designs.\n \n"
    "Donuts.\n \n"
    "Improvements:\n \n"
    "Cars are now less likely to take an offramp followed by an onramp.\n \n"
    "Education heatmap is now easier.\n \n"
    "Value is now more positively affected by education.\n \n"
    "Value is now more negatively affected by density.\n \n"
    "Value is now more negatively affected by cars and trucks.\n \n"
    "Value is now more negatively affected by crime, pollution.\n \n"
    "Value is now more difficult in Hard and Very Hard difficulty modes.\n \n"
    "Reduced pillar snapping distance when building roads.\n \n"
    "Now using higher-performance culling algo by default (CCullingAlgo=1).\n \n"
    "Bug Fixes:\n \n"
    "Fixing visual corruption when building transit lines\n \n"
    "Fixed a bug in the Amenity Tool where colliding buildings were highlighted in red.\n \n"
    "Fixed a periodic flicker when generating the satmap.\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.3 ALPHA",
    "Features:\n \n"
    "Task Scheduler: NewCity now has a frame-by-frame task scheduler to smooth out framerates and prevent lag spikes. All parts of the game update and user interface are broken up into tasks. The scheduler can delay tasks until future frames, mitigating long frames and creating a smoother framerate. Learn more by setting CLogUpdateTime = true.\n \n"
    "(Experimental) New Culling Algorithm: Disabled by default, we are experimenting with a new high-performance culling algorithm. Culling is the process of filtering all the entities in the game scene to only those which are visible by the camera. With hundreds of thousands of buildings, this has a significant performance impact. We are experimenting with a new algorithm which is faster and can be run on a separate thread. Set CCullingAlgo to 1 or 2 to test this feature.\n \n"
    "Improvements:\n \n"
    "Several smaller performance improvements.\n \n"
    "Added tooltips to the Charts Panel and other places they were missing.\n \n"
    "Farms and factories now produce more freight.\n \n"
    "Police Headquarters and Police Stand are now easier to unlock\n \n"
    "Preview images for cities are now zoomed closer. The zoom is controlled by CPreviewImageDistance.\n \n"
    "Roads are now slightly shiny.\n \n"
    "Water is now a darker color.\n \n"
    "Bug fixes:\n \n"
    "Fixed a bug where a new city would have the wrong amount of money\n \n"
    "Fixed a bug where game difficulty settings were loaded incorrectly\n \n"
    "Fixed a bug where the preview image was sometimes missing objects or terrain.\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.2 ALPHA",

    "Features:\n \n"
    "Added new waterfront amenities: Beach, Marina and Waterfront Amusement Park.\n \n"
    "Neighborhood Affinity: People now prefer to work, shop, and visit friends close to home rather than across the map. When a person wants to engage in an activity, the Route Broker searches for a destination in a spiral pattern. CBrokerSearchDistance determines the maximum search distance.\n \n"
    "Very Hard difficulty now works. Money, density, crime and pollution are harder, people can move out of the city and business can close in Very Hard difficulty.\n \n"
    "The Building Selection Panel now show the minimum value and density for the design.\n \n"
    "The Modpack Menu now shows preview images.\n \n"

    "Improvements:\n \n"
    "The performance of terrain rendering has improved. This will speed up load times and speed up the game in large cities when unpaused.\n \n"
    "The newspaper now won't publish the same article day after day.\n \n"
    "Density is now easier and less car-dependant.\n \n"
    "Easy difficulty now has easier density, amenities are cheaper, and eminent domain is very cheap.\n \n"

    "Bug fixes:\n \n"
    "Fixed a bug where statues wouldn't render.\n \n"
    "Fixed a crash when opening the modpack menu.\n \n"
    "Fixed a bug where modpack and workshop designs would not show the preview panel in the load menu.\n \n"
    "Fixed a bug where the image in the preview panel would not update when saving a design.\n \n"
    "Fixed a bug where music didn't play.\n \n"
    "Farms no longer glow at night.\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.1 ALPHA",

    "Steam Workshop support for designs and modpacks. It is now possible to upload building designs and modpacks, as well as subscribe to them. Once you subscribe to a workshop item, the game will automatically download the item. You might have to restart the game.\n \n"
    "On Linux without Wine, the game now launches via a script which preloads OpenGL and the Steam API. This should improve Linux compatibility.\n \n"
    "Fixed a bug where the edge of the map didn't render.\n \n"
    "Fixed a crash when switching to the Building Designer\n \n"
    "When placing a blueprint in Test Mode, zones will now be applied.\n \n"
    "Added some newspaper articles related to soccer (aka non-American football).\n \n"
  ),

  ChangelogData(
    "Version 0.0.57.0 ALPHA",
    "Added ambient fog and terrain fade-in.\n \n"
    "Added animated decorations using palette based animations.\n \n"
    "Added Park and Vfx group decorations.\n \n"
    "Remixed the Logging In music track for the vapor modpack.\n \n"
    "Fixed vehicles in vapor modpack.\n \n"
    "Fixed certain situations where the Steam Workshop thread would get stuck on an action and spam the console with an endless logging loop.\n \n"
    "Fixed bug where roads would clip on the edge of the screen.\n \n"
    "Broadened preview image capture in game mode.\n \n"
    "Added California cities to the list of neighboring cities.\n \n"
    "Added improved logging for missing building decorations.\n \n"
  ),

  ChangelogData(
    "Version 0.0.56.6 ALPHA",
    "Introducing experimental support for subscribing to Steam Workshop items.\n \n"
    "Fixing a crash when loading 125x125 maps.\n \n"
    "New and improved Building Selection Panel. This is the panel that pops up when using the Query Tool (1) on a building.\n \n"
    "Added the Statue decoration. The statues are a reward for IndieGoGo contributors. To see the statues in action, open G6x6_terracotta_army in the building designer. The statues will be added to buildings next week.\n \n"
    "Fixing a bug where building designs with no structures would get an image of just grass.\n \n"
    "Water no longer rises and lowers due to tides. This is because tides make it difficult to place items on water.\n \n"
    "The Earthworks Tool (5 > E) now has a cost.\n \n"
    "Transit stops will now be removed when they have no transit lines.\n \n"
    "Removed the ability to shut down a transit system. If you accidentally shut down your train or bus system, they will be restored, although you will need to replace the lines and stops.\n \n"
  ),

  ChangelogData(
    "Version 0.0.56.5c ALPHA",
    "Possible fix for a readFromFile() crash on Windows machines.\n \n"
    "Added ./images/house_icon.png into the final build package for the Steam Workshop placeholder image.\n \n"
  ),

  ChangelogData(
    "Version 0.0.56.5 ALPHA",
    "Improved Steam Workshop uploading to support four file types and proper tagging.\n \n"
    "Added placeholder text and a Steam Overlay redirect to Steam Workshop tabs that don't have unique functionality yet.\n \n"
    "Added staging area cleanup on launch for the Steam Workshop.\n \n"
    "Added Boat group decorations, like the sailboat, kayak, motorboat, and yacht.\n \n"
    "Added contributor statues as.obj files, with procedural placement in-game to come.\n \n"
    "Optimized the size of terrain render chunks and the tree simple distance (LOD) for improved performance.\n \n"
    "Changed the tree model to include multiple distinct trees, minimizing the number of tree entities needed and improving performance.\n \n"
    "The Lua API now has functions to read player input, control the game's camera, and read terrain elevation.\n \n"
    "Set the default Bulldozer subtool to the Destroy subtool.\n \n"
    "Made the Earthworks Bulldozer subtool unlockable with the Town achievement (5k population).\n \n"
  ),

  ChangelogData(
    "Version 0.0.56.4 ALPHA",
    "Steam Workshop Upload support. This is an early preview.\n \n"
    "The game now captures a preview image when saving a city or building design.\n \n"
    "When loading a city or building design, the preview image is shown. Extra info is displayed on building designs.\n \n"
    "Added the Memorial amenity and associated achievement.\n \n"
    "New aquatic building decorations.\n \n"
    "More work on vapor modpack.\n \n"
    "Metric units are now default for new players. This will not effect existing players.\n \n"
    "Capture a map of the city as a PNG file by running \"capture 0 map.png\" on the in-game console.\n \n"
    "Fixed a bug where the zoomed-out view, called the satmap, was too dark.\n \n"
  ),

  ChangelogData(
    "Version 0.0.56.3 ALPHA",
    "Added new UI sounds and a music track to the Vaporwave modpack\n \n"
    "Added the BART train model to the Vaporwave modpack, which replaces the diesel engine and passenger car.\n \n"
    "Added new textures to the Vaporwave modpack for government and mixed use buildings.\n \n"
    "Changed Vaporwave palette colors.\n \n"
    "Added new console commands: \"songinfo\" to display debug info for loaded songs, \"songplay\" to play a song by index, and \"songshuffle\" to shuffle the current song.\n \n"
    "Added documents on the Palette Texture File Format and the Building Textures File Format to the Citipedia.\n \n"
    "Added new Industrial and Residential designs and associated textures.\n \n"
    "Fixed backfaces with the satellite dish prop."
    "Buildings placed in Test mode will no longer be set as Historical.\n \n"
  ),

  ChangelogData(
    "Version 0.0.56.2 ALPHA",
    "The vapor modpack has been expanded with new art and content. Vapor now has different game rules, with pollution and crime being more difficult, but density being easier.\n \n"
    "The Classic modpack now starts in 1900 and has a custom font.\n \n"
    "Test Mode: It is now possible to place zones and buildings in Test Mode.\n \n"
    "Test Mode: When selecting \"Reset Test\" from the main menu, the New Game Panel is now shown. This makes it possible to create tests with complex (non-flat) terrain.\n \n"
    "Test Mode: It is now possible to modify terrain using the bulldozer tool.\n \n"
    "Modding: Control tree spawning frequency with CTreeInnerFrequency and CTreeOuterFrequency\n \n"
    "Modding: It is now possible to run Lua scripts and modify statistics, zone demand, and the player's budget. See the Citipedia documentation and the Sandbox modpack for more information.\n \n"
    "Zone demand is now measured from 0% to 100%. Before it was 0% to 50%.\n \n"
  ),

  ChangelogData(
      "Version 0.0.56.1 ALPHA",
      "Trees are no longer static cones. They now come from .obj files and are fully moddable. Deserts now have rocks and cacti instead of pine trees.\n \n"
      "Building trees now sway with the wind.\n \n"
      "The terrain now has a textured grain.\n \n"
      "It is now possible to add and remove trees using the Bulldozer Tool (5).\n \n"
      "Fixed some bugs related to achievement unlocking. NOTE: You may see certain achievements unlock again in old saves.\n \n"
      "Disabled the tax lock feature, as it was causing problems.\n \n"
      "Budget related achievements are now easier to unlock. We changed the text to use less crisis language.\n \n"
      "Park lots and unzoned lots now won't show a density stack.\n \n"
      "Fixed a bug where vehicles would jump around.\n \n"
      "The Hospital now fits between two roads placed one tile apart.\n \n"
      "Building Designer: Fixed a bug in the numerical inputs.\n \n"
      "Citipedia documentation for pollution now mentions parks.\n \n"
      "Citipedia documentation has been added for modding features.\n \n"
      "Modding: The game's font can be changed by placing a TTF font file at modpacks/<your modpack>/fonts/font.ttf. If it is a wide font, use CTextWidthAdjust = 0.8 to narrow it.\n \n"
      "Modding: Disable default building textures in a modpack by using CDisableDefaultBuildingTextures. Disable default building designs with CDisableDefaultBuildingSet. Disable default newspaper articles with CDisableDefaultNewspaperArticles. Disable default terrain config (trees) with CDisableDefaultTerrainConfig. Disable default vehicles with CDisableDefaultVehicleModels.\n \n"
      "Modding: Use CMoneyMultiplier to multiply money values by some factor. For example, if you want a more classic game feel where budgets are in the thousands, not millions, set CMoneyMultiplier = 0.001.\n \n"
      "Modding: It is now possible to overwrite vehicle model defaults.\n \n"
      "The Vapor modpack has been updated.\n \n"
  ),

  ChangelogData(
    "Version 0.0.56.0 ALPHA",
    "Added the Citipedia - Open it by clicking the Question Mark icon on the toolbar.It will be a dynamic and growing source of information for gameplay mechanics and FAQ style answers.NOTE: The Citipedia is in an early preview state and subject to significant change.\n \n"
    "Added the Newspaper - Once you have your first business, you'll start getting your city's newspaper once per day.It will have articles containing critical info to help you manage your city, as well as some more lighthearted and entertaining pieces.NOTE: The Newspaper is in an early preview state and subject to significant change.\n \n"
    "Added new Lua API constants for the Newspaper, as well as statistics available to modders for creating their own Newspaper articles with unique triggers.\n \n"
    "Added box delete to the Destroy Tool.\n \n"
    "Added a missing contributor to the list in the About Panel.\n \n"
    "Added new building decos for the Residential, Floor, Park, and Vehicle groups.\n \n"
    "Added new building designs for Residential and Retail.\n \n"
    "Added some hidden statistics formerly used for debugging to the Stats Panel.\n \n"
    "Added the ability to set a minimum year and a maximum year for designs in the Building Designer.\n \n"
    "Added the ability to hide lots visually.\n \n"
    "Achievements have migrated to Lua for easy modification.\n \n"
    "Building design can now have more than 50 homes or businesses.\n \n"
    "Changed the way saves are loaded with the Underground View active to prevent bugs with building visibility.\n \n"
    "Changed the amenity category Parks to Recreation to better reflect the variety available.\n \n"
    "Changed the Pavilion building design to require less density.\n \n"
    "Changed the 181 Fremont Skyscraper building design to be density 9, so it's properly considered a skyscraper by the game logic.\n \n"
    "Fixed a bug in how sales tax is applied to unemployment; the effect has been increased\n \n"
    "Fixed a bug in the Mass Transit Router.\n \n"
  ),

  ChangelogData(
    "Version 0.0.55.7 ALPHA",

    "Added new park decorative objects.\n \n"
    "Added new floor decorative objects.\n \n"
    "Added new residential decorative objects.\n \n"
    "Added new retail and residential designs.\n \n"
    "Added the currently playing song to the Options Audio tab.\n \n"
    "The save game menu now shows files with similar filenames to the current save.\n \n"
    "Fixed a crash related to undo and deleting objects in the Building Designer.\n \n"
    "Fixed an issue when connecting a road to the end of another road; the road now connects to the intersection properly.\n \n"
    "Fixed custom textures on giant apartment complexes.\n \n"
    "Fixed lot generation behavior by preventing it when the road is underground or otherwise shouldn't have lots associated.\n \n"
  ),

  ChangelogData(
    "Version 0.0.55.6 ALPHA",

    "Implemented a hashmap for the draw buffer as part of ongoing efforts to improve performance and memory usage.\n \n"
    "Added texture to snow cover on mountainous terrain.\n \n"
    "Added variance to land color and trees based on slope.\n \n"
    "Added new decorations, building textures, and palette changes.\n \n"
    "Fixed influences on Value heatmap.\n \n"
    "Fixed culling of labels, the cursor, and nodes when they should be visible.\n \n"
    "Prevent scaling and rotating at the same time in the Building Designer.\n \n"
    "Removed unlock for pedestrian paths, as they are not fully implemented.\n \n"
    "Removed unnecessary entities from certain terrain chunks.\n \n"
  ),

  ChangelogData(
    "Version 0.0.55.5 ALPHA",

    "When hiding labels, the labels of neighboring cities are hidden too.\n \n"
    "New decorations and building designs, including a designed police station, police headquarters, police stand, new giant mansion, trailer homes and mobile homes.\n \n"
    "Building Designer: it is now possible to move entire building designs in the X and Y axis. Check the paint tab.\n \n"
    "Fixed an issue where unzoned lots were displaying in the building designer.\n \n"
    "Low value and low density buildings are less likely to spawn in high value, high density areas.\n \n"
  ),

  ChangelogData(
    "Version 0.0.55.4 ALPHA",

    "Tunnels and viaducts are now more expensive. Road tunnels now cannot be built under deep water, but train tunnels still can. Messages about problems building roads are now more visible.\n \n"
    "Water is now visible in the Transit View, Road Map View, and Underground View.\n \n"
    "The load game menu now has a search bar. This is particularly useful in the Building Designer. When loading a building design, you can now see building designs from only the modpack directory.\n \n"
    "Increased retail demand. Retail gives more prosperity.\n \n"
    "Reduced the amount of negative community from offices and retail.\n \n"
    "Colleges now give a bonus to the cap on bachelor's degrees and doctorates.\n \n"
    "Prevented a case where both tax locks would happen at the same time.\n \n"
    "The camera can now be zoomed using the plus and minus keys, for players that do not have a mouse with a scroll wheel.\n \n"
    "When the Zone Tool is selected, unzoned lots are displayed in red.\n \n"
    "Scrollboxes can now be controlled with the mouse.\n \n"
    "The maximum Level of Detail setting has been increased, allowing for maximum-quality screenshots.\n \n"
    "Vehicles are now visible from further away.\n \n"
    "New house designs, including shanties.\n \n"
    "The sun now comes from a higher angle in the summer.\n \n"
    "In the Building Designer, roof-only structures can be made, for bungalow-style porches. With the Decoration Tool open and a decoration selected, press G to move that decoration without needing to click the mouse. Decorations and structures can now be hidden, making complex designs easier to modify. The season can now be changed.\n \n"
  ),

  ChangelogData(
    "Version 0.0.55.3 ALPHA",

    "Added an option to control Level of Detail, to tune for performance or quality.\n \n"
    "Added the Classic modpack, which has a retro color scheme.\n \n"
    "Added more building textures.\n \n"
    "Removed two designs for tall buildings that would spawn in low-density retail zones.\n \n"
    "Fixed an issue where vehicles would get stuck.\n \n"
    "Camera perspective is now limited to a FOV of 90 degree or less. (Was 120.)\n \n"
    "Fixed an issue with entity culling in perspective view.\n \n"
    "Fixed an issue with OpenAL, and improved sound logging.\n \n"
    "Reduced z-fighting on buildings with flat roofs.\n \n"
  ),

  ChangelogData(
    "Version 0.0.55.2 ALPHA",

    "Added smooth scrolling to scrollboxes\n \n"
    "Added toggle for contour lines on terrain\n \n"
    "Added new art assets for decorations\n \n"
    "Added more color and variety to the building illumination textures\n \n"
    "Added the Federal Building amenity and associated achievement\n \n"
    "Added LOD simple meshes for vehicles\n \n"
    "Fixed certain situations where University Buildings wouldn't unlock\n \n"
    "Fixed improper calculation of economic statistics when reloading saves\n \n"
    "Fixed amenities disappearing in certain situations\n \n"
    "Fixed an issue where vehicles would get stuck at the end of lanes\n \n"
    "Fixed incorrect amenity maintanence display\n \n"
    "Fixed counting error in the People Sick statistic\n \n"
    "Fixed some situations around Building Highlights that would result in a crash\n \n"
    "Improved error handling and logging for OpenAL errors\n \n"
    "New and Improved Building Designs, including new Skyscrapers\n \n"
    "Removed the negative effect of vehicles on the Health heatmap\n \n"
  ),

  ChangelogData(
    "Version 0.0.55.1 ALPHA",

    "Architect Update\n \n"
    "Major improvements to the Building Designer. Custom building decorations can be added to data/decos.lua. The Building Designer now features undo and redo.\n \n"
    "In the Building Designer, structures and decorations can now be individually selected. Once selected, they can be duplicated or deleted, and their parameters can be manipulated using the numerical inputs. Their type can be changed by clicking the pointer icons in the Structure Tool (2) or Decoration Tool (3).\n \n"
    "Structures and decorations can now be moved vertically (off the ground), by holding shift while dragging the green handle up or down. New decorations can be rotated and scaled (but legacy decorations cannot). The Building Designer now features a variety of new decorations, including modular pipes and catwalks. There are new building designs, and many designs have been updated.\n \n"
    "In the Building Designer, snap to grid can now be disabled with TAB, when the Structure Tool (2) or Decoration Tool (3) is open. With the Structure Tool (2) open, Change slope of structure roofs by holding shift and dragging on the green handle up or down.\n \n"
    "Gambrel roofs added.\n \n"
    "Graduate School - The \"Tech Hub\" achievement, which is unlocked with a Density 7 office building and a Technology score of 50, grants the Graduate School amenity, which provides college degrees and can be placed multiple times. It is the only unlimited source of college degrees.\n \n"
    "Buildings can now be marked as historical. Click \"Mark Historical\" after selecting a building with the Query Tool (1>Q). Historical buildings will not be destroyed by other buildings or roads, or due to abandonment, but they can still be bulldozed directly.\n \n"
    "The Options Panel is now organized into tabs.\n \n"
    "In the Zone Tool (3), overzone protection is now the TAB hotkey. Park zones are now the B hotkey. Density control no longer has a hotkey.\n \n"
  ),

  ChangelogData(
    "Version 0.0.54.9 ALPHA",

    "Accelerated Start - When starting a new game, select \"Accelerated Start\" to unlock all transportation features and zones from the start of the game.\n \n"
    "Map Seeds - When starting a new game, the map seed is shown and can be edited. The seed can be shared with friends or the fan community. The seed is also shown when clicking on the city name in the top left corner.\n \n"
    "Road Map - Click the road icon on the Infoview Toolbar (bottom of the screen) to see a road map.\n \n"
    "Create maps of custom sizes by setting CCustomLandSize.\n \n"
    "The right mouse button now always pans the camera. In the road tool, hold ctrl while clicking the right mouse button to copy a road type. In the zone tool, hold ctrl while clicking the right mouse button to copy a zone type.\n \n"
    "The performance of the perspective camera has been improved.\n \n"
    "Basic Dorms and the Dorm Tower are now cheaper.\n \n"
    "Citizens now visit the store less often, and amenities more often.\n \n"
    "Fixed a bug where transit time estimates didn't account for walking and waiting.\n \n"
    "The route inspector now states how long citizens are willing to wait to save one dollar.\n \n"
    "Snow no longer covers the land grid.\n \n"
    "Fixing a bug which would cause router threads to get stuck in an infinite loop.\n \n"
    "Fixing a bug where the game would quit if escape was pressed when an error message was showing.\n \n"
    "Fixed a crash related to vehicle physics.\n \n"
    "Fixed some instructional text that was out of date. Factories no longer require density, and the school is unlocked at 1,000 people, not 5,000.\n \n"
    "Various performance improvements.\n \n"
  ),
  ChangelogData(
    "Version 0.0.54.8 ALPHA",

    "Fixed a few building designs that were marked as agricultural zone but should have been industrial. Many players complained that this destroyed demand for retail, since both retail and industry uses high school educated workers.\n \n"
    "Buildings will now only spawn with designs that exist in the filesystem. If a design is deleted from the filesystem, it will not spawn again.\n \n"
    "The Transit View now shows transit coverage, so you can see which parts of your city can access transit.\n \n"
    "The catchment area of bus and train stops has been increased.\n \n"
    "It is now possible to prevent the zone tool from rezoning already zoned lots. Find the new button at the top of the zone tool.\n \n"
    "The Budget Panel now has tooltips explaining the purpose of each line of the budget.\n \n"
    "The Heatmap Toolbar buttons now open the Heatmap Tool, which displays helpful information about the Heatmap or Infoview.\n \n"
    "When querying a building, clicking on the heatmap block chart will open that heatmap. Tooltips have been added.\n \n"
    "The appearance of the sky has been improved.\n \n"
    "Contour lines now show on the terrain.\n \n"
    "Fixed a crash related to trains.\n \n"
    "Fixed a crash when launching the game.\n \n"
  ),

  ChangelogData(
    "Version 0.0.54.7 ALPHA",

    "When building roads, grid snapping can now be disabled by pressing TAB. Road building is now smarter, allowing expressways to be built closer together. A road or railway can be built above another by elevating it (Z) and holding SHIFT while building it.\n \n"
    "Changed Viaduct/Tunnel hotkey to H. (Formerly TAB)\n \n"
    "It is now possible to remove a joint (aka node) between two road segments with the bulldozer tool, in certain circumstances.\n \n"
    "Updated instructions to reflect changes in hotkeys.\n \n"
    "The game now takes an image of the map from above. It uses this image to render buildings and roads from far away, which amounts to a significant performance improvement in the orthographic view.We are still actively investigating improvements to the Perspective camera, and hope to have a fix soon.\n \n"
    "Fixed a bug where very few people took transit, because transit was estimated to take too long. The Route Inspector (Query Tool(1) > Route Inspector (R)) now shows more information about how citizens decide between car and transit.\n \n"
    "Added drama modpack for cinematic camera movement. The same mod we used in creating our trailers!\n \n"
    "Updated vapor modpack.\n \n"
    "Fixed performance modpack; it helps when running on low-end machines.\n \n"
    "Fixed missing illumination textures for certain building textures.\n \n"
    "Added redistributable libraries to package. (OpenAL32.dll and libOpenGL.so.0)\n \n"
    "Improved logging in several ways: Removed scan on launch which was causing long load times. Set up clearing of old game_log data to prevent the accumulation of large game_log files.Changed behavior around creating numbered game_log copies to only do so if the sentinel file is present, indicating a crash.\n \n"
    "Improved OpenGL error handling and notifications.\n \n"
    "Updated our IndieGoGo contributor info and in-game citizen names in the About panel with all Visible contributors as of the end of the campaign. (PM supersoup on Discord if you have any questions)\n \n"
    "Updated About panel links.\n \n"
    "Vehicles are no longer slowed down by short segments of road. Vehicle speed is now shown when selected by the query tool.\n \n"
    "Integrated NewCity application icon on Windows.\n \n"
    "Increased agricultural demand.\n \n"
    "Added pollution visual effect, which is colored and intensified based on the time-of-day.\n \n"
    "Reorganized palette texture. The heatmap colors and sky colors are now fetched from the palette.\n \n"
    "The game now slows itself down to prevent the vehicle simulation from falling behind, to prevent unexpected behavior.\n \n"
    "Removed unexpected Zone tool behavior when right-clicking to scroll around the map.\n \n"
    "Disable autosaves with CAutosaveDisabled = true in Lua.\n \n"
    "Various fixes for crashes and cleanup of leftover debug code.\n \n"
  ),

  ChangelogData(
    "Version 0.0.54.6 ALPHA",

    "Fixed shader issues for players with Intel HD Graphics. Note: Intel HD Graphics are not officially supported due to performance issues.\n \n"
    "Improved explanation of road elevation features. In the Transportation Tool (2), roads can be raised and lowered by pressing the up and down arrows (Q and Z). By default, engineers will build a trench or embankment under your roads, but if you select the tunnel/viaduct option (tab), they will build a tunnel or viaduct (bridge structure) instead.  Note: You do not need pillars to make elevated roads (or viaducts) over land.\n \n"
    "Added our artist, Mateus \"Gainos\" Schwaab to the About Panel."
  ),

  ChangelogData(
    "Version 0.0.54.5 ALPHA",

    "Increased population growth. Population growth is now controlled by CFamilySpawnRate\n \n"
    "Improved shader versioning and loading. \n \n"
    "Added new residential and vehicle textures. \n \n"
    "Added new building designs. \n \n"
    "Fixed a crash with certain custom textures. \n \n"
    "Removed overlapping text label in Transit subtool. \n \n"
    "Fixed tooltips for subtools like Expressway and Rail. \n \n"
    "Reduced amount of vehicles' negative Health influence. \n \n"
    "Added separate constant for trucks' negative health influence. \n \n"
    "Fixed certain situations where the instruction panel could not be closed. \n \n"
    "Fixed a crash in the Building Designer when making a modification then loading another design. \n \n"
    "Changed construction cost WSUI element to not render red with insufficient cash in Planner Mode. \n \n"
    "Changed modpack selection behavior to reset when backing out of the mod selection menu. \n \n"
    "Restored old land grid texture. \n \n"
    "Fixed a performance issue when querying buildings with large numbers of people inside.\n \n"
    "Added tooltips to explain viaducts and tunnels in the Transportation Tool. \n \n"
  ),

  ChangelogData(
    "Version 0.0.54.4 ALPHA",

    "Fixed crashes when building or removing transit lines.\n \n"
    "When using the building designer, the game now informs you of where building design files are loaded from and saved to.\n \n"
  ),

  ChangelogData(
    "Version 0.0.54.3 ALPHA",

    "Factories no longer require density.\n \n"
    "The bulldozer can now destroy any building, for the eminent domain cost.\n \n"
    "Adding more building textures, including more amenity textures.\n \n"
    "New terrain grid texture. Changed blending method of the terrain grid texture. We are experimenting with natural grass textures. Try renaming textures/land_alt.png to land.png to see a natural grass texture. Set CLandTextureFiltering to true to remove pixelation.\n \n"
    "Web links in the about panel now open in the Steam overlay browser or another browser.\n \n"
    "Smoother vehicle motion.\n \n"
    "Realigned several building designs with the roadway, so they would not appear in the middle of the street.\n \n"
    "Fixed a bug where train station platforms were not unlocked with NewTrack.\n \n"
    "Train and bus windows are now illuminated. Fixed train headlights.\n \n"
    "Fixed a bug where illegal roads could not be cancelled.\n \n"
    "Fixed a bug when snapping roads to bridge pillars.\n \n"
    "Fixed \"Double removeEntity\" crash.\n \n"
    "Fixing stacktraces (debug logging) on MSVC builds.\n \n"
    "Fixed a bug where big factories would unlock They Paved Paradise and Dreaming of Open Space (residential density achievements).\n \n"
    "Fixed some typos.\n \n"
  ),

  ChangelogData(
    "Version 0.0.54.2 ALPHA",

    "Fixed a crash when saving building designs.\n \n"
    "Fixed a bug in weather visuals.\n \n"
    "Added a command to change the weather.\n \n"
    "Fixing a bug where people would not enter the freight vehicle they spawned while on delivery.\n \n"
    "Fixing a crash when pressing back on the transit designer panel.\n \n"
    "Fixing a crash when removing a transit line.\n \n"
    "Fixing several issues related to routes.\n \n"
    "Trains no longer jump across intersections.\n \n"
    "Fixing a bug where amenities couldn't be selected when other buildings were hidden.\n \n"
    "Building textures are now mirrored and randomly staggered, creating more visual variation in building illumination.\n \n"
    "Fixing some building designs which overlapped with roads\n \n"
    "Restoring snow on building roofs. Correcting texture orientation of slant-type roofs.\n \n"
    "Fixing a failure-to-launch on Linux (native) by including libsteam_api.so.\n \n"

  ),

  ChangelogData(
    "Version 0.0.54.1 ALPHA",

    "Test Mode saves now have the extension \".test\". Test Mode autosaves no longer overwrite Game Mode saves.\n \n"
    "If you exit the game while using the Building Designer, it will start the Building Designer on next launch.\n \n"
    "The appearance of the land grid has been changed.\n \n"
    "New building textures\n \n"
    "Building textures can now be associated with a particular building, by using the filename attribute \"design-<keyword>\" where <keyword> is contained in the building design filename.\n \n"
    "Fixed a bug where buildings would not illuminate in the building designer.\n \n"
    "The building texture filename is now shown in the Building Designer and Selection Panel.\n \n"
    "Changes to the behavior of the Value heatmap.\n \n"
    "Issues icons are shown less often.\n \n"
    "There is now a 15 second delay between songs.\n \n"
    "Fixed some crashes, including a crash when using Test Mode.\n \n"
    "Fixed a bug where building designs were not saved correctly.\n \n"
  ),

  ChangelogData(
    "Version 0.0.54.0 ALPHA",

    "Steam Integration - The game now integrates with the Steam API and Steam Overlay.\n \n"
    "Density Control - It is now possible to limit the building density of zoned areas. Open the Zone Tool (3) and press B to access the density control. Use the Zone Tool brushes to \"paint\" density limits where desired.\n \n"
    "With the Zone Tool open, or in the Zone View, lots will show a stack of bricks to indicate the configured density limit.\n \n"
    "With the Zone Tool open, right click on a lot to \"sample\" it, selecting it's zone type and density limit.\n \n"
    "Fixed a bug where charts displayed certain statistics without time period or year labels.\n \n"
    "The appearances of cars, trucks, buses, and trains have changed. The new vehicle models are loaded from .obj files, allowing easy modification. See data/vehicles.lua for configured values and file paths.\n \n"
    "Vehicle colors now come from textures/palette.png, allowing easy modification. Buses and trains are colorized based on line and system colors.\n \n"
    "Trains now hold 1100 passengers.\n \n"
    "Vehicles now pitch up and down on sloped roads and rails.\n \n"
    "Vehicle rotation is now more consistent, being always aligned with the lane. However, merging now looks awkward, an issue that will be addressed in a future release.\n \n"
    "Shader code has been reorganized, with the goal of easier modification.\n \n"
    "The in-game console now shows the game's log.\n \n"
    "Updated the game's name in the window title and EULA. Updated the reddit link to reddit.com/r/NewCity, our new home on reddit.\n \n"
    "Fixed an issue with the Cut Tool.\n \n"
    "Fixed various bugs and crashes.\n \n"
  ),

  ChangelogData(
    "Version 0.0.53.1 ALPHA",

    "The Transportation Tool (formally Road Tool) has been reorganized. Constructing infrastructure is now separate from adding transit lines. Several keyboard hotkeys have been reassigned.\n \n"
    "Vehicles (including buses and trains) now reroute themselves when lost.\n \n"
    "Fixed a bug where transit lines would \"break\" and could not be repaired.\n \n"
    "Fixed a serious performance issue when adding stops to a transit line.\n \n"
    "The Zone Tool Instruction Panel now shows the factors that contribute to zone demand. Click the blue \"i\" button in the zone tool, with tutorial mode disabled.\n \n"
    "When the game speed is limited (due to the router queue), the actual game speed is now highlighted in red.\n \n"
    "Added a new skyscraper design.\n \n"
    "Community now has a positive influence on Health.\n \n"
    "Tunnels and viaducts now remove trees. The appearance of tunnels has been improved.\n \n"
    "The value of a building is now shown in the Building Designer.\n \n"
    "In the Traffic View, directional arrows (for one way roads and expressways) are now more visible.\n \n"
    "The selection panel now shows information about Lane Blocks. Lane Blocks are parts of the road system and are fundamental to routing and vehicle movement.\n \n"
    "Fixed a bug where trains would bunch up.\n \n"
    "Freight vehicles are now subject to the limit on total vehicles.\n \n"
    "Fixed bugs related to shutting down transit systems.\n \n"
    "Fixed bugs related to amenity highlighting.\n \n"
    "Fixed a bug where University Schools could not be placed, with message \"Not Enough Dorms\".\n \n"
    "Fixed a bug where certain objects were occluded by hills, including the road building cursor and construction price text.\n \n"
    "Planned underground roads are now visible.\n \n"
    "Fixed bugs related to showing and hiding Planner Mode widgets.\n \n"
    "Fixed a bug where the game couldn't be paused or unpaused using the spacebar while the UI is hidden.\n \n"
    "Fixed overlapping text on charts.\n \n"
    "Fixed a bug with UI text layout.\n \n"
    "Fixed launching in Linux from Steam via Wine.\n \n"
    "Fixed a memory leak.\n \n"
    "Various performance improvements.\n \n"
    "New Statistics: Freight Deliveries, Time Spent Traveling (aggregate across city), Average Travel Time (per trip)\n \n"
  ),

  ChangelogData(
    "Version 0.0.53.0 ALPHA",

    "Adding NewTrack, the national passenger rail system, as a pre-configured rail option. Railroad is built like roadways, and rail lines are placed like bus lines. Trains can only stop at station platforms, but station platforms have a lower speed limit and are more expensive.\n \n"
    "Known Issue: Train movement is glitchy. Train graphics are preliminary.\n \n"
    "Transit lines can now have player-selected colors. Train station platforms will show the line color of one of their lines.\n \n"
    "Transit vehicles now show passengers inside.\n \n"
    "When selecting a traveller, their destination is shown with a red arrow.\n \n"
    "The Clinic has been redesigned. It is now cheaper, can be placed anywhere regardless of density, and is unlocked early game.\n \n"
    "Tunnels can now be built underwater.\n \n"
    "Certain achievements are now associated with the building that unlocked them. Click the achievement message to see which building.\n \n"
    "Performance improvements (Reduced vehicle swap frequency)\n \n"
    "Routing now uses 8 threads. This can be configured with CNumRouterThreads.\n \n"
    "Smoother vehicle motion\n \n"
    "Cities in easy mode will not have tax locks.\n \n"
    "National macroeconomic statistics (price index, inflation rate, national unemployment, etc) are now displayed correctly.\n \n"
    "Fixed a bug where poor families would show as hungry but never go to the store.\n \n"
    "New Health Icon\n \n"
  ),

  ChangelogData(
    "Version 0.0.52.4 ALPHA",

    "Community Heatmap: Added Community heatmap. Community represents the strength of the bonds between citizens in your city. It is negatively affected by industrial zones, crime and pockets of dense retail establishments. Can be positively affected by agriculture, parks, low density residential, mixed use zones, and certain amenities.\n \n"
    "Health Heatmap: Added Health heatmap. Health represents the physical and mental health of your citizens. It starts high, and can be reduced by pollution, crime, traffic jams, and negative life events like losing a job or going hungry. It is positively affected by parks and certain amenities.\n \n"
    "Sickness and Going to Doctor: Citizens can now get sick and visit doctors. When the Health heatmap is low, citizens are more likely to get sick. When sick, they will not work or go to the store. If they visit a doctor while sick, they will return to health. Clinics, the Hospital, the School of Medicine, and the Safety Center (University) all provide doctors for sick citizens to visit.\n \n"
    "Observe the number of sick people in the economic charts under Local Economy > Population > People Sick. You can also see sick people when viewing the Health heatmap. They will have a thermometer icon above their home.\n \n"
    "Smoother Vehicle Motion: Fixed a glitch in vehicle interpolation.\n \n"
    "You can now find out your game's difficulty level in two ways: you can click on the city name, and it will show the difficulty level. Or, you can type \"diff\" into the game's console. You can also modify the game's difficulty level by typing \"diff X\", where X is a number between 0 and 3. You will need to restart the game for the change to apply.\n \n"
    "Added a button to hide labels.\n \n"
    "Fixed a bug where game difficulty was not saved correctly.\n \n"
    "University produces less Density.\n \n"
    "Fixed several issues related to business demand and growth.\n \n"
    "Fixed an issue that caused the game to the stutter. (Don't defrag except on save/load.)\n \n"
    ),

  ChangelogData(
      "Version 0.0.52.3 ALPHA",

      "Fixed a bug where building designs were corrupted.\n \n"
      "Adding the Police Stand amenity, which unlocks with the Police Headquarters amenity. The Police Stand is a small police station.\n \n"
      "Texture filtering has been switched to nearest-neighbor (GL_NEAREST_MIPMAP_LINEAR). This suits the retro look of the game, and will allow us to use lower resolution textures, which will reduce VRAM consumption.\n \n"
  ),

  ChangelogData(
    "Version 0.0.52.2 ALPHA",

    "Perspective Camera - NewCity's unusual camera is now optional. Switch to a perspective camera using the slider in the options panel. NOTE: Currently, the perspective camera has a significant FPS penalty. We are constantly working on improving performance. There are also some visual glitches with shadows and vehicle headlights.\n \n"
    "The camera's FOV can also be set using the console command \"fov\".\n \n"
    "The Query Tool has been reorganized. It now has four tabs, one for heatmaps, one for labels, one for the route inspector (explained below) and one for selecting buildings, roads, and vehicles.\n \n"
    "Route Inspector - Allows the player see how the game plans routes. Select two points, and the game will show a roadway route, a mass transit route (if available), time taken, fuel or ticket cost, a weighted time cost for each route, and the method selected (car or bus).\n \n"
    "The Heatmap Subtool now has a cursor to query the value of the heatmap at any location.\n \n"
    "Vehicles will no longer appear off-road.\n \n"
    "Route Broker - A system to reduce pressure on the heavily-overloaded router. The broker will be explained in detail in tomorrow's (2020-04-18) blog.\n \n"
    "Collect statistics on free and settling vehicle and travel group indexes.\n \n"
    "Fixed a bug that caused vehicle indexes to accumulate, slowing down the game.\n \n"
    "Automatic defragmenting and recovery of vehicle and travel group indexes. (This fixes a performance problem.)\n \n"
    "Log CPU Update Times - Evaluate system performance by selecting CLogUpdateTime = true in data/constants.lua, then read the game log.\n \n"),

  ChangelogData(
    "Version 0.0.52.1 ALPHA",

    "There are now two methods of camera panning, the default \"Steve Jobs\" style and the inverse \"SC3K\" or classic style.\n \n"
    "Adding a new song, \"Chance of Rain\", by Supersoup.\n \n"
    "Fixing a common source of crashes related to router heap reallocation.\n \n"
    "Park lots now respect the Recreation budget control.\n \n"
    "Fixed tooltips for park zoning.\n \n"
    "Now all 6 charts in the economy panel have buttons to change the econ shown.\n \n"
    "Fixed a small memory leak.\n \n"
    "Note: We missed a changelog item in 0.0.52.0. We added park lots. Park lots are unlocked at 20K population with the \"City\" achievement. A park lot is a special zone type which reduces pollution and increases value. Place park lots using the zone tool. The city pays for every park lot zoned, under the Recreation line item. Park lots can be stored in blueprints. Park lots near coasts and on hills give more value and reduce more pollution.\n \n"

  ),

  ChangelogData(
    "Version 0.0.52.0 ALPHA",

    "Labels - Add a label by selecting the query tool, then clicking the \"label\" button. The size of the label can be configured. It is also possible to make labels red.\n \n"
    "Neighboring Cities - In new maps, neighboring cities will spawn, if the option is checked. Connect to neighbors by building a road. Citizens can work in one city and live in another.\n \n"
    "Econs - The game now tracks seperate statistics for neighboring cities. The charts can be used to see this information. This system will be built out to represent the national economy as well as regions within the player's city.\n \n"
    "Cut Tool - Allows player to insert intersections (nodes) into roadways.\n \n"
    "City Name - The city now has a name. The name can be changed at any time by clicking on it in the top left corner.\n \n"
    "The order of the status bars has been changed.\n \n"
    "Buildings can now be renamed. Some buildings have generated names, including amenities. Amenities are named after IndieGoGo Contributors at the Factory tier and above. There is still time to upgrade to Factory tier ;)\n \n"
    "Added Work-in-Progress Perspective Camera via a Lua constant. See data/constant.lua, CCameraFOV. Currently, it is not possible to do mouse interactions using the perspective camera.\n \n"
    "Fixing a bug where building decorations were white on some machines.\n \n"
    "Pan Camera - It is now possible to pan the camera by holding down the right mouse button.\n \n"
  ),

  ChangelogData(
    "Version 0.0.51.4 ALPHA",

    "The name of the game has been changed to NewCity.\n \n"
    "The FPS widget in the top right corner can now show a clock. It will remember your selection.\n \n"
    "Added contributors to about panel. Simulated people will now occasionally be named after contributors.\n \n"
    "Road segments can now be renamed. Entire roads (chains of road segments) can be renamed using the \"Chain Rename\" feature.\n \n"
    "Rain and snow can now be disabled.\n \n"
    "The sun can now be synchronized with the system clock, resembling the desktop application f.lux\n \n"
    "A new song has been added, \"The Doozie\", written by team member Supersoup.\n \n"
    "Routing is now biased towards faster roads.\n \n"
    "Roads are now shinier, especially during rain and snow.\n \n"
    "General bug fixes and performance improvements.\n \n"
  ),


  ChangelogData(
    "Version 0.0.51.3 ALPHA",

    "The transit panel has been enlarged and redesigned. There is now a cursor on transit lines. New stops are added to the line at the cursor. The cursor can easily be moved to the beginning or end of the line.\n \n"
    "In tutorial mode, the supermarket now unlocks the Pollution heatmap. The factory now unlocks the Prosperity heatmap. (It was reversed before.)\n \n"
    "Superflat maps now have a higher default Value. Their land height is now adjustable using CSuperflatHeight.\n \n"
    "The University now gives less density.\n \n"
    "Adding Central Library amenity.\n \n"
    "Fixing a bug in version 51 designs.\n \n"
    "Improved UI shadows\n \n"
    "Information about file sizes is now logged.\n \n"
    "Hills will now get less density. Hills reduce pollution, but that effect is now reduced.\n \n"
    "The game now has slightly more daytime.\n \n"
    "Added a toggle to disable night time.\n \n"
    "Fixing a bug in the way the purple Amenity lots show in the Zone View.\n \n"
    "Passengers get on buses faster and more consistently.\n \n"
    "More variety in Residential building roof colors.\n \n"
),

  ChangelogData(
    "Version 0.0.51.2 ALPHA",

    "Roads will now be named more consistently.\n \n"
    "Implemented difficultly levels.\n \n"
    "Fixed an issue with travel group indexes.\n \n"
    "Added 'select' console command. Type ?select for more info.\n \n"
    ),

  ChangelogData(
    "Version 0.0.51.1 ALPHA",

    "NOTICE: Version 51 contains major backend changes. We are still testing the new transportation system. Make sure to keep a version 50 save of your favorite cities.\n \n"

    "Implemented bus system. To use, open the road tool (2) and select the transit tab (T). Add a new bus line (Q) and select \"Add Stops to Line\" (F). Then start placing a chain of routes. Select a headway (or frequency) for day and night. Then return to \"All Bus Lines\" to set the ticket and transfer price.\n \n"
    "Implemented a new router which can handle transit routes.\n \n"
    "Buses bill the city based on time spent on the road. Passengers pay a player-selectable ticket price and transfer price. Passengers are less likely to take an expensive bus. The city's transit budget is multiplied by 90 to account for the fact that we only simulate 4 days out of 365. Income is further multiplied by 5 to account for the fraction of trips that are not simulated.\n \n"
    "Transit View was added to the infoview bar. Transit View now shows lines, stops, passengers waiting, and number of people inside buses.\n \n"
    "Adding a Console. Access console by pressing ` (the tick mark or tilde key). Type \"help\" to get a list of commands. Put a question mark before a command to learn more about it, like so: \"?lua\". The console is still a work-in-progress.\n \n"
    "Added Budget Controls. Player can now reduce spending on specific sectors, but those sectors will be less effective. It is also possible to shut down a sector entirely.\n \n"
    "Budget estimates and Line of Credit are now based on Year+1, for stability.\n \n"
    "People who spend more than a day traveling are now sent directly to their destination.\n \n"
    "Improved performance of budget, frustum culling. Buildings are updated less frequently, for performance.\n \n"
    "Building selection panel now shows the design filename.\n \n"
    "Building roofs accumulate snow again.\n \n \n"

    "Version 0.0.51.1:\n \n \n"
    "Improved performance of transit.\n \n"
    "Fixed a crash related to pedestrian paths and zoning/buildings.\n \n \n \n"
  ),

  ChangelogData(
    "Version 0.0.50.15 ALPHA",

    "Fixing an issue where some cities would stop developing at "
    "population 980.\n \n"
    "Fixing shadow acne on land\n \n"
    "Adding Police Headquarters\n \n"
    "When closing blueprints, messages are reopened\n \n"
    "Correct unlocking of First Farm achievement\n \n"
    "Correct unlocking of University Minor Schools achievement\n \n"
    "Correct unlocking of Eureka achievement\n \n"
    "Entire traffic light pole now waves in the wind\n \n"
    "There will be no residential demand when unemployment is too high.\n \n"
    "Improved the apperance of text, especially on Linux\n \n"
    "Buildings can be built up to 250 stories tall.\n \n"
    "Fixing a crash when using the bulldozer\n \n"
    "People now move into homes immediately, instead of being homeless "
    "for a period of time.\n \n"
    "Homelessness will be reduced.\n \n"
    "Crime will be increased.\n \n"
    "Prosperity will be reduced.\n \n"
    "Uneducated people will now have a higher unemployment rate.\n \n"
    "Fixing a bug where stores with no customers would stay "
    "closed indefinitely\n \n"
    "Fixing a bug where some people would sleep indefinitely\n \n"
    "People will visit stores less and friends and amenities more.\n \n"
    "Unemployment now has an effect on the education level of people "
    "moving to the city.\n \n"
    "Educated people will move to high Value, high Density areas.\n \n \n"
  ),

  ChangelogData(
    "Version 0.0.50.14 ALPHA",
    "Adding zones view, which allows the player to see the zoning underneath buildings.\n \n"
    "Adding toolbar buttons to show heatmaps, undeground view, traffic view.\n \n"
    "Increased routing traffic awareness.\n \n"
    "Railroads and Pedestrian paths are now available in game mode.\n \n"
    "Pedestrian paths do not crash in test mode.\n \n"
    "Sidebar charts can be minimized to a bar.\n \n"
    "Hills will not disappear on the edge of the screen.\n \n"
    "Chart minimum values are now more accurate.\n \n"
    "Increased demand for retail.\n \n"
    "Snow and Rainfall are now less intense.\n \n"
  ),
  ChangelogData(
    "Version 0.0.50.13 ALPHA",
    "Introducing tunnels. Build tunnels by pressing tab in the road tool. Visuals are preliminary.\n \n"
    "Introducing underground view, for building tunnels and underground infrastructure.\n \n"
    "Introducing viaducts. Build viaducts by pressing tab in the road tool.\n \n"
    "Above-ground roadways now have support pillars.\n \n"
    "Above-ground roadways and bridges now have sidewalks.\n \n"
    "Introducing railways and pedestrian paths. They are not functional at this time. Railways cannot be zoned. Pedestrian paths will be zone-able in the future, but are not zone-able in this release, for technical reasons.\n \n"
    "Significant performance improvements thanks to better frustum culling.\n \n"
    "Water is now transparent, and has a much better appearance.\n \n"
    "Shadows, vehicle headlights and building lights are all more intense.\n \n"
    "Vehicle headlights flicker on gradually rather than suddenly.\n \n"
    "Hard modpack: People will move out and businesses will close in response to bad economic situations. This is a test of recession mechanics and enhanced economic simulation.\n \n"
    "People will not move into a city with high unemployment.\n \n"
    "There will be more retail and office businesses and buildings. There will be a temporary spike of unemployment in existing cities.\n \n"
    "Abandoned buildings are brown in heatmaps.\n \n"
    "Updates to About Panel\n \n"
  ),
  ChangelogData(
    "Version 0.0.50.12 ALPHA",
    "Added the new Building Texture System."
    " Read textures/buildings/README.txt to learn more."
  ),
  ChangelogData(
    "Version 0.0.50.11 ALPHA",
    "Added changelog.\n \n"
    "Added tooltips. Hovering over UI elements will provide"
    " additional info.\n \n"
    "Budget panel now shows cash available to spend.\n \n"
    "Fixed a bug in the statistic Open Positions.\n \n"
    "Updated the consequences of taxes:"
    " reduced growth, more unemployment.\n \n"
    "Cars are better at selecting expressway routes and are less likely to"
    " take onramp/offramp \"shortcuts\"."
  ),
  ChangelogData(
    "Version 0.0.50.10 ALPHA",
    "Performance Modpack: A modpack optimized to get the best"
    " out of low end machines. Select this modpack if you"
    " experience low framerates.\n \n"
    "Improvements to the appearance of land, water, and trees."
    " The movement of trees is now more obvious.\n \n"
    "Performance improvements to terrain rendering.\n \n"
    "Keep selected heatmap visible when query tool is closed"
    " by clicking the pin icon.\n \n"
    "Changed note icon to pin.\n \n"
    "Routing was improved.\n \n"
    "Water now freezes in winter."
  )
};

