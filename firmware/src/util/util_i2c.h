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


void util_i2c_init(void);
bool util_i2c_write(uint8_t address7, uint8_t *data_p, uint8_t data_len);
bool util_i2c_read(uint8_t address7, uint8_t *data_p, uint8_t data_len);

//Discover I2C devices
uint8_t util_i2c_count(void);
uint8_t util_i2c_get_next_device_address(uint8_t start_addr7);

#endif /* UTIL_I2C_H_ */
