/* This file is part of the SDS 200A library project.
 *
 * It is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Libsds200a is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libsds200a. If not, see <http://www.gnu.org/licenses/>.
 *
 * (c) 2014 Simon Schuster, Sebastian Rachuj
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb.h>

#include "libsds200a.h"

#define SDS_VENDOR_ID 0x0da8
#define SDS_PRODUCT_ID 0x0001

#define SDS_DEFAULT_TIMEOUT 250
#define SDS_RELAY_WAIT 500000

#define SDS_ENDPOINT_BULK_IN 0x82

/* Control out, Recipient = device */
#define SDS_BM_REQUEST_TYPE_OUT 0x40
/* Control in, Recipient = device */
#define SDS_BM_REQUEST_TYPE_IN  0xc0
#define SDS_REQUEST_RESET 0xd0
#define SDS_REQUEST_RELAY 0xb5
#define SDS_REQUEST_STATE2 0xb3
#define SDS_REQUEST_STATE1 0xb1
#define SDS_REQUEST_OFFSET 0xb2
#define SDS_REQUEST_DATA_AVAILABLE 0xc0

/* Request size of a 0xb1 and 0xb3 request */
#define SDS_STATE_SIZE 21

/* Since the format of 0xb3 was not understood, here are complete words that
 * were sent to change to the specific time scales. There might be a lot of
 * other information encoded in it. */
#define SDS_TIME_2NS   "\x0e\x01\x00\x00" "\x08\x00\x50\x00" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x01"
#define SDS_TIME_4NS   "\x0e\x01\x00\x00" "\x08\x00\x50\x00" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x01" "\x01"
#define SDS_TIME_10NS  "\x0e\x01\x00\x00" "\x08\x00\x51\x00" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x02" "\x01"
#define SDS_TIME_20NS  "\x0e\x01\x00\x00" "\x08\x00\x50\x00" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x03" "\x01"
#define SDS_TIME_40NS  "\x0e\x01\x00\x00" "\x08\x00\x50\x00" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x04" "\x01"
#define SDS_TIME_100NS "\x0e\x01\x00\x00" "\x08\x00\x50\x00" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x05" "\x01"
#define SDS_TIME_200NS "\x0e\x01\x00\x00" "\x08\x00\x50\x00" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x06" "\x01"
#define SDS_TIME_400NS "\x0e\x01\x00\x00" "\x0e\x00\x80\x00" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x07" "\x01"
#define SDS_TIME_1US   "\x0e\x01\x00\x00" "\x21\x00\x18\x01" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x08" "\x01"
#define SDS_TIME_2US   "\x0e\x01\x00\x00" "\x40\x00\x0e\x02" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x09" "\x01"
#define SDS_TIME_4US   "\x0e\x01\x00\x00" "\x7f\x00\x08\x04" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x0a" "\x01"
#define SDS_TIME_10US  "\x0e\x01\x00\x00" "\x3a\x01\xde\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x0b" "\x01"
#define SDS_TIME_20US  "\x1e\x03\x00\x00" "\x3a\x01\xdc\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x0c" "\x01"
#define SDS_TIME_40US  "\x1e\x07\x00\x00" "\x3a\x01\xdb\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x0d" "\x01"
#define SDS_TIME_100US "\x16\x09\x01\x00" "\x3a\x01\xdb\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x0e" "\x01"
#define SDS_TIME_200US "\x16\x09\x03\x00" "\x3a\x01\xdb\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x0f" "\x01"
#define SDS_TIME_400US "\x16\x09\x07\x00" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x10" "\x01"
#define SDS_TIME_1MS   "\x16\x09\x13\x00" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x11" "\x01"
#define SDS_TIME_2MS   "\x16\x09\x27\x00" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x12" "\x01"
#define SDS_TIME_4MS   "\x16\x09\x4f\x00" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x13" "\x01"
#define SDS_TIME_10MS  "\x16\x09\xc7\x00" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x14" "\x01"
#define SDS_TIME_20MS  "\x16\x09\x8f\x01" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x15" "\x01"
#define SDS_TIME_40MS  "\x16\x09\x1f\x03" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x16" "\x01"
#define SDS_TIME_100MS "\x16\x09\xcf\x07" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x17" "\x01"
#define SDS_TIME_200MS "\x16\x09\x9f\x0f" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x18" "\x01"
#define SDS_TIME_400MS "\x16\x13\x40\x1f" "\x3a\x01\xda\x09" "\x00\x00\x01\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x19" "\x01"
#define SDS_TIME_1S    "\x2e\x01\x00\x00" "\x00\x00\x10\x00" "\x00\x00\x05\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x9d" "\x01"
#define SDS_TIME_2S    "\x2e\x01\x00\x00" "\x00\x00\x10\x00" "\x00\x00\x05\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x9e" "\x01"
#define SDS_TIME_4S    "\x2e\x01\x00\x00" "\x00\x00\x10\x00" "\x00\x00\x05\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x9f" "\x01"
#define SDS_TIME_10S   "\x2e\x01\x00\x00" "\x00\x00\x10\x00" "\x00\x00\x05\x00" "\x00\x00\x00\x00" "\x00\x00\x00\xa0" "\x01" 

