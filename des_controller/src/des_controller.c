#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "./des.h"

/*
 program des_controller that runs in the background, operates the state machine
 for the controller, and directly maintains the persistent data and status for the controller. It
 should have separate functions for the state handler for each state. Each state handler should
 perform the actions required for that state, send a message to the des_display program to
 update the display (as required), and then check the exit conditions for that state. When an
 exit condition is met, the state handler should return the function pointer for the next state
 handler. The des_controller program should print out its process id when it first starts up.
 */

void receiveInputs();
void replyInputs();
void messageDisplay();

int leftDoor = LOCKED_CLOSED;
int rightDoor = LOCKED_CLOSED;

int person_id;
int weight;

// server variables
int rcvid;
int inputs_chid;

int received_inputs_message_array[2];
int *received_inputs_message;

int *response_inputs_message;

int *display_message;
int display_message_array[2] = { -1, -1 };

// client variables
int display_chid;
char *ptr;
pid_t serverpid;

//typedef void*(StateFunc)();

// States
void start();
void enter_id();
void id_scanned();
void left_door_unlocked();
void right_door_unlocked();
void left_door_opened();
void right_door_opened();
void ready_to_weigh();
void person_weighed();
void left_door_closed();
void right_door_closed();
void left_door_locked();
void right_door_locked();

// State pointer
void (*statefunc)() = start;

void start() {
	//fprintf(stderr, "start state %d\n", *(received_inputs_message + 0));
	if (*(received_inputs_message + 0) == LEFT_SCAN) {
		leftDoor = LOCKED_SCANNED;
		*response_inputs_message = SCAN;

		statefunc = enter_id;
	} else if (*(received_inputs_message + 0) == RIGHT_SCAN) {
		rightDoor = LOCKED_SCANNED;
		*response_inputs_message = SCAN;

		statefunc = enter_id;
	} else {
		statefunc = start;
	}
}

void enter_id() {
	//fprintf(stderr, "enter id state %d, %d\n", *(received_inputs_message + 0),
	//		*(received_inputs_message + 1));
	if ((*(received_inputs_message + 0) == NUM_INPUT + 1)
			&& (*(received_inputs_message + 1) > 0)) {
		person_id = *(received_inputs_message + 1);

		statefunc = id_scanned;

		*(display_message + 0) = SCANNED;
		*(display_message + 1) = person_id;
		messageDisplay();
	} else {
		*response_inputs_message = SCAN;
		statefunc = enter_id;
	}
}

void id_scanned() {
	//fprintf(stderr, "id scanned state %d\n",
	//				*(received_inputs_message + 0));
	if (*(received_inputs_message + 0) == GUARD_LEFT_UNLOCK
			&& leftDoor == LOCKED_SCANNED) {
		statefunc = left_door_unlocked;

		*(display_message + 0) = LEFT_UNLOCKED;
		messageDisplay();

	} else if (*(received_inputs_message + 0) == GUARD_RIGHT_UNLOCK
			&& rightDoor == LOCKED_SCANNED) {
		statefunc = right_door_unlocked;

		*(display_message + 0) = RIGHT_UNLOCKED;
		messageDisplay();

	} else {
		statefunc = id_scanned;
	}
}

void left_door_unlocked() {
	//fprintf(stderr, "left door unlocked state %d\n",
	//		*(received_inputs_message + 0));
	leftDoor = UNLOCKED_CLOSED;

	if (*(received_inputs_message + 0) == LEFT_OPEN) {
		statefunc = left_door_opened;

		*(display_message + 0) = LEFT_OPENED;
		messageDisplay();
	} else {
		statefunc = left_door_unlocked;
	}
}

void right_door_unlocked() {
	//fprintf(stderr, "right door unlocked state %d\n",
	//		*(received_inputs_message + 0));
	rightDoor = UNLOCKED_CLOSED;

	if (*(received_inputs_message + 0) == RIGHT_OPEN) {
		statefunc = right_door_opened;

		*(display_message + 0) = RIGHT_OPENED;
		messageDisplay();
	} else {
		statefunc = right_door_unlocked;
	}
}

void left_door_opened() {
	//fprintf(stderr, "left door opened state %d\n",
	//		*(received_inputs_message + 0));
	leftDoor = UNLOCKED_OPEN;

	if (*(received_inputs_message + 0) == WEIGHT_SCALE && weight <= 0) {
		*response_inputs_message = WEIGHT;
		statefunc = ready_to_weigh;
	} else if (*(received_inputs_message + 0) == LEFT_CLOSE && weight > 0) {
		*(display_message + 0) = LEFT_CLOSED;
		messageDisplay();
		statefunc = left_door_closed;
	} else {
		statefunc = left_door_opened;
	}
}

void right_door_opened() {
	//fprintf(stderr, "right door opened state %d\n",
	//		*(received_inputs_message + 0));
	rightDoor = UNLOCKED_OPEN;

	if (*(received_inputs_message + 0) == WEIGHT_SCALE) {
		*response_inputs_message = WEIGHT;
		statefunc = ready_to_weigh;
	} else if (*(received_inputs_message + 0) == RIGHT_CLOSE && weight > 0) {
		*(display_message + 0) = RIGHT_CLOSED;
		messageDisplay();
		statefunc = right_door_closed;
	} else {
		statefunc = right_door_opened;
	}
}

void ready_to_weigh() {
	//fprintf(stderr, "ready to weigh state %d, %d\n",
	//		*(received_inputs_message + 0), *(received_inputs_message + 1));
	if ((*(received_inputs_message + 0) == NUM_INPUT + 1)
			&& (*(received_inputs_message + 1) > 0)) {
		weight = *(received_inputs_message + 1);

		// *response_inputs_message = WEIGHT;

		statefunc = person_weighed;

		*(display_message + 0) = WEIGHED;
		*(display_message + 1) = weight;
		messageDisplay();
	} else {
		*response_inputs_message = WEIGHT;
		statefunc = ready_to_weigh;
	}
}

