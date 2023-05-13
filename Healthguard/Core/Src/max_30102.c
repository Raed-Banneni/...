#include "max_30102.h"


HAL_StatusTypeDef ret;
void MAX30102_Init()
{
    uint8_t data = 0;
    data = 0x40;
    HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_MODE_CONFIGURATION, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
    do
    {
        ret = HAL_I2C_Mem_Read(&hi2c2, MAX30102_ADDR_READ, RES_MODE_CONFIGURATION, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
    } while (ret != HAL_OK);
    data = 0x40;
    HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_INTERRUPT_ENABLE_1, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
    data = 0x63;
    HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_SPO2_CONFIGURATION, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
    MAX30102_SetLEDpulses(0x00,0x00);
    MAX30102_ClearFIFO();
    MAX30102_getInterruptStatus();
    data = 0x03;
    HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_MODE_CONFIGURATION, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
}

void MAX30102_SetLEDpulses(uint8_t LD1,uint8_t LD2)
{
    HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_LED_PLUSE_AMPLITUDE_1, I2C_MEMADD_SIZE_8BIT, &LD1, 1, 10);
    HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_LED_PLUSE_AMPLITUDE_2, I2C_MEMADD_SIZE_8BIT, &LD2, 1, 10);

}
void MAX30102_ClearFIFO()
{
	uint8_t data=0;
	 HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_FIFO_WRITE_POINTER, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
	 HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_OVERFLOW_COUNTER, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
	 HAL_I2C_Mem_Write(&hi2c2, MAX30102_ADDR_WRITE, RES_FIFO_READ_POINTER, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
}

uint8_t MAX30102_getUnreadSampleCount()
{
    uint8_t wr = 0, rd = 0;
    HAL_I2C_Mem_Read(&hi2c2, MAX30102_ADDR_READ, RES_FIFO_WRITE_POINTER, I2C_MEMADD_SIZE_8BIT, &wr, 1, 10);
    HAL_I2C_Mem_Read(&hi2c2, MAX30102_ADDR_READ, RES_FIFO_READ_POINTER, I2C_MEMADD_SIZE_8BIT, &rd, 1, 10);
    if ((wr - rd) < 0)
        return wr - rd + 32;
    else
        return wr - rd;
}

void MAX30102_getFIFO(SAMPLE *data, uint8_t sampleCount)
{
    uint8_t dataTemp[5 * 6];
    if (sampleCount > 5)
        sampleCount = 5;
    HAL_I2C_Mem_Read(&hi2c2, MAX30102_ADDR_READ, RES_FIFO_DATA_REGISTER,I2C_MEMADD_SIZE_8BIT, dataTemp,6 * sampleCount, 25);
    uint8_t i;
    for (i = 0; i < sampleCount; i++)
    {
        data[i].red = (((uint32_t)dataTemp[i * 6]) << 16 | ((uint32_t)dataTemp[i * 6 + 1]) << 8 | dataTemp[i * 6 + 2]) & 0x3ffff;
        data[i].iRed = (((uint32_t)dataTemp[i * 6 + 3]) << 16 | ((uint32_t)dataTemp[i * 6 + 4]) << 8 | dataTemp[i * 6 + 5]) & 0x3ffff;
    }
}

uint8_t MAX30102_getInterruptStatus()
{
    uint8_t data1 = 0, data2 = 0;
    HAL_I2C_Mem_Read(&hi2c2, MAX30102_ADDR_READ, RES_INTERRUPT_STATUS_1, I2C_MEMADD_SIZE_8BIT, &data1, 1, 10);
    HAL_I2C_Mem_Read(&hi2c2, MAX30102_ADDR_READ, RES_INTERRUPT_STATUS_2, I2C_MEMADD_SIZE_8BIT, &data2, 1, 10);
    return data1 | data2;
}

SAMPLE sampleBuff[BUFF_SIZE];

void Average_filter(SAMPLE *s)
{
    uint8_t i;
    uint32_t red = 0;
    uint32_t ired = 0;
    for (i = 0; i < FILTER_LEVEL - 1; i++)
    {
        red += sampleBuff[i].red;
        ired += sampleBuff[i].iRed;
    }
    s->red = (red + s->red) / FILTER_LEVEL;
    s->iRed = (ired + s->iRed) / FILTER_LEVEL;
}

void Kalman_filter(SAMPLE *s, int mode)
{
	float Q = 0.00016;
	float R = 0.012;
	float P = 1, K;
	float red,ired;

	switch (mode)
	{
	case 0:
	{	//heart rate
		for(uint8_t i=0; i<10; i++)
		{
	        // Prediction
	        red = red;
	        P = P + Q;
	        // Update
	        K = P / (P + R);
	        red = red + K * (s->red - red);
	        P = (1 - K) * P;
		}
		// Output the filtered value
		s->red = ((uint32_t)red);
	}
	case 1:
	{	//SpO2
		for(uint8_t i=0; i<10; i++)
			{
		        // Prediction
		        red = red;
		        ired = ired;
		        P = P + Q;
		        // Update
		        K = P / (P + R);
		        red = red + K * (s->red-red);
		        ired = ired + K * (s->iRed - ired);
		        P = (1 - K) * P;
			}
		// Output the filtered values
		s->red = ((uint32_t)red);
		s->iRed = ((uint32_t)ired);
	}
	}
}
//#define ALPHA 0.1
//float LowPass(uint16_t in, uint16_t prevout)
//{
//	float out = prevout * (in - ALPHA*prevout);
//	return out;
//}


void BuffInsert(SAMPLE s)
{
    uint8_t i;
    for (i = BUFF_SIZE - 1; i > 0; i--)
    {
        sampleBuff[i].red = sampleBuff[i - 1].red;
        sampleBuff[i].iRed = sampleBuff[i - 1].iRed;
    }
    sampleBuff[0].red = s.red;
    sampleBuff[0].iRed = s.iRed;
}

void ACDC(uint16_t *rac, uint32_t *rdc, uint16_t *iac, uint32_t *idc)
{
    uint32_t rMax = sampleBuff[0].red;
    uint32_t rMin = sampleBuff[0].red;
    uint32_t iMax = sampleBuff[0].iRed;
    uint32_t iMin = sampleBuff[0].iRed;

    uint8_t i;
    for (i = 0; i < BUFF_SIZE; i++)
    {
        if (sampleBuff[i].red > rMax)
            rMax = sampleBuff[i].red;
        if (sampleBuff[i].red < rMin)
            rMin = sampleBuff[i].red;
        if (sampleBuff[i].iRed > iMax)
            iMax = sampleBuff[i].iRed;
        if (sampleBuff[i].iRed < iMin)
            iMin = sampleBuff[i].iRed;
    }
    *rac = rMax - rMin;
    *rdc = (rMax + rMin) / 2;
    *iac = iMax - iMin;
    *idc = (iMax + iMin) / 2;
}

int16_t eachSampleDiff = 0;
SAMPLE sampleBuffTemp[5];

//uint8_t max30102_CalcSpO2()
//{
//	uint16_t previrDC,previrAC,prevrDC,prevrAC;
//	float rAC,rDC,irAC,irDC;
//	uint8_t UnreadSampleNbr = max30102_getUnreadSampleCount();
//	max30102_getFIFO(sampleBuffTemp,UnreadSampleNbr);
//
//	for (uint8_t i=0;i< UnreadSampleNbr;i++)
//	{
//		if (sampleBuffTemp[i].iRed<37000)
//		{
//			spo2=0;
//			continue;
//		}
//		buffInsert(sampleBuffTemp[i]);
//		calAcDc(&redAC, &redDC, &iRedAC, &iRedDC);
//		rDC=LowPass(redDC,prevrDC);
//		rAC=LowPass(redAC,prevrAC);
//		irDC=LowPass(iRedDC,previrDC);
//		irAC=LowPass(redDC,previrAC);
//		R=(rAC/rDC)/(irAC/irDC);
//		prevrDC=rDC;
//		prevrAC=rAC;
//		previrDC=irDC;
//		previrAC=irAC;
//		spo2=(uint8_t)(110-25*R);
//	}
//	return spo2;
//}
uint8_t heartRate = 0;
uint8_t spo2 = 0;

uint16_t redAC = 0;
uint32_t redDC = 0;
uint16_t iRedAC = 0;
uint32_t iRedDC = 0;
float R;
void MAX30102_Calc()
{
    uint8_t unreadSampleCount = MAX30102_getUnreadSampleCount();

    MAX30102_getFIFO(sampleBuffTemp, unreadSampleCount);
    static uint8_t eachBeatSampleCount = 0;
    static uint8_t lastTenBeatSampleCount[10];
    static uint32_t last_iRed = 0;
    for (uint8_t i = 0; i < unreadSampleCount; i++)
    {
        if (sampleBuffTemp[i].iRed < 37000)
        {
            heartRate = 0;
            spo2 = 0;
            eachSampleDiff = 0;
            continue;
        }
        BuffInsert(sampleBuffTemp[i]);
        ACDC(&redAC, &redDC, &iRedAC, &iRedDC);
        Average_filter(&sampleBuffTemp[i]);
        R = (((float)(redAC)) / ((float)(redDC))) / (((float)(iRedAC)) / ((float)(iRedDC)));
        if (R >= 0.36 && R < 0.66)
            spo2 = (uint8_t)(107 - (20 * R));
        else if (R >= 0.66 && R < 1)
            spo2 = (uint8_t)(128 - (43 * R));
        else if (R >=1)
        	spo2=(uint8_t)(102.4-(3*R));
        Kalman_filter(&sampleBuffTemp[i],0);
        eachSampleDiff = last_iRed - sampleBuffTemp[i].iRed;
        if (eachSampleDiff > 50 && eachBeatSampleCount > 12)
        {
            for (uint8_t j = 9; j > 0; j--)
                lastTenBeatSampleCount[i] = lastTenBeatSampleCount[i - 1];
            lastTenBeatSampleCount[0] = eachBeatSampleCount;
            uint32_t totalTime = 0;
            for (uint8_t j = 0; j < 10; j++)
                totalTime += lastTenBeatSampleCount[i];
            heartRate = (uint8_t)(60.0 * 10 / 0.02 / ((float)totalTime));
            eachBeatSampleCount = 0;
        }
        last_iRed = sampleBuffTemp[i].iRed;
        eachBeatSampleCount++;
    }
}

uint8_t max30102_getHeartRate() { return heartRate; }
uint8_t max30102_getSpO2() { return spo2; }
