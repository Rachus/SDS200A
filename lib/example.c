
#include "libsds200a.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	struct sds_device_list *devices;
	sds_context *context;
	char dummybuf[1024]; /* For "press enter" */
	struct sds_calibration calibration;

	/* Get a list of devices and check, that there is at least one */
	if (sds_get_devices(&devices) || devices->size < 1) {
		fprintf(stderr, "No SDS 200A found\n");
		return EXIT_FAILURE;
	}
	/* Initialize the first previously found device */
	if (sds_initialize(&devices->array[0], &context)) {
		fprintf(stderr, "Could not open device at %d:%d\n",
				devices->array[0].bus_no, devices->array[0].port_no);
		return EXIT_FAILURE;
	}
	printf("Successfully opened a SDS 200A at Bus %d, Port %d\n",
			devices->array[0].bus_no, devices->array[0].port_no);
	sds_free_devices(devices);

	/* Calibration */
	/* These is a dummy for how a (software) configuration might work */
	printf("--- Calibration\n"
			"For each probe connect both conductors of the coaxial cord with each other.\n"
			"If you are ready, press Enter.\n");
	fgets(dummybuf, sizeof(dummybuf), stdin);

	sds_calibrate_offset(context, NULL, NULL);

	printf("Now, please connect both probes with the square signal of the SDS 200A and then press Enter.\n");
	fgets(dummybuf, sizeof(dummybuf), stdin);

	sds_calibrate_scale(context, NULL, NULL);

	sds_get_calibration(context, &calibration);

	printf("Calibration data:\n"
			"Channel Zero uv_per_ticks\n"
			"   1     %f    %d\n"
			"   2     %f    %d\n",
			calibration.zero1, calibration.uv_per_tick1,
			calibration.zero2, calibration.uv_per_tick2);
	
	fgets(dummybuf, sizeof(dummybuf), stdin);

	sds_get_data(context, NULL, 0, NULL);

	/* TODO: More examples */

	sds_destroy(context);
	return EXIT_SUCCESS;
}


