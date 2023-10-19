require 'data/wanderer-movement'

addVehicleModel({
  code = "VhSeagulls",
  mesh = "models/vehicles/seagulls.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  density = 0.5,
  maxAge = 6*OneHour,
  speed = 50,
  passengers = 0,
  length = 1,
  update = birdUpdate,
});

addVehicleModel({
  code = "VhEagle",
  mesh = "models/vehicles/eagle.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  density = 0.5,
  maxAge = 6*OneHour,
  speed = 80,
  passengers = 0,
  length = 1,
  update = birdUpdate,
});

addVehicleModel({
  code = "VhHelicopterNews",
  mesh = "models/vehicles/helicopter-news.obj",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = 0.4,
  maxAge = 2*OneHour,
  speed = 200,
  passengers = 4,
  length = 1,
  update = helicopterUpdate,
});

addVehicleModel({
  code = "VhHelicopterPolice",
  mesh = "models/vehicles/helicopter-police.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = 1.0,
  maxAge = 6*OneHour,
  speed = 400,
  passengers = 4,
  length = 1,
  update = helicopterUpdate,
});

addVehicleModel({
  code = "VhHelicopterHospital",
  mesh = "models/vehicles/helicopter-hospital.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = 0.5,
  maxAge = 4*OneHour,
  speed = 400,
  passengers = 4,
  length = 1,
  update = helicopterUpdate,
});

addVehicleModel({
  code = "VhHelicopterMilitary",
  mesh = "models/vehicles/helicopter-military.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = 1.0,
  maxAge = 4*OneHour,
  speed = 400,
  passengers = 4,
  length = 1,
  update = helicopterUpdate,
});

addVehicleModel({
  code = "VhHotAirBalloon1",
  mesh = "models/vehicles/hot-air-balloon1.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = 0.5,
  maxAge = 4*OneHour,
  speed = 50,
  passengers = 4,
  length = 1,
  update = balloonUpdate,
});

addVehicleModel({
  code = "VhHotAirBalloon2",
  mesh = "models/vehicles/hot-air-balloon2.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = 0.5,
  maxAge = 4*OneHour,
  speed = 50,
  passengers = 4,
  length = 1,
  update = balloonUpdate,
});

addVehicleModel({
  code = "VhPartyBalloons",
  mesh = "models/decorations/balloons2.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = 0.1,
  maxAge = 2*OneHour,
  speed = 50,
  passengers = 0,
  length = 1,
  update = balloonUpdate,
});

addVehicleModel({
  code = "VhBlimpNewCity",
  mesh = "models/decorations/blimp-newcity.obj",
  simpleMesh = "models/decorations/blimp-newcity.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = 0.9,
  maxAge = 6*OneHour,
  speed = 50,
  passengers = 4,
  length = 1,
  update = blimpUpdate,
});

addVehicleModel({
  code = "VhBlimpBlue",
  mesh = "models/decorations/blimp-blue.obj",
  simpleMesh = "models/decorations/blimp-blue.obj",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  spawned = true,
  density = .5,
  maxAge = 6*OneHour,
  speed = 50,
  passengers = 4,
  length = 1,
  update = blimpUpdate,
});

addVehicleModel({
  code = "VhCruiseShip",
  mesh = "models/decorations/cruise-ship.obj",
  simpleMesh = "models/decorations/cruise-ship.obj",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  spawned = true,
  density = 1,
  maxAge = 6*OneHour,
  speed = 25,
  passengers = 100,
  length = 100,
  update = boatUpdate,
});

addVehicleModel({
  code = "VhContainerShip",
  mesh = "models/decorations/container-ship.obj",
  simpleMesh = "models/decorations/container-ship.obj",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  spawned = true,
  density = 0.1,
  maxAge = 6*OneHour,
  speed = 15,
  passengers = 200,
  length = 100,
  update = boatUpdate,
});


addVehicleModel({
  code = "VhKayak",
  mesh = "models/vehicles/kayak.obj",
  --simpleMesh = "models/vehicles/kayak-simple.obj",
  texture = "textures/vehicles/Kayak.png",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  spawned = true,
  density = 1,
  maxAge = 6*OneHour,
  speed = 10,
  passengers = 4,
  length = 1,
  update = boatUpdate,
});

addVehicleModel({
  code = "VhFishingShip",
  mesh = "models/vehicles/fishing-ship.obj",
  simpleMesh = "models/vehicles/fishing-ship-simple.obj",
  texture = "textures/vehicles/fishing-ship.png",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  spawned = true,
  density = 0.2,
  maxAge = 6*OneHour,
  speed = 50,
  passengers = 4,
  length = 1,
  update = boatUpdate,
});

addVehicleModel({
  code = "VhMotorboat",
  mesh = "models/vehicles/motorboat.obj",
  simpleMesh = "models/vehicles/motorboat-simple.obj",
  texture = "textures/vehicles/motorboat.png",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  spawned = true,
  density = 1,
  maxAge = 6*OneHour,
  speed = 100,
  passengers = 4,
  length = 1,
  update = boatUpdate,
});