/* XXX: DEBUG */
#include <stdio.h>

/* This struct contains the state of the driver for one device */
struct sds_context
{
	libusb_context *usb_context;
	libusb_device_handle *device_handle;

	/* Device data: Remember the state of the device in software */
	int channel_active[2]; /* both channels: true -> active, false -> inactive */
	int coupling[2]; /* both channels: true -> coupling active, false -> coupling inactive */
	double offset[2]; /* both channels: voltage offset */
	enum sds_voltage voltage[2]; /* both channels: voltage/div */
	enum sds_time time;
	enum sds_trigger_slope trigger_slope;
	enum sds_trigger_mode trigger_mode;
	enum sds_channel trigger; /* on which channel is triggered */
	char tt_state[SDS_STATE_SIZE]; /* content of last 0xb1/0xb3 request of the device */

	/* Calibration data */
	double zero[2]; /* default offset of 0V (add to user defined offset) */
	double uv_per_tick[2]; /* how many micro volts per tick (TODO) */
};

/* The relay bits for 0xb5 requests. Coupling relays are correct, 10/100
 * relays probably must be switched (?). (XXX) */
enum relay
{
	ch1_10_relay = 1,
	ch1_100_relay = 2,
	ch1_coupling_relay = 0,
	ch2_10_relay = 4,
	ch2_100_relay = 5,
	ch2_coupling_relay = 3,
};

/* Converts libusb-error values to the internal ones */
static sds_error convert_error(int libusbError)
{
	switch (libusbError) {
		case LIBUSB_SUCCESS:
			return SDS_ERROR_SUCCESS;
		case LIBUSB_ERROR_IO:
			return SDS_ERROR_IO;
		case LIBUSB_ERROR_INVALID_PARAM:
			return SDS_ERROR_INVALID_PARAM;
		case LIBUSB_ERROR_ACCESS:
			return SDS_ERROR_ACCESS;
		case LIBUSB_ERROR_NO_DEVICE:
			return SDS_ERROR_NO_DEVICE;
		case LIBUSB_ERROR_NOT_FOUND:
			return SDS_ERROR_NOT_FOUND;
		case LIBUSB_ERROR_BUSY:
			return SDS_ERROR_BUSY;
		case LIBUSB_ERROR_TIMEOUT:
			return SDS_ERROR_TIMEOUT;
		case LIBUSB_ERROR_OVERFLOW:
			return SDS_ERROR_OVERFLOW;
		case LIBUSB_ERROR_PIPE:
			return SDS_ERROR_PIPE;
		case LIBUSB_ERROR_INTERRUPTED:
			return SDS_ERROR_INTERRUPTED;
		case LIBUSB_ERROR_NO_MEM:
			return SDS_ERROR_NO_MEM;
		case LIBUSB_ERROR_NOT_SUPPORTED:
			return SDS_ERROR_NOT_SUPPORTED;
		case LIBUSB_ERROR_OTHER:
			return SDS_ERROR_UNKNOWN;
		default:
			if (libusbError < 0) {
				/* If the value is small than zero, we assume it is an
				 * error we did not know about */
				return SDS_ERROR_UNKNOWN;
			} else {
				/* so, it should not be negative */
				return SDS_ERROR_SUCCESS;
			}
	}
}

/* Simple wrapper for usb control transfers */
static sds_error control_transfer(libusb_device_handle *usbhandle,
				  uint8_t bmRequestType,
				  uint8_t bRequest,
				  uint16_t wValue,
				  uint16_t wIndex,
				  unsigned char *data,
				  uint16_t wLength,
				  unsigned int timeout)
{
	int written;
	sds_error err;

	written = libusb_control_transfer(usbhandle,
					  bmRequestType,
					  bRequest,
					  wValue,
					  wIndex,
					  data,
					  wLength,
					  timeout);

	/* Wrap the errors */
	if (written < 0) {
		err = convert_error(written);
	} else {
		/* TODO: Maybe check for Control-In transfers */
		err = (written != wLength) ? SDS_ERROR_IO
					   : SDS_ERROR_SUCCESS;
	}

	return err;
}

