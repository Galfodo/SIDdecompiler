..\bin\x64\SIDdecompiler.exe %* -p
@IF ERRORLEVEL 1 goto end
.\64tass.exe %~n1.asm -i -o %~n1.prg
:end