addVehicleModel({
  code = "VhSailboat",
  mesh = "models/vehicles/sailboat.obj",
  simpleMesh = "models/vehicles/sailboat-simple.obj",
  --texture = "textures/vehicles/sailboat.png",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  spawned = true,
  density = 0.2,
  maxAge = 6*OneHour,
  speed = 20,
  passengers = 4,
  length = 1,
  update = boatUpdate,
});

addVehicleModel({
  code = "VhYacht",
  mesh = "models/vehicles/yacht.obj",
  simpleMesh = "models/vehicles/yacht-simple.obj",
  texture = "textures/vehicles/yacht.png",
  colorized = true,
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  spawned = true,
  density = 1,
  maxAge = 6*OneHour,
  speed = 50,
  passengers = 4,
  length = 1,
  update = boatUpdate,
});

addVehicleModel({
  code = "VhSailfish",
  mesh = "models/decorations/sailfish.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  density = 1,
  maxAge = 6*OneHour,
  speed = 10,
  passengers = 0,
  length = 1,
  update = fishUpdate,
});

addVehicleModel({
  code = "VhSwordfish",
  mesh = "models/decorations/swordfish.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  density = 1,
  maxAge = 6*OneHour,
  speed = 10,
  passengers = 0,
  length = 1,
  update = fishUpdate,
});

addVehicleModel({
  code = "VhShark",
  mesh = "models/vehicles/shark.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  density = 0.1,
  maxAge = 6*OneHour,
  speed = 10,
  passengers = 0,
  length = 1,
  update = fishUpdate,
});

addVehicleModel({
  code = "VhWhale",
  mesh = "models/vehicles/whale.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  sea = true,
  density = 0.1,
  maxAge = 6*OneHour,
  speed = 4,
  passengers = 0,
  length = 1,
  update = fishUpdate,
});

addVehicleModel({
  code = "VhWinAnimation",
  mesh = "models/vehicles/win-animation.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  maxAge = 2*OneHour,
  speed = 0,
  passengers = 0,
  length = 1,
  update = nullUpdate,
});

-- TODO
-- Road Vehicles
-- decorations/ambulance.obj
-- decorations/ambulance-simple.obj
-- decorations/box-truck-food1.obj
-- decorations/box-truck-food1.obj
-- decorations/box-truck-food2-simple.obj
-- decorations/box-truck-food2-simple.obj
-- decorations/bus-school.obj
-- decorations/bus-school-simple.obj

-- Flying
-- vehicles/bird.obj
-- vehicles/hangglider1.obj
-- vehicles/hangglider1-simple.obj
-- vehicles/hangglider2.obj
-- vehicles/hangglider3.obj
-- vehicles/hangglider4.obj

-- Aquatic
-- decorations/cruise-ship.obj
-- vehicles/container-ship-average1.obj
-- vehicles/container-ship-average.obj
-- vehicles/container-ship-empty.obj
-- vehicles/container-ship-full.obj
-- vehicles/container-ship-low.obj

-- Land Roaming
-- decorations/tractor.obj
-- decorations/tractor-simple.obj

-- Duplicates
-- decorations/balloons1.obj
-- decorations/balloons1-simple.obj
-- decorations/container-ship2.obj
-- decorations/container-ship3.obj
-- decorations/container-ship4.obj
-- decorations/container-ship5.obj
-- decorations/container-ship6.obj
-- decorations/container-ship7.obj
-- decorations/container-ship.obj
-- decorations/fishing-ship1.obj
-- decorations/fishing-ship1-simple.obj
-- decorations/kayak.obj
-- decorations/sailboat.obj
-- decorations/sailboat1.obj
-- decorations/sailboat-simple.obj
-- decorations/yacht1.obj
-- decorations/yacht2.obj
-- decorations/yacht-simple.obj
-- vehicles/seagull-frame1.obj
-- vehicles/seagull-frame2.obj
-- vehicles/seagull-frame3.obj
-- vehicles/seagull.obj

-- DONE
-- decorations/balloons2.obj
-- decorations/balloons2-simple.obj
-- decorations/blimp-blue.obj
-- decorations/blimp-newcity.obj
-- decorations/sailfish.obj
-- decorations/shark.obj
-- decorations/swordfish.obj
-- vehicles/fishing-ship.obj
-- vehicles/fishing-ship-simple.obj
-- vehicles/helicopter-hospital.obj
-- vehicles/helicopter-military.obj
-- vehicles/helicopter-news.obj
-- vehicles/helicopter-police.obj
-- vehicles/hot-air-balloon1.obj
-- vehicles/hot-air-balloon2.obj
-- vehicles/kayak.obj
-- vehicles/kayak-simple.obj
-- vehicles/motorboat.obj
-- vehicles/motorboat-simple.obj
-- vehicles/sailboat.obj
-- vehicles/sailboat-simple.obj
-- vehicles/yacht.obj
-- vehicles/yacht-simple.obj

