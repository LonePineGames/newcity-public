
--[[
addAchievement({
  code = "AchNewCity",
  name = "A New City",
  text = "Welcome to NewCity. " ..
  "Your first task is to " ..
  "build some roads. Click " ..
  "on the Transportation Tool in the " ..
  "bottom left corner.",
  condition = function()
    return true
  end,
  effect = "FRoadTool,FRoadStreet",
  hint = ""
});

addAchievement({
  code = "AchFirstRoads",
  name = "First Roads",
  text = "You built some roads. " ..
  "Now place some zones. " ..
  "Use the Zone Tool, next " ..
  "to the Transportation Tool. " ..
  "You need green zones for " ..
  "housing. This is called " ..
  "Residential.",
  condition = "roads:4",
  effect = "FZoneTool,FZoneResidential,FBulldozerTool,FNewGame",
  hint = ""
});
]]--

addAchievement({
  code = "AchFirstHouse",
  name = "First House",
  text = "Someone moved in! ",
  --"Now let's get them a job. " ..
  --"Yellow zones, called " ..
  --"Agricultural, allow farms. " ..
  --"Farms need very large " ..
  --"amounts of flat space. " ..
  --"Expect to fill the map " ..
  --"with them.",
  condition = function()
    return get(StatPopulation) >= 1
  end,
  effect = "FZoneFarm",
  --hint = "Hint: For agricultural zones, place roads along the medium grid" ..
    --" lines.\nFor residential, place roads along the small grid dots.\n" ..
    --"You don't need to build a tight grid."
});

addAchievement({
  code = "AchFirstFarm",
  name = "First Farm",
  text = "People live and work here. " ..
  "Keep building!",
  condition = function()
    return get(StatNumEmployed) > 1 and numBuildings("farm") > 0
  end,
  associate_building = "farm",
  effect = "FSpeedControl",
  hint = ""
});

addAchievement({
  code = "AchHamlet",
  name = "Hamlet",
  text = "100 people live here. " ..
  "The state now legally " ..
  "recognizes our locality. ",
  condition = function()
    return get(StatPopulation) >= 100
  end,
  effect = "FZoneRetail,FPopulation"
});

addAchievement({
  code = "AchOutOfCash",
  name = "Out of Cash",
  text = "We are almost out of money to " ..
  "build roads. Click on the " ..
  "Money Indicator at the top " ..
  "of the screen to open the " ..
  "Budget Panel. You will need " ..
  "to extend the loan term. ",
  condition = "loc_below:1000",
  effect = "FLoanTerm",
  hint = ""
});

addAchievement({
  code = "AchVillage",
  name = "Village",
  text = "1,000 people live here.  There are officially more" ..
  " humans residents than bovine.",
  condition = function()
    return get(StatPopulation) >= 1000
  end,
  effect = "FQueryTool",
  hint = ""
});

addAchievement({
  code = "AchTown",
  name = "Town",
  text = "5,000 people live here. " ..
  "There is now a dot on " ..
  "the map here.",
  condition = function()
    return get(StatPopulation) >= 5000
  end,
  effect = "FRoadAve,FZoneFactory,FWeatherStation,FEconomyPanel,FUnemployment,FEarthworks",
  hint = "Next Achievement: At 20,000 we can build a City Hall.\n \n" ..
    "Hint: Factories employ more people than farms do."
});

addAchievement({
  code = "AchEarningsFallShort",
  name = "Earnings Fall Short",
  text = "We are earning less than we " ..
  "are spending. Click on the " ..
  "Money Indicator in the top " ..
  "left corner to open the " ..
  "Budget Panel. You will need " ..
  "to raise taxes. ",
  condition = "earnings_below:0",
  effect = "FNoteBudget,FNoteChart,FNoteObject",
  hint = ""
});

addAchievement({
  code = "AchHeinousMurder",
  name = "Heinous Murder Shocks Town",
  text = "Residents are shocked to read " ..
  "articles about a robbery gone " ..
  "wrong. Where are the police?",
  condition = "unemployed:2000",
  effect = "police_station,FObserveCrime",
  hint = ""
});

