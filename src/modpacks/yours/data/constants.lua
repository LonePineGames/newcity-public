------------------------------
-- NewCity Configuration --
------------------------------
-- To modify these values, remove the two dashes at the beginning (uncomment) and then modify the value.

-------------
-- General --
-------------
-- CMaxCorruptSaves = 20
-- CStartBuildingDesigner = false
-- CAutosaveDisabled = false
-- CDesignerGridSize = 2
-- CDisableDefaultEvents = false
-- CEnableDesignOrganizer = true
-- CFastAdvanceTutorial = false
-- CScrollSpeed = 4
-- CDifficultyLevelName = "Medium"

----------
-- Time --
----------
-- CStartYear = 1950
-- CDefaultLightTime = 0.45
-- CGameDayInRealSeconds = 24*60; -- One irl sec = One game minute at Normal Speed

-----------------------
-- Render and Camera --
-----------------------
-- CLandVisible = true
-- CObjectViewer = false
-- CGraphVisible = true
-- CRenderSky = true
-- CRenderWeather = true
-- CRenderSnow = true
-- CRenderNight = true
-- CRenderMouseLine = false
-- CShowIssueIcons = true
-- CShowFallbackElevators = false
-- CAquaticBuildingHeight = 2
-- CLotVisibleDistance = 25000
-- CEnableFPSCap = true
-- CGammaCorrect = true
-- CLogUpdateTime = false
-- CLogBusinessGrowthData = false
-- CBuildingTextureFiltering = true
-- CHideUnderwater = false
-- CShowPollution = true
-- CPaletteFrames = 12
-- CPaletteAnimationFramerate = 60
-- CPreviewImageDistance = 1000

-- CCullingAlgo = 3
-- 0 = Old Culling Algo
-- 1 = New Culling Algo
-- 2 = New Culling Algo, Separate Thread
-- 3 = New Culling Algo, Separate Thread, Collect Draw Buffer in Thread

pi_o = 3.141592
-- CMinCameraDistance = 10.0
-- CMaxPitch = pi_o*.49
-- CMinPitch = 0.00001
-- CPitchClassic = pi_o*.5 - 0.46 -- 2:1 axonometric projection ala SC2K
-- CMeshQuality = 1.0
-- CCameraFOV = 0 -- 0 = ortho, 5 = 5 degrees FOV, 90 = 90 degrees FOV
-- CSatMapResolution = 4096

-- CCameraLag = 0.1
-- CCameraSpringSpeed = 0.1
-- CCameraPitchSpeed = 1
-- CCameraYawSpeed = 3
-- CCameraYawKeySpeed = 2
-- CCameraSpeedX = 6
-- CCameraSpeedY = 4
-- CCameraZoomSpeed = 0.15
-- CCameraKeyZoomSpeed = 0.05
-- CCameraPanSpeed = 1.8
-- CCameraPanSpeedClassic = 5
-- CCameraPanSmoothing = 0.5
-- CEnableCameraMovement = true

-- CTextResolution = 256;
-- CTextAtlasWidth = 2048;
-- CTextAtlasPadding = 64;
-- CTextWidthAdjust = 1;
-- CXHeightAdjust = 1.2;
-- CZeroWidthAdjust = 0.9;

-- These constants decide how to hide buildings and their decorations
-- based on camera distance. In other words, Level of Detail.
-- CBuildingCullMin = 5000
-- CBuildingCullSizeFactor = 50
-- CBuildingCullZFactor = 250
-- CBuildingCullSimplify = .25
-- CBuildingCullDeco = .4
-- CBuildingCullSimplifyDeco = .1

-- CVehicleCull = 2000
-- CTransitVehicleCull = 16000
-- CWandererCull = 16000
-- CVehicleCullSimplify = 200
-- CHeadlightSizeY = 40
-- CHeadlightSizeZ = 20
-- CHeadlightCullDistance = 10000 -- distance to hide car headlights effect

-- CLandCull = 1000000
-- CTreeCull = 50000

-----------
-- Sound --
-----------
-- CEnvironmentSoundRate = 3 -- lower = more frequent enivornment sounds
-- CEnableSounds = true
-- CSongDelay = 15.0

----------
-- Land --
----------
-- CTileSize = 25
-- CZTileSize = 8
-- CMaxHeight = 500
-- CDefaultLandSize = 75
-- CCustomLandSize = 0
-- CSuperflatHeight = 20
-- CTreesAddRate = 50
-- CTreesAddCost = 100
-- CEarthworksCost = 2
-- CDisableDefaultTerrainConfig = false
-- CMinTreeSpacing = 12.51
-- CTreeSpacingDeviation = 0
-- CTreeInnerFrequency = 0.1 -- Applies to land rendering
-- CTreeOuterFrequency = 0.1 -- Applies to land generation

