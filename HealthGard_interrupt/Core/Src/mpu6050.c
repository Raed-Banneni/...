#include "mpu6050.h"

void MPU6050_init(void) {
	HAL_StatusTypeDef ret;
	uint8_t temp_data;
	int retry_count = 0;
	int max_retries = 10;

	while (retry_count < max_retries) {
		//check device address
		ret = HAL_I2C_IsDeviceReady(&hi2c1, (MPU6050_ADDR << 1) + 0, 1, 100);
		if (ret != HAL_OK) {
			retry_count++;
			continue;
		}
		//set int pin mode to active and open drain and activate the clear int flags on every read mode
		temp_data = INT_OD_AH_CLRRD;
		ret = HAL_I2C_Mem_Write(&hi2c1, (MPU6050_ADDR << 1) + 0, INT_PIN_CFG, 1,
				&temp_data, 1, 100);
		if (ret != HAL_OK) {
			retry_count++;
			continue;
		}
		//enable data ready  interrupt
		temp_data = DATA_RDY_EN;
		ret = HAL_I2C_Mem_Write(&hi2c1, (MPU6050_ADDR << 1) + 0, INT_ENABLE_REG,
				1, &temp_data, 1, 100);
		if (ret != HAL_OK) {
			retry_count++;
			continue;
		}

		// Set gyro sensitivity
		temp_data = FS_GYRO_250;
		ret = HAL_I2C_Mem_Write(&hi2c1, (MPU6050_ADDR << 1) + 0,
		GYRO_CONFIG_REG, 1, &temp_data, 1, 100);
		if (ret != HAL_OK) {
			retry_count++;
			continue;
		}
		// active digital low pass filter
		temp_data = 2;
		ret = HAL_I2C_Mem_Write(&hi2c1, (MPU6050_ADDR << 1) + 0, DLPF_REG, 1,
				&temp_data, 1, 100);
		if (ret != HAL_OK) {
			retry_count++;
			continue;
		}
		// set sample rate
		temp_data = 9;
		ret = HAL_I2C_Mem_Write(&hi2c1, (MPU6050_ADDR << 1) + 0, SMPLRT_DIV, 1,
				&temp_data, 1, 100);
		if (ret != HAL_OK) {
			retry_count++;
			continue;
		}
		// set accel full scale range
		temp_data = FS_ACCEL_16;
		ret = HAL_I2C_Mem_Write(&hi2c1, (MPU6050_ADDR << 1) + 0,
		ACCEL_CONFIG_REG, 1, &temp_data, 1, 100);
		if (ret != HAL_OK) {
			retry_count++;
			continue;
		}
		//set the power mode to normal
		temp_data = 0;
		ret = HAL_I2C_Mem_Write(&hi2c1, (MPU6050_ADDR << 1) + 0,
		POWER_MANAGEMENT_REG, 1, &temp_data, 1, 100);
		if (ret != HAL_OK) {
			retry_count++;
			continue;
		}

		// Initialization successful
		break;
	}

	if (retry_count == max_retries) {
		// Maximum number of retries reached, report an error
		send_USART2("Error initializing MPU6050\n");
	}
}

