#include "stdint.h"
#include "stdio.h"
#include "C:\Keil_v5\DoorLock\tm4c123gh6pm.h"

void systemInit(){}
	
/* delay n microseconds (16 MHz CPU clock) */
void delayUs(int n)
{
int i, j;
for(i = 0 ; i < n; i++)
for(j = 0; j < 3; j++)
{} /* do nothing for 1 us */
}	
void delayMs(int n)
{
int i, j;
for(i = 0 ; i < n; i++)
for(j = 0; j < 3180; j++)
{} /* do nothing for 1 ms */
}
/* this function initializes the ports connected to the keypad */
void keypad_init(void)
{
SYSCTL_RCGCGPIO_R |= 0x04; /* enable clock to GPIOC */
SYSCTL_RCGCGPIO_R |= 0x10; /* enable clock to GPIOE */
 
GPIO_PORTE_DIR_R |= 0x0F; /* set row pins 3-0 as output */
GPIO_PORTE_DEN_R |= 0x0F; /* set row pins 3-0 as digital pins */
GPIO_PORTE_ODR_R |= 0x0F; /* set row pins 3-0 as open drain */
 
GPIO_PORTC_DIR_R &= ~0xF0; /* set column pin 7-4 as input */
GPIO_PORTC_DEN_R |= 0xF0; /* set column pin 7-4 as digital pins */
GPIO_PORTC_PUR_R |= 0xF0; /* enable pull-ups for pin 7-4 */
}
 
/* This is a non-blocking function to read the keypad. */
/* If a key is pressed, it returns the key label in ASCII encoding. Otherwise, it returns a 0 (not ASCII 0). */
unsigned char keypad_getkey(void)
{
const unsigned char keymap[4][4] = {
{ '1', '2', '3', 'A'},
{ '4', '5', '6', 'B'},
{ '7', '8', '9', 'C'},
{ '*', '0', '#', 'D'},
};
 
int row, col;
 
/* check to see any key pressed first */
GPIO_PORTE_DATA_R = 0; /* enable all rows */
col = GPIO_PORTC_DATA_R & 0xF0; /* read all columns */
if (col == 0xF0) return 0; /* no key pressed */
 
/* If a key is pressed, it gets here to find out which key. */
/* Although it is written as an infinite loop, it will take one of the breaks or return in one pass.*/
while (1)
{
row = 0;
GPIO_PORTE_DATA_R = 0x0E; /* enable row 0 */
delayUs(2); /* wait for signal to settle */
col = GPIO_PORTC_DATA_R & 0xF0;
if (col != 0xF0) break;
 
row = 1;
GPIO_PORTE_DATA_R = 0x0D; /* enable row 1 */
delayUs(2); /* wait for signal to settle */
col = GPIO_PORTC_DATA_R & 0xF0;
if (col != 0xF0) break;
 
row = 2;
GPIO_PORTE_DATA_R = 0x0B; /* enable row 2 */
delayUs(2); /* wait for signal to settle */
col = GPIO_PORTC_DATA_R & 0xF0;
if (col != 0xF0) break;
 
row = 3;
GPIO_PORTE_DATA_R = 0x07; /* enable row 3 */
delayUs(2); /* wait for signal to settle */
col = GPIO_PORTC_DATA_R & 0xF0;
if (col != 0xF0) break;
 
return 0; /* if no key is pressed */
}
 
/* gets here when one of the rows has key pressed */
if (col == 0xE0) return keymap[row][0]; /* key in column 0 */
if (col == 0xD0) return keymap[row][1]; /* key in column 1 */
if (col == 0xB0) return keymap[row][2]; /* key in column 2 */
if (col == 0x70) return keymap[row][3]; /* key in column 3 */
return 0; /* just to be safe */
}
unsigned char keypad_kbhit(void)
{
int col;
 
/* check to see any key pressed */
GPIO_PORTE_DATA_R = 0; /* enable all rows */
col = GPIO_PORTC_DATA_R & 0xF0; /* read all columns */
if (col == 0xF0)
return 0; /* no key pressed */
else
return 1; /* a key is pressed */
}	
	
