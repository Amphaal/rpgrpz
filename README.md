# RPG-RPZ
[![Build status](https://ci.appveyor.com/api/projects/status/jfgl63a0p38h4ru7?svg=true)](https://ci.appveyor.com/project/Amphaal/rpgrpz)
[ ![Download](https://api.bintray.com/packages/amphaal/rpgrpz/rpgrpz-win/images/download.svg) ](https://dl.bintray.com/amphaal/rpgrpz/)

Officially for Windows only, but experimental builds on OSX.

Prerequisites to build:
- Qt + OpenSSL + Qt IFW, from official installer (https://www.qt.io/download-thank-you)
- GStreamer runtime + GStreamer development installers (https://gstreamer.freedesktop.org/download/)
- vcpkg, registered in PATH (https://github.com/microsoft/vcpkg)
- cmake, registered in PATH (https://cmake.org/)
- pkgconfiglite, from choco (https://chocolatey.org/)
- Visual Studio 2019 CE / VC++ 2019 / Windows 10 SDK from Visual Studio Installer (https://visualstudio.microsoft.com/fr/thank-you-downloading-visual-studio/?sku=Community&rel=16)
- latest sentry-native release, unzipped (https://github.com/getsentry/sentry-native/releases)
- ENV paths
    - GSTREAMER_1_0_ROOT_X86_64 (ex : D:\gstreamer\1.0\x86_64)
    - QTDIR (ex : C:\Qt\5.13.2\msvc2017_64)
    - QT_TOOLS (ex : C:\Qt\Tools)
    - VCPKG_ROOT (ex : F:\vcpkg)
    - SENTRY_NATIVE_ROOT (ex: F:\sentry-native)

Recommanded:
- Visual Studio Code, for builtin debugging helpers (https://code.visualstudio.com/)
- PowerShell 6 Core (https://github.com/PowerShell/PowerShell)

Instructions for building :
- "git clone" this repository
- VSCode : Open this project
- VSCode : Ctrl+Maj+P, then "Tasks : Run Test Task"
- VSCode : Ctrl+Maj+D, then run "Launch"