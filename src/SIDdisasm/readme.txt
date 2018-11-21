----------------------------------------
SIDdecompiler by Stein Pedersen/Prosonix 
----------------------------------------

If you're that special kind of nerd that likes to play with other people's
music drivers, then this is the tool for you!

This program takes a SID file and runs it through a 6502 emulator to produce a 
relocatable assembler source file. 

Rob Hubbard tunes will be slightly better documented than others, with 
non-generic label names for data of known purpose.

The output is very simple and should work with most assemblers. I have tested
it with 64tass.

[Known issues]

* The decompiler is very conservative when it comes to code and relocatable 
  address operand generation. Data that looks like code in the disassembly
  has never been executed by the emulator and is left alone. Likewise, data
  that looks like address operands is also ignored if it has never been
	read.
  
* Does not currently handle tunes with timer based sample playback

* A lot of rips in HVSC consist of multiple music driver instances
  in a single file. Often, these are moved around in memory and 
  are not very well handled by the address operand tracer.
  
* SIDs that set up IRQs or never return from init, are currently
  not supported

* Undocumented opcodes are only partially supported
  
[Why???]

This project started because I wanted to produce a Rob Hubbard plugin for the
Prosonix SID tracker. The tracing emulator and address operand resolution
became so effective that it works with a lot of other music drivers as well,
so instead of HubbardDecompiler, it became SIDdecompiler. Hopefully	others
will find it useful too.

[Other platform?]

If there is interest, I will probably release the source code once I have 
cleaned it up a bit.

[Thanks to]

* Geir Tjelta for encouragement and debugging assistance

* Lasse Öörni for "siddump", from which my 6502 emulator is based

Feedback and bugs reports are welcome, either on csdb or stein.pedersen@gmail.com