void init(){
	volatile unsigned long delay;
	SYSCTL_RCGCUART_R |=0x01;
	SYSCTL_RCGCGPIO_R |=0x20;
	SYSCTL_RCGCGPIO_R |=0x01;
	while ((SYSCTL_RCGCGPIO_R&0x20)==0){};
	
	/* UART0 initialization */
  UART0_CTL_R = 0; /* disable UART0 */
  UART0_IBRD_R = 104; /* 16MHz/16=1MHz, 1MHz/104=9600 baud rate */
  UART0_FBRD_R = 11; /* fraction part*/
  UART0_CC_R = 0; /* use system clock */
  UART0_LCRH_R = 0x0070; /* 8-bit, no parity, 1-stop bit, FIFO */
  UART0_CTL_R = 0x0301; /* enable UART0, TXE, RXE */
		
	/* UART0 TX0 and RX0 use PA0 and PA1. Set them up. */
  GPIO_PORTA_DEN_R = 0x03; /* Make PA0 and PA1 as digital */
  GPIO_PORTA_AFSEL_R = 0x03; /* Use PA0,PA1 alternate function */
  GPIO_PORTA_PCTL_R = 0x11; /* configure PA0 and PA1 for UART */
		
	/* PORTF initialization */
	GPIO_PORTF_LOCK_R = 0x4C4F434B;
	GPIO_PORTF_CR_R = 0x1F;
	GPIO_PORTF_AFSEL_R = 0x00;
	GPIO_PORTF_AMSEL_R = 0x00;
	GPIO_PORTF_DIR_R =0x0E;
	GPIO_PORTF_DEN_R =0x01;
	GPIO_PORTF_PUR_R = 0x11;
}

int main(){
	unsigned int roomStatus[64];
	unsigned int maxRoomNo;
	unsigned char roomPW[64][4];
	unsigned int roomNum;
	unsigned char roomNumber[2];
	unsigned char key;
	unsigned int flag=0;
	unsigned int passflag=1;
	unsigned int y=0;
	
	init();
	keypad_init();
	
	while(1){
		//***************** PC input ************************ 
		//Reciver is full
		while(UART0_FR_R == 0x0010){
			//Store Number of Rooms
			if((UART0_DR_R & 0xC0)==0x00){
				maxRoomNo = UART0_DR_R & 0x3F;
			}
			//Store Room No.
			if((UART0_DR_R & 0xC0)==0x10){
				roomNum = UART0_DR_R & 0x3F;
			}
			//Status of The Room (Free = 0, Occupied = 1,Cleaning = 2)
			else if((UART0_DR_R & 0xC0)==0x01){
				roomStatus[roomNum] = UART0_DR_R & 0x3F;
			}
			//Set Room Password
			else if((UART0_DR_R & 0xC0)==0x11){
				UART0_LCRH_R |= 0x0060; //Disable FIFO (character mode)
				unsigned int counter;
				for(counter=0;counter<4;counter++){
					while(UART0_FR_R != 0x0010){}
					roomPW[roomNum][counter]=UART0_DR_R;
				}
			}
		}
		//***************** Keypad input *********************
    key = keypad_getkey(); /* read the keypad */
		
    if (key != 0)
     { /* if a key is pressed */
			 //read room number
			 if(flag == 0){
				 roomNum=key;
				 flag = 1;				 
			 } 
			 //read room password
			 if(flag==1){
				 if(key != roomPW[roomNum][y]){passflag=0;}
				 y++;
				 if(y==4)
					 y=0;
			 }
			 if(y==3 && passflag==1){
				 //door opened
				 GPIO_PORTF_DATA_R = 0x01;
			 }
     }
 
    delayMs(20); /* wait for a while */
  }
}
