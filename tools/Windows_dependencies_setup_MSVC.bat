::  SPDX-License-Identifier: MIT
::
::  EmulationStation Desktop Edition
::  Windows_dependencies_setup_MSVC.bat
::
::  Downloads and prepares the external dependencies for building in-tree using MSVC.
::  If the directories already exist they will be removed and the libraries will be downloaded again.
::
::  This script needs to run from the root of the repository and 7z.exe and curl.exe need to be
::  reachable via the Path environment variable.
::
@echo off

if not exist .clang-format (
  echo You need to run this script from the root of the repository.
  goto end
)

where /Q 7z.exe
if %ERRORLEVEL% neq 0 (
  echo Can't find 7z.exe and it's required by this script, aborting.
  goto end
)

where /Q curl.exe
if %ERRORLEVEL% neq 0 (
  echo Can't find curl.exe and it's required by this script, aborting.
  goto end
)

echo Setting up dependencies in the .\external directory...
echo:

cd external

echo Setting up cURL

if exist curl-7.80.0-win64-mingw\ (
  rmdir /S /Q curl-7.80.0-win64-mingw
)

curl -O https://curl.se/windows/dl-7.80.0_2/curl-7.80.0_2-win64-mingw.zip
7z x curl-7.80.0_2-win64-mingw.zip
cd curl-7.80.0-win64-mingw\bin

dumpbin /exports libcurl-x64.dll > exports.txt
echo LIBRARY libcurl-x64 > libcurl-x64.def
echo EXPORTS >> libcurl-x64.def
for /f "skip=19 tokens=4" %%A in (exports.txt) do echo %%A >> libcurl-x64.def
lib /def:libcurl-x64.def /out:libcurl-x64.lib /machine:x64

copy /Y libcurl-x64.dll ..\..\..
copy /Y libcurl-x64.lib ..\..\..
cd ..\..

echo:
echo Setting up GLEW

if exist glew-2.1.0\ (
  rmdir /S /Q glew-2.1.0
)

curl -LO https://downloads.sourceforge.net/project/glew/glew/2.1.0/glew-2.1.0-win32.zip
7z x glew-2.1.0-win32.zip
copy /Y glew-2.1.0\bin\Release\x64\glew32.dll ..
copy /Y glew-2.1.0\lib\Release\x64\glew32.lib ..

echo:
echo Setting up FreeType

if exist freetype\ (
  rmdir /S /Q freetype
)

git clone https://github.com/freetype/freetype.git
cd freetype
git checkout VER-2-11-1
mkdir build
cd ..

echo:
echo Setting up FreeImage

if exist FreeImage\ (
  rmdir /S /Q FreeImage
)

curl -LO https://downloads.sourceforge.net/project/freeimage/Binary%%20Distribution/3.18.0/FreeImage3180Win32Win64.zip
7z x FreeImage3180Win32Win64.zip
copy /Y FreeImage\Dist\x64\FreeImage.dll ..
copy /Y FreeImage\Dist\x64\FreeImage.lib ..

echo:
echo Setting up pugixml

if exist pugixml\ (
  rmdir /S /Q pugixml
)

git clone https://github.com/zeux/pugixml.git
cd pugixml
git checkout v1.10
cd ..

echo:
echo Setting up SDL

if exist SDL2-2.0.20\ (
  rmdir /S /Q SDL2-2.0.20
)

curl -O https://www.libsdl.org/release/SDL2-devel-2.0.20-VC.zip
7z x SDL2-devel-2.0.20-VC.zip
cd SDL2-2.0.20
rename include SDL2
cd ..
copy /Y SDL2-2.0.20\lib\x64\SDL2.dll ..
copy /Y SDL2-2.0.20\lib\x64\SDL2.lib ..
copy /Y SDL2-2.0.20\lib\x64\SDL2main.lib ..

echo:
echo Setting up FFmpeg

if exist ffmpeg-n5.0-latest-win64-gpl-shared-5.0\ (
  rmdir /S /Q ffmpeg-n5.0-latest-win64-gpl-shared-5.0
)

curl -LO https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n5.0-latest-win64-gpl-shared-5.0.zip
7z x ffmpeg-n5.0-latest-win64-gpl-shared-5.0.zip
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\bin\avcodec-59.dll ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\bin\avfilter-8.dll ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\bin\avformat-59.dll ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\bin\avutil-57.dll ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\bin\postproc-56.dll ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\bin\swresample-4.dll ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\bin\swscale-6.dll ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\lib\avcodec.lib ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\lib\avfilter.lib ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\lib\avformat.lib ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\lib\avutil.lib ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\lib\swresample.lib ..
copy /Y ffmpeg-n5.0-latest-win64-gpl-shared-5.0\lib\swscale.lib ..

echo:
echo Setting up OpenSSL

if not exist "C:\Program Files\OpenSSL-Win64\libcrypto-1_1-x64.dll" (
  curl -O https://slproweb.com/download/Win64OpenSSL_Light-1_1_1m.exe
  :: Run the installer.
  .\Win64OpenSSL_Light-1_1_1m.exe
)

:: Return to the root of the repository.
cd ..

if exist "C:\Program Files\OpenSSL-Win64\libcrypto-1_1-x64.dll" (
  copy /Y "C:\Program Files\OpenSSL-Win64\libcrypto-1_1-x64.dll"
  copy /Y "C:\Program Files\OpenSSL-Win64\libssl-1_1-x64.dll"
)

echo:
echo Copying DLL files from Windows\System32

copy /Y C:\Windows\System32\MSVCP140.dll
copy /Y C:\Windows\System32\VCOMP140.DLL
copy /Y C:\Windows\System32\VCRUNTIME140.dll
copy /Y C:\Windows\System32\VCRUNTIME140_1.dll

:end
