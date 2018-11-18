#include <Servo.h>

// Constants
// Pins for turning the left and right wheel forwards and backwards
const int LEFT_FORWARD_PIN = 5;
const int LEFT_BACKWARD_PIN = 6;
const int RIGHT_FORWARD_PIN = 4;
const int RIGHT_BACKWARD_PIN = 3;
// pin for turning the servo the ultrasound sensor is attached to
const int US_SERVO_PIN = A1;
// The trigger pin for the ultrasound sensor
const int TRIGGER_PIN = 7;
// The echo pin for the ultrasound sensor
const int ECHO_PIN = 8;
// The pins for the front (F) back (B) right(R) and left(L) infrared sensors
const int IR_FR_PIN = 14;
const int IR_FL_PIN = 15;
const int IR_BR_PIN = 15;
const int IR_BL_PIN = 15;
// Threshold where the ultrasound sensor has detected something
const int US_THRESHOLD = 100;
// Threshold where the infrared sensor has detected something
const int IR_THRESHOLD = 200;

// Global variables
// The distance of the object detected by ultrasound sensor on the front of the
// robot
int distance_front = 0;
// Read times for the infrared and ultrasound sensor
unsigned int read_ir_time = 0;
unsigned int read_us_time = 0;
unsigned int move_us_time = 0;
// Position of the servo moving the ultrasound sensor
int us_servo_position = 90;
// Whether the IR sensors have detected a border
bool ir_fr = false;
bool ir_fl = false;
bool ir_br = false;
bool ir_bl = false;
// The servo moving the ultrasound
Servo us_servo;

// Type definitions
// Enum deciding the robots behaviour
enum Task {
	INIT_WAIT,
	POSITION_SELF,
	DODGE_EDGE,
	FIND_TARGET,
	CHARGE
};

// A direction the servo can move in
enum Direction {
	LEFT,
	RIGHT
};

enum Task task = INIT_WAIT;
enum Direction us_servo_direction = RIGHT;

void setup() {
	// Set pinmodes
	pinMode(LEFT_FORWARD_PIN, OUTPUT);
	pinMode(LEFT_BACKWARD_PIN, OUTPUT);
	pinMode(RIGHT_FORWARD_PIN, OUTPUT);
	pinMode(RIGHT_BACKWARD_PIN, OUTPUT);
	pinMode(US_SERVO_PIN, OUTPUT);

	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);
	pinMode(IR_FL_PIN, INPUT);
	pinMode(IR_FR_PIN, INPUT);
	pinMode(IR_BL_PIN, INPUT);
	pinMode(IR_BR_PIN, INPUT);
	us_servo.attach(US_SERVO_PIN);
	us_servo.write(90);
}

// Read the ultrasound sensor
void readUSound(int trigger, int echo, int *distance) {
	// Only read once every 10 ms
	if (read_us_time < millis() + 10) {
		// The ultrasound's trigger is turned on for 10 microsecond, then turned
		// off. This activates the sensor.
		digitalWrite(trigger, LOW);
		delayMicroseconds(2);
		digitalWrite(trigger, HIGH);
		delayMicroseconds(10);
		digitalWrite(trigger, LOW);

		// Read distance and convert to cm
		long duration = pulseIn(echo, HIGH);
		*distance = duration * 0.034 / 2;
	}
}

// Read the given IR sensor
void readIR(int ir, bool *result) {
	// Only read once per 50 milliseconds
	if (read_ir_time < millis() + 50) {
		// If the read value is above the threshold, return true
		if (analogRead(ir) > IR_THRESHOLD) {
			*result = false;
		} else {
			*result = true;
		}
	}
}

// Move the US servo's target position
void moveUSound() {
	// only move once per 9 milliseconds
	if (move_us_time < millis() + 9) {
		// Move left until position 0, then move right until position 180
		switch (us_servo_direction) {
		case LEFT:
			if (us_servo_position > 0) {
				us_servo_position--;
			} else {
				us_servo_direction = RIGHT;
			}
			break;
		case RIGHT:
			if (us_servo_direction < 180) {
				us_servo_position++;
			} else {
				us_servo_direction = LEFT;
			}
		}
	}
}

// Apply target position to the servo
void updateUSound() {
	us_servo.write(us_servo_position);
}

void loop() {
	// Read sensor values
	readUSound(TRIGGER_PIN, ECHO_PIN, &distance_front);
	readIR(IR_FR_PIN, &ir_fr);
	readIR(IR_FL_PIN, &ir_fl);
	readIR(IR_BR_PIN, &ir_br);
	readIR(IR_BL_PIN, &ir_bl);

	// Check if we need to dodge ledge
	if (ir_fr | ir_fl | ir_bl | ir_br) {
		task = DODGE_EDGE;
		goto perform_task;
	}

	perform_task:
	switch (task) {
	// Wait until start of battle
	case INIT_WAIT:
		if (millis() < 3000) {
			delay(1);
		} else {
			task = POSITION_SELF;
			digitalWrite(LEFT_FORWARD_PIN, HIGH);
			digitalWrite(RIGHT_FORWARD_PIN, HIGH);
		}
		break;

	// Move to safe starting position
	case POSITION_SELF:
		digitalWrite(LEFT_BACKWARD_PIN, HIGH);
		digitalWrite(RIGHT_BACKWARD_PIN, HIGH);
		delay(1);
		// Start searching for target
		if (millis() > 4000) {
			task = FIND_TARGET;
		}
		break;

	// Avoid edges
	case DODGE_EDGE:
		// If none of the sensors detect a ledge, find target
		if (!(ir_fl || ir_fr || ir_bl || ir_br)) {
			task = FIND_TARGET;
		}
		// If a sensor detects a ledge, set corresponding wheel to move away from
		// the target
		if (ir_bl) {
			digitalWrite(LEFT_FORWARD_PIN, HIGH);
			digitalWrite(LEFT_BACKWARD_PIN, LOW);
		}
		if (ir_br) {
			digitalWrite(RIGHT_FORWARD_PIN, HIGH);
			digitalWrite(RIGHT_BACKWARD_PIN, LOW);
		}
		if (ir_fl) {
			digitalWrite(LEFT_BACKWARD_PIN, HIGH);
			digitalWrite(LEFT_FORWARD_PIN, LOW);
		}
		if (ir_fr) {
			digitalWrite(RIGHT_BACKWARD_PIN, HIGH);
			digitalWrite(RIGHT_FORWARD_PIN, LOW);
		}
		break;

	// Rotate the robot until target detected
	case FIND_TARGET:
		//moveUSound();
		//updateUSound();
		// Spin right wheel forward and left wheel back
		digitalWrite(RIGHT_FORWARD_PIN, HIGH);
		digitalWrite(RIGHT_BACKWARD_PIN, LOW);
		digitalWrite(LEFT_BACKWARD_PIN, HIGH);
		digitalWrite(LEFT_FORWARD_PIN, LOW);
		// If target is within visible distance, charge towards it
		if (distance_front < US_THRESHOLD) {
			task = CHARGE;
		}
		break;

	// Charge towards the target
	case CHARGE:
		// If target no longer visible, search for it
		if (distance_front > US_THRESHOLD) {
			task = FIND_TARGET;
		}
		// Move directly forwards
		digitalWrite(LEFT_BACKWARD_PIN, LOW);
		digitalWrite(RIGHT_BACKWARD_PIN, LOW);
		digitalWrite(LEFT_FORWARD_PIN, HIGH);
		digitalWrite(RIGHT_FORWARD_PIN, HIGH);
		break;
	}
}
