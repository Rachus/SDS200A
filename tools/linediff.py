#!/usr/bin/env python3

# This file is part of the SDS 200A library project.
#
# It is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Libsds200a is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libsds200a. If not, see <http://www.gnu.org/licenses/>.
#
# (c) 2014 Simon Schuster, Sebastian Rachuj
#

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



