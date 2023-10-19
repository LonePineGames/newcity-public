Lua API - Vehicles

Vehicles are configured in data/decos.lua.

##### addVehicleModel(_object_) => void

Takes the configuration _object_ and adds a vehicle model to the list of possible vehicle models. _object_ should have properties _code_, _mesh_, _simpleMesh_, _texture_, _illumination_, _type_, _passengers_ and _length_.

When the game generates vehicles, it randomly chooses one of the vehicle models for each vehicle. The game also randomly colors the vehicles.

_code_ is a unique alphanumeric code for the vehicle model. If a modpack has an vehicle model with the same _code_ as a vanilla vehicle model, the modpack version will overwrite the vanilla version. When adding new vehicle models, modders should avoid naming conflicts by adding "<your name>::" before the code, as a form of namespacing.

_mesh_ is the path to an .obj file for the vehicle mesh. The filename should exclude "modpacks/<your modpack>", as the game knows to look inside the modpack directory. The origin of the mesh (0,0,0) should be at the center of rotation for the vehicle, at the point where the vehicle touches the road.

_simpleMesh_ is the path to an .obj file for the low level-of-detail mesh. This should be a simpler mesh with less triangles, for performance reasons.

_texture_ is the path to a .png file for texturing the vehicle mesh. Any pixels in the texture that are predominantly blue or green will be replaced with the randomly chosen vehicle color. For mass transit vehicles, any pixels in the texture that are predominantly blue will be replaced with the transit line color, and any pixels that are predominantly green will be replaced with the secondary transit system color for that vehicle's transit system.

_illumination_ is the path to a .png file for illuminating the vehicle at night. The illumination texture is only applied when the vehicle headlights are on (at night). The illumination texture is not randomly colorized.

_type_ determines when the vehicle model is used. It can be one of the following:
* VhTypePrivate - Used for private passenger vehicles.
* VhTypeTruck - Used for freight trucks.
* VhTypeTruckFront - Currently unused. Will be used for semi-trucks.
* VhTypeTruckMiddle - Currently unused. Will be used for semi-trucks.
* VhTypeBus - Used for buses.
* VhTypeTrainFront - Used for the first car in a train.
* VhTypeTrainMiddle - Used for the remaining cars in a train.
* VhTypeWanderer - Used for wanderers, which are vehicles (or moving objects) which do not travel on roads. Their movement is controlled by _update_.

_length_ determines how much space on the road the vehicle takes up. Should be twice the distance between the origin of the mesh (0,0,0) and the rear-most point in the mesh.

_wanderer_ (boolean) specifies if the vehicle model is a wanderer.

_update_ specifies an update (movement) function for the wanderer. It should have the following specification: update(number _ndx_, number _duration_) => void. _ndx_ is the index number of the vehicle to be updated, and _duration_ is the period of time represented by the update (as a fraction of a day). _update_ should use getVehicle(_ndx_) and setVehicle(_object_) to reposition the wanderer. This property only applies to wanderers.

_spawned_ (boolean) specifies if the wanderer is spawned from buildings, or is "natural".

_density_ (number) specifies how many of the wanderer should exist at any time, per spawn point (for spawned wanderers) or per chunk (for unspawned wanderers).

_speed_ (number) specifies how fast the vehicle is. Currently, this only applies to wanderers.

_maxAge_ (number) specifies how long the wanderer should stay alive before being automatically removed by the game. Currently, this only applies to wanderers.

_sea_ (boolean) specifies the wanderer for sea or water. The wanderer will not spawn if the water is iced over, and for unspawned sea wanderers, they will only spawn in water.

_air_ (boolean) specifies the wanderer for air or flying.

_colorized_ (boolean) specifies whether the vehicle texture should be colorized. If true, blue and green in the vehicle texture will be replaced with random colors, or transit line colors.

---

##### getVehicleModel(number _ndx_) => _object_

Returns _object_ which represents the vehicle model at _ndx_. _object_ has the following properties: _ndx_, _type_, _count_, _length_, _density_, _maxAge_, _speed_, _passengers_, and _code_. It also has boolean flags _colorized_, _wanderer_, _spawned_, _sea_, and _air_.

---

##### maxVehicleModelNdx() => number

Returns the maximum vehicle model index number. Valid vehicle models have index numbers between 1 and _maxVehicleModelNdx()_ inclusive.

maxVehicleNdx() => $formatInt(maxVehicleNdx())$

---

##### getVehicle(number _ndx_) => _object_

Returns _object_ which represents the vehicle at _ndx_. _object_ has the following properties: _ndx_, _yaw_, _pitch_, _trailer_, _trailing_, _yieldTo_, _yieldFrom_, _createdAt_, _numPassengers_, _transitLine_, _model_. _object_ also has _x_, _y_, _z_, which is the location of the vehicle in 3D space, and also _vx_, _vy_, _vz_, _ax_, _ay_, which are not used by the game and can be used to store velocity and acceleration data.

---

##### setVehicle(table _object_) => void

Takes _object_ and updates the vehicle at _object.ndx_.  _object_ should be a copy of the result returned by _getVehicle(ndx)_, with some properties updated.

---

##### removeVehicle(number _ndx_) => void

Removes the vehicle at _ndx_ from the game. Any passengers are delivered to their destination. The index number will later be reused by the game for a different vehicle.

---

##### maxVehicleNdx() => number

Returns the maximum vehicle index number. Valid vehicles have index numbers between 1 and _maxVehicleNdx()_ inclusive, but some of those vehicles have been removed.

maxVehicleNdx() => $formatInt(maxVehicleNdx())$

