#!/bin/bash

#eg tools/instruction_to_line.sh 0x8002b0e4

addr2line -e build/junkrunner64_test.elf -f $1