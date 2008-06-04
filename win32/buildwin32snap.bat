@echo off
IF EXIST win32\ProfileServ-Setup*.exe del win32\ProfileServ-Setup*.exe
"C:\Program Files\Pantaray\QSetup\Composer.exe" win32\ProfileServ.qsp /Compile /Exit
move win32\ProfileServ-Setup.exe win32\ProfileServ-Setup-3-0-0-%1.exe
