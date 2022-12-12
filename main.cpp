#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "Font.hpp"

/*

Row:
1-7 - leds
DS - 19
STCP - 20
SHCP - 18

Column:
DS - Q7S
STCP - 20
SHCP - 18

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
	gpio_put(SHIFT_SHCP, 0);

	asm volatile("nop \n nop \n nop");

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

	lastRow = row;
	lastCol = col;

	SendByte((~row) << 0);
	SendByte(col << 0);

	gpio_put(SHIFT_STCP, 0);
	busy_wait_us(1);
	gpio_put(SHIFT_STCP, 1);
}

// [Row][Column]

uint logoSH[7][7] = { // SH
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 1, 0, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 0 },
	{ 1, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 1, 1, 1, 1, 1, 1 },
};

uint8_t display[7][7] = { // SH
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00 },
	{ 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
};

void UpdateDisplay()
{
	for(unsigned column = 0; column < 7; column++)
	{
		for(unsigned pwmCounter = 0; pwmCounter < 256; pwmCounter++)
		{
			uint8_t byte = 0;
			for(unsigned a = 0; a < 7; a++)
			{
				if(pwmCounter < display[column][a])
					byte |= (!!display[column][a]) << (6 - a);
			}

			SendWord(byte, (1 << column));
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

void ClearScreen()
{
	for(unsigned x = 0; x < 7; x++)
	{
		for(unsigned y = 0; y < 7; y++)
			display[x][y] = 0;
	}
}

void CopyBuffer(uint buf[7][7])
{
	for(unsigned x = 0; x < 7; x++)
	{
		for(unsigned y = 0; y < 7; y++)
		{
			display[x][y] = buf[x][y] * 255;
		}
	}
}

void Demo1()
{
	// Fade-in
	for(unsigned column = 0; column < 7; column++)
	{
		for(unsigned brightness = 0; brightness < 255; brightness++)
		{
			for(unsigned row = 0; row < 7; row++)
			{
				if(logoSH[column][row])
					display[column][row]++;
			}
			sleep_ms(6);

			if(column == 1 || column == 5)
				break;
		}
	}

	// Fade-out
	for(unsigned column = 0; column < 7; column++)
	{
		for(unsigned brightness = 0; brightness < 255; brightness++)
		{
			for(unsigned row = 0; row < 7; row++)
			{
				if(logoSH[column][row])
					display[column][row]--;
			}
			sleep_ms(6);

			if(column == 1 || column == 5)
				break;
		}
	}
}

void Demo2()
{
	// Fade-in
	for(unsigned column = 0; column < 7; column++)
	{
		for(unsigned brightness = 0; brightness < 255; brightness++)
		{
			for(unsigned row = 0; row < 7; row++)
			{
				if(logoSH[column][row])
					display[column][row]++;
			}
			sleep_ms(1);

			if(column == 1 || column == 5)
				break;
		}
	}

	// Fade-out
	for(unsigned column = 0; column < 7; column++)
	{
		for(unsigned brightness = 0; brightness < 255; brightness++)
		{
			for(unsigned row = 0; row < 7; row++)
			{
				if(logoSH[column][row])
					display[column][row]--;
			}
			sleep_ms(1);

			if(column == 1 || column == 5)
				break;
		}
	}
}

void Demo2B()
{
	// Fade-in
	for(unsigned row = 0; row < 7; row++)
	{
		for(unsigned brightness = 0; brightness < 255; brightness++)
		{
			for(unsigned column = 0; column < 7; column++)
			{
				if(logoSH[column][row])
					display[column][row]++;
			}
			sleep_ms(1);
		}
	}

	// Fade-out
	for(unsigned row = 0; row < 7; row++)
	{
		for(unsigned brightness = 0; brightness < 255; brightness++)
		{
			for(unsigned column = 0; column < 7; column++)
			{
				if(logoSH[column][row])
					display[column][row]--;
			}
			sleep_ms(1);
		}
	}
}

void Demo3()
{
	ClearScreen();

	for(unsigned brightness = 0; brightness <= 255; brightness += 51)
	{
		for(unsigned y = 0; y < 7; y++)
		{
			for(unsigned x = 0; x < 7; x++)
			{
				if(!logoSH[x][y])
					continue;

				display[x][y] = logoSH[x][y] * brightness;
				sleep_ms(50);
			}
		}

		sleep_ms(100);
	}

	for(unsigned brightness = 255; brightness <= 255; brightness -= 51)
	{
		for(unsigned y = 0; y < 7; y++)
		{
			for(unsigned x = 0; x < 7; x++)
			{
				if(!logoSH[x][y])
					continue;

				display[x][y] = logoSH[x][y] * brightness;
				sleep_ms(50);
			}
		}
	}
}

void Demo7(const char* str, unsigned delay)
{
	ClearScreen();

	auto ShiftColumn = [](){
		for(unsigned y = 0; y < 7; y++)
		{
			for(unsigned x = 0; x < 6; x++)
			{
				display[x][y] = display[x+1][y];
			}
		}
	};

	constexpr unsigned spaceWidth = 1;
	for(unsigned a = 0; a < strlen(str); a++)
	{
		char character = str[a];

		for(unsigned col = 0; col < font[character].width + spaceWidth; col++)
		{
			uint8_t byte;
			if(col >= font[character].width)
				byte = 0;
			else
				byte = font[character].font[col];

			ShiftColumn();
			for(unsigned row = 0; row < 7; row++)
			{
				display[6][row] = (font[character].font[col] & (1 << (6-row))) * 255;
			}

			sleep_ms(delay);
		}
	}

	for(unsigned a = 0; a < 6; a++)
	{
		ShiftColumn();
		sleep_ms(delay);
	}
}

int main()
{
	stdio_init_all();

	Init();
	multicore_launch_core1(UpdateDisplayThread);

	Demo7("SEMIHALFINO", 100);

	ClearScreen();
	for(;;)
	{
		printf("Demo 1\n");
		Demo1();
		printf("Demo 2\n");
		Demo2();
		printf("Demo 2B\n");
		Demo2B();
		printf("Demo 3\n");
		Demo3();

		for(unsigned a = 0; a < 5; a++)
			Demo7("SEMIHALF", 500);
	}

	return 0;
}