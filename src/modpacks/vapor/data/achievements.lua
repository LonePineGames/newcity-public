
addAchievement({
  code = "AchRadioactive",
  name = "System War Fallout",
  text = "To deal with the radioactive fallout of the first system war," ..
    " we'll need to build some air scrubbers.",
  condition = "building:c.d1",
  effect = "atmospheric_filter,FObservePollution",
  hint = ""
});

addAchievement({
  code = "AchBigBrother",
  name = "Big Brother Education",
  text = "Thanks to reprogramming technology, it's a lot easier " ..
    " educate (and control) citizens these days.",
  condition = "building:r.d2",
  effect = "reprogramming_tower,FObserveEducation,FObserveCrime",
  hint = ""
});

addAchievement({
  code = "AchBionic",
  name = "Would You Like an Upgrade?",
  text = "Bionic technology is a miracle medical" ..
    " intervention, in addition to being trendy among the youth.",
  condition = "building:r.d3",
  effect = "bionics_lab,FObserveHealth",
  hint = ""
});

