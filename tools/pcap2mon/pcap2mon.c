//	pcap_throughput
//
//	 reads in a pcap file and outputs basic throughput statistics 

#include <stdio.h>
#include <inttypes.h>
#include <pcap.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdlib.h>

#include "pcap_types.h"

int hostToDevice(unsigned char info) {
	return !(info & 1);
}

int getEndpoint(unsigned char endpoint) {
	return endpoint & 0b01111111;
}

int getDirection(unsigned char endpoint) {
	return endpoint >> 7;
}

const char *endpointToDirection(unsigned char endpoint) {
	if (getDirection(endpoint)) {
		// in
		return "i";
	} else {
		// out
		return "o";
	}
}

struct timeval start = {0, 0};

void normalizeTimeval(struct timeval *recorded, struct timeval *result) {
	if (!timerisset(&start)) {
		// This is the first packet
		start = *recorded;
		// This packet has an offset of 0
		timerclear(result);
	} else {
		// Get the offset
		timersub(recorded, &start, result);
	}
}

unsigned long timevalToMicroseconds(struct timeval *time) {
	return time->tv_sec * 1000000 + time->tv_usec;
}

void printData(FILE *fd, uint32_t length, unsigned char *pointer) {
	for(int i=0; i<length; i++) {
		fprintf(fd, "%02x", pointer[i]);
		if(i != 0 && ((i + 1) % 4) == 0) {
			fprintf(fd, " ");
		}
	}
}