/* Sets a relay and returns 0 on success. */
static sds_error relay_set(sds_context *context, enum relay which, int set)
{
	unsigned char status = 1 << which;
	sds_error err = SDS_ERROR_SUCCESS;
	int written;
	if (!set)
		status = ~status;

	err = control_transfer(context->device_handle,
			       SDS_BM_REQUEST_TYPE_OUT,
			       SDS_REQUEST_RELAY,
			       0,
			       0,
			       &status,
			       sizeof(status),
			       SDS_DEFAULT_TIMEOUT);

	/* The relays seem to require some sleep time to be in the correct
	 * position. A signal might interrupt this call, but since this is
	 * a dirty fix anyways, we just ignore it. */
	usleep(SDS_RELAY_WAIT);

	/* Seems to flush - we just send it, since the original driver also
	 * does it. Don't care if it fails. */
	status = 0;
	control_transfer(context->device_handle,
			 SDS_BM_REQUEST_TYPE_OUT,
			 SDS_REQUEST_RELAY,
			 0,
			 0,
			 &status,
			 sizeof(status),
			 SDS_DEFAULT_TIMEOUT);
	return err;
}

/* Sets two voltage relays to requested states */
static sds_error set_voltage_relays(sds_context *context,
				    int relay1_state,
				    enum relay relay1,
				    int relay2_state,
				    enum relay relay2)
{
	sds_error err = SDS_ERROR_SUCCESS;
	if ((err = relay_set(context, relay1, relay1_state)))
		return err;
	err = relay_set(context, relay2, relay2_state);
	return err;
}

/* Sends the context->tt_state as 0xb1 and 0xb3 requests. If they are not
 * completely equivalent, this function is obsolete (XXX) */
static sds_error send_state_word(sds_context *context)
{
	sds_error err;
	err = control_transfer(context->device_handle,
			       SDS_BM_REQUEST_TYPE_OUT,
			       SDS_REQUEST_STATE1,
			       0,
			       0,
			       (unsigned char *) context->tt_state,
			       sizeof(context->tt_state),
			       SDS_DEFAULT_TIMEOUT);

	if (err)
		return err;

	err = control_transfer(context->device_handle,
			       SDS_BM_REQUEST_TYPE_OUT,
			       SDS_REQUEST_STATE2,
			       0,
			       0,
			       (unsigned char *) context->tt_state,
			       sizeof(context->tt_state),
			       SDS_DEFAULT_TIMEOUT);
	return err;
}

/* Converts a time to the command string (see #define at the beginning of this
 * file). */
char *get_time_command(enum sds_time time)
{
	switch(time) {
		case SDS_2ns:
			return SDS_TIME_2NS;
		case SDS_4ns:
			return SDS_TIME_4NS;
		case SDS_10ns:
			return SDS_TIME_10NS;
		case SDS_20ns:
			return SDS_TIME_20NS;
		case SDS_40ns:
			return SDS_TIME_40NS;
		case SDS_100ns:
			return SDS_TIME_100NS;
		case SDS_200ns:
			return SDS_TIME_200NS;
		case SDS_400ns:
			return SDS_TIME_400NS;
		case SDS_1us:
			return SDS_TIME_1US;
		case SDS_2us:
			return SDS_TIME_2US;
		case SDS_4us:
			return SDS_TIME_4US;
		case SDS_10us:
			return SDS_TIME_10US;
		case SDS_20us:
			return SDS_TIME_20US;
		case SDS_40us:
			return SDS_TIME_40US;
		case SDS_100us:
			return SDS_TIME_100US;
		case SDS_200us:
			return SDS_TIME_200US;
		case SDS_400us:
			return SDS_TIME_400US;
		case SDS_1ms:
			return SDS_TIME_1MS;
		case SDS_2ms:
			return SDS_TIME_2MS;
		case SDS_4ms:
			return SDS_TIME_4MS;
		case SDS_10ms:
			return SDS_TIME_10MS;
		case SDS_20ms:
			return SDS_TIME_20MS;
		case SDS_40ms:
			return SDS_TIME_40MS;
		case SDS_100ms:
			return SDS_TIME_100MS;
		case SDS_200ms:
			return SDS_TIME_200MS;
		case SDS_400ms:
			return SDS_TIME_400MS;
		case SDS_1s:
			return SDS_TIME_1S;
		case SDS_2s:
			return SDS_TIME_2S;
		case SDS_4s:
			return SDS_TIME_4S;
		case SDS_10s:
			return SDS_TIME_10S;
	}
}

/* Bitwise xors the status word. Requires the state to be 21 chars. */
static void xor_on_state(sds_context *context, char *state)
{
	int i = 0;
	for (; i < SDS_STATE_SIZE; ++i)
		context->tt_state[i] ^= state[i];
}

/* Changes the time state to the new time */
static sds_error change_time(sds_context *context, enum sds_time time)
{
	char *ntime = get_time_command(time);
	char *otime = get_time_command(context->time);
	xor_on_state(context, otime);
	xor_on_state(context, ntime);
	memcpy(context->tt_state, ntime, SDS_STATE_SIZE);
	return send_state_word(context);
}

/* Unused function that sets all relays and then unsets all relays
 * (sounds nice) ;) */
static void fun_with_relays(sds_context *context)
{
	int i;
	while (1) {
		for (i = 0; i < 6; ++i)
			relay_set(context, i, 1);
		for (i = 0; i < 6; ++i)
			relay_set(context, i, 0);
	}
}
/* Initializes the device to a known state and sets all device members of the
 * given context. Returns 0 on success. */
