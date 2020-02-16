# RPG-RPZ
[![Build status](https://ci.appveyor.com/api/projects/status/jfgl63a0p38h4ru7?svg=true)](https://ci.appveyor.com/project/Amphaal/rpgrpz)
[ ![Download](https://api.bintray.com/packages/amphaal/rpgrpz/rpgrpz-win/images/download.svg) ](https://dl.bintray.com/amphaal/rpgrpz/)

Supported:
- Windows + MSYS2 : WIP
- MacOS : Obsolete
- Linux : WIP

Prerequisites to build:
- For Windows + MSYS2
    - Install MSYS2 : https://www.msys2.org/
    - Create env. variable MSYS2_ROOT (ex : C:/msys64)
    - Run : pacman -Syu (From msys2_shell.cmd)
    - Run : pacman -Su (From msys2_shell.cmd)
    - Run : pacman -S --needed - < ./prerequisites/msys2_pkglist.txt (From msys2_shell.cmd)

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