//------------------------------------------------------------------- 
int main(int argc, char **argv) 
{ 
 
	//temporary packet buffers 
	struct pcap_pkthdr header; // The header that pcap gives us 
	const u_char *packet; // The actual packet 
	
	//check command line arguments 
	if (argc < 2) { 
	  fprintf(stderr, "Usage: %s [input pcaps]\n", argv[0]); 
	  exit(1); 
	} 
	
	//-------- Begin Main Packet Processing Loop ------------------- 
	//loop through each pcap file in command line args 
	for (int fnum=1; fnum < argc; fnum++) {  
		unsigned int pkt_counter=0;	// packet counter 
		timerclear(&start); // reset timervalues
	 
		//----------------- 
		//open the pcap file 
		pcap_t *handle; 
		char errbuf[PCAP_ERRBUF_SIZE]; //not sure what to do with this, oh well 
		handle = pcap_open_offline(argv[fnum], errbuf);		//call pcap library function 
	 
		if (handle == NULL) { 
			fprintf(stderr,"Couldn't open pcap file %s: %s\n", argv[fnum], errbuf); 
			return(2); 
		} 
	 
		//----------------- 
		//begin processing the packets in this particular file, one at a time 
	 
		while (packet = pcap_next(handle,&header)) { 
			if (header.len < sizeof(USBPCAP_BUFFER_PACKET_HEADER)) {
				fprintf(stderr, "Packet %d smaller than usb-packet header\n", pkt_counter);
				exit(EXIT_FAILURE);
			}

			// header contains information about the packet (e.g. timestamp) 
			USBPCAP_BUFFER_PACKET_HEADER *pkt = (USBPCAP_BUFFER_PACKET_HEADER *)packet; //cast a pointer to the packet data 

			// endpoint (offset 17) is the endpoint number used on the USB bus
			// (the MSB describes transfer direction)
			int endpoint  = getEndpoint(pkt->endpoint);
			int direction = getDirection(pkt->endpoint);
			int bus = pkt->bus;
			int device = pkt->device;
			uint64_t requestid = pkt->irpId;
			unsigned char info = pkt->info;
			//struct timeval normalized;
			//normalizeTimeval(&header.ts, &normalized);

			printf("%lx ", requestid);
			printf("%lu ", timevalToMicroseconds(&header.ts));
			if(!pkt->status) {
				if (hostToDevice(info) && !pkt->status) {
					printf("S ");
				} else if (!hostToDevice(info) && !pkt->status) {
					printf("C ");
				}
			} else {
				printf("E ");
			}


			switch (pkt->transfer) {
			case USBPCAP_TRANSFER_CONTROL:
				// Transfertype
				printf("C%s:", endpointToDirection(pkt->endpoint));
				// Bus, device, endpoint
				printf("%03d:%03d:%d ", bus, device, endpoint);
				if (header.len < sizeof(USBPCAP_BUFFER_CONTROL_HEADER)) {
					fprintf(stderr, "Packet %d is a control packet but too small for the controlheader\n", pkt_counter);
					exit(EXIT_FAILURE);
				}
				USBPCAP_BUFFER_CONTROL_HEADER *controlheader = (USBPCAP_BUFFER_CONTROL_HEADER *) pkt;

				switch (controlheader->stage) {
				case USBPCAP_CONTROL_STAGE_SETUP:
					if(header.len < sizeof(USBPCAP_BUFFER_CONTROL_HEADER) + sizeof(USB_SETUP)) {
						fprintf(stderr, "Packet %d is a setup packet but does not carry the setup header\n", pkt_counter);
						exit(EXIT_FAILURE);
					}

					USB_SETUP *setupdata = (USB_SETUP *) (((char *) pkt) + sizeof(USBPCAP_BUFFER_CONTROL_HEADER));
					// Setuppacket + setupdata
					printf("s %02x %02x %04x %04x %04x ", setupdata->bmRequestType, setupdata->bRequest, setupdata->wValue, setupdata->wIndex, setupdata->wLength);
					// Length
					printf("%d ", setupdata->wLength);

					if (header.len > sizeof(USBPCAP_BUFFER_CONTROL_HEADER) + sizeof(USB_SETUP)) {
						fprintf(stderr, "Spare data in setup packet %d\n", pkt_counter);
					} else {
						// empty data
						printf("<");
					}
					break;
				case USBPCAP_CONTROL_STAGE_DATA:
					// Status (usually 0)
					printf("%u ", pkt->status);
					// Length of the data
					printf("%d ", pkt->dataLength);
					// Data (that is present) starts with a =
					printf("= ");
					// print the data
					printData(stdout, pkt->dataLength, ((char *) pkt) + sizeof(USBPCAP_BUFFER_CONTROL_HEADER));
					break;
				case USBPCAP_CONTROL_STAGE_STATUS:
					// Build a demo packet with the appropriate statuscode
					// The actual information: statuscode
					printf("%u ", pkt->status);
					// the data (should be empty)
					printf("%d ", pkt->dataLength);
					if (pkt->dataLength > 0) {
						fprintf(stderr, "Warning: a STAGE_STATUS package with datasize > 0 received, parsing assumptions may not hold\n");
					}
					printf("= ");
					printData(stdout, pkt->dataLength, ((char *) pkt) + sizeof(USBPCAP_BUFFER_CONTROL_HEADER));
					break;
				default:
					fprintf(stderr, "Unknown control stage received\n");
				}
				break;

			case USBPCAP_TRANSFER_BULK:
				// This is a bulk-transfer
				printf("B%s:", endpointToDirection(pkt->endpoint));
				// Bus, device, enpoint
				printf("%03d:%03d:%d ", bus, device, endpoint);

				// Status code:
				printf("%u ", pkt->status);
				// Data length:
				printf("%d = ", pkt->dataLength);
				printData(stdout, pkt->dataLength, ((char *) pkt) + sizeof(USBPCAP_BUFFER_PACKET_HEADER));
				break;

			case USBPCAP_TRANSFER_ISOCHRONOUS:
				fprintf(stderr, "Isochronous Transfer not implemented, skipping\n");
				break;

			case USBPCAP_TRANSFER_INTERRUPT:
				fprintf(stderr, "Interrupt Transfer not implemented, skipping\n");
				break;

			default:
				fprintf(stderr, "Unknown transfertype found, skipping\n");
				break;
			}
	 
			// End of the current packet
			printf("\n");

			pkt_counter++; //increment number of packets seen 

		} //end internal loop for reading packets (all in one file) 
	 
		pcap_close(handle);  //close the pcap file 
 
	} //end for loop through each command line argument 
	//---------- Done with Main Packet Processing Loop --------------  
	return 0; //done
} //end of main() function