------------------------
-- Neighboring Cities --
------------------------
-- CCityDistance = 2500
-- CCityExtent = 1000
-- CCityGrowthRate = 0.005
-- CCityGrowthMinDemand = 0.4

---------------------
-- Land Rendering ---
---------------------
-- CUseOldTerrainRendering = false
-- CLandTextureFiltering = false
-- CMaxLandErrorClose = 4
-- CMaxLandErrorFar = 10
-- CLandErrorConcaveFactor = 10
-- CLandErrorColorFactor = 5
-- CLandSimpleDistance = 2000
-- CTreeSimpleDistance = 5000
-- CWaterSimpleDistance = 5000
-- CTerrainChunking = 5

-- CShowLandTiling = false -- Diagnostic Tool
-- Careful, turning this feature on with a large map can overuse your system RAM

-- CDefaultChunkSize = 25
-- CHeightFudgeMultiplier = 15
-- CHillOctaves = 6
-- CHillSize = defaultChunkSize
-- CTreeOctaves = 3
-- CForestSize = defaultChunkSize / 4
-- CHeightAboveSeaLevel = 32

-- CMaxLandSize = 200
-- CMaxMound = tileSize*6
-- CMaxSlope = 0.75
-- CMaxMoundHeight = maxMound*maxSlope

------------------
-- Achievements --
------------------

-- CDisableDefaultAchievements = false

----------------
-- Activities --
----------------
-- Activities are compared to eachother using a score. Person transitions
-- to activity with highest score, as the scores change during the day.
-- Each person has an "energy" value, randomly chosen between 0.5 and 2.
-- All times are in units of days. One hour is 1/24th.

-- dayProximity(hour) returns the difference between parameter and current time
-- range: 0 to 1.0 (double fraction of a day, 1.0 at when time == hour)
-- dayProximity(A,B,C) return dayProximity((A+B*energy)*oneHour)*C*energy;
-- This causes the activity to be done during the right time of day

-- activityInertia(targetTime) returns targetTime - timeInActivity;
-- result: difference in days
-- activityInertia(A,B,C) returns A*oneHour +
--   B*activityInertia(C*energy*oneHour);

-- score is generally computed as follows:
-- score = C___BaseScore + dayProximity(A,B,C) +
--   activityInertia(A,B,C) + C___LightLevel*lightLevel;

-- CMinActScore = -100

-- SLEEP (energy is 1/energy)
-- CSleepBaseScore = -0.5
-- CSleepSick = 4
-- CSleepDayProxA = 22
-- CSleepDayProxB = 4
-- CSleepDayProxC = 2
-- CSleepLightFactor = -2
-- CSleepInertiaA = 0
-- CSleepInertiaB = 2
-- CSleepInertiaC = 12

-- HOME (energy is 1/energy)
-- CHomeBaseScore = 2.5
-- CHomeSick = 1
-- CHomeDayProxA = 20
-- CHomeDayProxB = 2
-- CHomeDayProxC = 0.25
-- CHomeLightFactor = -0.5
-- CHomeInertiaA = -4
-- CHomeInertiaB = 1.5
-- CHomeInertiaC = 12
-- CHomeUnemployed = 2.0

-- FRIEND
-- CFriendBaseScore = .75
-- CFriendSick = 0
-- CFriendDayProxA = 10
-- CFriendDayProxB = 4
-- CFriendDayProxC = 0.1
-- CFriendLightFactor = 0.1
-- CFriendEnergy = 0.5
-- CFriendHomeless = 1.0
-- CFriendInertiaA = 0
-- CFriendInertiaB = 1
-- CFriendInertiaC = 4

-- WORK
-- CWorkBaseScore = 0.25
-- CWorkSick = -5
-- CWorkDayProxA = 4
-- CWorkDayProxB = 10
-- CWorkDayProxC = 0.5
-- CWorkLightFactor = 0
-- CWorkOpenFactor = 0.05 -- bonus if employer is open
-- CWorkTimeSince = 1 -- time in days since last fam member arrival at work
-- CWorkInertiaA = 0
-- CWorkInertiaB = 1
-- CWorkInertiaC = 6

-- INTERVIEW
-- CInterviewBaseScore = 5
-- CInterviewSick = -5
-- CInterviewDayProxA = 6
-- CInterviewDayProxB = 10
-- CInterviewDayProxC = 2
-- CInterviewLightFactor = 2.25
-- CInterviewTimeSince = 2
-- CInterviewEducation = 0.25
-- CInterviewInertiaA = 0
-- CInterviewInertiaB = 16
-- CInterviewInertiaC = 1

