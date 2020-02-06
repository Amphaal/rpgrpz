# RPG-RPZ
[![Build status](https://ci.appveyor.com/api/projects/status/jfgl63a0p38h4ru7?svg=true)](https://ci.appveyor.com/project/Amphaal/rpgrpz)
[ ![Download](https://api.bintray.com/packages/amphaal/rpgrpz/rpgrpz-win/images/download.svg) ](https://dl.bintray.com/amphaal/rpgrpz/)

Windows builds with MSYS2
MacOS builds obsoletes right now
Linux builds are WIP

Prerequisites to build:
- latest sentry-native release, unzipped (https://github.com/getsentry/sentry-native/releases)
- see env/msys2_pkglist.txt

Recommanded:
- Visual Studio Code, for builtin debugging helpers (https://code.visualstudio.com/)

Instructions for building :
- "git clone" this repository
- VSCode : Open this project
- VSCode : Ctrl+Maj+P, then "Tasks : Run Test Task"
- VSCode : Ctrl+Maj+D, then run "Launch"