void accel_read(void) {
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t data[6];
	int retry_count = 0;
	int MAX_RETRY_COUNT = 5;

	while (retry_count < MAX_RETRY_COUNT) {
		ret = HAL_I2C_Mem_Read(&hi2c1, (MPU6050_ADDR << 1) + 1, ACCEL_DATA_REG,
				1, data, 6, 100);
		// if data read successfully, calculate the acceleration values
		if (ret == HAL_OK) {
			// extract raw acceleration data from the buffer
			Accel_X_RAW = (int16_t) (data[0] << 8 | data[1]);
			Accel_Y_RAW = (int16_t) (data[2] << 8 | data[3]);
			Accel_Z_RAW = (int16_t) (data[4] << 8 | data[5]);
			// calculate the acceleration values in g (where 1g = 9.81 m/s^2)
			Ax = (Accel_X_RAW / 2048.0) - (accel_x_offset);
			Ay = (Accel_Y_RAW / 2048.0) - (accel_y_offset);
			Az = (Accel_Z_RAW / 2048.0) - (accel_z_offset);
			// check if the accelerometer is lying flat on the XZ plane
			if ((Accel_X_RAW / 2048.0) < 1 && (Accel_X_RAW / 2048.0) > 0
					&& (Accel_Z_RAW / 2048.0) < 1
					&& (Accel_Z_RAW / 2048.0) > 0) {
				// if so, set the X and Z acceleration to zero
				Ax = (Accel_X_RAW / 2048.0);
				Ay = (Accel_Y_RAW / 2048.0) - offset;
				Az = (Accel_Z_RAW / 2048.0);
			} else if ((Accel_Z_RAW / 2048.0) < 1 && (Accel_Z_RAW / 2048.0) > 0
					&& (Accel_Z_RAW / 2048.0) < 1
					&& (Accel_Z_RAW / 2048.0) > 0) {
				// if not, check if the accelerometer is lying flat on the YZ plane
				// if so, set the Y and Z acceleration to zero
				Ax = (Accel_X_RAW / 2048.0) - offset;
				Ay = (Accel_Y_RAW / 2048.0);
				Az = (Accel_Z_RAW / 2048.0);
			} else {
				// otherwise, the accelerometer is oriented in default way (XY plane)
				// and all three acceleration components are valid
				Ax = (Accel_X_RAW / 2048.0);
				Ay = (Accel_Y_RAW / 2048.0);
				Az = (Accel_Z_RAW / 2048.0) - offset;
			}
			// clamp the acceleration values to 1 if they are within a certain range
//			Ax = ((Ax < 1.5) & (Ax > 0.9)) ? 1 : Ax;
//			Ay = ((Ay < 1.5) & (Ay > 0.9)) ? 1 : Ay;
//			Az = ((Az < 1.5) & (Az > 0.9)) ? 1 : Az;
			// apply a low-pass filter to the acceleration values to reduce noise
			ax_filtred = ALPHA * Ax + (1 - ALPHA) * ax_filtred;
			ay_filtred = ALPHA * Ay + (1 - ALPHA) * ay_filtred;
			az_filtred = ALPHA * Az + (1 - ALPHA) * az_filtred;
			break;
		} else {
			//Handle i2c error
			retry_count++;
			if (retry_count == MAX_RETRY_COUNT) {
						// Maximum number of retries reached, report an error
						send_USART2("Error in accel reading\n");
					}
		}
	}

}
void gyro_read(void) {
	uint8_t data[6];
	HAL_StatusTypeDef ret;
	int retry_count = 0;
	int MAX_RETRY_COUNT = 5;
	while (retry_count < MAX_RETRY_COUNT) {
		ret = HAL_I2C_Mem_Read(&hi2c1, (MPU6050_ADDR << 1) + 1, GYRO_DATA_REG,
				1, data, 6, 100);

		if (ret == HAL_OK) {
			// Read was successful
			Gyro_X_RAW = (int16_t) (data[0] << 8 | data[1]);
			Gyro_Y_RAW = (int16_t) (data[2] << 8 | data[3]);
			Gyro_Z_RAW = (int16_t) (data[4] << 8 | data[5]);

			Gx = (Gyro_X_RAW / 131.5) - gyro_x_offset;
			Gy = (Gyro_Y_RAW / 131.5) - gyro_y_offset;
			Gz = (Gyro_Z_RAW / 131.5) - gyro_z_offset;

			// Filter gyro readings using EMA filter
			gx_filtred = ALPHA * Gx + (1 - ALPHA) * gx_filtred;
			gy_filtred = ALPHA * Gy + (1 - ALPHA) * gy_filtred;
			gz_filtred = ALPHA * Gz + (1 - ALPHA) * gz_filtred;

			break; // Exit retry loop
		} else {
			// Handle I2C read error
			retry_count++;
		}
	}
	if (retry_count == MAX_RETRY_COUNT) {
			// Maximum number of retries reached, report an error
			send_USART2("Error in gyro readings \n");
		}
}
void MPU6050_Calibrate(void) {
    uint8_t data[6];
    int32_t accel_x_total = 0;
    int32_t accel_y_total = 0;
    int32_t accel_z_total = 0;

    // Accelerometer calibration
    for (int i = 0; i < 2000; i++) {
        int retry_count = 0;
        while (retry_count < 3) {
            if (HAL_I2C_Mem_Read(&hi2c1, (MPU6050_ADDR << 1) + 1, ACCEL_DATA_REG, 1,
                                 data, 6, 100) == HAL_OK) {
                break;  // Read successful, exit retry loop
            }
            retry_count++;

            if (retry_count == 3) {
            		// Maximum number of retries reached, report an error
            		send_USART2("Error initializing accel\n");
            	}
        }
        if (retry_count >= 3) {
            // Failed to read, handle the error (e.g., log or return an error code)
            // Add your error handling code here
            return;
        }

        // Process the read data
        Accel_X_RAW = (int16_t) (data[0] << 8 | data[1]);
        Accel_Y_RAW = (int16_t) (data[2] << 8 | data[3]);
        Accel_Z_RAW = (int16_t) (data[4] << 8 | data[5]);
        Ax = (Accel_X_RAW / 2048.0);
        Ay = (Accel_Y_RAW / 2048.0);
        Az = (Accel_Z_RAW / 2048.0);
        accel_x_total += Ax;
        accel_y_total += Ay;
        accel_z_total += Az;
    }

    accel_x_offset = accel_x_total / 2000;
    accel_y_offset = accel_y_total / 2000;
    accel_z_offset = accel_z_total / 2000;

    // Find the normal axe offset
    offset = accel_x_offset ? accel_x_offset : accel_y_offset ? accel_y_offset : accel_z_offset ? accel_z_offset : 0;

    // Reset readings
    Ax = 0;
    Ay = 0;
    Az = 0;

    // Gyro calibration
    int32_t gyro_x_total = 0;
    int32_t gyro_y_total = 0;
    int32_t gyro_z_total = 0;

    for (int i = 0; i < 1000; i++) {
        int retry_count = 0;
        while (retry_count < 3) {
            if (HAL_I2C_Mem_Read(&hi2c1, (MPU6050_ADDR << 1) + 1, GYRO_DATA_REG, 1,
                                 data, 6, 100) == HAL_OK) {
                break;  // Read successful, exit retry loop
            }
            retry_count++;
            if (retry_count == 3) {
            		// Maximum number of retries reached, report an error
            		send_USART2("Error While calibrating Gyro\n");
            	}
        }
        Gyro_X_RAW = (int16_t) (data[0] << 8 | data[1]);
		Gyro_Y_RAW = (int16_t) (data[2] << 8 | data[3]);
		Gyro_Z_RAW = (int16_t) (data[4] << 8 | data[5]);
		Gx = Gyro_X_RAW / 131.5;
		Gy = Gyro_Y_RAW / 131.5;
		Gz = Gyro_Z_RAW / 131.5;
		gyro_x_total += Gx;
		gyro_y_total += Gy;
		gyro_z_total += Gz;

	}
	gyro_x_offset = Gx / 1000;
	gyro_y_offset = Gy / 1000;
	gyro_z_offset = Gz / 1000;

	//reset readings
	Gx = 0;
	Gy = 0;
	Gz = 0;
	// set the calib status flag to indicate that calibration has terminated successfully
	calibstatus=1;

}
// calculate acceleration magnitude and gyro angle
void accelmag(void) {
	Accel_mag = sqrt(
			pow(ax_filtred, 2) + pow(ay_filtred, 2) + pow(az_filtred, 2));
}
void pitchandroll(float alpha) // alpha is the complementary filter coefficient
{
	float dt = 0.001;
	// complemontary filter .... check chat pfa discussion ....
	// Calculate pitch and roll angles from accelerometer data
	pitch = atan2(ax_filtred,
			sqrt(ay_filtred * ay_filtred + az_filtred * az_filtred))
			* 180/ M_PI;
	roll = atan2(ay_filtred,
			sqrt(ax_filtred * ax_filtred + az_filtred * az_filtred))
			* 180/ M_PI;

	// Integrate gyroscope data to get change in pitch and roll angles
	pitch += gx_filtred * dt;
	roll -= gy_filtred * dt;

	// Combine accelerometer and gyroscope data using complementary filter
	pitch =
			alpha
					* pitch+ (1 - alpha) * atan2(ax_filtred, sqrt(ay_filtred * ay_filtred + az_filtred * az_filtred)) * 180 / M_PI;
	roll =
			alpha
					* roll+ (1 - alpha) * atan2(ay_filtred, sqrt(ax_filtred * ax_filtred + az_filtred * az_filtred)) * 180 / M_PI;
}
int isfall(void) {
	if ((Accel_mag > accel_mag_threshold) && (fabs(roll < 10)))
		return 1;
	else
		return 0;
}