-- STORE
-- CStoreBaseScore = -4.0
-- CStoreBaseScoreTourist = 1.0
-- CStoreSick = -4
-- CStoreDayProxA = 12
-- CStoreDayProxB = 12
-- CStoreDayProxC = 0.25
-- CStoreLightFactor = 0.25
-- CStoreProsperityFactor = 1
-- CStoreTimeSince = 4
-- CStoreInertiaA = 0
-- CStoreInertiaB = 16
-- CStoreInertiaC = 1

-- FREIGHT
-- CFreightBaseScore = 1
-- CFreightSick = -5
-- CFreightDayProxA = 2
-- CFreightDayProxB = 8
-- CFreightDayProxC = 1
-- score += energy * CFreightA *
--   clamp(timeSinceFreightOut - CFreightB, -CFreightB, CFreightC);
-- CFreightA = 2
-- CFreightB = 0.5
-- CFreightC = 2

-- AMENITY
-- score += clamp(numOpenAmenities*CAmenityOpenPerPop/pop, 0, CAmenityOpenMax);
-- CAmenityBaseScore = -2
-- CAmenityBaseScoreTourist = 1
-- CAmenitySick = -2
-- CAmenityStudentScore = 0.25
-- CAmenityOpenPerPop = 10000
-- CAmenityOpenMax = 0.5
-- CAmenityDayProxA = 8
-- CAmenityDayProxB = 4
-- CAmenityDayProxC = 1.0
-- CAmenityHomeless = 1.0
-- CParkTime = 2
-- CEducationTime = 6
-- CUniversityTime = 6
-- CServicesTime = 4
-- CAmenityInertiaA = 0
-- CAmenityInertiaB = 2
-- CAmenityCapacity = 2 -- max people in amenity per million dollars cost

-- DOCTOR
-- CDoctorBaseScore = -2.0
-- CDoctorDayProxA = 8
-- CDoctorDayProxB = 4
-- CDoctorDayProxC = 0.5
-- CDoctorSick = 8.0
-- CDoctorInertiaA = 0
-- CDoctorInertiaB = 8
-- CDoctorInertiaC = 2

-----------------------
-- Education Heatmap --
-----------------------

-- CEducationInit = 0.0
-- CEducationEffect = 0.075
-- CEducationDissipationRate = 0.001

-- CPhdEducationEffect = 10
-- CBclEducationEffect = 5
-- CHSEducationEffect = 1
-- CNoEducationEffect = -1

------------------------
-- Prosperity Heatmap --
------------------------

-- CProsperityInit = 0.5
-- CProsperityEffect = 0.00004
-- CProsperityDissipationRate = 0.001

-- Events
-- CRoadBuiltProsperity = 2000.0 -- multiplied by number of lots
-- CLotZonedProsperity = 1000.0
-- CNewAmenityProsperity = 400 -- per million dollars
-- CNewBuildingProsperity = 2000
-- CMoveInProsperity = 1000

-- CRouteFailedProsperity = -20 -- penalty for disconnected roads
-- CLongTripProsperity = -5 -- these are rare
-- CSuccessfulTripProsperity = 15
-- CShoppingTripProsperity = 35
-- CFreightDeliveryPropserity = 25

-- Time multiplied effects
-- CEmployedProsperity = 0.25
-- CTouristProsperity = 0.5
-- CHomelessProsperity = -1.0
-- CHungryFamilyProsperity = -1.0 -- hungry means no store in driving distance
-- CJoblessFamilyProsperity = -1.0
-- CJoblessProsperity = -0.5
-- CSickProsperity = -0.5
-- CPhdEducationProsperity = 0.020
-- CBclEducationProsperity = 0.010
-- CHSEducationProsperity = 0.005
-- CNoEducationProsperity = -0.05
-- CBusinessProsperity = 6
-- CUnfilledPositionProsperity = -0.5
-- CNoFreightProsperity = -2
-- CNoCustomerProsperity = -2
-- CAbandonedProsperity = -4.0
-- CAmenityProsperityEffect = 50 -- per point
-- CAmenityProsperityBonus = 15 -- per million dollars/year maintenence
-- CTruckProsperity = 1.0 -- roads only, distance based
-- CVehicleProsperity = 0.1 -- roads only, distance based

-------------------
-- Value Heatmap --
-------------------

-- CValueInit = 0.4
-- CValueEffect = 0.2
-- CValueDissipationRate = 0.01

-- Cap - value cannot exceed base + sum of constant*heatmap^2
-- CValueCapBase = 0.2
-- CValueCapEducation = 1
-- CValueCapProsperity = 0.5
-- CValueCapCrime = -1
-- CValueCapPollution = -1
-- CValueCapDensity = -1
-- CValueCapHealth = 0.5
-- CValueCapCommunity = 0.5

