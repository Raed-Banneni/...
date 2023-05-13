/*
 * mpu6050.h
 *
 *  Created on: 24 Mar 2023
 *      Author: lukachangretta
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include <stdint.h>
#include "main.h"
#include <math.h>
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;
extern int16_t  TEMP_RAW, Accel_X_RAW,Accel_Y_RAW,Accel_Z_RAW,Gyro_X_RAW,Gyro_Y_RAW,Gyro_Z_RAW,gyro_x_offset,gyro_y_offset,gyro_z_offset,accel_x_offset,accel_y_offset,accel_z_offset;
extern uint16_t offset;
extern int calibstatus;
extern float pitch,roll,ALPHA,Accel_mag,TC,Ax,Ay,Az,Gx,Gy,Gz,ax_filtred,ay_filtred,az_filtred,gx_filtred,gy_filtred,gz_filtred,accel_mag_threshold;
#define SMPLRT_DIV	25
#define DLPF_REG	26

#define INT_PIN_CFG 	55
#define INT_OD_AH_CLRRD	0B00010000

#define INT_ENABLE_REG  56
#define DATA_RDY_EN 	1

#define INT_STATUS_REG  58

#define MPU6050_ADDR	0X68
#define GYRO_CONFIG_REG	27
#define FS_GYRO_250		0
#define FS_GYRO_500		8
#define FS_GYRO_1000	9
#define FS_GYRO_2000	10

#define ACCEL_CONFIG_REG 	28
#define FS_ACCEL_2			0
#define FS_ACCEL_4			8
#define FS_ACCEL_8			9
#define FS_ACCEL_16			10

#define POWER_MANAGEMENT_REG 107
#define ACCEL_DATA_REG		 59
#define GYRO_DATA_REG		 67

#define TEMP_OUT			0x41


void MPU6050_init(void);
void accel_read(void);
void gyro_read(void);
void temp_read(void);
void MPU6050_Calibrate(void);
void accelmag(void);
void pitchandroll(float alpha);
int isfall(void);
#endif /* INC_MPU6050_H_ */
