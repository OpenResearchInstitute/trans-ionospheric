/*****************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 * 	@sconklin
 * 	@mustbeart
 * 	@abraxas3d
 *****************************************************************************/
#include "../system.h"

// This uses a deprecated API in the nRF5 SDK to implement a software-based
// I2C (TWI) interface.

// The GPIO pins used for SDA and SCL are defined by macros:
// TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER
// TWI_MASTER_CONFIG_DATA_PIN_NUMBER
// which will need to be set (0 to 31) in an appropriate config file.

bool m_i2c_available = false;
bool m_smeter_available = false;

void util_i2c_init(void) {

	if (twi_master_init()) {
		m_i2c_available = true;
	}
}


bool util_i2c_write(uint8_t address7, uint8_t *data_p, uint8_t data_len) {
	if (m_i2c_available) {
		return twi_master_transfer(address7 << 1, data_p, data_len, TWI_ISSUE_STOP);
	} else {
		return false;
	}
}


bool util_i2c_read(uint8_t address7, uint8_t *data_p, uint8_t data_len) {
	if (m_i2c_available) {
		return twi_master_transfer(TWI_READ_BIT | (address7 << 1), data_p, data_len, TWI_ISSUE_STOP);
	} else {
		return false;
	}
}


//Discover I2C devices
uint8_t util_i2c_count(void) {
	uint8_t count = 0;
	uint8_t buf[2];

	if (m_i2c_available) {
		for (uint8_t addr7 = UTIL_I2C_FIRST_NORMAL_I2C_ADDRESS7; addr7 <= UTIL_I2C_LAST_NORMAL_I2C_ADDRESS7; addr7++) {
			if (util_i2c_read(addr7, buf, 1)) {
				count++;
			}
		}
	}

	return count;
}

uint8_t util_i2c_get_next_device_address(uint8_t start_addr7) {
	uint8_t buf[2];

	if (m_i2c_available) {
		for (uint8_t addr7 = start_addr7; addr7 <= UTIL_I2C_LAST_NORMAL_I2C_ADDRESS7; addr7++) {
			if (util_i2c_read(addr7, buf, 1)) {
				return addr7;
			}
		}
	}

	return UTIL_I2C_INVALID_I2C_ADDRESS7;
}

// 16-bit output to a MC23017 I/O expander via I2C
#define	MC23017	0x20		// 7-bit I2C address for the device (all address pins grounded)

void util_i2c_ioexp_out(uint16_t output_bitmap) {
	uint8_t i2c_config_data[] = { 0x00, 0x00, 0x00 };	// Port A+B config, all outputs
	uint8_t i2c_write_data[] = { 0x12, 0x00, 0x00 };	// Port A+B write

	i2c_write_data[1] = output_bitmap & 0x00FF;
	i2c_write_data[2] = (output_bitmap >> 8) & 0x00FF;

	util_i2c_write(MC23017, &i2c_config_data[0], 3);
	util_i2c_write(MC23017, &i2c_write_data[0], 3);

}


//
// S-meter support for 24 red-green LED bargraph on a HT16K33, a la Adafruit backpack
//
#define	HT16K33	0x70	// Default 7-bit I2C address for the device (no jumpers)

#define BAR_RED		1
#define	BAR_GREEN	2
#define BAR_YELLOW	(BAR_GREEN | BAR_RED)
#define BAR_BLACK	0

// For our S-meter display, the color of each bar is predetermined.
// This function defines it.
// Note that the high end of the bargraph is bar 0, and the low end is 23.
static uint8_t __color_for_bar(uint8_t bar) {

		if (bar < 2) {
			return BAR_RED;
		} else if (bar < 5) {
			return BAR_YELLOW;
		} else {
			return BAR_GREEN;
		}
}

// These two functions encode most of how the bargraph LED modules are
// connected to the HT16K33 in the Adafruit backpack.
//
// Address	(msbit)    Contents     (lsbit)
// =======	===============================
//		0	R15 R14 R13 R12 R03 R02 R01 R00
//		1	G15 G14 G13 G12 G03 G02 G01 G00
//		2	R19 R18 R17 R16 R07 R06 R05 R04
//		3	G19 G18 G17 G16 G07 G06 G05 G04
//		4	R23 R22 R21 R20 R11 R10 R09 R08
//		5	G23 G22 G21 G20 G11 G10 G09 G08
//	   6+	no LEDs connected to any of these
//
static uint8_t __byte_for_bar(uint8_t bar) {
	if (bar < 12) {
		return (bar/4)*2 + 1;	// +1 accounts for write register address in buffer
	} else {
		return ((bar-12)/4)*2 + 1;
	}
}

static uint8_t __mask_for_bar(uint8_t bar) {
	if (bar < 12) {
		return 0x01 << (bar % 4);
	} else {
		return 0x10 << (bar % 4);
	}
}

// Set up the HT16K33 device to standard configuration.
// Returns true if initialization succeeded
bool util_i2c_smeter_start(void) {
	uint8_t i2c_init_data0[] = { 0x21 };	// turn oscillator on
	uint8_t i2c_init_data1[] = { 0x81 };	// blinking off
	uint8_t i2c_init_data2[] = { 0xEF };	// max brightness
	uint8_t i2c_init_data3[17];

	memset(i2c_init_data3, 0, 17);	// initialize RAM to all zeroes (LEDs off)

	m_smeter_available = util_i2c_write(HT16K33, &i2c_init_data0[0], 1) &&
						util_i2c_write(HT16K33, &i2c_init_data1[0], 1) &&
						util_i2c_write(HT16K33, &i2c_init_data2[0], 1) &&
						util_i2c_write(HT16K33, &i2c_init_data3[0], 17);

	return m_smeter_available;
}

// Disable the HT16K33 device with all LEDs off.
void util_i2c_smeter_stop(void) {
	uint8_t i2c_init_data0[] = { 0x20 };	// turn oscillator off
	uint8_t i2c_init_data1[] = { 0x80 };	// blinking off, disable LEDs

	util_i2c_write(HT16K33, &i2c_init_data1[0], 1);
	util_i2c_write(HT16K33, &i2c_init_data0[0], 1);

	m_smeter_available = false;
}

// Write data to the S-meter bargraph.
//   level = 0 to 24, 0 = dark, 24 = all bars on
void util_i2c_smeter_write(uint8_t level) {
	uint8_t i2c_write_buffer[7] = {
		0,		// command to begin writing RAM at address 0
		0, 0, 0, 0, 0, 0	// first six bytes spans 24 bicolor LEDs
		// there are ten more bytes in the chip's RAM, but those
		// correspond to LEDs that don't exist.
	};

	if (level > 24) {
		level = 24;
	}

	if (level > 0) {
		for (uint8_t i=23; 24-i < level; i--) {
			uint8_t color = __color_for_bar(i);
			if ((color & BAR_RED) != 0) {
				i2c_write_buffer[__byte_for_bar(i)] |= __mask_for_bar(i);
			}
			if ((color & BAR_GREEN) != 0) {
				i2c_write_buffer[1+__byte_for_bar(i)] |= __mask_for_bar(i);
			}
		}
	}

	// Finally dump the data to the device
	util_i2c_write(HT16K33, &i2c_write_buffer[0], 7);
	// No need to write the other ten bytes of RAM.
}
