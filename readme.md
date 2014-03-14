# SDS 200A

Here you can find remains of an attempt to reverse engineer the Softscope
SDS 200A oscilloscope.

## Documentation

We offer the documentation of what we could understand. It can be used
under the terms of the [GFDL](http://www.gnu.org/copyleft/fdl.html).
[External Documentation](resources.md) is also available.

We provide the following documents:

* [Configurations](configurations.md): What to send to the device to configure it. You can also find the [typical startup sequence](configurations.md#typical-startup-sequence) there.
* [Data format](dataformat.md): How to interpret the data that the device returns in a bulk request.
* [Reading EEPROM](devicedata.md): The device has an internal EEPROM that can be read.

## Library

An unfinished driver implementation can be found in the lib directory. The source code should be very well commented and additionally you can find a [small documentation](lib/readme.md).

## Tools

The tools directory contains tools we wrote for reverse engineering. A short description for every tool can [also be found](tools/readme.md).

