#ifndef DES_H_
#define DES_H_

#define NUM_PROMPTS 3
typedef enum {
	START = 0,
	SCAN = 1,
	WEIGHT = 2
} Prompt;

const char *promptStrings[NUM_PROMPTS] = {
	"\nEnter the event type:\n"
	"ls= left scan, rs= right scan\n"
	"glu = guard left unlock, gru = guard right unlock\n"
	"lo =left open, ro=right open\n"
	"ws= weight scale\n"
	"lc = left closed, rc = right closed\n"
	"gll=guard left lock, grl = guard right lock\n"
	"exit = exit programs\n",
	"\nEnter Person's ID: \n",
	"\nEnter the Person's weight: \n"
};

#define NUM_OUTPUT 10
typedef enum {
	SCANNED = 0,
	LEFT_UNLOCKED = 1,
	RIGHT_UNLOCKED = 2,
	LEFT_OPENED = 3,
	RIGHT_OPENED = 4,
	WEIGHED = 5,
	LEFT_CLOSED = 6,
	RIGHT_CLOSED = 7,
	LEFT_LOCKED = 8,
	RIGHT_LOCKED = 9
} Output;

const char *outputStrings[NUM_OUTPUT] = {
	"Person scanned ID, ID = ",
	"Left door unlocked by Guard",
	"Right door unlocked by Guard",
	"Person opened the left door",
	"Person opened the right door",
	"Person weighed, weight = ",
	"Left door closed (automatically)",
	"Right door closed (automatically)",
	"Left door locked by Guard",
	"Right door locked by Guard"
};

#define NUM_INPUT 12
typedef enum {
	LEFT_SCAN = 0,
	RIGHT_SCAN = 1,
	GUARD_LEFT_UNLOCK = 2,
	GUARD_RIGHT_UNLOCK = 3,
	LEFT_OPEN = 4,
	RIGHT_OPEN = 5,
	WEIGHT_SCALE = 6,
	LEFT_CLOSE = 7,
	RIGHT_CLOSE = 8,
	GUARD_LEFT_LOCK = 9,
	GUARD_RIGHT_LOCK = 10,
	EXIT = 11
} Input;

const char *inputStrings[NUM_INPUT] = {
	"ls",
	"rs",
	"glu",
	"gru",
	"lo",
	"ro",
	"ws",
	"lc",
	"rc",
	"gll",
	"grl",
	"exit"
};


#define NUM_DOOR 4
typedef enum {
	LOCKED_CLOSED = 0,
	LOCKED_SCANNED = 1,
	UNLOCKED_CLOSED = 2,
	UNLOCKED_OPEN = 3
} Door;

#endif /* DES_H_ */
