/* This file is part of the SDS 200A library project.
 *
 * It is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar. If not, see <http://www.gnu.org/licenses/>.
 *
 * (c) 2014 Simon Schuster, Sebastian Rachuj
 */

#include <stddef.h>
#include <stdint.h>

/*!
 * Represents an oscilloscope context.
 */
typedef struct sds_context sds_context;

/*!
 * Represents a list of devices.
 */
struct sds_device_list
{
	unsigned int size; /*!< The amount of elements in the array. */
	struct sds_device *array; /*!< The array of devices. Not NULL-terminated! */
	void *devices_ptr; /*!< An opaque pointer to internal data. Do not modify! */
};

/*!
 * Represents one device that can be used to initialize a sds_context.
 */
struct sds_device
{
        int bus_no; /*!< The bus number where the device is located. */
        int port_no; /*!< The port number where the device is located. */
        void *device_ptr; /*!< An opaque pointer to internal data. Do not modify! */
};

/*!
 * This represents the calibration of the device (as known in software). Except
 * for saving the current settings, this should not be interesting for user.
 */
struct sds_calibration
{
	double zero1; /*!< Offset for zero volts of channel 1 */
	double zero2; /*!< Offset for zero volts of channel 2 */
	unsigned int uv_per_tick1; /*!< Micro volts per ticks of channel 1 */
	unsigned int uv_per_tick2; /*!< Micro volts per ticks of channel 2*/
};

/*!
 * This represents the raw measurement-data as returned by the device. This data
 * consists of 8 leading bytes of unknown information, followed by an array of
 * actual measurement data (that should be converted using sds_decode_to_raw()
 * or sds_decode_to_volt())
 *
 * The samples array returned by the device represent a stream of data
 * alternating between ch1 and ch2:
 * 	samples[0] = ch1;
 * 	samples[1] = ch2;
 * 	samples[2] = ch1;
 * 	samples[3] = ch2;
 * 	samples[4] = ch1;
 * 	...
 */
struct __attribute__((packed)) sds_samples
{
	uint8_t unknown_padding[8];
	uint16_t samples[];
};

/*!
 * Represents a probe channel.
 */
enum sds_channel
{
	SDS_CH1 = 1, /*!< Channel 1 */
	SDS_CH2 = 2, /*!< Channel 2 */
};

/*!
 * Represents a possible voltage division (volts/div) settings.
 */
enum sds_voltage
{
	SDS_10mV = 1, /*!< 10mV/DIV */
	SDS_20mV, /*!< 20mV/DIV */
	SDS_40mV, /*!< 40mV/DIV */
	SDS_100mV, /*!< 100mV/DIV */
	SDS_200mV, /*!< 200mV/DIV */
	SDS_400mV, /*!< 400mV/DIV */
	SDS_1V, /*!< 1V/DIV */
	SDS_2V, /*!< 2V/DIV */
	SDS_4V, /*!< 4V/DIV */
	SDS_10V, /*!< 10V/DIV */
};

/*!
 * Represents a possible time division (time/div) settings.
 */
enum sds_time
{
	SDS_2ns = 1, /*!< 2ns/DIV */
	SDS_4ns, /*!< 4ns/DIV */
	SDS_10ns, /*!< 10ns/DIV */
	SDS_20ns, /*!< 20ns/DIV */
	SDS_40ns, /*!< 40ns/DIV */
	SDS_100ns, /*!< 100ns/DIV */
	SDS_200ns, /*!< 200ns/DIV */
	SDS_400ns, /*!< 400ns/DIV */
	SDS_1us, /*!< 1us/DIV */
	SDS_2us, /*!< 2us/DIV */
	SDS_4us, /*!< 4us/DIV */
	SDS_10us, /*!< 10us/DIV */
	SDS_20us, /*!< 20us/DIV */
	SDS_40us, /*!< 40us/DIV */
	SDS_100us, /*!< 100us/DIV */
	SDS_200us, /*!< 200us/DIV */
	SDS_400us, /*!< 400us/DIV */
	SDS_1ms, /*!< 1ms/DIV */
	SDS_2ms, /*!< 2ms/DIV */
	SDS_4ms, /*!< 4ms/DIV */
	SDS_10ms, /*!< 10ms/DIV */
	SDS_20ms, /*!< 20ms/DIV */
	SDS_40ms, /*!< 40ms/DIV */
	SDS_100ms, /*!< 100ms/DIV */
	SDS_200ms, /*!< 200ms/DIV */
	SDS_400ms, /*!< 400ms/DIV */
	SDS_1s, /*!< 1s/DIV */
	SDS_2s, /*!< 2s/DIV */
	SDS_4s, /*!< 4s/DIV */
	SDS_10s, /*!< 10s/DIV */
};