addAchievement({
  code = "AchAboveAverage",
  name = "Every Child is Above Average",
  text = "Our educated populace is " ..
  "asking for a public library.",
  condition = "education:11",
  effect = "library_basic,FObserveEducation",
  hint = "Next Achievement: When a big factory is built, they will sponsor " ..
    "a Technical College."
});

addAchievement({
  code = "AchGrandOpening",
  name = "Grand Opening!",
  text = "Did you hear about that new " ..
  "supermarket going up?",
  condition = function()
    return numBuildings("supermarket") > 0
  end,
  associate_building = "supermarket",
  effect = "park_small,FObservePollution,FPlaceTrees",
  hint = "Next Achievement: If a corporation chooses our city for" ..
    " their new headquaters, they'll sponsor a Community College."
});

addAchievement({
  code = "AchCity",
  name = "City",
  text = "20,000 people live here. " ..
  "It's official!",
  condition = function()
    return get(StatPopulation) >= 20000
  end,
  effect = "city_hall,FRoadElevation,FRoadPillars,FZonePark,FObserveZones",
  hint = "Next Achievement: At 50,000, we can build Ballparks, which" ..
    " promote Health and Community. Also, you get a mansion.\n \n" ..
    "Hint: In the Transportation Tool (2), roads can be raised and lowered" ..
    " by pressing the up and down arrows (Q and Z). By default," ..
    " engineers will build a trench or embankment under your roads," ..
    " but if you select the tunnel/viaduct option (H), they will" ..
    " build a tunnel or viaduct" ..
    " (bridge structure) instead."
});

addAchievement({
  code = "AchMeasureOnce",
  name = "Measure Twice, Cut Once",
  text = "You've built 1,000 roads. " ..
  "Now plan the rest of them.",
  condition = "roads:1000",
  effect = "FRoadPlanner,FRoadCut,FGrid,FTestMode,FBlueprint,FRouteInspector",
  hint = "Hint: NewCity features robust planning and blueprinting tools." ..
  " Try out Blueprints by clicking on the blueprint button in the" ..
  " top right bar. Or try out Planner Mode by opening the" ..
  " Transportation Tool (2) and clicking the checkmark."
});

addAchievement({
  code = "AchFactory",
  name = "A Factory Comes to Town",
  text = "The owners of the new " ..
  "factory need more " ..
  "engineers. They want to " ..
  "open an engineering school.",
  condition = function()
    return numBuildings("factory") > 0
  end,
  associate_building = "factory",
  effect = "technical_college,FObserveProsperity,FZoneOffice",
  hint = ""
});

addAchievement({
  code = "AchMall",
  name = "Let's Go to the Mall",
  text = "A new commercial building is " ..
  "driving a lot of traffic. " ..
  "Time for biggers roads.",
  condition = function()
    return numBuildings("big_mall") + numBuildings("hugemall") + numBuildings("supermall") + numBuildings("outdoor_mall") > 0
  end,
  associate_building = "mall",
  effect = "FRoadBlvd,FObserveDensity,FObserveTraffic,FBuildingDesigner",
  hint = ""
});

addAchievement({
  code = "AchBudgetManagement",
  name = "Budget Management",
  text = "Now that we have a school, " ..
    "there is an additional line " ..
    "item expense in the budget " ..
    "to be mindful of. As an " ..
    "emergency measure, you can " ..
    "now shut down or sell " ..
    "amenities to reduce expenses.",
  condition = "amenity_built:_school",
  effect = "FShutDownBuilding,FSellBuilding",
  hint = ""
});

addAchievement({
  code = "AchItsHotOutside",
  name = "Baby It's Hot Outside",
  text = "Let's cool down with a pool.",
  condition = "temp_above:28,building:r.d3",
  effect = "public_pool,FObserveCommunity",
  hint = "Next Achievement: If a Density Tier 5 Residential building is" ..
    " built, we can place a Big Park, which will mitigate 6 times as" ..
    " much pollution as a Small Park, and at the same time" ..
    " promote environmentalism." 
});

