#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include "../../des_controller/src/des.h"

/*
 *
 program des_inputs that prompts the user for inputs to the controller. This
 program is simulating all of the input events from the devices; e.g., card readers, door latches,
 scale.
 Prompt for the input:
 a. The first prompt should be:
 Enter the event type (ls= left scan, rs= right scan, ws= weight scale, lo =left open,
 ro=right open, lc = left closed, rc = right closed , gru = guard right unlock, grl = guard
 right lock, gll=guard left lock, glu = guard left unlock)
 b. If the event is the lo, ro, lc, rc, glu, gll, gru, or grl, no further prompt is required.
 c. If the event is ls or rs, prompt for the person_id.
 Enter the person_id
 d. If the event is ws, prompt for the weight.
 Enter the weight
 Once the input is finished, send a message to the des_controller program to provide the input
 “event”, and loop back to prompt again.
 This scenario represents a Person entering the building, beginning with a left scan (ls) of the
 person’s ID:
 ls
 12345
 glu
 lo
 ws
 123
 lc
 gll
 gru
 ro
 rc
 grl
 exit
 */

int main(int argc, char *argv[]) {
	int coid;
	pid_t serverpid;
	int client_message_array[2] = { -1, -1 };
	int *client_message;
	int *server_response;
	int numInput;
	//char inputStr[100];

	server_response = malloc(sizeof(int));
	*server_response = 0;
	client_message = client_message_array;

	// check number of command line arguments
	if (argc < 2) {
		fprintf(stderr, "Must be at least 2 command line arguments \n");
		exit(EXIT_FAILURE);
	}

	// store server pid for connection
	serverpid = atoi(argv[1]);

	// connect to server
	coid = ConnectAttach(ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		fprintf(stderr, "Could not ConnectAttach\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	// loop until they enter exit
	while (*(client_message + 0) != EXIT) {
		// fprintf(stderr, "server response\"%d\"\n", *server_response);

		char inputStr[100];
		fprintf(stderr, promptStrings[*server_response]);
		fgets(inputStr, 100, stdin);
		inputStr[strcspn(inputStr, "\r\n")] = 0;

		// check if input is one of the defined input events
		if (strcmp(inputStr, inputStrings[LEFT_SCAN]) == 0) {
			*(client_message + 0) = LEFT_SCAN;
		} else if (strcmp(inputStr, inputStrings[RIGHT_SCAN]) == 0) {
			*(client_message + 0) = RIGHT_SCAN;
		} else if (strcmp(inputStr, inputStrings[GUARD_LEFT_UNLOCK]) == 0) {
			*(client_message + 0) = GUARD_LEFT_UNLOCK;
		} else if (strcmp(inputStr, inputStrings[GUARD_RIGHT_UNLOCK]) == 0) {
			*(client_message + 0) = GUARD_RIGHT_UNLOCK;
		} else if (strcmp(inputStr, inputStrings[LEFT_OPEN]) == 0) {
			*(client_message + 0) = LEFT_OPEN;
		} else if (strcmp(inputStr, inputStrings[RIGHT_OPEN]) == 0) {
			*(client_message + 0) = RIGHT_OPEN;
		} else if (strcmp(inputStr, inputStrings[WEIGHT_SCALE]) == 0) {
			*(client_message + 0) = WEIGHT_SCALE;
		} else if (strcmp(inputStr, inputStrings[LEFT_CLOSE]) == 0) {
			*(client_message + 0) = LEFT_CLOSE;
		} else if (strcmp(inputStr, inputStrings[RIGHT_CLOSE]) == 0) {
			*(client_message + 0) = RIGHT_CLOSE;
		} else if (strcmp(inputStr, inputStrings[GUARD_LEFT_LOCK]) == 0) {
			*(client_message + 0) = GUARD_LEFT_LOCK;
		} else if (strcmp(inputStr, inputStrings[GUARD_RIGHT_LOCK]) == 0) {
			*(client_message + 0) = GUARD_RIGHT_LOCK;
		} else if (strcmp(inputStr, inputStrings[EXIT]) == 0) {
			*(client_message + 0) = EXIT;
		} else {
			// check if input is an integer for weight or id
			// convert string to integer
			numInput = atoi(inputStr);

			if (numInput <= 0) {
				// if not an int or less than 1, invalid input set message to -1
				*(client_message + 0) = -1;
			} else {
				// otherwise send int
				*(client_message + 0) = NUM_INPUT + 1;
				*(client_message + 1) = numInput;
			}
		}

		// fprintf(stderr, "client message \"%d\", \"%d\"\n", *(client_message + 0), *(client_message + 1));

		// send and receive message to server
		if (MsgSend(coid, client_message, sizeof(client_message_array) + 1,
				server_response, sizeof(int) + 1) == -1L) {
			fprintf(stderr, "Error during MsgSend\n");
			perror(NULL);
			exit(EXIT_FAILURE);
		}

		sleep(1);
	}
	//Disconnect from the channel --- Phase III
	ConnectDetach(coid);
	free(server_response);
	return EXIT_SUCCESS;
}