/*!
 * Represents the trigger slope modes.
 */
enum sds_trigger_slope
{
	SDS_RISING = 1, /*!< Trigger on rising edge */
	SDS_FALLING, /*!< Trigger on falling edge */
};

/*!
 * Represents the trigger modes.
 */
enum sds_trigger_mode
{
	SDS_NORMAL = 1, /*!< Normal trigger mode
			     (if not triggered no data is returned) */
	SDS_AUTOMATIC, /*!< Automatic trigger mode
			    (automatically trigger even if there was no trigger event) */
};

/*!
 * Represents an error code.
 */
typedef enum
{
	SDS_ERROR_SUCCESS = 0, /*!< Everything went smoothly. */
	SDS_ERROR_IO = -1, /*!< An I/O error occured. */
	SDS_ERROR_INVALID_PARAM = -2, /*!< An invalid parameter was passed to the function. */
	SDS_ERROR_ACCESS = -3, /*!< Insufficient permissions. */
	SDS_ERROR_NO_DEVICE = -4, /*!< Device is not available (any more). */
	SDS_ERROR_NOT_FOUND = -5, /*!< Entity not found. */
	SDS_ERROR_BUSY = -6, /*!< Resource is busy. */
	SDS_ERROR_TIMEOUT = -7, /*!< Operation timed out. */
	SDS_ERROR_OVERFLOW = -8, /*!< An overflow occured. */
	SDS_ERROR_PIPE = -9, /*!< An error occured while writing to/reading from a pipe . */
	SDS_ERROR_INTERRUPTED = -10, /*!< A (internal) system call was interrupted (e.g. by a signal).
					  You might consider setting SA_RESTART or retrying the operation. */
	SDS_ERROR_NO_MEM = -11, /*!< Insufficient memory. */
	SDS_ERROR_NOT_SUPPORTED = -12, /*!< An unsupported operation was requested. */
	SDS_ERROR_UNKNOWN = -99 /*!< An error that is unknown (e.g. was introduced by a newer versions of libusb) has occured */
} sds_error;

/*!
 * Allocates ressources that are necessary for communicating with the
 * oscilloscope.
 * \remark sds_initialize has to be called before using the device in any way
 *
 * \param device The oscilloscope that is used. It can be obtained by
 *               sds_get_devices().
 * \param context A pointer to a context pointer, that will contain the
 *                pointer to the created context.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_initialize(struct sds_device *device, sds_context **context);

/*!
 * Obtains an array for all connected oscilloscopes which this library can
 * handle.
 *
 * \param [out] list After the call, the pointer referenced by the list parameter
 *                   will be set to an malloced sds_device_list, containing an
 *                   array of connected oscilloscopes that this library should be
 *                   able to handle.
 *                   This list has to be freed via sds_free_devices() after
 *                   usage.
 * \see              See the structure definition of sds_device_list for more
 *                   information.
 *
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_devices(struct sds_device_list **list);

/*!
 * Frees the device_list that was allocated by sds_get_devices().
 *
 * \remark This frees the member "array" as well. 
 *         Typically one would use sds_get_devices() and select the matching
 *         device(s) from that list. Those should then be opended using
 *         sds_initialize(). Only *then* the list may be freed using
 *         sds_free_devices().
 *
 * \param device_list The list that should be freed. After this operation the
 *                    elements **must not** be used any more. A NULL pointer
 *                    results in an no-op.
 */
void sds_free_devices(struct sds_device_list *device_list);

/*!
 * Frees the ressources of an sds_context.
 *
 * \param context The pointer to the oscilloscope context. After this operation,
 *                this context **must not** be used any more. A NULL pointer
 *                results in an no-op.
 */
void sds_destroy(sds_context *context);

