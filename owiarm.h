#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <stdio.h>

#define ARM_VENDOR       0x1267
#define ARM_PRODUCT      0

#define CMD_DATALEN      3

libusb_device * find_arm(libusb_device **devs);
struct libusb_device_handle * init_arm(libusb_device **devs);
int send_arm_control(libusb_device_handle *devh,  unsigned char cmd[3]);
void close_arm(libusb_device **devs, libusb_device_handle *devh);
void stop_arm(libusb_device_handle *devh);