-- Influence - effect of one heatmap on another
-- CCrimeInfluenceOnValue = -1.
-- CPollutionInfluenceOnValue = -0.5
-- CEducationInfluenceOnValue = 0.5
-- CDensityInfluenceOnValue = -0.5

-- Terrain effects -- time multiplied per heatmap tile
-- CValueCoastTarget = 0.8
-- CValueCoastEnd = 35
-- CValueHillTarget = 0.6
-- CValueHillStart = 100
-- CValueHillEnd = 300
-- CValueSuperflat = 0.6
-- CValueTerrainEffect = 0.25

-- Time multiplied effects
-- CAmenityValueEffect = 0.25
-- CAmenityValueThrow = 5*CTileSize
-- CTruckValue = -0.001
-- CVehicleValue = -0.0005
-- CParkLotValue = 0.03
-- CParkHillValue = 0.04
-- CParkBeachValue = 0.05
-- CNiceBuildingValue = 0.004
-- CBadBuildingValue = -0.004
-- CAbandonedBuildingValue = -0.008

---------------------
-- Density Heatmap --
---------------------

-- CDensityInit = 0.0
-- CDensityEffect = 0.01
-- CDensityDissipationRate = 0.005
-- CDensityInertia = 0.99
-- CPopForMaxDensity = 200000
-- CPavementDensity = 0.5
-- CDensePavementZ = 0.25

-- Cap - density cannot exceed base + sum of constant*heatmap^2
-- CDensityCapBase = -0.5
-- CDensityCapProsperity = 1.5
-- CDensityCapEducation = 1.5
-- CDensityCapCrime = -0.5
-- CDensityCapPollution = -0.5
-- CDensityCapHealth = 0.5

-- CDensityCapCoastEffect = 0.4
-- CDensityCapCoastEnd = 50
-- CDensityCapHillEffect = -0.2
-- CDensityCapHillStart = 100
-- CDensityCapHillEnd = 300

-- Events
-- CShoppingTripDensity = 2

-- Time multiplied effects
-- CVehicleDensity = 0.0005 -- roads only, distance based
-- CTruckDensity = 0.002 -- roads only, distance based
-- CTransitDensity = 0.0025 -- distance * passengers
-- CEmptyLotDensity = -0.25
-- CEmptyTenancyDensity = -0.01
-- CAbandonedDensity = -0.05
-- COfficeDensity = 0.1
-- CFactoryDensity = 0.075
-- CAmenityDensityEffect = 0.25 -- per point
-- CAmenityDensityBonus = 1 -- per million dollars/year maintenence

-----------------------
-- Pollution Heatmap --
-----------------------

-- CPollutionInit = 0.0
-- CPollutionEffect = 0.01
-- CPollutionDissipationRate = 0.001

-- Terrain effects -- time multiplied, per heatmap tile
-- CPollutionCoastEffect = -0.025
-- CPollutionCoastEnd = 25
-- CPollutionHillEffect = -0.05
-- CPollutionHillStart = 150
-- CPollutionHillEnd = 300

-- time multiplied effects
-- CFamilyPollution = .001
-- CStudentPollution = .00001
-- CFarmPollution = .02
-- CFactoryPollution = .4
-- CCommercialPollution = .01
-- CTruckPollution = 0.0025
-- CVehiclePollution = 0.0015

-- CAmenityPollutionEffect = -2
-- CAmenityPollutionThrow = 15*CTileSize
-- CParkLotPollution = -0.075
-- CParkBeachPollution = -0.1
-- CParkHillPollution = -0.1

-------------------
-- Crime Heatmap --
-------------------

-- CCrimeInit = 0.0
-- CCrimeEffect = 0.0004
-- CCrimeDissipationRate = 0.01

-- time multiplied effects
-- CTouristCrime0 = 100 -- Crime from one tourist in a city with tourist rating 0
-- CTouristCrime5 =  10 -- Crime from one tourist in a city with tourist rating 5
-- CHungryFamilyCrime = 50
-- CJoblessFamilyCrime = 100
-- CJoblessCrime = 100
-- CAbandonedCrime = 100
-- CPhdEducationCrime = -10
-- CBclEducationCrime = -10
-- CHSEducationCrime = 0
-- CNoEducationCrime = 50
-- CAmenityCrimeEffect = -100
-- CAmenityCommunityEffect = -20
-- CAmenityCrimeThrow = 75*CTileSize

-- CFinesAndFeesCrime = 0.5 -- effect of fines and fees on crime
-- CDensityCrime = 1.5 -- effect of a 200k city on crime, compared to a small town

