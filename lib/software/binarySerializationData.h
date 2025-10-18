#ifndef BINARYSERIALIZATIONDATA_H
#define BINARYSERIALIZATIONDATA_H

#include <cstdint>
#include <cstddef>

#pragma pack(push, 1)

struct BinarySerializationData {
	// Pico modified variables
	int16_t yaw;
	uint8_t floor_color;
	uint16_t motor_distance;
	uint16_t front_wall;
	uint16_t left_wall;
	uint16_t right_wall;
	bool start;

	float _test_pid_error_sum;

	// Zero modified variables
	uint16_t motor_speed_target;
	int16_t motor_facing_target;
	uint8_t motor_move_form; // 2 -> Forward, 1 -> Rotate, 0 -> stop
	bool reset_distance;
	bool reset_pid;

	bool servo_active;
	bool kicker_active;
};

#pragma pack(pop)

const size_t BINARY_SERIALIZATION_DATA_SIZE = sizeof(BinarySerializationData);

#endif //BINARYSERIALIZATIONDATA_H