addAchievement({
  code = "AchNiceNeighborhood",
  name = "Nice Neighborhood",
  text = "Location, location, location!",
  condition = "building:r.v7.d2",
  effect = "marina,FObserveValue,FZoneMixedUse",
  hint = "Hint: Mixed Use Zones combine retail with apartments. " ..
    "They will only develop in dense, high land value areas.\n" ..
    "Next Achievement: If someone builds a mansion, they'll sponsor a Zoo. " ..
    "A mansion will develop at Residential Value Tier 9, Density Tier 0."
});

addAchievement({
  code = "AchGivingBack",
  name = "Giving Back",
  text = "A wealthy resident is " ..
  "sponsoring a zoo.",
  condition = "building:r.v10",
  effect = "zoo",
  hint = ""
});

addAchievement({
  code = "AchBeautifulDay",
  name = "It's A Beautiful Day",
  text = "Let's head down to the beach!",
  condition = "building:c.d4,temp_above:22",
  effect = "public_beach",
  hint = ""
});

addAchievement({
  code = "AchEntreprenuersDream",
  name = "Entreprenuer's Dream",
  text = "Lee Longway built an empire -- a chain of dollar stores beloved by" ..
    " rich and poor alike. Now he wants to build an amusement park on the" ..
    " waterfront.",
  condition = "building:c.d6",
  effect = "pier_amusement_park",
  hint = ""
});

addAchievement({
  code = "AchFinesAndFees",
  name = "Fines and Fees",
  text = "With the addition of a police " ..
    "station, crime will see a  " ..
    "reduction around it. With " ..
    "police comes the ability to " ..
    "leverage petty fees and fines " ..
    "for city revenue. It's not " ..
    "dignified, and it'll reduce the " ..
    "effectiveness of our police, " ..
    "but it's an option.",
  condition = "amenity_built:police_station",
  effect = "FFinesAndFees",
  hint = ""
});

addAchievement({
  code = "AchSuburb",
  name = "Suburb",
  text = "50,000 people live here. " ..
  "The city is big enough to " ..
  "make another city important.",
  condition = function()
    return get(StatPopulation) >= 50000
  end,
  effect = "ballpark,mayoral_mansion,FRoadExpressways,FObserveRoadMap",
  hint = "Next Achievement: At 100,000, we can build a Courthouse to" ..
    " improve the effectiveness of our police."
});

addAchievement({
  code = "AchWornOut",
  name = "Worn Out",
  text = "Drivers are complaining " ..
  "about a road full of " ..
  "potholes. Fix it now!",
  condition = "wear",
  effect = "FRoadRepair",
  hint = ""
});

addAchievement({
  code = "AchCorporateHeadquarters",
  name = "Corporate Headquarters",
  text = "A company has decided to " ..
  "locate their international " ..
  "headquarters in our city! " ..
  "And they need all the college " ..
  "grads they can find.",
  condition = function()
    return numBuildings("corporate") > 0
  end,
  associate_building = "corporate",
  effect = "community_college",
  hint = "Next Achievement: Get an Art Museum at Office Density" ..
    " Tier 5/Value Tier 7"
});

addAchievement({
  code = "AchAlternativeTax",
  name = "Alternative Tax",
  text = "City hall serves as the " ..
    "beating heart of city " ..
    "bureaucracy. It also " ..
    "opens up more options for " ..
    "taxation and revenue. If " ..
    "citizens won't tolerate more " ..
    "property taxes, you can tax " ..
    "sales instead. ",
  condition = "amenity_built:city_hall",
  effect = "FBudget,FSalesTax",
  hint = ""
});

addAchievement({
  code = "AchCheckUp",
  name = "Time for Your Check Up",
  text = "Healthy citizens are happy citizens. " ..
  "Let's build a clinic.",
  condition = "sick:200",
  effect = "clinic,FObserveHealth",
  hint = ""
});

addAchievement({
  code = "AchJail",
  name = "Crime and Punishment",
  text = "Crime is no longer an anecdote. " ..
  "With holding cells filling up, " ..
  "we now need a municipal jail.",
  condition = "unemployed:4000",
  effect = "jail",
  hint = ""
});

addAchievement({
  code = "AchHistoryMuseum",
  name = "Do You Remember When...",
  text = "The local historical society " ..
  "wants to open a museum.",
  condition = "year:10",
  effect = "history_museum",
  hint = ""
});

