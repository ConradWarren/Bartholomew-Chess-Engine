#include "Random_Number_Generation.h"

#define U64 unsigned long long

static unsigned int random_seed = 1804289383;

unsigned int Get_Random_U32_Number() {

	unsigned int num = random_seed;

	num ^= (num << 13);
	num ^= (num >> 17);
	num ^= (num << 5);

	random_seed = num;

	return num;
}

U64 Get_Random_U64_number() {

	U64 num1 = Get_Random_U32_Number();
	U64 num2 = Get_Random_U32_Number();
	U64 num3 = Get_Random_U32_Number();
	U64 num4 = Get_Random_U32_Number();

	num1 &= 0xFFFF;
	num2 &= 0xFFFF;
	num3 &= 0xFFFF;
	num4 &= 0xFFFF;

	return (num1 | (num2 << 16) | (num3 << 32) | (num4 << 48));
}

