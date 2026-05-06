# Fast Sand Sim in C

![Local Image](assets/screenshot.png)

This is a Sand Particle Simulator writin in C with the Raylib Library.

## Features

Particle Types:

- Sand
- Water
- Lava
- Stone
- Obsidian
- Ice
- Steam
- Napalm

Other:

- Hammer to destroy any particles
- Change amount of particles placed at once

## About the code

The core function of the code works by using two large arrays, the first is an array called pixelColors, it is screenWidth \* screenHeight in length and holds color data for every pixel on the screen. The second array named pixels is an array of length gridWidth \* gridHeight where gridWidth = screenWidth / drawSize and vice versa, and holds data for every particle such as its type and cordinates (drawSize^2 is the number of physical pixels in a single particle). Using these, the program runs an update loop with definied behavior for each type.

Through my testing it runs extreamly well rendering fluid 960,000 particles (960,000 pixels at drawSize 1) at 70-100 fps (without hjeat detection from lava). Using a bigger monitor I was able to push the program a bit more. It ran 3,250,000 fluid particles or 3,250,000 pixels at draw size 1 at ~ 30 fps (without heat detection). Unfortunatley it struggles when adding in heat detection for water pixels and cooling detection for lava pixels.

## Credits

All Code writin by Cory Pearl for free use to anyone.
