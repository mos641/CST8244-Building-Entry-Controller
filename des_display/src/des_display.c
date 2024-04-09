#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "../../des_controller/src/des.h"

/*
 program des_display that displays the status of the system – which door is
 open/closed if there is a user waiting to enter from the left or right, etc. The program should
 run in the background and print out status information to the console each time a status update
 is sent to it using a message from the des_controller program. The des_display program can
 print out its process id when it first starts up (as in Lab5)
 */

int main(int argc, char *argv[]) {
	int rcvid;
	int controller_chid;
	int controller_message_array[2] = { -1, -1 };
	int *controller_message;

	controller_message = controller_message_array;

	// create a channel for inputs--- Phase I
	controller_chid = ChannelCreate(0);

	if (controller_chid == -1) {
		perror("error - cannot create channel");
		exit(EXIT_FAILURE);
	}

	// print pid
	fprintf(stderr, "The display is running as process_id %d\n", getpid());

	while (*(controller_message + 0) != EXIT) {
		// get the message from inputs
		rcvid = MsgReceive(controller_chid, controller_message,
				sizeof(controller_message_array) + 1,
				NULL);

		// print message depending on received message
		if (*(controller_message + 0) != EXIT) {
			if (*(controller_message + 0) == 0
					|| *(controller_message + 0) == 5) {
				fprintf(stderr, "%s%d\n",
						outputStrings[*(controller_message + 0)],
						*(controller_message + 1));
			} else {
				fprintf(stderr, "%s\n",
						outputStrings[*(controller_message + 0)]);
			}
		}

		// send a message to controller
		MsgReply(rcvid, EOK, controller_message,
				sizeof(controller_message_array) + 1);

		sleep(1);
	}

	// ConnectDetach(controller_chid);
	ChannelDestroy(controller_chid);

	// fprintf(stderr, "chid %d\n", controller_chid);
	fprintf(stderr, "Exit Display\n");

	exit(EXIT_SUCCESS);
}

