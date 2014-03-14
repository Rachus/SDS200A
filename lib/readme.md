# The SDS 200A library

This library was intended to offer a driver interface for the Softscope
SDS 200A oscilloscope. Unfortunately it is not near to be ready and we
are not able to work on it any more. But we hope, that our resources
will help someone with this device.

## General Aspects

For the communication with the device the library uses [libusb](http://libusb.org) 1.0.
Every non void function returns a sds_error which is a typedef around
an enum. To return values the functions expect pointers to the variable
to store the values in there. A example file (for the current state of
the project) shows the intended usage.

The simple Makefile supports the following targets (next to all and clean):
* `example`: Build the example
* `doc`: Create a doxygen (html) documentation

## Device Opening

The device obtaining and opening mechanism is currently the only part
of the library that might be suitable for daily work. It mostly is a
thin wrapper around libusb. To open a device, first a list of devices
must be created (sds_get_devices) and one of its elements must be given
to sds_initialize. It initializes a context that must be used for any
further function calls. Do not forget to free allocated resources. After
initialization you can free the list of devices and when the oscilloscope
is not used any more, the context can be destroyed.

## Calibration

Due to a missing knowledge about the device configuration and calibration
this library has the interface (and some implementation) of a calibration
functionality that tries to find the zero volt point. Unfortunately this
requires the user to ground the probes. However this is just a dirty fix
and someone who understands the data format of the device might want to
replace this functionality.

## Configuration

To configure the device (trigger, offset, voltage, etc.) there are getter
and setter. Some of them are even implemented. At initialization time, the
device is set to a known state. Since it is not known if it is possible to
acquire the configuration data from the device, the state is also stored in
software.

For more information about how to configure the device, see
[../readme.md](../readme.md).

## Data Acquiring

This is the most unfinished part of the library. But a small stub already
works. The interface might have to be changed. Currently there is a
function that can be used to acquire raw data (sds_get_raw_data). With
another function (decode_to_raw) you can decode the samples, but note
that the samples might not be channel alternating (according to
configuration). Converting this value to a voltage number is not yet
implemented.

