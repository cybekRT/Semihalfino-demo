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
	gpio_put(SHIFT_SHCP, 0);

	gpio_put(SHIFT_DS, !!v);

	asm volatile("nop");
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

	gpio_put(SHIFT_STCP, 0);

	SendByte((~row) << 0);
	SendByte(col << 0);

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

// void Demo4()
// {
// 	ClearScreen();

// 	unsigned phase = 0;
// 	uint64_t phaseTimer;
// 	for(unsigned delay = 250; delay > 0; )
// 	{
// 		for(unsigned y = 0; y < 7; y++)
// 		{
// 			for(unsigned x = 0; x < 7; x++)
// 			{
// 				if(!logoSH[x][y])
// 					continue;

// 				for(unsigned a = 1; a < 256; a++)
// 				{
// 					display[x][y] = logoSH[x][y] * a;
// 					sleep_us(delay);
// 				}

// 				display[x][y] = 0;
// 			}
// 		}

// 		if(delay > 10 && phase == 0)
// 			delay /= 2;
// 		else if(delay > 3 && phase == 0)
// 		{
// 			delay--;
// 		}
// 		else if(phase == 0)
// 		{
// 			phaseTimer = time_us_64();
// 			phase++;
// 		}
// 		else if(time_us_64() > phaseTimer + 2000000ULL)
// 		{
// 			break;
// 		}
// 	}
// }

// void Demo5()
// {
// 	ClearScreen();

// 	for(unsigned a = 0; a < 7; a++)
// 	{
// 		display[a][0] = 0xff;
// 		display[a][6] = 0xff;
// 		display[0][a] = 0xff;
// 		display[6][a] = 0xff;

// 		display[2][3] = 0xff;
// 		display[3][3] = 0xff;
// 		display[4][3] = 0xff;
// 		display[3][2] = 0xff;
// 		display[3][4] = 0xff;
// 	}

// 	for(unsigned a = 0; a < 4; a++)
// 	{
// 		screenNegate = !screenNegate;
// 		sleep_ms(500);
// 	}
// }

// void Demo6()
// {
// 	auto DrawRect = [](int radius, uint8_t brightness) {
// 		if(radius == 1)
// 			display[3][3] = brightness;
// 		else
// 		{
// 			radius--;
// 			for(int a = -radius; a <= radius; a++)
// 			{
// 				display[3-radius][3+a] = brightness;
// 				display[3+radius][3+a] = brightness;
// 				display[3+a][3-radius] = brightness;
// 				display[3+a][3+radius] = brightness;
// 			}
// 		}
// 	};

// 	struct Rect
// 	{
// 		int radius;
// 		int brightness;
// 	}

// 	for(unsigned a = 0; a < 4; a++)
// 	{
// 		for(unsigned br = 51; br < 256; br += 51)
// 		{
// 			ClearScreen();
// 			DrawRect(a+1, br);

// 			sleep_ms(200);
// 		}

// 		sleep_ms(1000);
// 	}
// }

int main()
{
	stdio_init_all();

	Init();
	multicore_launch_core1(UpdateDisplayThread);

	// printf("Demo 5\n"); // Just random image
	// Demo5();

	// printf("Demo 4\n");
	// Demo4();

	ClearScreen();
	for(;;)
	{
		printf("Demo 1\n");
		Demo1();
		printf("Demo 2\n");
		Demo2();
		printf("Demo 3\n");
		Demo3();
		// printf("Demo 6\n");
		// Demo6();
	}

	return 0;
}