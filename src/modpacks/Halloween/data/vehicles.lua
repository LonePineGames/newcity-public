
addVehicleModel({
  code = "VhGhost1",
  mesh = "modpacks/halloween/models/vehicles/ghost1.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  density = 0.5,
  maxAge = 6*OneHour,
  speed = 15,
  passengers = 0,
  length = 1,
  update = birdUpdate,
});

addVehicleModel({
  code = "VhGhost2",
  mesh = "modpacks/halloween/models/vehicles/ghost2.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  density = 0.5,
  maxAge = 6*OneHour,
  speed = 15,
  passengers = 0,
  length = 1,
  update = birdUpdate,
});

addVehicleModel({
  code = "VhWitch1",
  mesh = "modpacks/halloween/models/vehicles/Witch1.obj",
  type = "VhTypeWanderer",
  wanderer = true,
  air = true,
  density = 0.5,
  maxAge = 6*OneHour,
  speed = 100,
  passengers = 0,
  length = 1,
  update = birdUpdate,
});

