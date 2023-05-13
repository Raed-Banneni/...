
#include "mpu6050.h"

void MPU6050_init(void)
{
	HAL_StatusTypeDef ret= HAL_I2C_IsDeviceReady(&hi2c1,(MPU6050_ADDR<<1)+0,1,100	);
	// GYRO scale range CONFIG
	uint8_t	temp_data = FS_GYRO_250;
	ret = HAL_I2C_Mem_Write(&hi2c1,(MPU6050_ADDR<<1)+0,GYRO_CONFIG_REG,1,&temp_data,1,100);
	// activate dlpf
	temp_data=2;
	ret = HAL_I2C_Mem_Write(&hi2c1,(MPU6050_ADDR<<1)+0,DLPF_REG,1,&temp_data,1,100);
	//set sample rate to 100 khz
	temp_data=9;
	ret = HAL_I2C_Mem_Write(&hi2c1,(MPU6050_ADDR<<1)+0,SMPLRT_DIV,1,&temp_data,1,100);
	// ACCEL scale range CONFIG
	temp_data = FS_ACCEL_16;
	ret = HAL_I2C_Mem_Write(&hi2c1,(MPU6050_ADDR<<1)+0,ACCEL_CONFIG_REG,1,&temp_data,1,100);
	//Exit sleep mode and enable temperature sensor
	temp_data= 0;
	ret = HAL_I2C_Mem_Write(&hi2c1,(MPU6050_ADDR<<1)+0,POWER_MANAGEMENT_REG,1,&temp_data,1,100);

}
void accel_read(void)
{

	uint8_t data[6];
	HAL_I2C_Mem_Read(&hi2c1,(MPU6050_ADDR<<1)+1,ACCEL_DATA_REG,1,data,6,100);

	Accel_X_RAW = (int16_t)(data[0] << 8 | data [1]);
	Accel_Y_RAW = (int16_t)(data[2] << 8 | data [3]);
	Accel_Z_RAW = (int16_t)(data[4] << 8 | data [5]);

	/*if ((Accel_X_RAW/2048.0)<1 && ((Accel_X_RAW/2048.0))>0 && (Accel_Y_RAW/2048.0)<1 && (Accel_Y_RAW/2048.0)>0)
	{*/
		Ax = (Accel_X_RAW/2048.0)-(accel_x_offset);
		Ay = (Accel_Y_RAW/2048.0)-(accel_y_offset);
		Az = (Accel_Z_RAW/2048.0)-(accel_z_offset);
//}
	/*else*/ if ((Accel_X_RAW/2048.0)<1 && (Accel_X_RAW/2048.0)>0 && (Accel_Z_RAW/2048.0)<1 && (Accel_Z_RAW/2048.0)>0)
	{
		Ax = (Accel_X_RAW/2048.0);
		Ay = (Accel_Y_RAW/2048.0)-offset;
		Az = (Accel_Z_RAW/2048.0);
	}
	else if ((Accel_Z_RAW/2048.0)<1 && (Accel_Z_RAW/2048.0)>0 && (Accel_Z_RAW/2048.0)<1 && (Accel_Z_RAW/2048.0)>0)
	{
		Ax = (Accel_X_RAW/2048.0)-offset;
		Ay = (Accel_Y_RAW/2048.0);
		Az = (Accel_Z_RAW/2048.0);
	}
	else
	{
		Ax = (Accel_X_RAW/2048.0);
		Ay = (Accel_Y_RAW/2048.0);
		Az = (Accel_Z_RAW/2048.0)-offset;
	}
/*
	Ax = (Ax < 0.8) ? 0 : Ax;
	Ay = (Ay < 0.8) ? 0 : Ay;
	Az = (Az < 0.8) ? 0 : Az;*/
	Ax = ((Ax < 1.5) & (Ax > 0.9)) ? 1 : Ax;
	Ay = ((Ay < 1.5) & (Ay > 0.9)) ? 1 : Ay;
	Az = ((Az < 1.5) & (Az > 0.9)) ? 1 : Az;

	// filter accel readings
	ax_filtred=ALPHA * Ax + (1 - ALPHA) * ax_filtred;
	ay_filtred=ALPHA * Ay + (1 - ALPHA) * ay_filtred;
	az_filtred=ALPHA * Az + (1 - ALPHA) * az_filtred;
	// calculate accel magnitude

}
void gyro_read(void)
{
	uint8_t data[6];
	HAL_I2C_Mem_Read(&hi2c1,(MPU6050_ADDR<<1)+1,GYRO_DATA_REG,1,data,6,100);
	Gyro_X_RAW = (int16_t)(data[0] << 8 | data [1]);
	Gyro_Y_RAW = (int16_t)(data[2] << 8 | data [3]);
	Gyro_Z_RAW = (int16_t)(data[4] << 8 | data [5]);
	Gx = (Gyro_X_RAW/131.5)-gyro_x_offset;
	Gy = (Gyro_Y_RAW/131.5)-gyro_y_offset -3 ;
	Gz = (Gyro_Z_RAW/131.5)-gyro_z_offset;
/*
	Gx = (Gx < 0.5) ? 0 : Gx;
	Gy = (Gy < 0.5) ? 0 : Gy;
	Gz = (Gz < 0.5) ? 0 : Gz;
*/
	//filter gyro readings using EMA filter
	gx_filtred=ALPHA * Gx + (1 - ALPHA) * gx_filtred;
	gy_filtred=ALPHA * Gy + (1 - ALPHA) * gy_filtred;
	gz_filtred=ALPHA * Gz + (1 - ALPHA) * gz_filtred;

}