/*!
 * Activates or deactivates a channel.
 *
 * \param context The device context.
 * \param channel The channel to be enabled or disabled.
 * \param on      A value that evaluates to true if the channel should be activated.
 *                A value that evaluates to false if the channel should be
 *                deactivated.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_channel(sds_context *context, enum sds_channel channel, int on);

/*!
 * Returns the current activation state of a channel.
 *
 * \param context  The device context.
 * \param channel  The channel to be queried.
 * \param [out] on A pointer to the variable that will contain the activation state
 *                 of the specified channel.
 */
sds_error sds_get_channel(sds_context *context, enum sds_channel channel, int *on);

/*!
 * Sets the voltage per div for the specified channel.
 *
 * \param context The device context
 * \param channel The channel to be adjusted
 * \param voltage The new voltage per div that is to be set.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_voltage(sds_context *context, enum sds_channel channel, enum sds_voltage voltage);

/*!
 * Returns the voltage per div for the specified channel.
 *
 * \param context       The device context
 * \param channel       The queried channel
 * \param [out] voltage A pointer to a variable that will contain the voltage
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_voltage(sds_context *context, enum sds_channel channel, enum sds_voltage *voltage);

/*!
 * Sets the channel coupling for the specified channel.
 *
 * \param context The device context
 * \param channel The channel to be adjusted
 * // TODO: Which values for ac/dc
 * \param on      0 to deactivate channel coupling, values != 0 to activate
 *                coupling
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_coupling(sds_context *context, enum sds_channel channel, int on);

/*!
 * Returns the channel coupling.
 *
 * \param context        The device context
 * \param channel        The queried channel
 * // TODO: Which values for ac/dc
 * \param [out] coupling A pointer to a variable that will contain a true value
 *                       if coupling is enabled on the specified channel, a false
 *                       value otherwise.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_coupling(sds_context *context, enum sds_channel channel, int *coupling);

/*!
 * Sets the time per div.
 *
 * \param context The device context
 * \param time    The time/div to set
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_time(sds_context *context, enum sds_time time);

/*!
 * Returns the time div.
 *
 * \param context    The device context
 * \param [out] time A pointer to an enum sds_time where the current time/div
 *                   will be stored
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_time(sds_context *context, enum sds_time *time);

/*!
 * Sets the voltage offset.
 *
 * \param context The device context
 * \param channel The channel to be adjusted
 * \param factor  Adjustment factor for the voltage. Must be a value between
 *                -1.0 and 1.0.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_offset(sds_context *context, enum sds_channel channel, double factor);

/*!
 * Returns the voltage offset.
 *
 * \param context      The device context
 * \param channel      The channel to be queried
 * \param [out] offset A pointer to the variable that will contain the current
 *                     offset of the specified channel.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_offset(sds_context *context, enum sds_channel channel, double *offset);

/*!
 * Sets the trigger source.
 *
 * \param context The device context
 * \param channel The channel to be set as the new trigger source
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_trigger_source(sds_context *context, enum sds_channel channel);

/*!
 * Returns the trigger source.
 *
 * \param context      The device context
 * \param [out] source A pointer to an enum that will contain the selected trigger
 *                     source.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_trigger_source(sds_context *context, enum sds_channel *source);

/*!
 * Sets the trigger slope.
 *
 * \param context The device context
 * \param slope   When to trigger (e.g. SDS_RISING or SDS_FALLING)
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_trigger_slope(sds_context *context, enum sds_trigger_slope slope);

/*!
 * Returns the trigger slope.
 *
 * \param context     The device context
 * \param [out] slope A pointer to an enum that will contain the currently selected
 *                    slope mode
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_trigger_slope(sds_context *context, enum sds_trigger_slope *slope);

/*!
 * Sets the trigger mode.
 *
 * \param context The device context
 * \param mode    The trigger mode to set
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_trigger_mode(sds_context *context, enum sds_trigger_mode mode);

/*!
 * Returns the trigger mode.
 *
 * \param context    The device context
 * \param [out] mode A pointer to an enum that will contain the currently selected
 *                   mode
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_trigger_mode(sds_context *context, enum sds_trigger_mode *mode);

/*!
 * Sets the trigger offset.
 *
 * \param context The device context
 * \param channel The channel whose trigger is adjusted.
 * \param offset  A value between -1.0 and +1.0 that adjusts the trigger on
 *                the currently selected scale.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_set_trigger_offset(sds_context *context, enum sds_channel channel, double offset);

/*!
 * Returns the trigger offset.
 *
 * \param context       The device context
 * \param channel       The channel to be queried
 * \param [out] offset  A pointer to a variable that will contain the current
 *                      trigger of the selected channel.
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_trigger_offset(sds_context *context, enum sds_channel channel, double *offset);

/*!
 * Calibrates the zero point of the voltage scale. You probably want to call
 * this function before using another function on the device for obtaining
 * data.
 *
 * \param context The device context
 * \param zero1   A pointer to a previously saved zero calibration for
 *                channel 1. This function restores the state. If it is NULL,
 *                the channel will be calibrated again. Therefore the probe
 *                should be grounded (by connecting both lines of the coaxial
 *                cord).
 * \param zero2   A pointer to a previously saved zero calibration for
 *                channel 2. This function restores the state. If it is NULL,
 *                the channel will be calibrated again. Therefore the probe
 *                should be grounded (by connecting both lines of the coaxial
 *                cord).
 */