-----------------------
-- Community Heatmap --
-----------------------

-- CCommunityInit = 0.0
-- CCommunityEffect = 0.001
-- CCommunityDissipationRate = 0.01
-- CCommunityThrow = 10*CTileSize

-- Terrain effects -- time multiplied per heatmap tile
-- CCommunityCoastTarget = 0.6
-- CCommunityCoastEnd = 35

-- CJoblessCommunity = -0.3

-- CCommunityAmenityEffect = 30
-- CCommunityParkEffect = 1.0
-- CCommunityResidentialDens0Effect = 0.1
-- CCommunityResidentialDens10Effect = -0.07
-- CCommunityMixedUseBuildingEffect = 4.0
-- CCommunityRetailBuildingEffect = -1.0
-- CCommunityRetailBusinessEffect = -1.5
-- CCommunityOfficeBuildingEffect = -4.0
-- CCommunityFarmBuildingEffect = 5
-- CCommunityFactoryBuildingEffect = -4.0
-- CCommunitySickEffect = -0.4
-- CCommunityProsperityInfluence = 20
-- CAbandonedCommunity = -20.0
-- CCommunityDensityInfluence = -1

--------------------
-- Health Heatmap --
--------------------

-- CHealthInit = 1.0
-- CHealthEffect = 0.08
-- CHealthDissipationRate = 0.005
-- CHealthParkEffect = 0.002
-- CHealthPollutionInfluence = -1.25
-- CHealthCrimeInfluence = -1.15
-- CHealthCommunityInfluence = 0.5
-- CHealthThrow = 120*CTileSize

-- CPersonHealth = -0.001
-- CHealthVehicleEffect = 0 ---0.0015
-- CHealthTruckEffect = 0 ---0.00225
-- CAmenityHealthEffect = 8.0
-- CSickRateMinHealth = 0.05 -- probability of getting sick in a given day
-- CSickRateMaxHealth = 0.001
-- CSickTransferRate = 0.05
-- CSickHealRate = 0.25 -- probability of healing in a given day
-- CSickHealthEffect = -1.25
-- CJoblessHealth = -2
-- CHungryHealth = -0.5
-- CHomelessHealth = -4

-------------
-- Timings --
-------------

-- CMapRenderInterval = 30
-- CAutosaveInterval = 300 -- in seconds
-- CMaxAutosaveInterval = 1800 -- in seconds
-- CGameSpeedSmoothing = 10
-- CGameUpdateTime = 0.2
-- CMaxPendingGameUpdate = 1.0
-- CHeatMapTime = 10
-- CVehicleUpdateTime = 0.2
-- CVehicleHeatmapTime = 6 -- how often to sample vehicles for the heatmap
-- CBuildingUpdateTime = 10
-- CBuildingUpdateSpread = 5
-- CPersonEvaluateTime = 4 -- in hours
-- CPersonUpdateTime = 2 -- in hours

-----------
-- Roads --
-----------
-- CLaneWidth = 3
-- CRailGauge = 1.9
-- CSholderWidth = 1.5
-- CMaxIntersectionLegs = 6
-- CLeftHandTraffic = false
-- CMaxBridgeLength = CTileSize * 25 + 1
-- CMaxSuspensionBridgeLength = CTileSize * 75 + 1
-- CMinRoadLength = CTileSize * 2
-- CMaxGrade = 0.25
-- CMinEdgeAngle = pi_o / 8
-- CMinPillarAngle = pi_o / 3
-- CMinSuspensionPillarAngle = pi_o / 4
-- CRoadRise = 0.5
-- CPillarHeight = 40

-- CSidewalkRise = 0.2
-- CSidewalkWidth = 2
-- CSidewalkBezel = 4
-- CSidewalkDown = -4
-- CSidewalkDensity = 0.2

-- CPavedRailDensity = 0.3
-- CExpwyWallHeight = 4
-- CExpwyWallWidth = 0.5

-- CTruckWear = 1.0 -- Distance based
-- CVehicleWear = 2.0 -- Distance based
-- CWearRate = 0 --.0002
 -- 1.0 = every vehicle wears (to 100%) one square meter of road every second

---------------
-- Road Tool --
---------------
-- CSnapDistance = CTileSize*.75
-- CMinNodeDistance = CTileSize*.25
-- CBulldozerSnapDistance = CTileSize*2
-- CMinNodeDist = CTileSize*.25
-- CEnableCutTool = true
-- CEnableUTurn = true
-- CEnableEdgePlacement = true
-- CRoadContributorNameChance = 0.05