void temp_read(void)
{
	uint8_t data[2];
	HAL_I2C_Mem_Read(&hi2c1,(MPU6050_ADDR<<1)+1,GYRO_DATA_REG,1,data,2,100);
	TEMP_RAW = (int16_t)(data[0] << 8 | data [1]);
	TC = (TEMP_RAW/340)+36.53;
}
void MPU6050_Calibrate(void)
{

	uint8_t data[6];
	// accel calibration
	int32_t accel_x_total = 0;
	int32_t accel_y_total = 0;
    int32_t accel_z_total = 0;
	for (int i=0;i<2000;i++){


		HAL_I2C_Mem_Read(&hi2c1,(MPU6050_ADDR<<1)+1,ACCEL_DATA_REG,1,data,6,100);
		Accel_X_RAW = (int16_t)(data[0] << 8 | data [1]);
		Accel_Y_RAW = (int16_t)(data[2] << 8 | data [3]);
		Accel_Z_RAW = (int16_t)(data[4] << 8 | data [5]);
		Ax = (Accel_X_RAW/2048.0);
		Ay = (Accel_Y_RAW/2048.0);
		Az = (Accel_Z_RAW/2048.0);
		accel_x_total +=Ax;
		accel_y_total +=Ay;
		accel_z_total +=Az;

	 }
	 accel_x_offset = accel_x_total / 2000;
	 accel_y_offset = accel_y_total / 2000;
	 accel_z_offset = accel_z_total / 2000;

	 // find the normal axe offset
	 offset = accel_x_offset ? accel_x_offset : accel_y_offset ? accel_y_offset : accel_z_offset ? accel_z_offset :0;
	 // reset readings
	 Ax = 0;
	 Ay = 0;
	 Az = 0;

	 // gyro calibration
     int32_t gyro_x_total = 0;
     int32_t gyro_y_total = 0;
	 int32_t gyro_z_total = 0;

	 for (int i=0;i<1000;i++){

	 	HAL_I2C_Mem_Read(&hi2c1,(MPU6050_ADDR<<1)+1,GYRO_DATA_REG,1,data,6,100);
	 	Gyro_X_RAW = (int16_t)(data[0] << 8 | data [1]);
	 	Gyro_Y_RAW = (int16_t)(data[2] << 8 | data [3]);
	 	Gyro_Z_RAW = (int16_t)(data[4] << 8 | data [5]);
	 	Gx = Gyro_X_RAW/131.5;
	 	Gy = Gyro_Y_RAW/131.5;
	 	Gz = Gyro_Z_RAW/131.5;
	    gyro_x_total += Gx;
	 	gyro_y_total += Gy;
        gyro_z_total += Gz;

	 }
	 gyro_x_offset = Gx / 1000;
	 gyro_y_offset = Gy / 1000;
	 gyro_z_offset = Gz / 1000;

	 //reset readings
	 Gx=0;
	 Gy=0;
	 Gz=0;
}
// calculate acceleration magnitude and gyro angle
void magandangle(void)
{
	Accel_mag=sqrt(pow(ax_filtred,2)+pow(ay_filtred,2)+pow(az_filtred,2));
}
void pitchandroll(float alpha) // alpha is the complementary filter coefficient
{
	float dt =0.001;
	 // complemontary filter .... check chat pfa discussion ....
	 // Calculate pitch and roll angles from accelerometer data
	  pitch = atan2(ax_filtred, sqrt(ay_filtred * ay_filtred + az_filtred * az_filtred)) * 180 / M_PI;
	  roll = atan2(ay_filtred, sqrt(ax_filtred * ax_filtred + az_filtred * az_filtred)) * 180 / M_PI;

	  // Integrate gyroscope data to get change in pitch and roll angles
	  pitch += gx_filtred * dt;
	  roll -= gy_filtred * dt;

	  // Combine accelerometer and gyroscope data using complementary filter
	  pitch = alpha * pitch + (1 - alpha) * atan2(ax_filtred, sqrt(ay_filtred * ay_filtred + az_filtred * az_filtred)) * 180 / M_PI;
	  roll = alpha * roll + (1 - alpha) * atan2(ay_filtred, sqrt(ax_filtred * ax_filtred + az_filtred * az_filtred)) * 180 / M_PI;
}
int isfall(void)
{
	if ((Accel_mag > accel_mag_threshold) && (fabs(roll<10)))
		return 1;
	else
		return 0;
}