void person_weighed() {
	//fprintf(stderr, "person weighed state %d\n",
	//		*(received_inputs_message + 0));

	if (*(received_inputs_message + 0) == LEFT_CLOSE
			&& leftDoor == UNLOCKED_OPEN) {
		statefunc = left_door_closed;
		rightDoor = LOCKED_SCANNED;

		*(display_message + 0) = LEFT_CLOSED;
		messageDisplay();
	} else if (*(received_inputs_message + 0) == RIGHT_CLOSE
			&& rightDoor == UNLOCKED_OPEN) {
		statefunc = right_door_closed;
		leftDoor = LOCKED_SCANNED;

		*(display_message + 0) = RIGHT_CLOSED;
		messageDisplay();
	} else {
		statefunc = person_weighed;
	}
}

void left_door_closed() {
	//fprintf(stderr, "left door closed state %d\n",
	//		*(received_inputs_message + 0));
	leftDoor = UNLOCKED_CLOSED;

	if (*(received_inputs_message + 0) == GUARD_LEFT_LOCK) {
		statefunc = left_door_locked;

		*(display_message + 0) = LEFT_LOCKED;
		messageDisplay();
	} else {
		statefunc = left_door_closed;
	}
}

void right_door_closed() {
	//fprintf(stderr, "right door closed state %d\n",
	//		*(received_inputs_message + 0));
	rightDoor = UNLOCKED_CLOSED;

	if (*(received_inputs_message + 0) == GUARD_RIGHT_LOCK) {
		statefunc = right_door_locked;

		*(display_message + 0) = RIGHT_LOCKED;
		messageDisplay();
	} else {
		statefunc = right_door_closed;
	}
}

void left_door_locked() {
	//fprintf(stderr, "left door locked state %d\n",
	//		*(received_inputs_message + 0));
	leftDoor = LOCKED_CLOSED;

	if (rightDoor == LOCKED_SCANNED) {
		if (*(received_inputs_message + 0) == GUARD_RIGHT_UNLOCK) {
			statefunc = right_door_unlocked;

			*(display_message + 0) = RIGHT_UNLOCKED;
			messageDisplay();
		} else {
			statefunc = left_door_locked;
		}
	} else {
		weight = -1;
		person_id = -1;
		statefunc = start;
		start();
	}
}

void right_door_locked() {
	//fprintf(stderr, "right door locked state %d\n",
	//		*(received_inputs_message + 0));
	rightDoor = LOCKED_CLOSED;

	if (leftDoor == LOCKED_SCANNED) {
		if (*(received_inputs_message + 0) == GUARD_LEFT_UNLOCK) {
			statefunc = left_door_unlocked;

			*(display_message + 0) = LEFT_UNLOCKED;
			messageDisplay();
		} else {
			statefunc = right_door_locked;
		}
	} else {
		weight = -1;
		person_id = -1;
		statefunc = start;
		start();
	}
}

int main(int argc, char *argv[]) {
	// fprintf(stderr, "The controller starting1");
	// StateFunc statefunc = start;

	display_message = display_message_array;
	//fprintf(stderr, "The controller starting2");
	received_inputs_message = received_inputs_message_array;
	//fprintf(stderr, "The controller starting3");
	response_inputs_message = malloc(sizeof(int));
	*response_inputs_message = -1;

	//fprintf(stderr, "The controller starting4");

	// check number of command line arguments
	if (argc < 2) {
		fprintf(stderr, "Must be at least 2 command line arguments \n");
		exit(EXIT_FAILURE);
	}

	// store server pid for connection
	serverpid = atoi(argv[1]);

	// create a channel for inputs
	inputs_chid = ChannelCreate(0);

	if (inputs_chid == -1) {
		perror("error - cannot create channel");
		exit(EXIT_FAILURE);
	}

	// print pid
	fprintf(stderr, "The controller is running as process_id %d\n", getpid());

	// connect to display server
	display_chid = ConnectAttach(ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL,
			0);
	if (display_chid == -1) {
		fprintf(stderr, "Could not ConnectAttach\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (*(received_inputs_message + 0) != EXIT) {
		*response_inputs_message = START;
		receiveInputs();
		(*statefunc)();
		replyInputs();

		sleep(1);
		// fprintf(stderr, "LDOOR %d, RDOOR %d, ID %d, WEIGHT %d\n", leftDoor, rightDoor, person_id, weight);
	}

	*(display_message + 0) = EXIT;
	messageDisplay();

	sleep(3);

	ChannelDestroy(inputs_chid);
	ConnectDetach(display_chid);

	// fprintf(stderr, "chid %d\n", inputs_chid);
	fprintf(stderr, "Exiting controller\n");

	free(response_inputs_message);
	return EXIT_SUCCESS;
}

void receiveInputs() {
	// get the message from inputs
	rcvid = MsgReceive(inputs_chid, received_inputs_message,
			sizeof(received_inputs_message_array) + 1,
			NULL);
}

void replyInputs() {
	// send a message to inputs
	MsgReply(rcvid, EOK, response_inputs_message, sizeof(int) + 1);
}

void messageDisplay() {
	// send and receive message to display server
	if (MsgSend(display_chid, display_message,
			sizeof(received_inputs_message_array) + 1, display_message,
			sizeof(int[2]) + 1) == -1L) {
		fprintf(stderr, "Error during MsgSend\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}
}
