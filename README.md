# NewCity

[![CC-BY-NC-SA](https://mirrors.creativecommons.org/presskit/buttons/88x31/png/by-nc-sa.png)](https://creativecommons.org/licenses/by-nc-sa/4.0/)

## Building

These instructions are rough and intended as guideposts. I haven't had the time to properly test them.

### Windows

- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/)
- You can also use Visual Studio Code, but you'll still need to install a C++ compiler (MSVC or MinGW or zig maybe?).
- Install the CMake extension.
- Open the project folder in VS Code or Visual Studio.
- Wait a little bit for the CMake extension to configure the project.
- In Visual Studio, click on the CMake menu and then click on Build All.
- In VS Code, look to the bottom left and click on the CMake button. Select RelWithDebugInfo or Release. If asked for architecture, select x64.
- Select `newcity`. Then click on the Build button.
- Probably get build errors, ask Dr ChatGippity what the problem is, you'll figure it out.
- Run `newcity` by clicking on the play button.

### Linux (Ubuntu)

- Install: cmake build-essential gdb libopenal-dev libopenal1 libglu1-mesa-dev (and your graphics card drivers) (probably some other stuff too)
- Using a terminal, do:

        cd src
        ./fix-cmake.sh
        ./fast.sh

