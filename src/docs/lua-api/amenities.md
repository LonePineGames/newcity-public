NewCity LUA API - Amenities

##### amenitiesBuilt(string _designName_) => number

Returns the number of amenities built where the design file name ends with _designName_.

> amenitiesBuilt("_school") => $amenitiesBuilt("_school")$
> amenitiesBuilt("_convention_center") => $amenitiesBuilt("_convention_center")$
> amenitiesBuilt("_uni") => $amenitiesBuilt("_uni")$

---

##### amenityEnabled(string _designName_) => number

Returns true if there is an amenity type that the player has unlocked, where the design file name ends with _designName_.

> amenityEnabled("_school") => $amenityEnabled("_school")$
> amenityEnabled("_convention_center") => $amenityEnabled("_convention_center")$
> amenityEnabled("_uni") => $amenityEnabled("_uni")$

---

