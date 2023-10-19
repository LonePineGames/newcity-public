Lua API - Terrain

Terrain is configured in data/terrain.lua.

##### getMapSize() => number

Returns the size of the map in meters. Maps are always square.

> getMapSize() => $getMapSize()$

##### getTerrainElevation(table _location_) => number

Takes the x,y coordinate _location_ and returns that coordinate's elevation, in meters, above sea level. _location_ must be a table with number fields _x_ and _y_.

> getTerrainElevation({x=100,y=100}) => $getTerrainElevation({x=100,y=100})$

##### addTreeType(_object_) => void

Takes the configuration _object_ and adds a tree type to the list of possible tree types. _object_ should have properties _code_, _weight_, _wind_, _biomes_, _mesh_, and _simpleMesh_.

When the game generates terrain trees, it randomly chooses one of the tree types for each tree. The "trees" don't need to be literal trees but can be any kind of terrain decoration, such as bushes, rocks, or even ruins for a post-apocalyptic mod. The game randomly scales the trees, and can rotate the trees 90, 180, or 270 degrees.

_code_ is a unique alphanumeric code for the tree type. If a modpack has an tree type with the same _code_ as a vanilla tree type, the modpack tree type will overwrite the vanilla version. Modders should avoid naming conflicts by adding "<your name>::" before the code, as a form of namespacing.

_weight_ is a number which applies a bias to how the game randomly choses a tree type for any given tree. The probably of choosing this tree type is _weight_ / _sum_, where _sum_ is the sum of _weight_ for each valid tree type.

_wind_ is true or false, and determines if this tree type should be effected by wind. For example, rocks should have _wind_ = false.

_biomes_ is a comma seperated list of biomes which the tree is valid for. Currently there are only two biomes, "alpine" and "desert".

_mesh_ is the path to an .obj file for the tree mesh. The texture UVs much match texture/palette.png. The filename should exclude "modpacks/<your modpack>", as the game knows to look inside the modpack directory.

_simpleMesh_ is the path to an .obj file for the low level-of-detail mesh. This should be a simpler mesh with less triangles, for performance reasons.

