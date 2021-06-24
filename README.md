# Strawhat Game Engine

## Overview
A C framework for making games based on the cross-platform SDL2 library.
It can draw text and images, play music and sound effects, create GUI elements like buttons and sliders, draw spritesheet animations and much more.
Below is an example that creates a simple window.

```C
#include "SGE.h"

int main(int argc, char *argv[])
{
    SGE_Init("SGE Demo Window", 800, 600);
    SGE_Run("");
    return 0;
}
```

## Features
* Text and Image Rendering
* Audio Playback
* Sprite Animation System
* Built-in GUI controls
* Game State Management System
* Debug Logging System

## Building and Running the Example
* Save the above example code as *SGE_Demo.c* on a folder in your PC.
* Create a new folder called *SGE* and copy the *include* and *src* folders inside of it.
* The *SGE* folder and the *assets* folder should be in the same folder as *SGE_Demo.c*
* Compile and link against *SDL2main*, *SDL2*, *SDL2_image*, *SDL2_ttf* and *SDL2_mixer* like below:

```
gcc SGE_Demo.c SGE/src/*.c -o SGE_Demo -ISGE/include -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
```

* Run the demo by opening *SGE_Demo.exe* or using the terminal:

```
./SGE_Demo
```

A blue window should appear, along with a console window with a lot of debug information.
For more examples, checkout the [demos](demos) and read the [manual](https://drive.google.com/file/d/17F2VKthwgbvBpEL6PzgpCmCxaauE8VWZ/view?usp=sharing).

## Dependencies
* [SDL2](https://www.libsdl.org/)
* [SDL_image 2.0](https://www.libsdl.org/projects/SDL_image/)
* [SDL_ttf 2.0](https://www.libsdl.org/projects/SDL_ttf/)
* [SDL_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/)
