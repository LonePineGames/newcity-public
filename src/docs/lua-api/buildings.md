Lua API - Buildings

Building decorations are configured in data/decos.lua.

See also [[building-textures|Building Textures File Format]].

---

##### addBuildingDecoration(_object_) => void

Takes the configuration _object_ and adds a decoration to the list of available building decorations. _object_ should have properties _code_, _name_, _group_, _mesh_, _simpleMesh_, and optionally _scale_.

_code_ is a unique alphanumeric code for the decoration. If a modpack has an decoration with the same _code_ as a vanilla decoration, the modpack version will overwrite the vanilla version. When adding new decorations, modders should avoid naming conflicts by adding "<your name>::" before the code, as a form of namespacing.

_name_ is the human-friendly name for the decoration.

_group_ is the decoration group that the decoration is in. This determines which menu the decoration is found in when using the Decoration Tool in the building designer. Groups can be added with addBuildingDecorationGroup().

_mesh_ is the path to an .obj file for the decoration. The filename should exclude "modpacks/<your modpack>", as the game knows to look inside the modpack directory. The origin of the mesh (0,0,0) should be at the center of rotation for the decoration, at the point where the decoration touches the ground. The decoration should extend 2 units (meters) below the ground so that it connects to uneven ground.

_simpleMesh_ is the path to an .obj file for the low level-of-detail mesh. This should be a simpler mesh with less triangles, for performance reasons.

_scale_ (optional) can be "x", "y", "z", "xy", "xz", or "yz". It determines which dimension to scale the mesh by when the decoration is scaled. This can be used, for example, for pipes or fences which can scale in dimension.

---

##### addBuildingDecorationGroup(_object_) => void

Takes the configuration _object_ and add a decoration group. _object_ should have properties _code and _name.

_code_ is a unique alphanumeric code for the decoration group. If a modpack has an decoration group with the same _code_ as a vanilla decoration group, the modpack version will overwrite the vanilla version. When adding new decoration groups, modders should avoid naming conflicts by adding "<your name>::" before the code, as a form of namespacing.

_name_ is the human-friendly name for the decoration group.

---

##### getDesign(_designNdx_) => _design_

Returns _design_ which represents the building design at _ndx_. _design_ has the following properties: _ndx_, _numBuildings_, _minYear_, _maxYear_, _minLandValue_, _minDensity_, _lowTide_, _highTide_, _zone_, _numFamilies_, _numRetail_, _numOffices_, _numFarms_, _numFactories_, _numInstitutions_, _name_, _displayName_. _design_ also has _sx_, _sy_, _sz_, which is the size of the design.

---

##### setDesign(_design_) => void

Takes _design_ and updates the building design at _design.ndx_. _design_ should be a copy of the result returned by _getDesign(ndx)_, with some properties updated.

---

##### renderDesign(_designNdx_) => void

Renders the building design at _designNdx_. This will affect all buildings of that design.

---

##### maxDesignNdx() => number

Returns the maximum building design index number. Valid building designs have index numbers between 1 and _maxDesignNdx()_ inclusive.

maxDesignNdx() => $formatInt(maxDesignNdx())$

---

##### getBuilding(_buildingNdx_) => table _building_

Returns _building_ which represents the building at _ndx_. _building_ has the following properties: _ndx_, _design_, _color_, _econ_, _value_, _lastUpdatedAt_, _builtAt_, _zone_, _plan_, _name_. _building also has _x_, _y_, _z_, which is the location of the building in 3D space, and also _nx_, _ny_, _nz_, which is the building's normal vector, a representation of it's orientation or yaw.

---

##### setBuilding(table _building_) => void

Takes _building_ and updates the building at _building.ndx_. _building_ should be a copy of the result returned by _getBuilding(ndx)_, with some properties updated.

---

##### removeBuilding(_buildingNdx_) => void

Removes the building at _buildingNdx_.

---

##### renderBuilding(_buildingNdx_) => void

Renders the building at _buildingNdx_.

---

##### maxBuildingNdx() => number

Returns the maximum building index number. Valid buildings have index numbers between 1 and _maxBuildingNdx()_ inclusive, but some of those buildings have been removed.

maxBuildingNdx() => $formatInt(maxBuildingNdx())$

---

##### numZones() => number

Returns the number of zones.

numZones() => $formatInt(numZones())$

---

##### zoneName(_zoneNdx_) => string

Returns the user-friendly zone name for _zoneNdx_, as a string.

zoneName(MixedUseZone) => $zoneName(MixedUseZone)$

---

##### zoneCode(_zoneNdx_) => string

Returns the zone code name for _zoneNdx_, as a string.

The zone codes are:
* NoZone
* ResidentialZone
* RetailZone
* AgriculturalZone
* GovernmentZone
* OfficeZone
* IndustrialZone
* MixedUseZone
* ParkZone

