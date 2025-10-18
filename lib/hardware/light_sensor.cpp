#include "light_sensor.h"

Light_Sensor::Light_Sensor() {};

void Light_Sensor::begin(BinarySerializationData& data) {
	dataPtr = &data;

	spi_init(MCP_SPI_PORT, 1000 * 1000);
	spi_set_format(MCP_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

	gpio_set_function(MCP_SPI_RX, GPIO_FUNC_SPI);
	gpio_set_function(MCP_SPI_SCK, GPIO_FUNC_SPI);
	gpio_set_function(MCP_SPI_TX, GPIO_FUNC_SPI);

	gpio_init(MCP_SPI_CSN);
	gpio_set_dir(MCP_SPI_CSN, GPIO_OUT);
	gpio_put(MCP_SPI_CSN, true);

	ldr_readings.resize(NUM_LDR_SENSORS);
	previous_read = time_us_64() / 1000;
}

int Light_Sensor::getValue(uint8_t channel) {
	if (channel > 7) return 0;

	uint8_t tx_buf[3];
	uint8_t rx_buf[3];

	tx_buf[0] = 0x01; // Start bit
	tx_buf[1] = (0x08 | channel) << 4; // Single-Ended + Channel Number
	tx_buf[2] = 0x00; // Dummy byte

	gpio_put(MCP_SPI_CSN, false);
	sleep_us(1);

	spi_write_read_blocking(MCP_SPI_PORT, tx_buf, rx_buf, 3);

	sleep_us(1);
	gpio_put(MCP_SPI_CSN, true);

	const int result = ((rx_buf[1] & 0x03) << 8) | rx_buf[2];
	return result;
}

int Light_Sensor::blue() {
	return ldr_readings.at(4);
}

int Light_Sensor::orange() {
	return ldr_readings.at(5);
}

int Light_Sensor::red() {
	return ldr_readings.at(1);
}

int Light_Sensor::green() {
	return ldr_readings.at(0);
}

int Light_Sensor::white(int position) {
	switch (position) {
		case 0:
			return ldr_readings.at(2);
		case 1:
			return ldr_readings.at(3);
		case 2:
			return ldr_readings.at(7);
		case 3:
			return ldr_readings.at(6);
		default:
			return 0;
	}
}

int Light_Sensor::white_average() {
	return (white(0) + white(3)) / 2;
}



void Light_Sensor::tick() {
	unsigned long long timestamp_ms = time_us_32();
	if (timestamp_ms - previous_read >= 10) {
		uint16_t value = getValue(current_channel);

		ldr_readings[current_channel] = value;

		current_channel++;
		if (current_channel >= NUM_LDR_SENSORS) {
			current_channel = 0;
		}

		previous_read = timestamp_ms;
	}
}