addAchievement({
  code = "AchOpenSpace",
  name = "Dreaming of Open Space",
  text = "Downtown apartment dwellers are " ..
  "petitioning for a large open " ..
  "space.",
  condition = "building:r.d5",
  effect = "park_big",
  hint = "Next Achievement: Get a Pavilion at Residential Density Tier 6"
});

addAchievement({
  code = "AchRegionalCenter",
  name = "Regional Center",
  text = "100,000 people live here. " ..
  "Residents no longer have to " ..
  "tell out-of-state relatives " ..
  "what state they live in.",
  condition = function()
    return get(StatPopulation) >= 100000
  end,
  effect = "courthouse,FRoadSuspensionBridge,FMods",
  hint = "Next Achievement: Get a Hospital at 200,000"
});

addAchievement({
  code = "AchTaxpayerRevolt",
  name = "Taxpayer Revolt",
  text = "Citizens won a legislative " ..
  "initiative to permanently freeze property " ..
  "taxes at 4%. What this will " ..
  "do to our budget is anyone's " ..
  "guess.",
  condition = "building:r.d2.v7,earnings_above:500000,loc_above:10000000," ..
  "year:11,temp_above:25,amenity_built:city_hall," ..
  "amenity_built:police_station," ..
  "unemp_rate_below:5,tax_lock",
  effect = ":tl1",
  hint = "Property Tax Locked"
});

addAchievement({
  code = "AchSeeForMiles",
  name = "I Can See for Miles",
  text = "Did you hear about that new " ..
  "building they're putting up?",
  condition = "building:o.z19",
  effect = "convention_center",
  hint = ""
});

addAchievement({
  code = "AchTheyPavedParadise",
  name = "They Paved Paradise",
  text = "Surrounded by tall buildings, " ..
  "downtown residents report that even a " ..
  "little open space would be nice.",
  condition = "building:r.d6",
  effect = "pavilion",
  hint = ""
});

addAchievement({
  code = "AchSisterCities",
  name = "Sister Cities",
  text = "200,000 people live here. " ..
  "The city is now engaged in " ..
  "international diplomacy.",
  condition = function()
    return get(StatPopulation) >= 200000
  end,
  effect = "hospital",
  hint = "Next Achievement: Stadium at 500,000"
});

addAchievement({
  code = "AchBusinessInterestsPrevail",
  name = "Business Interests Prevail",
  text = "Businesses won a legislative " ..
  "initiative to permanently freeze sales " ..
  "taxes at 4c/$. Average citizens " ..
  "are enraged. ",
  condition = "building:o.d5,earnings_above:1000000,loc_above:20000000," ..
  "year:15,temp_below:10,amenity_built:city_hall," ..
  "amenity_built:police_station" ..
  "unemp_rate_below:4,tax_lock",
  effect = ":tl2",
  hint = "Sales Tax Locked"
});

addAchievement({
  code = "AchArtMuseum",
  name = "From Business to Art",
  text = "A local corporation has offered to " ..
  "cosponsor a regional center " ..
  "for art and culture.",
  condition = "building:o.v7.d5",
  effect = "artmuseum",
  hint = "Next Achievement: At Office Density Tier 6, " ..
  "we will get a Convention Center, " ..
  "which will drive densification and prosperity."
});

addAchievement({
  code = "AchNatureMuseum",
  name = "Return to Nature",
  text = "The local conservation society " ..
  "wants to open a museum.",
  condition = "nature:20",
  effect = "nature_museum",
  hint = ""
});

addAchievement({
  code = "AchHomelessness",
  name = "Looking for Home",
  text = "Over 200 people are sleeping on the street tonight. " ..
    "Let's find them a home.",
  condition = function()
    return get(StatNumHomeless) > 200 and timeNow() > OneHour*22
  end,
  effect = "social_housing,public_housing,social_services",
  hint = "In NewCity, homelessness is caused by unemployment, unfulfilled residential zone demand (high rent), and poor Community. Public Housing and Social Housing is the cheapest way reduce the number of homeless."
});

