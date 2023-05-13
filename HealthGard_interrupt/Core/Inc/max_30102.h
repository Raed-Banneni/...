
#ifndef INC_MAX_30102_H_
#define INC_MAX_30102_H_
#include "main.h"

#define MAX30102_ADDR_WRITE 0xAE
#define MAX30102_ADDR_READ 0xAF

#define RES_INTERRUPT_STATUS_1 0x00
#define RES_INTERRUPT_STATUS_2 0x01
#define RES_INTERRUPT_ENABLE_1 0x02
#define RES_INTERRUPT_ENABLE_2 0x03
#define RES_FIFO_WRITE_POINTER 0x04
#define RES_OVERFLOW_COUNTER 0x05
#define RES_FIFO_READ_POINTER 0x06
#define RES_FIFO_DATA_REGISTER 0x07
#define RES_FIFO_CONFIGURATION 0x08
#define RES_MODE_CONFIGURATION 0x09
#define RES_SPO2_CONFIGURATION 0x0A
#define RES_LED_PLUSE_AMPLITUDE_1 0x0C
#define RES_LED_PLUSE_AMPLITUDE_2 0x0D
#define RES_MULTI_LED_MODE_CONTROL_1 0x11
#define RES_MULTI_LED_MODE_CONTROL_2 0x12
#define RES_DIE_TEMP_INTEGER 0x1F
#define RES_DIE_TEMP_FRACTION 0x20
#define RES_DIE_TEMPERATURE_CONFIG 0x21
#define BUFF_SIZE 50
#define FILTER_LEVEL 8

extern I2C_HandleTypeDef hi2c3;
extern uint8_t heartRate;
extern uint8_t spo2 ;

typedef struct samplestruct
{
    uint32_t red;
    uint32_t iRed;
} SAMPLE;

void MAX30102_Init();
void MAX30102_SetLEDpulses(uint8_t LD1,uint8_t LD2);
void MAX30102_ClearFIFO();
uint8_t MAX30102_getUnreadSampleCount();
void MAX30102_getFIFO(SAMPLE *data, uint8_t sampleCount);
uint8_t MAX30102_getInterruptStatus();
void MAX30102_Calc();
uint8_t max30102_getHeartRate();
uint8_t max30102_getSpO2();
#endif /* INC_MAX_30102_H_ */
