/**
 * Jamie Jones
 * 
 * Using Wiiuse Fork https://github.com/rpavlik/wiiuse
 * http://notbrainsurgery.livejournal.com/38622.html for OWI Arm protocol
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include <wiic/wiic.h>
#include "owiarm.h"

#define MAX_WIIMOTES 1

#define EP_INTR	(1 | LIBUSB_ENDPOINT_IN)

#define CMD_DATALEN 3


int exiting = 0;

/**
 *	@brief Callback that handles an event.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *
 *	This function is called automatically by the wiiuse library when an
 *	event occurs on the specified wiimote.
 */
void handle_event(struct wiimote_t* wm, libusb_device_handle *devh, unsigned char *armLedStatus) {

	/*the control bytes*/
	unsigned char cmd[3];

	cmd[0]=(unsigned char)0x0;
    cmd[1]=(unsigned char)0x0;
	cmd[2]=*armLedStatus;

	//exit
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_ONE)) {
		printf("Exiting\n");
		exiting =1;
		return;
	}
	
	//LED control
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_B)) {
		if(*armLedStatus == 0x0) {
			cmd[2]=	(unsigned char)0x01;
			printf ("Turning on LED.\n");
		} else {
			cmd[2]= (unsigned char)0x0;
			printf ("Turning off LED.\n");
		}
		*armLedStatus = (unsigned char)cmd[2];
	}
	
	//gripper control +/-
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_PLUS)) {
		cmd[0] = cmd[0] + 0x02;			
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_MINUS)) {
		cmd[0] = cmd[0] + 0x01;
	}
	
	//wrist control
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_UP)) {
		cmd[0] = cmd[0] + 0x04;			
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_DOWN)) {
		cmd[0] = cmd[0] + 0x08;
	}
	
	//base control/could also use yaw
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_LEFT)) {
		cmd[1] = 0x01;			
	}
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_RIGHT)) {
		cmd[1] = 0x02;
	}

	/* if the accelerometer is turned on then use angles */
	if (WIIC_USING_ACC(wm)) {
		
		if(wm->orient.angle.pitch > 20) {
			cmd[0] = 0x80;
		} else if (wm->orient.angle.pitch < -20) {
			cmd[0] = 0x40;
		} 
		
		if(wm->orient.angle.roll > 20) {
			cmd[0] = cmd[0] + 0x20;
		} else if (wm->orient.angle.roll < -20) {
			cmd[0] = cmd[0] + 0x10;
		}

		if(wm->orient.angle.yaw > 20) {
			cmd[1] = cmd[1] + 0x01;
		} else if (wm->orient.angle.yaw < -20) {
			cmd[1] = cmd[1] + 0x10;
		}
	}
	/* Use nunchuck to control wrist and grabber */
	if (wm->exp.type == EXP_NUNCHUK) {
		/* nunchuk */
		struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;



		if(nc->orient.angle.pitch > 20) {
			cmd[0] = cmd[0] + 0x04;
		} else if (nc->orient.angle.pitch < -20) {
			cmd[0] = cmd[0] + 0x08;
		} 

	}
	if(devh != NULL) {
		printf ("Sending Arm Contrl\n");
		send_arm_control(devh, cmd);
	}
}

void handle_disconnect(wiimote* wm) {
	printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
	exiting = 1;
}


int main(int ac, char **av) {

	wiimote** wiimotes;
	int loaded, connected, found;

	unsigned char armLedStatus = 0x0;

	/*
	 *	Initialize an array of wiimote objects.
	 *
	 *	The parameter is the number of wiimotes I want to create.
	 */
	wiimotes =  wiic_init(MAX_WIIMOTES);

	loaded = wiic_load(wiimotes);
    if (!loaded) {
		/*
		 *	Find wiimote devices
		 *
		 *	Now we need to find some wiimotes.
		 *	Give the function the wiimote array we created, and tell it there
		 *	are MAX_WIIMOTES wiimotes we are interested in.
		 *
		 *	Set the timeout to be 5 seconds.
		 *
		 *	This will return the number of actual wiimotes that are in discovery mode.
		 */
		found = wiic_find(wiimotes, MAX_WIIMOTES, 15);
    }

	if (!found) {
		printf ("No wiimotes found.\n");
		return 0;
	}

	/*
	 *	Connect to the wiimotes
	 *
	 *	Now that we found some wiimotes, connect to them.
	 *	Give the function the wiimote array and the number
	 *	of wiimote devices we found.
	 *
	 *	This will return the number of established connections to the found wiimotes.
	 */
	connected = wiic_connect(wiimotes, MAX_WIIMOTES, 5);
	if (connected)
		printf("Connected to %i wiimotes (of %i found).\n", connected, found);
	else {
		printf("Failed to connect to any wiimote.\n");
		return 0;
	}

	/*
	 *	Now set the LEDs and rumble for a second so it's easy
	 *	to tell which wiimotes are connected (just like the wii does).
	 */
	wiic_set_leds(wiimotes[0], WIIMOTE_LED_1);

	wiic_rumble(wiimotes[0], 1);

	#ifndef WIN32
		usleep(200000);
	#else
		Sleep(200);
	#endif

	wiic_rumble(wiimotes[0], 0);
	
	wiic_motion_sensing(wiimotes[0], 1);
    
	libusb_device **devs;

	struct libusb_device_handle *devh = init_arm(devs);
	
	if(devh == NULL) {
		printf ("No arm found\n");
	}
	while (!exiting) {
		if (wiic_poll(wiimotes, connected)) {
			/*
			 *	This happens if something happened on any wiimote.
			 *	So go through each one and check if anything happened.
			 */
			int i = 0;
			for (; i < MAX_WIIMOTES; ++i) {
				switch (wiimotes[i]->event) {
					case WIIC_EVENT:
						// a generic event occured
						handle_event(wiimotes[i], devh, &armLedStatus);
						break;


					case WIIC_DISCONNECT:
					case WIIC_UNEXPECTED_DISCONNECT:
						// the wiimote disconnected
						handle_disconnect(wiimotes[i]);
						break;

					case WIIC_NUNCHUK_INSERTED:
						/*
						 *	a nunchuk was inserted
						 *	This is a good place to set any nunchuk specific
						 *	threshold values.  By default they are the same
						 *	as the wiimote.
						 */
						printf("Nunchuk inserted.\n");
						break;
						
					case WIIC_MOTION_PLUS_INSERTED:
						printf("Motion Plus inserted.\n");
						break;
						
					case WIIC_BALANCE_BOARD_INSERTED:
						printf("Balance Board connected.\n");
						break;
						
					default:
						break;
				}
			}
		}
	}
	
	if(devh != NULL) {
		/* stop any arm movement */
		stop_arm(devh);
	}
	
	/*	Disconnect the wiimote*/
	wiic_cleanup(wiimotes, MAX_WIIMOTES);

	if(devh != NULL) {
		/* close arm connection */
		close_arm(devs, devh);
	}
	
    fprintf(stderr, "Done\n");
	return 0;
}
