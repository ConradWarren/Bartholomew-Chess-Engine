#pragma once
#include<string>

#define U64 unsigned long long

class Board_State {

public:
	int side;
	int en_passant;
	int castling_rights;

	U64 Bitboards[12];
	U64 Occupancies[3];

	Board_State();
	Board_State(std::string Fen);
	~Board_State();
private:


};

class moves {
public:
	int moves[256];
	int count;
};
