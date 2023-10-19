
addAchievement({
  code = "AchGraveyard",
  name = "A Grave Matter",
  text = "Reports of the dead coming back to life are spooking the city. A graveyard is needed to make sure what's past is past.",
  condition = function()
    return sum(StatDeaths, OneYear) >= 1000
  end,
  effect = "graveyard"
});