-- Mass Transit
-- CStopRadius = 40 * CTileSize
-- CWalkingSpeed = 20 -- meters per second (exaggerated for balance)
-- CTransitLineWidth = CLaneWidth*10 -- visual
-- CDefaultTransitHeadway = 30 -- in minutes
-- CNumTransitBids = 5
-- CTransitDesignTime = 1 -- time to design a transit system, in days
-- CTransitDesignDelayChance = 0.25 -- chance that there is a delay.
-- CMaxCarsPerVehicle = 24
-- CAlightingRate = 20 -- per vehicle update

-------------
-- Traffic --
-------------

-- CTrafficRate0    = 0.5
-- CTrafficRate100K = 0.12
-- CTrafficRate1M   = 0.12
-- CTransitBias = 2.0 -- larger = more transit usage
-- CInterpolateVehicles = true
-- CMaxSpawn = 20
-- CMaxVehicles = 60000
-- CMinLaneSpace = 20
-- CVehicleSpeed = 60.
-- CVehicleAcceleration = 200
-- CMaxCommute = 4 -- in hours
-- CTripLimitMultiplier = 3 -- vehicles are culled if they exceed CTripLimitMultiplier times their estimated travel time.
-- CBumperDistance = 1 -- meters
-- CMergeDistance = 20 -- meters
-- CMergeSpace = 2 -- meters, mininum space to make a merge
-- CMergeMaxSpeedDiff = 5 -- meters per second, max difference in speed
-- CVehicleLengthForCapacity = 20 -- meters
-- CBackpressureFactor = .95 -- backpressure starts at 95% capacity
-- CBackpressureBias = 0
-- CStopLightMinPhaseDuration = 1;
-- CStopLightMaxPhaseDuration = 5;
-- CStopLightYellowDuration = 0.2;
-- CStopSignDuration = 0.2;
-- CMaxVehicleAge = 1.0; -- in days
-- CEnableSemis = false;
-- CDisableDefaultVehicleModels = false;

-- CRouterCacheCapacity = 5*1000*1000
-- CAStarFactor = 0.25 -- 0 = use djikstra's, over 1.0 = inaccurate routes
-- CRoutingTrafficAwareness = 0.8 -- 0 = no traffic aware routing
-- CTraversalRecordDynamism = 0.002
-- CTraversalRecordMax = 10
-- CRouterCacheDiscardRate = 5 -- number of routes to discard (recompute) per sec
-- CRouterStopDiscardingThreshold = 80000
-- CRouterQueueCapacity = 100000
-- CRouterBatchSize = 5000
-- CNumRouterThreads = 8
-- CRouterSlowGameThreshold3 = 20000 -- limit to game speed 3
-- CRouterSlowGameThreshold2 = 50000 -- limit to game speed 2
-- CRouterSlowGameThreshold1 = 80000 -- limit to game speed 1
-- CHeapOutdegree = 10 -- disabled
-- CSupplyTableUpdateRate = 0.1
-- CBrokerSearchDistance = 12 -- in chunks

-- measurement
-- CMsToMph = 1.5
-- CMsToKmph = 2.5

---------------
-- Buildings --
---------------
-- CBuildDistance = CLaneWidth*4+CSholderWidth
-- CBuildingAddRateBase = 10 -- per second
-- CBuildingAddRate = 0.001 -- additional building per second per existing building
-- CBuildingUpgradeRate = 0.05 -- try to add building to non-empty lot
-- CDisableDefaultBuildingSet = false
-- CDisableDefaultBuildingTextures = false
-- CBuildingNameworthyValue = 500000

-- CSkyscraperMinDensity = 9
-- CSkyscraperMinHeight = 100

------------
-- People --
------------

-- CEnableMoveOuts = false
-- CMaxUnemploymentDiff = 0.05
-- CRejobRate = 0.9 -- when losing a job due to a long commute, workers will immediately find a new job this percent of the time.
-- CTargetEmptyHomes = 0.01
-- CUnderemploymentRate = 0.25
-- CPersonContributorNameChance = 0.5
-- CFamilySpawnRate = 100
-- CMoveEconRate = 0.1
-- CUnemploymentHomelessness = 0.005 -- fraction of unemployed who end up homeless
-- CResDemandHomelessness = 0.0002 -- effect of residential demand on homelessness
-- CCommunityHomelessness = 1 -- fraction of homeless helped by community

-- probOfDeathPerYear = 1.0/pow(2, (CMaxAgeBase+CMaxAgeHealthAdjust*heatmapHeath-age)/c(CDeathFactor));
-- CMaxAgeBase = 90
-- CMaxAgeHealthAdjust = 30
-- CDeathFactor = 4
-- CRetirementAge = 65

-- Days before lack of work/store causes stress
-- CStoreDays = 4;
-- CStoreIssueDays = 6;
-- CWorkDays = 2;
-- CWorkIssueDays = 1;