addAchievement({
  code = "AchBigCity",
  name = "Big City",
  text = "500,000 people live here. " ..
  "Locals just refer to it as \"The City\"",
  condition = function()
    return get(StatPopulation) >= 500000
  end,
  effect = "stadium",
  hint = "Next Achievement: Science Museum at 1,000,000"
});

addAchievement({
  code = "AchMetropolis",
  name = "Metropolis",
  text = "1,000,000 people live here. " ..
  "We are known the world over.",
  -- Except when you find out there is a city in China with 11 million people
  -- in it, you've never heard of it, and it's going to kill everyone lol.
  condition = function()
    return get(StatPopulation) >= 1000000
  end,
  effect = "science_museum",
  hint = ""
});

addAchievement({
  code = "AchSkyscraper",
  name = "Skyscraper",
  text = "Your city is home to its " ..
  "first truly tall building. " ..
  "The city has arrived on the " ..
  "world stage. " ..
  "You won the game. " ..
  "You beat NewCity. ",
  condition = function()
    return get(StatSkyscrapers) > 0
  end,
  associate_building = "skyscraper",
  effect = "park_grand,:w",
  hint = "Congratulations!"
});

addAchievement({
  code = "AchGlobalCity",
  name = "Global City",
  text = "2,000,000 people live here. " ..
  "The city projects economic " ..
  "influence across the continent.",
  condition = function()
    return get(StatPopulation) >= 2000000
  end,
  effect = "",
  hint = ""
});

addAchievement({
  code = "AchMegalopolis",
  name = "Megalopolis",
  text = "5,000,000 people live here. " ..
  "This city will go down " ..
  "in history!",
  condition = function()
    return get(StatPopulation) >= 5000000
  end,
  effect = "",
  hint = ""
});

addAchievement({
  code = "AchLonePower",
  name = "Lone Power",
  text = "10,000,000 people live here. " ..
  "You now challenge the president " ..
  "for national authority.",
  condition = function()
    return get(StatPopulation) >= 10000000
  end,
  effect = "",
  hint = ""
});

addAchievement({
  code = "AchEcumenopolis",
  name = "Ecumenopolis",
  text = "100,000,000 people live here. " ..
  "This city challenges not only " ..
  "the global environment, but " ..
  "your CPU.",
  condition = function()
    return get(StatPopulation) >= 100000000
  end,
  effect = "",
  hint = ""
});

addAchievement({
  code = "AchTheoreticalLimit",
  name = "Theoretical Limit",
  text = "2,000,000,000 people live here. " ..
  "A signed 32-bit integer can only " ..
  "represent 2.15 billion people. " ..
  "Congrats, you broke NewCity!",
  condition = function()
    return get(StatPopulation) >= 2000000000
  end,
  effect = "",
  hint = ""
});

addAchievement({
  code = "AchUni1",
  name = "I'm Off to College!",
  text = "Our city is being offered an opportunity to host a major University.",
  condition = "education:20,population:40000,building:r.v7",
  effect = "quad_uni",
  hint = "Hint: If you want to build the University, " ..
  "place the Quad to get started. Every University building must be near " ..
  "the Quad."
});

addAchievement({
  code = "AchUni2",
  name = "I Got Accepted!",
  text = "The University has been founded!",
  condition = "amenity_built:quad_uni",
  effect = "dorms_uni",
  hint = "Next, build a few dorms, or one dorm tower."
});

addAchievement({
  code = "AchUni3",
  name = "What Should I Major In?",
  text = "You get to choose what the University will specialize in.",
  condition = "amenity_built:dorms_uni,bunks:1000",
  effect = "_school_uni",
  hint = "Hint: University buildings will fit neatly together between" ..
    " two roads spaced one grid square apart."
});

addAchievement({
  code = "AchUni4",
  name = "And Where Do I Eat Lunch?",
  text = "The University can have its own amenities.",
  condition = "amenity_built:_school_uni",
  effect = "_am_uni",
  hint = "Hint: The Universiy Schools provide bachelor's" ..
   " degrees and doctorates."
});

addAchievement({
  code = "AchUni5",
  name = "I'm Thinking About a Minor.",
  text = "The University looks great!",
  condition = "amenity_built:am_uni",
  effect = "_mschool_uni",
  hint = ""
});

