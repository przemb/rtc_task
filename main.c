#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"


void ledInit(void);
void uartConfig(void);
void sendChar(char c);
void sendString(const char* s);
void ITMsendString(const char * s);
void delay(int time);
void blinkGreenLed(void);
void rtcConfigPins(void);
void rtcConfigI2C(void);
void i2cWorkaround(void);

uint8_t data1 = 0;
uint8_t data2 = 0;

int main(void){
	ledInit();
	//uartConfig();
	
	rtcConfigPins();

	blinkGreenLed();
	//i2cWorkaround();
	rtcConfigI2C();

	

// -------------------------- write --------------------------------------------------

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); // wait if I2C1 is busy 
	I2C_GenerateSTART(I2C1, ENABLE); // generate START 
	
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)); // wait for EV5 
	I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter); // send slave addr
	
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); // wait for EV6
	I2C_SendData(I2C1, 0x10); // addr within slave
	
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING)); // wait for EV8
	I2C_SendData(I2C1, 0x0F); // send data to slave
	
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // wait for EV8_2
	I2C_GenerateSTOP(I2C1, ENABLE); // generate STOP


// --------------------------- read  --------------------------------------------------
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); 
	I2C_GenerateSTART(I2C1, ENABLE); 
	
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter);
	
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 
	I2C_SendData(I2C1, 0x10); 
	
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Receiver);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) ); 
	
	data1 = I2C_ReceiveData(I2C1);
	I2C_GenerateSTOP(I2C1, ENABLE); 


	for(;;){
       blinkGreenLed();
    }
}
	
	


void ledInit(){
    GPIO_InitTypeDef  gpio;
    //Configure PA5, push-pull output
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8| GPIO_Pin_9;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &gpio);
	GPIO_Init(GPIOC, &gpio);
}

void uartConfig(void){
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef uart;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); //alternatywne linie IO
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //USART2

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_3;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio);

	USART_StructInit(&uart);
	uart.USART_BaudRate = 9600;
	USART_Init(USART2, &uart);

	USART_Cmd(USART2, ENABLE);
}

void sendString(const char * s){
	while (*s)
		sendChar(*s++);
	//sendChar('\n');	
}

void ITMsendString(const char * s){
	while (*s)
		ITM_SendChar(*s++);
	ITM_SendChar('\n');
}

void sendChar(char c){
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
		;
	USART_SendData(USART2, c);
}

void delay(int time){
    int i;
    for (i = 0; i < time * 4000; i++) {}
}


void rtcConfigPins(void){
	GPIO_InitTypeDef gpio;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 , ENABLE);
	
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_AF_OD; //open-drain
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);
}

void rtcConfigI2C(void){
	I2C_InitTypeDef myI2C;
	
	I2C_StructInit(&myI2C);
	myI2C.I2C_Mode = I2C_Mode_I2C;
	myI2C.I2C_ClockSpeed = 100000;
	I2C_Init(I2C1, &myI2C);
	I2C_Cmd(I2C1, ENABLE);
}


void blinkGreenLed(void){
	
	GPIOA->ODR ^= GPIO_Pin_5;
	delay(500);
}

void i2cWorkaround(void){
	I2C1->CR1 &= ~I2C_CR1_PE; //1. disable I2C peripherial by clearing PE bit
	
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //SCL, SDA
	gpio.GPIO_Mode = GPIO_Mode_Out_OD; //2. output open-drain
	GPIO_Init(GPIOB, &gpio);
	
	GPIOB->ODR |= (GPIO_Pin_6 | GPIO_Pin_7); 	// 2. set SCL and SDA high
	
	while((GPIOB->IDR & (GPIO_Pin_6 | GPIO_Pin_7)) != (GPIO_Pin_6 | GPIO_Pin_7)) //3. check if SCL and SDA are high
		;

	
	GPIOB->ODR &= ~GPIO_Pin_7; 	//4. set SDA low - 7 bit
	while((GPIOB->IDR & GPIO_Pin_7) != 0) //5. check if SDA is low
		;
	
	
	GPIOB->ODR &= ~GPIO_Pin_6; 	//6. set SCL low - 6 bit
	while((GPIOB->IDR & GPIO_Pin_6) != 0) //7. check if SCL is low
		;
	
	GPIOB->ODR |= GPIO_Pin_6; 	// 8. set SCL high
	while((GPIOB->IDR & GPIO_Pin_6) == 0) //9. check if SCL is high
		;
	
	GPIOB->ODR |= GPIO_Pin_7; 	// 10. set SDA high
	while((GPIOB->IDR & GPIO_Pin_7) == 0) //11. check if SDA is high
		;
	
	
	GPIO_InitTypeDef nextGpio;
	GPIO_StructInit(&nextGpio);
	gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; 
	gpio.GPIO_Mode = GPIO_Mode_AF_OD; //12. conf. SCL, SDA as alternate function open-drain
	GPIO_Init(GPIOB, &nextGpio);
	
	I2C1->CR1 |= I2C_CR1_SWRST; //13. set SWRST bit
	I2C1->CR1 &= ~I2C_CR1_SWRST; //14. clear SWRST bit
	
	I2C1->CR1 |= I2C_CR1_PE; //15. enable I2C peripherial by setting PE bit
}
	
