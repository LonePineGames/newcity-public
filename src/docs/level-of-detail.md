-- These constants decide how to hide buildings and their decorations
-- based on camera distance. In other words, Level of Detail.
CBuildingCullMin = $CBuildingCullMin$
CBuildingCullSizeFactor = $CBuildingCullSizeFactor$
CBuildingCullZFactor = $CBuildingCullZFactor$
CBuildingCullSimplify = $CBuildingCullSimplify$
CBuildingCullDeco = $CBuildingCullDeco$
CBuildingCullSimplifyDeco = $CBuildingSimplifyDeco$

The game uses the x size plus the y size of the building's bounding box, in meters, times CBuildingCullSizeFactor, plus the height in meters of the tallest structure times CBuildingCullZFactor, plus CBuildingCullMin, to computer the "cull distance" beyond which the building will be hidden. The game will hide smaller structures at 25% (CBuildingCullSimplify) of the cull distance, and it will hide the decorations at 40% of the cull distance (CBuildingCullDeco). It will use the simple meshes for decorations, and hide any decorations that don't have a simple mesh, at 10% of the cull distance (CBuildingSimplifyDeco).

All those values are then multiplied by the Level of Detail slider, which you can find in the options menu