addAchievement({
  code = "AchEducationReform",
  name = "Education Reform",
  text = "A new report says that our growing school district isn't" ..
    " doing enough to prepare kids for college and beyond." ..
    " Furious administrators want action.",
  condition = "amenity_built:college,population:80000,education:15",
  effect = "magnet_school,opportunity_school,charter_school,college_prep",
  hint = ""
});

addAchievement({
  code = "AchApartmentLiving",
  name = "Apartment Living",
  text = "It's starting to feel like a real city!",
  condition = "building:r.d3",
  effect = "FRoadOneWay2,FBus",
  hint = ""
});

addAchievement({
  code = "AchEureka",
  name = "Eureka!",
  text = "This city now has the most PhD educated citizens in the Nation!",
  condition = function()
    return hasArticleAppeared("TooMuchEducation01");
  end,
  effect = "",
  hint = ""
});

addAchievement({
  code = "AchBaseOfOperations",
  name = "Base of Operations",
  text = "Your police chief has asked for a central office to coordinate " ..
  "police activity.",
  condition = "law:30,amenity_built:police_station,unemployed:8000",
  effect = "police_hq,police_ops",
  hint = ""
});

addAchievement({
  code = "AchOverflowingBooks",
  name = "Overflowing with Books",
  text = "The city colleges and municipal library system have teamed up" ..
    " to pool their collections. There's only one problem --" ..
    " there's no place to store all the books!",
  condition = "education:201,amenity_built:library,colleges:2,bcledu:25",
  effect = "library_central",
  hint = ""
});

addAchievement({
  code = "AchNewTrack",
  name = "Outta Town on a Rail",
  text = "NewTrack, the national passenger rail system, is adding" ..
    " our city to their network! NewTrack is popular with" ..
    " commuting office workers and broke college students." ..
    " You can connect the suburbs and neighboring cities to your" ..
    " downtown, then use buses to distribute passengers around the city.",
  condition = "building:r.d4",
  effect = "FRail,FRoadOneWay4",
  hint = "Hint: Place rail just like roads." ..
    " Then place rail lines just like bus lines." ..
    "\n \nNote: Train stops can only be placed on Station Platforms."
});

addAchievement({
  code = "AchTechHub",
  name = "Tech Hub",
  text = "Our city is now recognized as a major global hub for technology" ..
    " and innovation. Our fast-growing tech firms can never seem to find" ..
    " enough college-educated workers. We need more colleges!",
  condition = "building:o.d7,tech:50",
  effect = "gradschool",
  hint = ""
});

addAchievement({
  code = "AchFederalBuilding",
  name = "Administrative Priority",
  text = "This region has really grown over the last decade!" ..
    " The Federal Government can hardly keep up with their important" ..
    " administrative duties. They've decided to situate an large" ..
    " office building downtown. They'll pay for it, of course --" ..
    " sparing no expense.",
  condition = "building:o.d4,population:120000,year:10",
  effect = "federal_building",
  hint = ""
});

addAchievement({
  code = "AchMemorial",
  name = "In Remembrance",
  text = "A veteran's society has decided to sponsor a memorial to" ..
    " the city's heros.",
  condition = "building:m.d4,population:100000,year:25",
  effect = "memorial",
  hint = "Due to the sacred nature of this memorial," ..
    " you won't have to pay eminent domain costs."
});

addAchievement({
  code = "AchTheater",
  name = "Worldly City",
  text = "Ten thousand tourists, from all around the world," ..
    " are visiting our fair city tonight. Let's give them a show!",
  condition = function()
    return get(StatNumTouristsNow) > 10000 and timeNow() > OneHour*20
  end,
  effect = "theater",
  hint = ""
});

addAchievement({
  code = "AchIceRink",
  name = "Winter Sports",
  text = "It's may be cold, but we can still have fun outside.",
  condition = function()
    return get(StatWeatherTempStat) < 0 and now() > OneDay
  end,
  effect = "ice_rink",
  hint = ""
});

addAchievement({
  code = "AchFiveStars",
  name = "Five Stars",
  text = "Space Magazine just updated their rankings for best tourist" ..
    " destinations in the nation. And guess what -- we're #1 baby!",
  condition = function()
    return get(StatTouristRating) >= 5
  end,
  effect = "",
  hint = "",
});

