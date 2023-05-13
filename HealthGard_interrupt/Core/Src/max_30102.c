#include "max_30102.h"


HAL_StatusTypeDef ret;
void MAX30102_Init()
{
    uint8_t data = 0;
    data = 0x40;

    // Reset the mode configuration
    uint8_t retry = 0;
    HAL_StatusTypeDef ret;
    do
    {
        ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, MODE_CONFIGR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while Reseting the mode configuration register \n");
    		}
    // Make sure that the reset was done
    retry = 0;
    do
    {
        ret = HAL_I2C_Mem_Read(&hi2c3, MAX30102_ADDR_READ, MODE_CONFIGR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while Reseting the mode configuration register \n");
    		}
    // Enable an interrupt to initialize the sensor
    retry = 0;
    do
    {
        ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, INTERRUPT_EN1R, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while Enabling an interrupt to initialize the sensor \n");
    		}
    // set the SpO2 configuration
    retry = 0;
    data = 0x63;
    do
    {
        ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, SPO2_CONFIGR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error setting the SpO2 configuration  \n");
    		}
    // Set the red and infra-red LED current to zero
    MAX30102_SetLEDpulses(0x00, 0x00);

    // Clear FIFO
    MAX30102_ClearFIFO();

    // Get the interrupt status to remove any predefined interrupts
    MAX30102_getInterruptStatus();

    // Set the led  mode to SpO2 mode (red and infra-red LEDs)
    retry = 0;
    data = 0x03;
    do
    {
        ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, MODE_CONFIGR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while Setting the led  mode to SpO2 mode \n");
    		}
}

//setting the LED amplitude conveniently
void MAX30102_SetLEDpulses(uint8_t LD1, uint8_t LD2)
{
    HAL_StatusTypeDef ret;
    uint8_t retry = 0;

    do
    {
        ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, LED_PULSEAMP1R, I2C_MEMADD_SIZE_8BIT, &LD1, 1, 10);
        if (ret == HAL_OK)
        {
            ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, LED_PULSE_AMP2R, I2C_MEMADD_SIZE_8BIT, &LD2, 1, 10);
        }
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while setting the LED amplitude conveniently \n");
    		}
}

//setting all bits in pointers to zero
void MAX30102_ClearFIFO()
{
    HAL_StatusTypeDef ret;
    uint8_t retry = 0;
    uint8_t data = 0;

    do
    {
        ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, FIFO_WR_PTR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
        if (ret == HAL_OK)
        {
            ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, OVF_COUNTER, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
            if (ret == HAL_OK)
            {
                ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, FIFO_RD_PTR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
            }
        }
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while setting all bits in pointers to zero \n");
    		}
}

//get the number of unread samples in the circular FIFO
uint8_t MAX30102_getUnreadSampleCount()
{
    HAL_StatusTypeDef ret;
    uint8_t retry = 0;
    uint8_t wr = 0, rd = 0;

    do
    {
        ret = HAL_I2C_Mem_Read(&hi2c3, MAX30102_ADDR_READ, FIFO_WR_PTR, I2C_MEMADD_SIZE_8BIT, &wr, 1, 10);
        if (ret == HAL_OK)
        {
            ret = HAL_I2C_Mem_Read(&hi2c3, MAX30102_ADDR_READ, FIFO_RD_PTR, I2C_MEMADD_SIZE_8BIT, &rd, 1, 10);
        }
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while getting the number of unread samples in the circular FIFO\n");
    		}
    if ((wr - rd) < 0)
        return wr - rd + 32;
    else
        return wr - rd;
}

//get the data from the circular FIFO with a burst-reading
void MAX30102_getFIFO(SAMPLE *data, uint8_t sampleCount)
{
    HAL_StatusTypeDef ret;
    uint8_t retry = 0;
    uint8_t dataTemp[5 * 6];

    if (sampleCount > 5)
        sampleCount = 5;

    do
    {
        ret = HAL_I2C_Mem_Read(&hi2c3, MAX30102_ADDR_READ, FIFO_DATA_REG, I2C_MEMADD_SIZE_8BIT, dataTemp, 6 * sampleCount, 25);
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while getting the data from the circular FIFO with a burst-reading\n");
    		}
    if (ret == HAL_OK)
    {
        uint8_t i;
        for (i = 0; i < sampleCount; i++)
        {
            data[i].red = (((uint32_t)dataTemp[i * 6]) << 16 | ((uint32_t)dataTemp[i * 6 + 1]) << 8 | dataTemp[i * 6 + 2]) & 0x3ffff;
            data[i].iRed = (((uint32_t)dataTemp[i * 6 + 3]) << 16 | ((uint32_t)dataTemp[i * 6 + 4]) << 8 | dataTemp[i * 6 + 5]) & 0x3ffff;
        }
    }
}

