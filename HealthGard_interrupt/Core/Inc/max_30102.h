
#ifndef INC_MAX_30102_H_
#define INC_MAX_30102_H_
#include "main.h"

#define MAX30102_ADDR_WRITE 0xAE
#define MAX30102_ADDR_READ 0xAF

#define INTERRUPT_STATUS1R 0x00
#define INTERRUPT_STATUS2R 0x01
#define INTERRUPT_EN1R 0x02
#define INTERRUPT_EN2R 0x03
#define FIFO_WR_PTR 0x04
#define OVF_COUNTER 0x05
#define FIFO_RD_PTR 0x06
#define FIFO_DATA_REG 0x07
#define MODE_CONFIGR 0x09
#define SPO2_CONFIGR 0x0A
#define LED_PULSEAMP1R 0x0C
#define LED_PULSE_AMP2R 0x0D
#define BUFF_SIZE 50
#define FILTER_LEVEL 8
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c3;
extern uint8_t heartRate;
extern uint8_t spo2;

typedef struct samplestruct
{
    uint32_t red;
    uint32_t iRed;
} SAMPLE;

void MAX30102_Init();
void MAX30102_SetLEDpulses(uint8_t LD1,uint8_t LD2);
void MAX30102_ClearFIFO();
void MAX30102_SHDN_CTRL();
uint8_t MAX30102_getUnreadSampleCount();
void MAX30102_getFIFO(SAMPLE *data, uint8_t sampleCount);
uint8_t MAX30102_getInterruptStatus();
void MAX30102_Calc();
uint8_t max30102_getHeartRate();
uint8_t max30102_getSpO2();
#endif /* INC_MAX_30102_H_ */
