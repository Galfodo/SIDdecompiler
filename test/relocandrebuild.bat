..\bin\x64\SIDdecompiler.exe %* -p
64tass.exe %~n1.asm -o %~n1_r.prg
p2s.exe  %~n1_r.prg %~n1_r.sid
