#include <Servo.h>

const int LEFT_FORWARD_PIN = 5;
const int LEFT_BACKWARD_PIN = 6;
const int RIGHT_FORWARD_PIN = 4;
const int RIGHT_BACKWARD_PIN = 3;
const int US_SERVO_PIN = A1;
const int US_THRESHOLD = 100;
const int TRIGGER_PIN = 7;
const int ECHO_PIN = 8;
const int IR_FR_PIN = 14;
const int IR_FL_PIN = 15;
const int IR_BR_PIN = 15;
const int IR_BL_PIN = 15;
const int IR_THRESHOLD = 200;

int distance_front = 0;
unsigned int read_ir_time = 0;
unsigned int read_us_time = 0;
unsigned int move_us_time = 0;
int us_servo_position = 90;
bool ir_fr = false;
bool ir_fl = false;
bool ir_br = false;
bool ir_bl = false;

Servo us_servo;

enum Task {
	INIT_WAIT,
	POSITION_SELF,
	DODGE_EDGE,
	FIND_TARGET,
	CHARGE
};

enum Direction {
	LEFT,
	RIGHT
};

enum Task task = INIT_WAIT;
enum Direction us_servo_direction = RIGHT;

void setup() {
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

void readUSound(int trigger, int echo, int *distance) {
	if (read_us_time < millis() + 10) {
		// Flush trigger pin
		digitalWrite(trigger, LOW);
		delayMicroseconds(2);
		// Activate trigger
		digitalWrite(trigger, HIGH);
		delayMicroseconds(10);
		digitalWrite(trigger, LOW);
		// Read distance and convert to cm
		long duration = pulseIn(echo, HIGH);
		*distance = duration * 0.034 / 2;
	}
}

void readIR(int ir, bool *result) {
	if (read_ir_time < millis() + 50) {
		if (analogRead(ir) > IR_THRESHOLD) {
			*result = false;
		} else {
			*result = true;
		}
	}
}

void moveUSound() {
	if (move_us_time < millis() + 9) {
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
	case INIT_WAIT:
		if (millis() < 3000) {
			delay(1);
		} else {
			task = POSITION_SELF;
			digitalWrite(LEFT_FORWARD_PIN, HIGH);
			digitalWrite(RIGHT_FORWARD_PIN, HIGH);
		}
		break;

	case POSITION_SELF:
		digitalWrite(LEFT_FORWARD_PIN, HIGH);
		digitalWrite(RIGHT_FORWARD_PIN, HIGH);
		delay(1);
		if (millis() > 4000) {
			task = FIND_TARGET;
		}
		break;

	// Avoid edges
	case DODGE_EDGE:
		if (!(ir_fl || ir_fr || ir_bl || ir_br)) {
			task = FIND_TARGET;
		}
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

	// Roter indtil målet er synligt
	case FIND_TARGET:
		//moveUSound();
		//updateUSound();
		digitalWrite(RIGHT_FORWARD_PIN, HIGH);
		digitalWrite(RIGHT_BACKWARD_PIN, LOW);
		digitalWrite(LEFT_BACKWARD_PIN, HIGH);
		digitalWrite(LEFT_FORWARD_PIN, LOW);
		if (distance_front < US_THRESHOLD) {
			task = CHARGE;
		}
		break;
	// Kør hen mod målet så længe det er synligt
	case CHARGE:
		if (distance_front > US_THRESHOLD) {
			task = FIND_TARGET;
		}
		digitalWrite(LEFT_BACKWARD_PIN, LOW);
		digitalWrite(RIGHT_BACKWARD_PIN, LOW);
		digitalWrite(LEFT_FORWARD_PIN, HIGH);
		digitalWrite(RIGHT_FORWARD_PIN, HIGH);
		break;
	}
}
