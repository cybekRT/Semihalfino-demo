#pragma once

#include<cstdint>

struct FontNode
{
	uint8_t width;

	uint8_t font[7];
};

extern FontNode font[];
