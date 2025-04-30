[i have a feeling that there's just about one more good flight left in me - and I hope this trip it is!](https://open.spotify.com/track/5MNsXpG2Ntv2dEslmpeCNu?si=2f5248396ff748f2)


![nota: a foto é en Fisterra, non en Mazaricos. saqueina eu :D](screenshot.png)

***
# MAZARICOS FLIGHT SIMULATOR

opengl 3.3 (+glfw3 +glad) based flying simulator that automatically generates terrain geometry from heightmap files

## to run
included already are a valid makefile, the stb_image.h file and several maps (located in mapas/). you can:
- download/clone the project
- use the makefile to make and run, or compile manually
- choose one of the pre-made maps (just type the folder name - ie. mazaricos, vigo, santiago...)

controls are shown once the sim loads

## to make new maps
you can make a new map from any region - as long as you can get the **heightmap** and several **texture files**.
the app expects a single hmap.png file per map, and a tiles/ folder with the different .png textures that will be assigned to each terrain chunk. **the program splits the heightmap into chunks itself, but expects already split textures**. for proper generation, the number of map chunks should equal the number of texture tiles - adjust TAM_CHUNK in mapa.h to ensure this.

**recommended**: use https://terraining.ateliernonta.com/ [map type: Unity] to get matching texture files and heightmaps of a specific region. you can use the given *split.py* code to split a larger .png texture into the texture tiles! (or if you're more patient, download several .png textures from atelier to get better quality). you can check the already included maps to get an idea of how the program loads them

## how it works
the program creates the terrain geometry - position, normals and texture coords - from a **heightmap image file**: a grayscale image where the 0-255 brightness of the pixel represents the height (y) of that specific (x,z) coordinate

the program loads an entire image file, then creates several mesh structures (chunks) with the loaded geometry. splitting into chunks is important so that it doesn't run at like 2 fps. only the chunks that the camera can see at that specific moment will be drawn
