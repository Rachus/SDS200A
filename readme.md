# SDS 200A

Here you can find remains of an attempt of trying to reverse engineer the Softscope SDS 200A oscilloscope. This repository consists of a driver which is far from being finished, the tools we used to understand the usb transfers, and a documentation about what we already have or have not found out.

## Driver

An incomplete driver implementation can be found in the lib directory. The source code should be documented very well. Additionally you can find a [small documentation](lib/readme.md) here.

## Documentation

We offer the documentation of what we could figure out. It can be used under the terms of the [GFDL](http://www.gnu.org/copyleft/fdl.html). You can find more information about this device at [other places](resources.md).

We provide the following documents:

* [Configurations](configurations.md): What to send to the device to configure it. You can also find the [typical startup sequence](configurations.md#typical-startup-sequence) there.
* [Data format](dataformat.md): How to interpret the data that the device returns in a bulk request.
* [Reading EEPROM](devicedata.md): The device has an internal EEPROM that can be accessed.

## Tools

The tools directory contains tools we wrote for reverse engineering. A short description for every tool can [be found here](tools/readme.md).

