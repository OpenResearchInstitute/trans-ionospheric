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

 #ifndef UTIL_I2C_H_
 #define UTIL_I2C_H_

#define	UTIL_I2C_FIRST_NORMAL_I2C_ADDRESS7	0x03
#define UTIL_I2C_LAST_NORMAL_I2C_ADDRESS7	0x77
#define	UTIL_I2C_INVALID_I2C_ADDRESS7		0xFF

extern bool m_i2c_available;
extern bool m_smeter_available;

extern void util_i2c_init(void);
extern bool util_i2c_write(uint8_t address7, uint8_t *data_p, uint8_t data_len);
extern bool util_i2c_read(uint8_t address7, uint8_t *data_p, uint8_t data_len);

//Discover I2C devices
extern uint8_t util_i2c_count(void);
extern uint8_t util_i2c_get_next_device_address(uint8_t start_addr7);

//Support for specific I2C devices
extern void util_i2c_ioexp_out(uint16_t output_bitmap);

// S-meter support for 24 red-green LED bargraph on a HT16K33, a la Adafruit backpack
// Returns true if initialization succeeded
extern bool util_i2c_smeter_start(void);
extern void util_i2c_smeter_stop(void);

// Write data to the S-meter bargraph.
//   level = 0 to 24, 0 = dark, 24 = all bars on
extern void util_i2c_smeter_write(uint8_t level);

#endif /* UTIL_I2C_H_ */
