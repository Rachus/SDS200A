#!/usr/bin/env python3

import sys
import signal

oldline = None
changed_bits = []

def sighandler(signal, frame):
        global changed_bits
        changed_bits = []
        print(changed_bits, file=sys.stderr);

signal.signal(signal.SIGUSR1, sighandler)

while True:
        line = sys.stdin.readline()
        if not line:
                break
        #if len(line) != 36:
        #        continue
        if oldline == None:
                oldline = line
        for i in range(0, len(line)):
                if len(line) == len(oldline) and (line[i] != oldline[i] or i in changed_bits):
                        if not i in changed_bits:
                                changed_bits.append(i)
                        print('\033[35m' + line[i] + '\033[0m', end='')
                else:
                        print(line[i], end='')
        oldline = line



