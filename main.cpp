#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

/*

Row:
1-7 - leds

Vcc - ...
Q0 - ...
DS - 19
OE - ...
STCP - 20
SHCP - 18
MR - ...
Q7S - ...

Column:
Vcc - ...
Q0 - ...
DS - Q7S
OE - ...
STCP - 20
SHCP - 18
MR - ...
Q7S - ...

*/

constexpr uint SHIFT_DS = 19;
constexpr uint SHIFT_STCP = 20;
constexpr uint SHIFT_SHCP = 18;

// Row - 1 active
// Col - 0 active

void Init()
{
	gpio_init(SHIFT_DS);
	gpio_set_dir(SHIFT_DS, GPIO_OUT);

	gpio_init(SHIFT_STCP);
	gpio_set_dir(SHIFT_STCP, GPIO_OUT);

	gpio_init(SHIFT_SHCP);
	gpio_set_dir(SHIFT_SHCP, GPIO_OUT);
}

void SendBit(uint v)
{
	gpio_put(SHIFT_DS, !!v);

	// gpio_put(SHIFT_STCP, 0);
	gpio_put(SHIFT_SHCP, 0);

	// sleep_us(1);
	// busy_wait_us(1);
	asm volatile("nop \n nop \n nop");

	// gpio_put(SHIFT_STCP, 1);
	gpio_put(SHIFT_SHCP, 1);
}

void SendByte(uint v)
{
	for(unsigned a = 0; a < 8; a++)
	{
		SendBit(v & 0x80);
		v <<= 1;
	}
}

// Bottom - left
void SendWord(uint row, uint col)
{
	static uint lastRow = 0;
	static uint lastCol = 0;

	// if(lastRow == row && lastCol == col)
	// 	return;

	lastRow = row;
	lastCol = col;

	SendByte((~row) << 0);
	SendByte(col << 0);

	gpio_put(SHIFT_STCP, 0);
	// sleep_us(1);
	busy_wait_us(1);
	gpio_put(SHIFT_STCP, 1);
}

uint logoSH[] = {
	0b1111111,
	0b0000000,
	0b1100111,
	0b0001000,
	0b1110011,
	0b0000000,
	0b1111111,
};

// [Row][Column]

uint display[7][7] = { // SH
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 1, 0, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 0 },
	{ 1, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 1, 1, 1, 1, 1, 1 },
};

// bool display[7][7] = { 0 };
uint8_t displayBrightness[7] = { 0 };

void UpdateDisplay()
{
	for(unsigned column = 0; column < 7; column++)
	{
		uint8_t byte = 0;
		for(unsigned a = 0; a < 7; a++)
			byte |= (!!display[column][a]) << (6 - a);

		for(unsigned pwmCounter = 0; pwmCounter < 256; pwmCounter++)
		{
			SendWord(byte, (pwmCounter < displayBrightness[column]) ? (1 << column) : 0);
			busy_wait_us(1);
		}
	}
}

void UpdateDisplayThread()
{
	for(;;)
	{
		UpdateDisplay();
	}
}

void Demo1()
{
	for(unsigned a = 0; a < 7; a++)
		displayBrightness[a] = 0;

	// Fade-in
	for(unsigned a = 0; a < 7; )
	{
		displayBrightness[a]++;
		sleep_ms(10);

		if(displayBrightness[a] == 255)
			a++;

		if(a == 1 || a == 5)
			a++;
	}

	// Fade-out
	for(unsigned a = 0; a < 7; )
	{
		displayBrightness[a]--;
		sleep_ms(10);

		if(displayBrightness[a] == 0)
			a++;

		if(a == 1 || a == 5)
			a++;
	}
}

void Demo2()
{
	for(unsigned a = 0; a < 7; a++)
		displayBrightness[a] = 0;

	for(unsigned loop = 0; loop < 3; loop++)
	{
		// Fade-in
		for(unsigned a = 0; a < 7; )
		{
			displayBrightness[a]++;
			sleep_ms(1);

			if(displayBrightness[a] == 255)
				a++;

			if(a == 1 || a == 5)
				a++;
		}

		// Fade-out
		for(unsigned a = 0; a < 7; )
		{
			displayBrightness[a]--;
			sleep_ms(1);

			if(displayBrightness[a] == 0)
				a++;

			if(a == 1 || a == 5)
				a++;
		}
	}
}

int main()
{
	stdio_init_all();

	Init();
	multicore_launch_core1(UpdateDisplayThread);

	for(;;)
	{
		printf("Demo 1\n");
		Demo1();
		printf("Demo 2\n");
		Demo2();
	}

	return 0;
}