static sds_error initialize_device(sds_context *context)
{
	sds_error err = SDS_ERROR_SUCCESS;

	context->channel_active[0] = 0;
	context->channel_active[1] = 0;
	context->coupling[0] = 0;
	context->coupling[1] = 0;
	context->offset[0] = 0.0;
	context->offset[1] = 0.0;
	context->voltage[0] = SDS_10mV;
	context->voltage[1] = SDS_10mV;
	context->time = 0;
	context->trigger_slope = 0;
	context->trigger_mode = 0;

	if ((err = control_transfer(context->device_handle,
				    SDS_BM_REQUEST_TYPE_OUT,
				    SDS_REQUEST_RESET,
				    0,
				    0,
				    NULL,
				    0,
				    SDS_DEFAULT_TIMEOUT)))
		return err;
	if ((err = relay_set(context, ch1_10_relay, 0)))
		return err;
	if ((err = relay_set(context, ch1_100_relay, 0)))
		return err;
	if ((err = relay_set(context, ch1_coupling_relay, 0)))
		return err;
	if ((err = relay_set(context, ch2_10_relay, 0)))
		return err;
	if ((err = relay_set(context, ch2_100_relay, 0)))
		return err;
	if ((err = relay_set(context, ch2_coupling_relay, 0)))
		return err;
	/* Probably: Read config data here! */
	if ((err = control_transfer(context->device_handle,
				    SDS_BM_REQUEST_TYPE_OUT,
				    SDS_REQUEST_RESET,
				    0,
				    0,
				    NULL,
				    0,
				    SDS_DEFAULT_TIMEOUT)))
		return err;

	if ((err = sds_set_offset(context, SDS_CH1, 0.0)))
		return err;
	if ((err = sds_set_offset(context, SDS_CH2, 0.0)))
		return err;
	if ((err = sds_set_time(context, SDS_2us)))
		return err;
	if ((err = sds_set_trigger_source(context, SDS_CH1)))
		return err;
	if ((err = sds_set_trigger_mode(context, SDS_NORMAL)))
		return err;
	err = sds_set_trigger_slope(context, SDS_RISING);
	/* TODO: Might be incomplete */
	return err;
}

/* Returns 0 if the dev is a SDS200A or 1 if it is not. */
static int probe_usb_device(libusb_device *dev)
{
	struct libusb_device_descriptor desc;
	if (libusb_get_device_descriptor(dev, &desc) < 0)
		return 1;
	if (desc.idVendor == SDS_VENDOR_ID &&
	    desc.idProduct == SDS_PRODUCT_ID)
		return 0;
	return 1;
}

sds_error sds_initialize(struct sds_device *device, sds_context **context)
{
	sds_error err = SDS_ERROR_SUCCESS;
	if (!device || !context)
		return SDS_ERROR_INVALID_PARAM;
	*context = calloc(1, sizeof(*context));
	if (!*context)
		return SDS_ERROR_NO_MEM;
	if ((err = convert_error(libusb_init(&(*context)->usb_context))))
		goto context_remove;
	if ((err = convert_error(libusb_open((libusb_device *) device->device_ptr,
				 &(*context)->device_handle))))
		goto libusb_deinit;
	if ((err = initialize_device(*context)))
		goto libusb_deinit;
	return err;

libusb_deinit:
	libusb_exit((*context)->usb_context);

context_remove:
	free(*context);
	return err;
}

/* Frees a sds_device struct. length contains the array length. */
static void free_device_array(struct sds_device *devices, unsigned int length)
{
	unsigned int i = 0;
	for (; i < length; ++i)
		free(devices[i].device_ptr);
	free(devices);
}

sds_error sds_get_devices(struct sds_device_list **ulist)
{
	libusb_device **list;
	ssize_t usb_count;
	libusb_context *local_context;
	unsigned int dev_count = 0;
	struct sds_device *devices = NULL;
	ssize_t i = 0;
	struct sds_device_list *device_list = NULL;
	sds_error err = SDS_ERROR_SUCCESS;

	if (!ulist)
		return SDS_ERROR_INVALID_PARAM;
	if ((err = convert_error(libusb_init(&local_context))))
		return err;
	usb_count = libusb_get_device_list(NULL, &list);

	if (usb_count < 0) {
		libusb_exit(local_context);
		return SDS_ERROR_NO_DEVICE;
	}
	/* Iterate all USB devices that libusb can handle, find the SDS 200A
	 * devices and add them to the devices struct (that will be returned
	 * to the user). */
	for (i = 0; i < usb_count; i++) {
		if (!probe_usb_device(list[i])) {
			struct sds_device *more_devices;
			struct sds_device *current;
			unsigned int new_size = dev_count + 1;
			more_devices = realloc(devices,
					       new_size * sizeof(*devices));
			if (!more_devices) {
				err = SDS_ERROR_NO_MEM;
				goto error_handling;
			}
			devices = more_devices;
			current = &devices[dev_count];
			current->bus_no = libusb_get_bus_number(list[i]);
			current->port_no = libusb_get_port_number(list[i]);
			current->device_ptr = list[i];
			dev_count = new_size;
		}
	}

	/* Assures, that there is at least one element in the array when returned,
	 * else we suppose an error occured. */
	if (dev_count) {
		device_list = malloc(sizeof(*device_list));
		if (!device_list) {
			err = SDS_ERROR_NO_MEM;
			goto error_handling;
		}
		device_list->size = dev_count;
		device_list->array = devices;
		device_list->devices_ptr = list;
		goto function_exit;
	}
	err = SDS_ERROR_NO_DEVICE;

error_handling:
	free(devices);
	libusb_free_device_list(list, 1);

function_exit:
	libusb_exit(local_context);
	*ulist = device_list;
	return err;
}