addAchievement({
  code = "AchCasino",
  name = "The House Always Wins",
  text = "Giles Gatzby, a famous casino magnate, wants to open a new luxury resort" ..
    " in our city. However, a group of concerned citizens wants to block" ..
    " the construction. They worry that gambling will bring crime to the city." ..
    " Maybe it's not so luxurious after all -- but the money Gatzby will pay us sure is!",
  condition = function()
    return get(StatTouristRating) >= 1 and get(StatNumTouristsNow) > 1000
  end,
  effect = "casino",
  hint = "The Casino Resort will pay the city a fair amount every year to compensate for extra policing.",
});

addAchievement({
  code = "AchToxic",
  name = "Biffco keeps it safe!",
  text = "Bill Biffco, CEO of Biffco Enterprises, has a proposition:" ..
    " He needs a port and processing plant for dangerous toxic chemicals." ..
    " Biffco boasts that they've \"never had a spill\" -- " ..
    " but there's still some \"non-toxic\" pollution that will need to be dumped." ..
    " In exchange for one of our pristine coastlines, he'll give us some clean cash.",
  condition = "building:i.d2",
  effect = "toxic",
  hint = "Biffco Enterprises will pay the city a fair amount every year to compensate for the inevitable pollution from the plant.",
});

addAchievement({
  code = "AchTwinCities",
  name = "Twin Cities",
  text = "Two distinct city centers now exist in your city. Residents argue over which one is better.",
  condition = function()
    return get(StatCityCenters) > 1
  end,
  associate_building = "skyscraper",
  effect = "",
});

-- addAchievement({
--  code = "AchTransitSystems",
--  name = "Cramped Quarters",
--  text = "Everyone agrees: A city like this needs a better" ..
--    " mass transit system.",
--  condition = "building:r.d5",
--  effect = "FTransitSystems",
--  hint = ""
-- });

addAchievement({
  code = "AchSportsComplex",
  name = "Do you like sports?",
  text = "We got all the sports!",
  condition = "amenity_built:tennis,amenity_built:basketball,amenity_built:ballpark,",
  effect = "Sport_complex",
  hint = "",
});

addAchievement({
  code = "AchToTheMoon",
  name = "To the Moon!",
  text = "Citizens from our city were selected to go to space!",
  condition = function()
    return hasArticleAppeared("AstronautsSelected01");
  end,
  effect = "",
  hint = "",
});

addAchievement({
  code = "AchThereThere",
  name = "There is a There There",
  text = "Our city is recognized as having a real sense of place.",
  condition = function()
    return hasArticleAppeared("FriendlyCity01") and hasArticleAppeared("Restaurants01") and amenityEffectScore(PrestigeScore) > 200 and amenityEffectScore(CommunityScore) > 200 and
      numBuildings("artdeco07") > 0
  end,
  associate_building = "artdeco07",
  effect = "",
  hint = "",
});

addAchievement({
  code = "AchSail",
  name = "Sail Away",
  text = "Our city is now one of the top tourist destinations in the world!",
  condition = function()
      return numBuildings("skyscrapersail") > 0
  end,
  associate_building = "skyscrapersail",
  effect = "",
  hint = "",
});

addAchievement({
  code = "AchBrutal",
  name = "Brutal",
  text = "You won the game on Very Hard difficulty.",
  condition = function()
    return CDifficultyLevelName == "Very Hard" and get(StatSkyscrapers) > 0
  end,
  associate_building = "skyscraper",
  effect = "",
  hint = "\"Very Hard? I didn't break a sweat!\"",
});

addAchievement({
  code = "AchFuelTax",
  name = "Tax Gas, Fix Traffic?",
  text = "The city is considering a new tax on gasoline to fund" ..
    " transit infrastructure. The tax might reduce traffic congestion" ..
    " by encouraging people to drive less. But it might also" ..
    " hurt the economy by increasing the cost of doing business.",
  condition = function()
      return get(StatTransitTrips) > 500 and get(StatTimeSpentTraveling) > 50
  end,
  effect = "FFuelTax",
  hint = "",
});

