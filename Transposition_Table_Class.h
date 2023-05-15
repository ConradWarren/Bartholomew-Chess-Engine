#pragma once

#define U64 unsigned long long


class transpostion_table {
	
public:
	U64 key;	
	int depth;
	int flag;
	int value;

	transpostion_table();
	~transpostion_table();
private:

};