void sds_free_devices(struct sds_device_list *device_list)
{
	if (!device_list)
		return;
	libusb_free_device_list(
		(libusb_device **) device_list->devices_ptr,
		1);
	free(device_list->array);
	free(device_list);
}

void sds_destroy(sds_context *c)
{
	if (!c)
		return;
	libusb_close(c->device_handle);
	libusb_exit(c->usb_context);
	free(c);
}

sds_error sds_set_channel(sds_context *context, enum sds_channel channel, int on)
{
	if (!context)
		return SDS_ERROR_INVALID_PARAM;

	/* We currently do not know if the hardware supports channel activation
	 * and deactivation. Therefore we just remember the user's choice.
	 * XXX: Adjust? */
	switch(channel) {
		case SDS_CH1:
			context->channel_active[0] = on;
			break;
		case SDS_CH2:
			context->channel_active[1] = on;
			break;
	}
	return SDS_ERROR_SUCCESS;
}

sds_error sds_get_channel(sds_context *context, enum sds_channel channel, int *on)
{
	if (!context || !on)
		return SDS_ERROR_INVALID_PARAM;

	switch(channel) {
		case SDS_CH1:
			*on = context->channel_active[0];
			break;
		case SDS_CH2:
			*on = context->channel_active[1];
			break;
	}
	return SDS_ERROR_SUCCESS;
}

sds_error sds_set_voltage(sds_context *context, enum sds_channel channel, enum sds_voltage voltage)
{
	sds_error err = SDS_ERROR_SUCCESS;
	if (!context)
		return SDS_ERROR_INVALID_PARAM;

	/* We currently do not know if it is possible to adjust the voltage scale
	 * other than setting the relays. Therefore we just set them.
	 * TODO: confirm?
	 * 10mV - 100mV: No relay
	 * 200mV - 1V: Small relay
	 * 2V - 10V: Great relay
	 */
	switch(channel) {
		case SDS_CH1:
			if (!(err = set_voltage_relays(context,
						       (voltage <= SDS_100mV) ? 0 : 1,
						       ch1_10_relay,
						       (voltage <= SDS_1V) ? 0 : 1,
						       ch1_100_relay)))
				context->voltage[0] = voltage;
			break;
		case SDS_CH2:
			if (!(err = set_voltage_relays(context,
						       (voltage <= SDS_100mV) ? 0 : 1,
						       ch2_10_relay,
						       (voltage <= SDS_1V) ? 0 : 1,
						       ch2_100_relay)))
				context->voltage[1] = voltage;
			break;
	}
	return err;
}

sds_error sds_get_voltage(sds_context *context, enum sds_channel channel, enum sds_voltage *voltage)
{
	if (!context || !voltage)
		return SDS_ERROR_INVALID_PARAM;

	switch(channel) {
		case SDS_CH1:
			*voltage = context->voltage[0];
			break;
		case SDS_CH2:
			*voltage = context->voltage[1];
			break;
	}
	return SDS_ERROR_SUCCESS;
}

sds_error sds_set_coupling(sds_context *context, enum sds_channel channel, int on)
{
	sds_error err = SDS_ERROR_SUCCESS;
	if (!context)
		return SDS_ERROR_INVALID_PARAM;

	switch(channel) {
		case SDS_CH1:
			if ((err = relay_set(context, ch1_coupling_relay, on)))
				return err;
			context->coupling[0] = on;
			break;
		case SDS_CH2:
			if ((err = relay_set(context, ch2_coupling_relay, on)))
				return err;
			context->coupling[1] = on;
			break;
	}
	return err;
}

sds_error sds_get_coupling(sds_context *context, enum sds_channel channel, int *on)
{
	if (!context || !on)
		return SDS_ERROR_INVALID_PARAM;

	switch(channel) {
		case SDS_CH1:
			*on = context->coupling[0];
			break;
		case SDS_CH2:
			break;
			*on = context->coupling[0];
	}
	return SDS_ERROR_SUCCESS;
}

