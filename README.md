# RPG-RPZ
[![Build Status](http://zonme.to2x.ovh:8090/buildStatus/icon?job=rpgrpz%2Fmaster)](http://zonme.to2x.ovh:8090/job/rpgrpz/job/master/)
[![Download](https://api.bintray.com/packages/amphaal/rpgrpz/install-packages/images/download.svg) ](https://dl.bintray.com/amphaal/rpgrpz/RPGRPZ-latest-win64.exe)

```
RPG-RPZ
A simple Pen & Paper experience
Copyright (C) 2019-2020 Guillaume Vara

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Any graphical resources available within the source code may 
use a different license and copyright : please refer to their metadata
for further details. Graphical resources without explicit references to a
different license and copyright still refer to this GNU General Public License.
```

#### The Windows Installer is available on Bintray https://bintray.com/amphaal/rpgrpz/install-packages#files

Supported (64bits only) :
- Windows : OK (CI + CD)
- Archlinux : OK (CI)
- MacOS : OK (Local)

Prepare your build environement:
- For Windows
    - Install MSYS2 : https://www.msys2.org/
    - Create env. variable MINGW64_ROOT (ex : C:/msys64/mingw64)
    - Run : `pacman -Syu` (From msys2_shell.cmd)
    - Run : `pacman -Su` (From msys2_shell.cmd)
    - Run : `pacman -S --needed - < ./prerequisites/msys2/pkglist_build.txt` (From msys2_shell.cmd)
    - Run : `pacman -S --needed - < ./prerequisites/msys2/pkglist_dev.txt` (From msys2_shell.cmd)
- For ArchLinux
    - Run : `pacman -S --needed - < ./prerequisites/arch/pkglist_build.txt`
    - Run : `pacman -S --needed - < ./prerequisites/arch/pkglist_dev.txt`
- For OSX
    - Install latest XCode from the App Store
    - Run : `brew upgrade`
    - Run : `brew install $(cat ./prerequisites/osx/pkglist_build.txt)`
    - Run : `brew install $(cat ./prerequisites/osx/pkglist_dev.txt)`

Recommanded:
- Visual Studio Code, for builtin debugging helpers (https://code.visualstudio.com/)

Building Requirements :
- CMake 3.10.2 or higher (to comply to Ubuntu 18.04 LTS CMake version)

Instructions for building :
- `git clone --recurse-submodules <this repository URL>`
- VSCode : Open this project
- VSCode : Ctrl+Maj+P, then "Tasks : Run Test Task"
- VSCode : Ctrl+Maj+D, then run "Launch"
