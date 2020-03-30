# RPG-RPZ
[![Build Status](http://zonme.to2x.ovh:8090/buildStatus/icon?job=rpgrpz%2Fmaster)](http://zonme.to2x.ovh:8090/job/rpgrpz/job/master/)
[![Download](https://api.bintray.com/packages/amphaal/rpgrpz/install-packages/images/download.svg) ](https://dl.bintray.com/amphaal/rpgrpz/RPGRPZ-latest-win64.exe)

Supported (64bits only) :
- Windows : OK
- Linux : OK
- MacOS : WIP

Prerequisites to build:
- For Windows
    - Install MSYS2 : https://www.msys2.org/
    - Create env. variable MINGW64_ROOT (ex : C:/msys64/mingw64)
    - Run : pacman -Syu (From msys2_shell.cmd)
    - Run : pacman -Su (From msys2_shell.cmd)
    - Run : pacman -S --needed - < ./prerequisites/msys2/pkglist_build.txt (From msys2_shell.cmd)
    - Run : pacman -S --needed - < ./prerequisites/msys2/pkglist_dev.txt (From msys2_shell.cmd)
- For ArchLinux
    - Run : pacman -S --needed - < ./prerequisites/arch/pkglist_build.txt
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