sds_error sds_set_time(sds_context *context, enum sds_time time)
{
	if (!context)
		return SDS_ERROR_INVALID_PARAM;
	return change_time(context, time);
}

sds_error sds_get_time(sds_context *context, enum sds_time *time)
{
	if (!context || !time)
		return SDS_ERROR_INVALID_PARAM;
	*time = context->time;
	return SDS_ERROR_SUCCESS;
}

sds_error sds_set_offset(sds_context *context, enum sds_channel channel, double offset)
{
	unsigned char data[3];
	sds_error error;

	/* This calculation might be horribly broken, but for now it seems to work. */
	/* TODO */
	unsigned int calc = ((1 << 11) / 2) + (unsigned int) (offset * ((1 << 11) / 2));
	unsigned char *cdata = (unsigned char *) &calc;

	if (!context)
		return SDS_ERROR_INVALID_PARAM;

	switch (channel) {
		/* TODO CH1 vs CH2 */
		case SDS_CH1:
			data[2] = 1;
		case SDS_CH2:
			data[2] = 0;
	}

	data[0] = cdata[0];
	data[1] = cdata[1] & 0x0f;

	/* XXX: Correct format? */
	error = control_transfer(context->device_handle,
				 SDS_BM_REQUEST_TYPE_OUT,
				 SDS_REQUEST_OFFSET,
				 0x0,
				 0x0,
				 data,
				 sizeof(data),
				 SDS_DEFAULT_TIMEOUT);

	return error;
}

sds_error sds_get_offset(sds_context *context, enum sds_channel channel, double *offset)
{
	if (!context || !offset)
		return SDS_ERROR_INVALID_PARAM;

	switch(channel) {
		case SDS_CH1:
			*offset = context->offset[0];
			break;
		case SDS_CH2:
			*offset = context->offset[1];
			break;
	}
	return SDS_ERROR_SUCCESS;
}

sds_error sds_set_trigger_source(sds_context *context, enum sds_channel channel)
{
	sds_error err;

	if (!context || !channel)
		return SDS_ERROR_INVALID_PARAM;

	/* in the 16th byte, the second bit from the right endcodes the channel */
	switch (channel) {
		/* TODO: Check CH1 vs CH2 */
		case SDS_CH1:
			/* Mask: 11111101 = 0xfd */
			/* == 0 : ch1 */
			context->tt_state[15] &= 0xfd;
			break;
		case SDS_CH2:
			/* Mask: 00000010 = 0x2 */
			/* == 1 : ch2 */
			context->tt_state[15] |= 0x2;
			break;
	}

	err = send_state_word(context);

	if (err != SDS_ERROR_SUCCESS) {
		return err;
	}

	context->trigger = channel;
	return SDS_ERROR_SUCCESS;
}

sds_error sds_get_trigger_source(sds_context *context, enum sds_channel *channel)
{
	if (!context || !channel)
		return SDS_ERROR_INVALID_PARAM;
	*channel = context->trigger;
	return SDS_ERROR_SUCCESS;
}

sds_error sds_set_trigger_slope(sds_context *context, enum sds_trigger_slope slope)
{
	sds_error err;

	if (!context)
		return SDS_ERROR_INVALID_PARAM;

	/* in the 16th byte, the first bit from the right endcode the slope:
	 * Mask: 00000001 = 0x1 */
	switch (slope) {
		/* TODO: Check CH1 vs CH2 */
		case SDS_RISING:
			/* at the 16th byte, the first bit from the right should be 0
			 * Mask: 11111110 = 0xfe */
			context->tt_state[15] &= 0xfe;
			break;
		case SDS_FALLING:
			/* at the 16th byte, the first bit from the right should be 1
			 * Mask: 00000001 = 0x01 */
			context->tt_state[15] |= 0x01;
			break;
	}

	err = send_state_word(context);

	if (err != SDS_ERROR_SUCCESS) {
		return err;
	}

	context->trigger_slope = slope;
	return SDS_ERROR_SUCCESS;
}

sds_error sds_get_trigger_slope(sds_context *context, enum sds_trigger_slope *slope)
{
	if (!context || !slope)
		return SDS_ERROR_INVALID_PARAM;
	*slope = context->trigger_slope;
	return SDS_ERROR_SUCCESS;
}

sds_error sds_set_trigger_mode(sds_context *context, enum sds_trigger_mode mode)
{
	sds_error err;

	if (!context)
		return SDS_ERROR_INVALID_PARAM;

	switch (mode) {
		/* TODO: Check CH1 vs CH2 */
		case SDS_NORMAL:
			/* at the 20th byte, the most significant bit should be 1
			 * Mask: 10000000 = 0x8 */
			context->tt_state[19] |= 0x8;
			break;
		case SDS_AUTOMATIC:
			/* at the 20th byte, the most significant bit should be 0
			 * Mask: 01111111 = 0x7f */
			context->tt_state[19] &= 0x7f;
			break;
	}

	err = send_state_word(context);

	if (err != SDS_ERROR_SUCCESS) {
		return err;
	}

	context->trigger_mode = mode;
	return SDS_ERROR_SUCCESS;
}

