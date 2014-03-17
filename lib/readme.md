# The SDS 200A library

This library was intended to offer a driver interface for the Softscope
SDS 200A oscilloscope. Unfortunately it is far away from being ready to use
and we are not able to work on it any more. But we hope, that our resources
will help someone with this device.

## General Aspects

The library uses [libusb](http://libusb.org) 1.0 for the communication with the device.
An example file (for the current state of the project) shows the intended usage.

The simple Makefile supports the following targets (next to all and clean):
* `example`: Build the example
* `doc`: Create a doxygen (html) documentation

## Device Opening

The device obtaining and opening mechanism is currently the only part
of the library that might be suitable for daily work. It mostly is a
thin wrapper around libusb. To open a device, first a list of devices
must be created (sds_get_devices) and one of its elements must be passed
to sds_initialize. It initializes a context that must be used for any
further function calls. Do not forget to free allocated resources. After
initialization you can free the list of devices. When the oscilloscope
is not used any more, the context can be destroyed.

## Calibration

Due to a lack of knowledge about the device configuration and calibration
this library has the interface (and some implementation) of a calibration
functionality that tries to find the zero volt point. Unfortunately this
requires the user to ground the probes or to connect them with the square
signal of the device. However this is just a dirty fix and someone who
understands the data format of the device might want to replace this
functionality.

## Configuration

For configuring the device (trigger, offset, voltage, etc.) there are getter
and setter. Even some of them are implemented right now. At initialization time, the
device is set to a fixed state. Since we do not know if it is possible to
acquire the configuration data from the device, the state is also stored in
software.

For more information about how to configure the device, have a look at
[../readme.md](../readme.md).

## Data Acquiring

This is the most unfinished part of the library. But a small stub already
works. The interface might have to be changed. Currently there is a
function that can be used to acquire raw data (sds_get_raw_data). With
another function (decode_to_raw) you can decode the samples, but note
that the samples might not be interleaved (according to
configuration). Converting this value to a voltage number is not yet
implemented.