sds_error sds_calibrate_offset(sds_context *context, unsigned int *zero1, unsigned int *zero2);

/*!
 * Calibrates the value distance of the voltage scale. You probably want to
 * call this function before using another function on the device for obtaining
 * data.
 *
 * \param context The device context
 * \param us_per_tick1 A pointer to a previously saved us_per_tick calibration
 *                for channel 1. This function restores the state. If it is NULL,
 *                the channel will be calibrated again. Therefore the probe
 *                should be connected to the square signal and ground.
 * \param us_per_tick2 A pointer to a previously saved us_per_tick calibration
 *                for channel 2. This function restores the state. If it is NULL,
 *                the channel will be calibrated again. Therefore the probe
 *                should be connected to the square signal and ground.
 */
sds_error sds_calibrate_scale(sds_context *context, unsigned int *us_per_tick1, unsigned int *us_per_tick2);

/*!
 * Returns the calibration.
 *
 * \param context     The device context
 * \param calibration A pointer to a calibration struct that will contain the
 *                    data of the software calibration.
 */
sds_error sds_get_calibration(sds_context *context, struct sds_calibration *calibration);

/*!
 * Waits for data from the device. (Blocking)
 *
 * \param context       The device context
 * \param [out] data    A user provided buffer for data from the device
 * \param length        The size of the user provided buffer (data)
 * \param [out] written A pointer to a variable that will contain the amount of
 *                      bytes written to data
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_data(sds_context *context, char *data, size_t length, size_t *written);

/*!
 * Reads raw samples from the device for maximum throughput
 *
 * The sample ist represented by FIXME
 *
 * \remark Waits for data from the device. (Blocking)
 * \remark The library mallocs the buffer, which **has to be freed** by the user
 *
 * \param context       The device context
 * \param [out] data    A the pointer will be set to the result from a sampled
 *                      request (FIXME)
 * \param [out] written A pointer to a variable that will contain the amount of
 *                      bytes written to data
 *
 * \return An error value to indicate the success.
 */
sds_error sds_get_raw_data(sds_context *context, struct sds_samples **data, size_t *written);

/*!
 * Decodes the passed samplevalue to the raw 10bit A/D value (after calibration)
 *
 * \param context       The context of the device that generated the samples
 *                      (necessary to retrive calibartion-data)
 * \param sample        The captured value from the samples returned by FIXME
 * \param [out] advalue A pointer where the advalue sould be returned to
 *                      (10bit, range 0-1023)
 *
 * \return An error value to indicate the success.
 */
sds_error sds_decode_to_raw(sds_context *context, uint16_t sample, uint16_t *advalue);

/*!
 * Decodes the passed samplevalue to a 64bit double value (applys both calibartion
 * data and volts/div settings to the A/D value
 *
 * \param context       The context of the device that generated the samples
 *                      (necessary to retrive calibartion-data)
 * \param sample        The captured value from the samples returned by FIXME
 * \param [out] advalue A pointer where the value should be returend to
 *
 * \return An error value to indicate the success.
 */
sds_error sds_decode_to_volt(sds_context *context, uint16_t sample, double *advalue);
