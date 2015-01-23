@echo off

call "H:\Programs\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
::g++ -o main.exe ../code/main.cpp -lmingw32 -lSDL2main -lSDL2
::cl -DVC=1 -FC -Zi -IH:\Programs\SDL2-2.0.3\include\ ..\code\main.cpp H:\Programs\SDL2-2.0.3\lib\x64\SDL2.lib H:\Programs\SDL2-2.0.3\lib\x64\SDL2main.lib /link /SUBSYSTEM:WINDOWS /NODEFAULTLIB:msvcrt.lib
cl -DVC=1 -FC -Zi^
  /MDd -nologo -Oi -Gm- -GR- -EHa- -WX -W4 -wd4100 -wd4201^
  /Iinclude^
  ../code/main_platform.cpp^
  /link^
  lib/x64/SDL2.lib^
  lib/x64/SDL2main.lib^
  lib/x64/SDL2_mixer.lib^
  /SUBSYSTEM:WINDOWS^
  /NODEFAULTLIB:msvcrt.lib

  ::cl -MT -nologo -Gm- -GR- -EHa- -Oi -WX -W4 -wd4100 -DVC=1 -FC -Zi -Iinclude/ ../code/main.cpp /link lib/x64/SDL2.lib lib/x64/SDL2main.lib lib/x64/SDL2_mixer.lib /SUBSYSTEM:WINDOWS /NODEFAULTLIB:msvcrt.lib
popd
