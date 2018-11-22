#!/bin/sh
cd ../vs2017
python dumpprojectcontents.py > ../gcc/Makefile.in
cd ../gcc
