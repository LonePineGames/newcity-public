Building Textures File Format

### File Format

All building textures are ordinary PNG image files. A building texture file must be placed in modpacks/<your modpack>/textures/buildings. The dimensions of the image should be 4 times as tall as wide, but there is no set resolution. The image is divided into 4 sections, top to bottom.

The first section is the roof texture. Ancillary objects, such as chimneys and air conditioners, are colored using the top middle pixel of the image. The triangular space under a pitched roof, and the flat part of a flat roof, are also colored using this pixel.

The second section is the upper stories of the building. It represents a section 4 stories tall and just as wide. The section starts from the top, allowing you to create an interesting trim for the top of the building. 

The third section is for the middle of the building. This section is only used for tall buildings. This section starts from the bottom.

The fourth section is for the first 4 stories of the building, as well as a basement. Doorways should be placed 5/8ths of a story, or 5/128ths of the total image, above the bottom of the image. This will align the doorway with walking paths and sidewalks.

There are two types of texture file, albedo, illumination. Albedo textures are ordinary RGB colors and provide the color for the building.

Illumination textures appear at night as appropriate. They are generally all black, or transparent, with a light yellow color over windows. There are generally four levels of illumination, with level 4 being mostly (but not entirely) illuminated, and level 1 having just a few windows lit up. We suggest you blur or texturize the illumination, which will create a better sense of depth and realism.

### Filenames

The filename tells NewCity how to use the building texture. The filename is parsed without it's path or file extension. It is tokenized into attributes divided by underscores. Each attribute is an attribute name, a dash, and then a parameter, or in some cases an attribute name with no parameter. The parameter must not have an underscore or a dash in it.

* Zones: Some of the attributes set a zone. Zones only apply to albedo textures. An albedo texture can apply to multiple zones. Albedo textures that don't have any zone will not be used.

* Design: Use the "design" attribute to assign a texture to a specific building design, based on design filename. The word used must be a substring match. This attribute can be used multiple times.

* Size: Always add "size-4-4-4" to the filename. This attribute isn't used yet, but we might support different sizes/shapes in the future.

* Illum groups: the illum attribute sets the illum group for the texture. Illum groups make it possible to have multiple albedo textures with different colors or materials, but the same window arrangement. Then they can share illumination textures, reducing the number of textures. The key is that they have to have the EXACT same window arrangement for it to work. Make sure that you are not re-using illum groups for textures with different window arrangements.

The illum group can be any alphanumeric string (no dashes or underscores). An albedo with an illum group set is matched with illumination textures with the same illum group. The illumination texture with the most appropriate illumination level is chosen, based on the building's level of activity and time of day.

For illumination texture filenames, only include the illum group, the size attribute, and the illumination attribute: "illum-9pane_size-4-4-4_illumination-4.png"

* Norm groups: the norm attribute sets the norm group for the texture. The norm group can be any alphanumeric string (no dashes or underscores). An albedo with a norm group set is matched with a normal texture with the same norm group AND the same illum group. We assume that if a building has a different arrangement of windows, then it will certainly need a different normal texture too. 

For normal texture filenames, only include the norm group, the illum group, the size attribute, and the normal attribute: "illum-9pane_norm-corrugatedvertical_size-4-4-4_normal.png"

* Other Attributes: Other attributes are provided to filter what type of building and neighborhood the texture is best suited for.

### Attributes ###

* "size" - indicates the size of the texture, using three numbers. The first number is how many segments (currently always 4). The second number is the width of the texture, and the third number is the height in floors. Currently only size-4-4-4 is supported. This attribute is provided for future proofing.
* "residential" - residential buildings will get this texture.
* "retail" - retail buildings will get this texture. 
* "farm" - farm buildings will get this texture. 
* "government" - government buildings, aka amenities, will get this texture.
* "university" - university buildings will get this texture.
* "mixeduse" - mixed use buildings will get this texture. 
* "office" - office buildings will get this texture. 
* "industrial" - industrial buildings will get this texture. 
* "design" - restricts this texture to building designs where there is a substring match with the attribute parameter.
* "minyear" - sets the minimum in-game year for which this texture will apply. The game compares this to the date the building was built.
* "maxyear" - sets the maximum in-game year for which this texture will apply. The game compares this to the date the building was built.
* "minvalue" - sets the minimum land value for which this texture will apply.
* "maxvalue" - sets the maximum land value for which this texture will apply.
* "mindensity" - sets the minimum density for which this texture will apply.
* "maxdensity" - sets the maximum density for which this texture will apply.
* "minfloors" - sets the minimum number of floors tall a building must be for his texture to apply.
* "maxfloors" - sets the minimum number of floors tall a building must be for his texture to apply.
* "illum" - sets the illum group. See documentation on illum groups above.
* "norm" - sets the norm group. See documentation on norm groups above.
* "illumination" - declares this texture an illumination texture. Is generally paired with a parameter setting the light level of the illumination texture. For example, a illumination texture with almost all windows lit up would be labeled "illumination-4".
* "albedo" - declares this texture an albedo texture. It is paired with an appropriate illumination texture and normal texture based on it’s illum group, norm group, and light level.
* "normal" - declares this texture a normal texture. Normal textures are currently unimplemented.

### Quality of Life ###

Set CStartBuildingDesigner = true to start the game in building designer mode. Use the paint and illumination sliders to see building designs under multiple textures, and to see which textures are valid for particular designs.
