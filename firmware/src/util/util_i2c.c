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