sds_error sds_get_trigger_mode(sds_context *context, enum sds_trigger_mode *mode)
{
	if (!context || !mode)
		return SDS_ERROR_INVALID_PARAM;
	*mode = context->trigger_mode;
	return SDS_ERROR_SUCCESS;
}

sds_error sds_set_trigger_offset(sds_context *context, enum sds_channel channel, double offset)
{
	if (!context)
		return SDS_ERROR_INVALID_PARAM;
	/* TODO */
	return SDS_ERROR_UNKNOWN;
}

sds_error sds_get_trigger_offset(sds_context *context, enum sds_channel channel, double *offset)
{
	if (!context || !offset)
		return SDS_ERROR_INVALID_PARAM;
	/* TODO */
	return SDS_ERROR_UNKNOWN;
}

sds_error sds_calibrate_offset(sds_context *context, unsigned int *zero1, unsigned int *zero2)
{
	struct sds_samples *samples;
	size_t written;
	double offset[2];
	int passed_zero_val; /* 1 when we passed 511 */
	double current_offset;
	double delta = 0.001;
	enum sds_channel current_channel = SDS_CH1;
	int first;
	int i;
	uint16_t old_decoded;
	sds_error err = SDS_ERROR_SUCCESS;

	/* What was thought about this functionality:
	 * Since the calibration format of the device was not understood, we
	 * decided that this library might just check itself where the 0 volt
	 * position can be found. This should be stored somewhere that it can
	 * be restored without doing everything here again.
	 *
	 * We expect that this function is run, when the probes are grounded.
	 *
	 * For now it works like this:
	 * For each channel we start with the smallest offset (our library will
	 * handle) and step up, until 511 is reached (this is 1023/2, 1023 is
	 * the number of different values the device can deliver).
	 *
	 * TODO: Note that until now no other function notices this value! */
	
	initialize_device(context);

	if (!context)
		return SDS_ERROR_INVALID_PARAM;
	if (zero1 && *zero1 < 1.0 && *zero2 > -1.0)
		context->zero[0] = *zero1;
	if (zero2 && *zero2 < 1.0 && *zero2 > -1.0)
		context->zero[1] = *zero2;

	/* TODO: Configure test settings */
	/* Save old offset */
	if ((err = sds_get_offset(context, SDS_CH1, &offset[0])))
		return err;
	if ((err = sds_get_offset(context, SDS_CH2, &offset[1])))
		return err;
	
	for (i = 0; i < 2; ++i) {
		first = 1;
		current_offset = -1.0;
		passed_zero_val = 0;
		/* Try as long as not accurate */
		while (!passed_zero_val) {
			int work_further = 1;
			uint16_t decoded;
			int cur_ch = (current_channel == SDS_CH1) ? 0 : 1;
			/* Avoid an endless loop */
			if (current_offset > 1.0)
				return SDS_ERROR_UNKNOWN;
			/* Data acquiring */
			if ((err = sds_set_offset(context, current_channel, current_offset)))
				return err;
			if ((err = sds_get_raw_data(context, &samples, &written)) || written == 0)
				return err;
			/* TODO: How to decide, which channel? */
			if ((err = sds_decode_to_raw(context, samples->samples[cur_ch], &decoded))) {
				free(samples);
				continue;
			}
			if (first) {
				first = 0;
			} else {
				if (decoded >= 511) {
					passed_zero_val = 1;
				}
			}
			old_decoded = decoded;
			/* TODO */
			free(samples);
			current_offset += delta;
		}
		if (current_channel == SDS_CH1 && !zero1)
			context->zero[0] = current_offset;
		if (current_channel == SDS_CH2 && !zero2)
			context->zero[1] = current_offset;
		current_channel = SDS_CH2;
	}

	/* Restore the old offset */
	sds_set_offset(context, SDS_CH1, offset[0]);
	sds_set_offset(context, SDS_CH2, offset[1]);
	return err;
}

sds_error sds_calibrate_scale(sds_context *context, unsigned int *uv_per_tick1, unsigned int *uv_per_tick2)
{
	/* TODO */
	/* This function was supposed to do similiar things like
	 * sds_calibrate_offset. Therefore the probes should be connected
	 * to the square signal of the device. We could obtain the micro volts
	 * per changed value by measuring it (since the signal has an amplitude
	 * of 3V). */
	return SDS_ERROR_UNKNOWN;
}

