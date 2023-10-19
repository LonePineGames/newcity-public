addGraphClass({
  code = "GrClassRoad",
  name = "Road",
  maxSpeed = 75
})

addGraphClass({
  code = "GrClassExpwy",
  name = "Expressway",
  maxSpeed = 150
})

addGraphClass({
  code = "GrClassRailStandard",
  name = "Rail (Standard Gauge)",
  maxSpeed = 150
})

addGraphClass({
  code = "GrClassWalkway",
  name = "Walkway",
  maxSpeed = 50
})

addGraphClass({
  code = "GrClassMagLev",
  name = "MagLev",
  maxSpeed = 400
})

addGraphClass({
  code = "GrClassMonorail",
  name = "MagLev",
  maxSpeed = 100
})

addGraphClass({
  code = "GrClassMonoHang",
  name = "MagLev",
  maxSpeed = 100
})

addGraphClass({
  code = "GrClassGuideway",
  name = "Guideway",
  maxSpeed = 150
})

addGraphFeature({
  code = "GrTracLeather",
  name = "Pedestrians",
  maxSpeed = 10,
  icon = {5,5,0},
})

addGraphFeature({
  code = "GrTracRubber",
  name = "Bus",
  type = "traction",
  maxSpeed = 150,
  noise = 10,
  value = -10,
  maxCars = 1,
  vehicleCost = 10000,
  stopCost = 10000,
  laneCost = 10000,
  icon = {20,8,0},
  text = "Rubber tyred vehicle, versitile and cheap," ..
    " but unglamorous with a low capacity.",
})

addGraphFeature({
  code = "GrTracMetal",
  name = "Rail",
  type = "traction",
  maxSpeed = 150,
  noise = 50,
  value = 10,
  maxCars = 12,
  vehicleCost = 20000,
  stopCost = 20000,
  laneCost = 20000,
  icon = {21,8,0},
  text = "Faster and higher capacity than buses," ..
    " but requires expensive infrastructure." ..
    " The time tested choice for the biggest problems.",
})

--[[
addGraphFeature({
  code = "GrTracMonorail",
  name = "Monorail",
  type = "traction",
  maxSpeed = 150,
  noise = 10,
  value = 10,
  maxCars = 12,
  vehicleCost = 10000,
  stopCost = 10000,
  laneCost = 10000,
  icon = {22,8,0},
  text = "Fancy but not pricey - the salesman's choice.",
})

addGraphFeature({
  code = "GrTracMonoHong",
  name = "Hanging Monorail",
  type = "traction",
  maxSpeed = 150,
  noise = 30,
  value = 30,
  maxCars = 12,
  vehicleCost = 10000,
  stopCost = 10000,
  laneCost = 10000,
  icon = {23,8,0},
  text = "Give your city a unique vibe.",
})

addGraphFeature({
  code = "GrTracMagLev",
  name ="Magnetic Levitation",
  type = "traction",
  maxSpeed = 400,
  noise = 10,
  value = 10,
  maxCars = 12,
  vehicleCost = 50000,
  stopCost = 50000,
  laneCost = 50000,
  icon = {25,8,0},
  text = "High speed transit for cities of the future."
})
]]--

addGraphFeature({
  code = "GrPowerDiesel",
  name = "Diesel Engine",
  type = "power",
  maxSpeed = 200,
  noise = 30,
  airPollution = 50,
  icon = {20,10,0},
  text = "Cheap, powerful on-board engine.",
})

--[[
addGraphFeature({
  code = "GrPowerNatGas",
  name = "Natural Gas Engine",
  type = "power",
  maxSpeed = 150,
  noise = 20,
  value = 10,
  airPollution = 25,
  vehicleCost = 25000,
  icon = {21,10,0},
  text = "Reduce pollution and promote environmentalism," ..
    " but pay more for fuel.",
})

addGraphFeature({
  code = "GrPowerOHL",
  name = "Overhead Line",
  type = "power",
  maxSpeed = 400,
  noise = 10,
  airPollution = 0,
  laneCost = 10000,
  icon = {22,10,0},
  text = "Save on fuel costs and eliminate pollution." ..
    " Requires special infrastructure.",
})

addGraphFeature({
  code = "GrPowerCable",
  name = "Cable",
  type = "power",
  maxSpeed = 150,
  noise = 10,
  airPollution = 0,
  laneCost = 5000,
  vehicleCost = -25000,
  icon = {23,10,0},
  text = "An externally powered tow rope. Ideal for hilly terrain." ..
    " Allows building track at any grade." ..
    " Cheapest vehicles, but requires special infrastrucutre.",
})

addGraphFeature({
  code = "GrPowerFlywheel",
  name = "Flywheel",
  type = "power",
  maxSpeed = 100,
  noise = 0,
  airPollution = 0,
  maxLeg = 2000,
  icon = {24,10,0},
  text = "A  flywheel, or gyro, stores energy, acting as a large battery." ..
    " The vehicle must charge at every stop and can only travel a short" ..
    " distance. Stops are more expensive.",
})

addGraphFeature({
  code = "GrPowerInduction",
  name = "Magnetic Induction",
  type = "power",
  maxSpeed = 400,
  noise = 0,
  airPollution = 0,
  laneCost = 25000,
  icon = {25,10,0},
  text = "Magnetic coils in the guideway transmit power to the" ..
    " vehicle, driving it forward. Expensive, but powerful and versitile.",
})
]]--

addGraphFeature({
  code = "GrAutoManual",
  name = "Manual Drive",
  type = "automation",
  maxSpeed = 100,
  icon = {5,5,0},
  text = "No computers, just human drivers.",
})

--[[
addGraphFeature({
  code = "GrAutoPartial",
  name = "Partial Automation",
  type = "automation",
  maxSpeed = 200,
  vehicleCost = 5000,
  laneCost = 5000,
  icon = {12,0,0},
  text = "Increase speed and improve timeliness" ..
    " by delegating some tasks to electronics.",
})

addGraphFeature({
  code = "GrAutoFull",
  name = "Full Automation",
  type = "automation",
  maxSpeed = 400,
  vehicleCost = 20000,
  laneCost = 20000,
  icon = {13,0,0},
  text = "Vehicles that drive themselves. No driver required." ..
    " High speed, perfect timeliness.",
})

addGraphFeature({
  code = "GrAutoPRT",
  name = "Personal Rapid Transit",
  type = "automation",
  maxSpeed = 100,
  vehicleCost = 50000,
  laneCost = 50000,
  icon = {21,11,0},
  text = "Use computers to individually route every traveller" ..
    " directly to their destination. No lines, no schedules, just transit."
})
]]--

addGraphConfiguration({
  code = "GrConfStreet",
  name = "Street",
  class = "GrClassRoad",
  lanes = {
    {features={"GrTracRubber"}, num=1,
      options={"GrTracMetal","GrPowrCatenary"}},
    {features={"GrTracLeather"}, num=1},
  },
  oneWay = false,
  maxSpeed = 10,
  strategy = "GrStratSign",
});

--[[
sidewalk_xs = {0,1/graphRes,0}
shoulder_xs = {0,1/graphRes,0}
pave_xs = {0,0,0}

function renderGraphElement(ndx, tx)
  makeCenterRing({0,0,0}, tx);
  --//if (drawSidewalk) {
    --//makeRing(vec3(0, 0, 0), vec3(10, 0, 0), sidewalk_xs);
    --//makeRing(vec3(10, 0, 0), vec3(10, 0, -10), sholder_xs);
  --//} else {
    --// Sholder
    makeRing({0,0,0}, {10,0,-10}, sholder_xs);
  --//}
end
]]--