----------------
-- Tourism --
----------------
-- CTouristsPerPoint = 200
-- CMemoriesForFiveStars = 0.5 -- per tourist per hour
-- CTouristStay0 = 1;
-- CTouristStay5 = 5;

-- per event
-- CMemoriesPerTourismPoint = 1.0
-- CTransitMemories = 0.5

-- per hour
-- CPrestigeMemories = 0.1
-- CHungryTouristMemories = -0.1
-- CSickTouristMemories = -0.2
-- CNoHotelMemories = -0.5
-- CTouristMemoriesPollution = -0.5
-- CTouristMemoriesCrime = -0.5
-- CTouristMemoriesValue = 1.5
-- CTouristMemoriesDensity = 1.0
-- CTouristMemoriesProsperity = 0.5
-- CTouristMemoriesCommunity = 1.0
-- CTouristMemoriesHeath = 0.5

----------------
-- Education --
----------------
-- CAllowUniversityAtStart = false
-- CNationalPhdEdu = 0.01
-- CNationalBclEdu = 0.10
-- CNationalHSEdu = 0.20
-- CEducationMix = 0.8
-- CEducationNationalMix = 0.25

----------------
-- Businesses --
----------------

-- if (stress * randFloat() > CBusinessStressThreshold) removeBusiness()
-- CBusinessSpawnRate = 4
-- CEnableBusinessClosing = false
-- CNoCustomersStress = 5
-- CNoFreightStress = 2
-- CUnfilledPositionStress = 4
-- CBusinessProsperityStress = -2
-- CBusinessStressBase = 10
-- CBusinessUnemploymentStress = -2
-- CBusinessStressThreshold = 15
-- COnTheJobTarget = 0.25 -- Fraction of workforce that should be working at any given time
-- CBusinessOpenHour = 6
-- CBusinessCloseHour = 20
-- CTargetEmptyCommercial = 0.01
-- CTargetEmptyIndustrial = 0.01
-- CRetailEconomyPerBiz = 10000 -- How much $/hr Retail Transactions per retail biz
-- CMaxOpenJobs = 0.1 -- fraction of total jobs
-- CUnemploymentRetailBias = 0.08
-- CUnemploymentFarmBias = 0.14
-- CUnemploymentFactoryBias = 0.10
-- CUnemploymentOfficeBias = 0.14
-- CBusinessUnemploymentFactor = 5
-- CTargetUnemploymentBias0 = 0
-- CTargetUnemploymentBias1M = 0
-- CBizPerBizPoint = 5

-- Days before lack of freight/customers or number of unfilled jobs before stress
-- CFreightInDays = 4;
-- CFreightOutDays = 1;
-- CCustomerDays = 1;
-- CFreightInIssue = 6;
-- CFreightOutIssue = 4;
-- CCustomerIssue = 4;
-- CUnfilledJobsIssue = 8;

---------------
-- Positions --
---------------

-- CRetailNoEduPositions = 8
-- CRetailHSPositions = 2
-- CRetailBclPositions = 0
-- CRetailPhdPositions = 0

-- COfficeNoEduPositions = 0
-- COfficeHSPositions = 30
-- COfficeBclPositions = 30
-- COfficePhdPositions = 2

-- CFarmNoEduPositions = 25
-- CFarmHSPositions = 5
-- CFarmBclPositions = 0
-- CFarmPhdPositions = 0

-- CFactoryNoEduPositions = 10
-- CFactoryHSPositions = 50
-- CFactoryBclPositions = 5
-- CFactoryPhdPositions = 0

-- CParkNoEduPositions = 5
-- CParkHSPositions = 5
-- CParkBclPositions = 0
-- CParkPhdPositions = 0

-- CEducationNoEduPositions = 0
-- CEducationHSPositions = 10
-- CEducationBclPositions = 20
-- CEducationPhdPositions = 0

-- CServicesNoEduPositions = 0
-- CServicesHSPositions = 20
-- CServicesBclPositions = 20
-- CServicesPhdPositions = 0

---------------
-- Amenities --
---------------

-- CRecommendedAmenityCost =  5000000 -- per effect point
-- CRecommendedAmenityMaint = 2500000 -- per effect point
-- CAmenityCostMult = 0.1
-- CAmenityMaintMult = 0.025
-- CMaxEffect = 25
-- CAmenityEduThrow = 50*CTileSize
-- CMaxHSEdu = 0.9
-- CMaxBclEdu = 0.75
-- CMaxPhdEdu = 0.15
-- CMaxHSEduPerEffect = 500 -- people that can be educated per effect point
-- CMaxBclEduPerEffect = 250
-- CMaxPhdEduPerEffect = 20
-- CEduFreq = 1 -- how many people a school with 1 Education Point educates per second 
-- CCommunityFreq = 0.1

