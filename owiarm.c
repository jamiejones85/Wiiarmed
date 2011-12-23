#include "owiarm.h"

struct libusb_device_handle * init_arm(libusb_device **devs) {
	ssize_t cnt;
    libusb_device *dev;
    struct libusb_device_handle *devh = NULL;

	int r = libusb_init(NULL);
	if (r < 0)
    {
	    fprintf(stderr, "failed to initialize libusb\n");
	    return NULL;
    }

    libusb_set_debug(NULL,2);
    
	cnt = libusb_get_device_list(NULL, &devs);

	if (cnt < 0)
		return NULL;
		
    dev=find_arm(devs);

    if(!dev)
    {
	    fprintf(stderr, "Robot Arm not found\n");
	    return NULL;
    }

    r = libusb_open(dev, &devh);
    if(r!=0)
    {
	    fprintf(stderr, "Error opening device\n");
       	libusb_free_device_list(devs, 1);
        libusb_exit(NULL);
	    return NULL;
    }

	return devh;
}

libusb_device * find_arm(libusb_device **devs) {
	libusb_device *dev;
	int i = 0;
	fprintf(stderr, "find_arm\n");
	
	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return NULL;
		}
		
		if(desc.idVendor == ARM_VENDOR &&
		   desc.idProduct == ARM_PRODUCT)
		  {
			fprintf(stderr, "Arm found\n");
		    return dev;
		  }
	}
	return NULL;
}

int send_arm_control(libusb_device_handle *devh,  unsigned char cmd[3]) {
	int r;
    int actual_length=-1;

    /*fprintf(stderr, "Sending %02X %02X %02X\n",
            (int)cmd[0],
            (int)cmd[1],
            (int)cmd[2]
    );*/
	
	r = libusb_control_transfer(devh,
	                            0x40, //uint8_t 	bmRequestType,
	                            6, //uint8_t 	bRequest,
	                            0x100, //uint16_t 	wValue,
	                            0,//uint16_t 	wIndex,
	                            cmd,
	                            CMD_DATALEN,
	                            0	 
	);

	if(!(r == 0 && actual_length >= CMD_DATALEN))
	{
	    fprintf(stderr, "Write err %d. len=%d\n",r,actual_length);
	}
	
	return r;
}

void stop_arm(libusb_device_handle *devh) {
	unsigned char cmd[3];

	cmd[0]=(unsigned char)0x0;
    cmd[1]=(unsigned char)0x0;
    cmd[2]=(unsigned char)0x0;

	send_arm_control(devh, cmd);
}

void close_arm(libusb_device **devs, libusb_device_handle *devh) {
	libusb_close(devh);
	libusb_free_device_list(devs, 1);
	libusb_exit(NULL);
}