//this function returns the interrupt status when needed and also clears the interrupt status
uint8_t MAX30102_getInterruptStatus()
{
    HAL_StatusTypeDef ret;
    uint8_t retry = 0;
    uint8_t data1 = 0, data2 = 0;

    do
    {
        ret = HAL_I2C_Mem_Read(&hi2c3, MAX30102_ADDR_READ, INTERRUPT_STATUS1R, I2C_MEMADD_SIZE_8BIT, &data1, 1, 10);
        if (ret == HAL_OK)
        {
            ret = HAL_I2C_Mem_Read(&hi2c3, MAX30102_ADDR_READ, INTERRUPT_STATUS2R, I2C_MEMADD_SIZE_8BIT, &data2, 1, 10);
        }
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
    			// Maximum number of retries reached, report an error
    			send_USART2("Error while getting the interrupt status or also clearing the interrupt status\n");
    		}
    return data1 | data2;
}

//this function is used to set an interrupt when needed

void MAX30102_setInterrupt(uint8_t data1, uint8_t data2)
{
    HAL_StatusTypeDef ret;
    uint8_t retry = 0;

    do
    {
        ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, INTERRUPT_EN1R, I2C_MEMADD_SIZE_8BIT, &data1, 1, 10);
        if (ret == HAL_OK)
        {
            ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, INTERRUPT_EN2R, I2C_MEMADD_SIZE_8BIT, &data2, 1, 10);
        }
        retry++;
    } while (ret != HAL_OK && retry < 10);
    if (retry== 10) {
      			// Maximum number of retries reached, report an error
      			send_USART2("Error while set an interrupt\n");
      		}
}


/*this function has 2 options :
 * 0: set the sensor in standby mode
 * 1: set the sensor out of the standby mode
 * did not use it at the moment
 */
void MAX30102_SHDN_CTRL(int a)
{
    HAL_StatusTypeDef ret;
    uint8_t retry = 0;

    switch (a)
    {
        case 0:
        {
            uint8_t data = 0;
            do
            {
                ret = HAL_I2C_Mem_Read(&hi2c3, MAX30102_ADDR_READ, MODE_CONFIGR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
                retry++;
            } while (ret != HAL_OK && retry < 10);

            if (ret == HAL_OK)
            {
                data |= 0x80;
                retry = 0;
                do
                {
                    ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, MODE_CONFIGR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
                    retry++;
                } while (ret != HAL_OK && retry < 10);
            }
            break;
        }
        case 1:
        {
            uint8_t data = 0;
            do
            {
                ret = HAL_I2C_Mem_Read(&hi2c3, MAX30102_ADDR_READ, MODE_CONFIGR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
                retry++;
            } while (ret != HAL_OK && retry < 10);

            if (ret == HAL_OK)
            {
                data &= ~0x80;
                retry = 0;
                do
                {
                    ret = HAL_I2C_Mem_Write(&hi2c3, MAX30102_ADDR_WRITE, MODE_CONFIGR, I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
                    retry++;
                } while (ret != HAL_OK && retry < 10);
            }
            break;
        }
    }
}


SAMPLE sampleBuff[BUFF_SIZE];
//the average filter is a simple yet an effective way to treat data read from the FIFO
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
//the Kalman filter is used to get close to accurate heart beats, yet, it has 2 options
void Kalman_filter(SAMPLE *s, int mode)
{
	float Q = 0.00016;
	float R = 0.012;
	float P = 1, K;
	float red,ired;

	switch (mode)
	{
	case 0:
	{	//red samples
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
	{	//infra-red and red samples
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



//the function that sets the heart beats and SpO2 to readable data
void MAX30102_Calc()
{
	uint16_t redAC = 0;
	uint32_t redDC = 0;
	uint16_t iRedAC = 0;
	uint32_t iRedDC = 0;
	int16_t eachSampleDiff = 0;
	SAMPLE sampleBuffTemp[5];

	float R;
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
