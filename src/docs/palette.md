Palette Texture

The palette texture (textures/palette.png) is used to color many things in NewCity, including UI, terrain, skybox, decorations, and more.

![The Palette Texture](textures/palette.png)

#### Light and Skybox Colors

Light and the skybox is colored using the three patches of color in the bottom left. The right patch is the color of light falling on the scene, the middle patch is clouds, and the left patch is the sky. The bottom of each patch is used for sunset and sunrise. The top left of each patch is used for daytime, and the top right of each patch is used for nighttime.

![The Blue Noise Texture](textures/blue-noise.png)

The game uses the blue noise texture to create a pattern of clouds. Even when the cloud cover is zero, there is still some texture. This texture is used to mix the sky and cloud colors.

The skybox is always a gradient. The lower parts of the sky (closer to or below the horizon) use the lower part of the palette color patch, and the higher parts of the sky use the higher part of the palette color patch.

The code for the skybox is shaders/skybox.fragment.shader.

