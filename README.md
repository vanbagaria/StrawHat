# Strawhat Game Engine

## Introduction
[Click here](https://www.youtube.com/watch?v=f3fTrK9X9AU)
to watch the SGE Introduction Video.

## Overview
A C framework for making games based on the cross-platform SDL2 library.
SGE provides a simple state system, logging functions and even a basic GUI framework.
It abstracts away some common SDL functions and structures to make it quicker to get a game running.

Below is an example that creates a simple window.

```C
#include "SGE.h"

int main(int argc, char *argv[])
{
    SGE_Init("SGE Demo Window", 800, 600);
    SGE_Run(NULL);
    return 0;
}
```
**Note:** I have MinGW setup on windows such that I can `#include "SDL.h"`.
Depending on how you set up your environment, you might have to change the includes in SGE to `#include "SDL2/SDL.h"`.

## Running the demo:
* Save the above example code as *SGE_Demo.c* on a folder in your PC.
* Copy the contents of the *include* and *src* folders to be in the same folder as *SGE_Demo.c*
* Link against *SDL2main*, *SDL2*, *SDL2_image*, *SDL2_ttf* and *SDL2_mixer* like below:

```
gcc *.c -o SGE_Demo -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
```

* Copy the *assets* folder to be in the same location as your game executable.
* The dll files should be in the same directory as well, if SDL is linked dynamically.
* Run the demo by opening *SGE_Demo.exe* or using the terminal:

```
./SGE_Demo
```

A blue window should appear, along with a console window with a lot of debug information.
For more examples, checkout the [demos](demos) and read the [manual](SGE_Manual.pdf).

## Dependencies
* [SDL2](https://www.libsdl.org/)
* [SDL_image 2.0](https://www.libsdl.org/projects/SDL_image/)
* [SDL_ttf 2.0](https://www.libsdl.org/projects/SDL_ttf/)
* [SDL_mixer 2.0](https://www.libsdl.org/projects/SDL_mixer/)
