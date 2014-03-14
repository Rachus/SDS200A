# Tools for reverse engineering SDS 200A

Here we offer our (self-written) tools that we used to understand the device.
All these tools are under GPLv3.

Pcap tools are limited to control and bulk transfers since the device only
uses them.

They were used to track a whole amount of traces (that were created with
USBPcap by sniffing to the communication of the Softscope software). When
piping the output of tabelize to hextobin, greping for a request and then
again piping the result to linediff.py, you can see differences of single
bits highlighted.

## hextobin

This tool reads linewise from stdin and just echos the input. If it reads
a ' it starts converting hexadecimal digits to binary digits until it finds
another '.

## linediff.py

A dirty python3 tool that reads from stdin and marks differences in
following lines. It only works with lines that have the same length.

## pcap2mon

This tool converts a pcap file to a (broken!) mon file. It requires the
pcap file as only argument and prints the mon file to stdout. We used it
to diff usb traces with [vusb-analyzer](http://vusb-analyzer.sourceforge.net/).

## pcap2python

A tool that reads a pcap file specified as first argument and prints python
function calls to stdout. The functions are defined in other files. This
can be used to quickly create python files that can replay recorded traces.

## pcapdump

Dumps a pcap file (first argument) to stdout. Shares the same code base as
pcap2python.

## tabelize

Dumps the control transfers of a pcap file in a ascii table. Takes the
pcap file as first argument. Shares the same code base as pcap2python.

