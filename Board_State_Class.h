#pragma once
#include<string>

#define U64 unsigned long long

extern U64 piece_keys[12][64];
extern U64 en_passant_keys[64];
extern U64 castle_keys[16];
extern U64 side_key;

class Board_State {

public:
	int side;
	int en_passant;
	int castling_rights;

	U64 Bitboards[12];
	U64 Occupancies[3];

	U64 position_key;

	Board_State();
	Board_State(std::string Fen);
	~Board_State();
	U64 Generate_Zobrist_Key();
private:


};

class moves {

public:
	int moves[256];
	int count;
};

void Init_Zobrist_Keys();