sds_error sds_get_calibration(sds_context *context, struct sds_calibration *calibration)
{
	if (!context || !calibration)
		return SDS_ERROR_INVALID_PARAM;
	calibration->zero1 = context->zero[0];
	printf("%f\n", calibration->zero1);
	calibration->zero2 = context->zero[1];
	calibration->uv_per_tick1 = context->uv_per_tick[0];
	calibration->uv_per_tick2 = context->uv_per_tick[1];
	return SDS_ERROR_SUCCESS;
}

/* TODO: Type-signature */
static unsigned int decode_data(unsigned char bytelow, unsigned char bytehigh)
{
	/* Structure of samples returned from the device (10 bit)
	 * 	      Low      High
	 *	      00XXXXXX 0000YYYY
	 *	Mask: 0x3f     0xf
	 *
	 * 		-> YYYYXXXXXX, 10bit unsigned
	 *
	 * TODO: other bits maybe encode the following:
	 * 		- Channel
	 * 		- Sample present
	 */
	return ((bytehigh & 0xf) << 6) | (bytelow & 0x3f);
}

static sds_error data_available(struct sds_context *context, unsigned char *data)
{
	if(data == NULL){
		return SDS_ERROR_INVALID_PARAM;
	}

	/* if data != NULL we may safely assume it is at least 1 byte large */

	/* This request returns an number > 0 in data if the next bulk read will
	 * not block (as data is available)
	 */
	return control_transfer(context->device_handle,
			        SDS_BM_REQUEST_TYPE_IN,
			        SDS_REQUEST_DATA_AVAILABLE,
			        0,
			        0,
			        data,
			        1,
			        SDS_DEFAULT_TIMEOUT);
}

static sds_error read_data(struct sds_context *context, unsigned char *data, unsigned int *length)
{
	unsigned char dataavail;
	sds_error err = SDS_ERROR_SUCCESS;
	int libusb_error;
	int transferred; /* TODO: autoadjust */

	err = data_available(context, &dataavail);
	if (err)
		return err;

	/* printf(" %d", dataavail); */
	if (dataavail) {
		libusb_error = libusb_bulk_transfer(context->device_handle,
						    SDS_ENDPOINT_BULK_IN,
						    data,
						    *length,
						    &transferred,
						    SDS_DEFAULT_TIMEOUT);

		if (libusb_error) {
			printf("USB error: %s, %d\n", libusb_strerror(libusb_error), dataavail);
			return convert_error(libusb_error);
		}

		*length = transferred;
		/* TODO: Parse values */
		/* Debugging:
		printf("\r %04u %04u",  decode_data(data[8], data[9]), decode_data(data[10], data[11]));
		fflush(stdout);
		*/
	} else {
		*length = 0;
	}
	return err;
}

sds_error sds_get_data(sds_context *context, char *data, size_t length, size_t *written)
{
	/* TODO */
	unsigned char dat[65536];
	unsigned int size = sizeof(dat);
	/* TODO - currently just for debugging: */
	while (1) {
		read_data(context, dat, &size);
	}
}

sds_error sds_get_raw_data(sds_context *context, struct sds_samples **data, size_t *written)
{
	unsigned int size;
	sds_error err;

	/* Select the appropriate size for the current time/div setting */
	switch(context->time)
	{
		default:
			size = 8192;
	}

	/* Allocate memory to store the buffers */
	*data = malloc(size);
	if (*data == NULL) {
		*written = 0;
		return SDS_ERROR_NO_MEM;
	}

	/* Read the data */
	if ((err = read_data(context, (unsigned char *) *data, &size)) || size == 0) {
		goto get_raw_free_buffer;
	}

	/* Do not report negative sizes */
	if (size < sizeof((*data)->unknown_padding)) {
		err = SDS_ERROR_IO;
		goto get_raw_free_buffer;
	}
	else
		*written = size - sizeof((*data)->unknown_padding);
	/* The actual size of the samples is 2 bytes */
	*written /= sizeof((*data)->samples[0]);

	return SDS_ERROR_SUCCESS;

get_raw_free_buffer:
	/* Error */
	*written = 0;
	free(*data);
	*data = NULL;
	return err;
}

sds_error sds_decode_to_raw(sds_context *context, uint16_t sample, uint16_t *advalue) {
	if (context == NULL || advalue == NULL) {
		return SDS_ERROR_INVALID_PARAM;
	}

	/* Split into bytes */
	uint8_t *bytes = (uint8_t *) &sample;

	/* TODO: big endian vs. little endian */
	/* TODO: Calibration-data */
	*advalue = decode_data(bytes[0] & 0xff, bytes[1] & 0xff);

	return SDS_ERROR_SUCCESS;
}

sds_error sds_decode_to_volt(sds_context *context, uint16_t sample, double *voltage)
{
	uint16_t advalue;
	sds_error err;

	if (context == NULL || voltage == NULL) {
		return SDS_ERROR_INVALID_PARAM;
	}

	if ((err = sds_decode_to_raw(context, sample, &advalue))) {
		return err;
	}

	/* TODO: multiply with volt/div */

	return advalue;
}
