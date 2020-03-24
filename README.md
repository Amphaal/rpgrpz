# RPG-RPZ
[![Build status](https://ci.appveyor.com/api/projects/status/jfgl63a0p38h4ru7?svg=true)](https://ci.appveyor.com/project/Amphaal/rpgrpz)
[ ![Download](https://api.bintray.com/packages/amphaal/rpgrpz/rpgrpz-win/images/download.svg) ](https://dl.bintray.com/amphaal/rpgrpz/)

Supported (64bits only) :
- Windows : OK
- MacOS : WIP
- Linux : WIP

Prerequisites to build:
- For Windows
    - Install MSYS2 : https://www.msys2.org/
    - Create env. variable MINGW64_ROOT (ex : C:/msys64/mingw64)
    - Run : pacman -Syu (From msys2_shell.cmd)
    - Run : pacman -Su (From msys2_shell.cmd)
    - Run : pacman -S --needed - < ./prerequisites/msys2/pkglist_build.txt (From msys2_shell.cmd)
    - Run : pacman -S --needed - < ./prerequisites/msys2/pkglist_dev.txt (From msys2_shell.cmd)
- For OSX
    - Install latest XCode from the App Store
    - Run : brew install $(cat ./prerequisites/osx/pkglist_build.txt)
    - Run : brew install $(cat ./prerequisites/osx/pkglist_dev.txt)

Recommanded:
- Visual Studio Code, for builtin debugging helpers (https://code.visualstudio.com/)

Instructions for building :
- "git clone" this repository
- VSCode : Open this project
- VSCode : Ctrl+Maj+P, then "Tasks : Run Test Task"
- VSCode : Ctrl+Maj+D, then run "Launch"

Debug tips:
- Debug with GDB on Windows : Activate "_NO_DEBUG_HEAP": 1 as Env variable (https://stackoverflow.com/questions/27380443/why-is-gdb-so-slow-in-windows)
- Debug GSTREAMER : set "GST_DEBUG" : 5 as Env variable (https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html)