-- Impacts of global effects
-- (Technology, Business, Tourism, Prestige, Environmentalism, Order, Health)
-- CTechEduBasis = 1.01
-- CBusinessUnempBasis = 0.99
-- CMaxResidentialDensity = 0.4
-- CPrestigeMaxResDensity = 0.005
-- CEnvironmentalismPollutionBasis = 0.985
-- COrderLawBasis = 1.012
-- CCommunityCrimeBasis = 0.99
-- CUniversityMaxQuadDistance = 50*CTileSize;

-- CMaxHSEduPerCollege = 0 -- people that can be educated per college
-- CMaxBclEduPerCollege = 500
-- CMaxPhdEduPerCollege = 100

-----------
-- Money --
-----------

-- CGoodFaithLOC = 20000002
-- CStartingMoney = 0
-- CMaxBudgetControl = 1.5
-- CBudgetControlFactor = 0.5
-- CMoneyMultiplier = 1 -- money is multiplied by this value before being displayed to player

-- Property Value
-- CLandValue = .2
-- CLandValueMultiplier = 2
-- CDensityMultiplier = 2
-- CDensityMultiplierIndustrial = 0.5
-- CBuildingValue = 10000
-- CHomeBuildingValue = 1
-- CBusinessBuildingValue = 10
-- CHotelRoomBuildingValue = 4
-- CAquaticBuildingValue = 2
-- CMaxDesignCommonness = 0.2 -- If a building design makes up 20% of the buildings in that zone, the buildings will be worth almost nothing and are likely to be replaced.

-- Property Tax
-- CPropertyTaxUnemploymentEffect = 0.05
-- CPropertyTaxResidentialEffect = 10
-- CEnableTaxLock = false

-- Sales Tax
-- CCollegeEduRetailSpending = 20000
-- CHSEduRetailSpending = 10000
-- CNoEduRetailSpending = 5000
-- CSalesTaxUnemploymentEffect = 0.15
-- CSalesTaxRetailEffect = 5

-- Fines and Fees
-- CFinesPerPerson = 2500
-- CFinesPerBusiness = 10000
-- CFinesAndFeesTaxUnemploymentEffect = 0.05
-- See also CFinesAndFeesCrime

-- Buildings
-- CEminentDomainFactor = 0.5
-- CBuildingSalesFactor = 0.5
-- CAssetLOC = 0.25
-- CParkLotMaint = 1000
-- CParkLotThrow = CTileSize * 25

-- Roads
-- CLaneCostRoad = 25000
-- CLaneCostExpwy = 50000
-- CLaneCostRail = 50000
-- CPlatformCost = 25000
-- CWalkwayCost = 10000
-- CMinReconfigureCostRoad = 5000
-- CMinReconfigureCostExpwy = 10000
-- CMinReconfigureCostRail = 10000
-- CStrategyCostStopSign = 5000
-- CStrategyCostTrafficLight = 10000
-- CStrategyCostJunction = 20000
-- CPillarCost = 4000
-- CPillarCostSuspension = 16000
-- CPillarCostExponent = 2
-- CDestroyRoadCost = 5000
-- CDestroyPillarCost = 500000

-- Elevated Construction Costs
-- CEmbankmentCost = 1000
-- CTrenchCost = 1000
-- CViaductCost = 4000
-- CTunnelCost = 10000

-- Transit
-- CStopCost = 40000
-- CDefaultTicketPrice = 0.1
-- CDefaultTransferPrice = 0.02
-- CNewTrackTicketPrice = 0.5
-- CNewTrackTransferPrice = 0.2

-- CLitersPerMinuteBus = 0.75 -- fuel used by a bus driven for 1 minute
-- CLitersPerMinuteCar = 0.25 -- fuel used by a car driven for 1 minute
-- CMaintPerMinuteBus = 0.025 -- cost to maintain a bus operated for 1 minute
-- CMaintPerMinuteCar = 0.010 -- cost to maintain a car operated for 1 minute
-- CValueOfTime = 1.00 -- 1950 dollars per hour

-------------
-- Weather --
-------------

-- CSnowCollectionRate = 0.1

---------------
-- Newspaper --
---------------

-- CEnableNewspaper = true
-- CLogNewspaperData = true
-- CDisableDefaultNewspaperArticles = false
-- CHourlyNewspaper = false -- Causes the newspaper to be published once an hour
-- CNewspaperPrice = 0.15 -- In 1950
-- CNumNewspaperArticles = 4
-- CNewspaperRandomness = 4
-- CNewspaperMinLength = 50
-- CNewspaperTextured = true
