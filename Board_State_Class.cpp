#include <string>
#include <unordered_map>

#include "Board_State_Class.h"

#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))

Board_State::Board_State(std::string Fen) {
	for (int i = 0; i < 12; i++) Bitboards[i] = 0ULL;

	for (int i = 0; i < 3; i++) Occupancies[i] = 0ULL;

	side = 0;
	castling_rights = 0;
	en_passant = 64;

	std::unordered_map<char, int> Peice_Map = { {'P', 0}, {'N', 1},{'B', 2}, {'R', 3}, {'Q', 4}, {'K', 5}, {'p', 6},
	{'n', 7}, {'b', 8}, {'r', 9}, {'q', 10}, {'k', 11} };

	int index = 0;

	for (int square = 0; square < 64; square++) {
		if ((int(Fen[index]) >= int('a') && int(Fen[index]) <= int('z')) || (int(Fen[index]) >= int('A') && int(Fen[index]) <= int('Z'))) {
			set_bit(Bitboards[Peice_Map[Fen[index]]], square);
		}
		else if (Fen[index] != '/') square += (int(Fen[index]) - 48) - 1;
		else square--;

		index++;
	}

	index++;
	if (Fen[index] == 'w') side = 0;
	else if (Fen[index] == 'b') side = 1;

	index += 2;
	while (Fen[index] != ' ') {
		if (Fen[index] == 'K') castling_rights |= 1;
		else if (Fen[index] == 'Q') castling_rights |= 2;
		else if (Fen[index] == 'k') castling_rights |= 4;
		else if (Fen[index] == 'q') castling_rights |= 8;
		index++;
	}
	index++;
	if (Fen[index] != '-') {
		int file = int(Fen[index]) - 97;
		int rank = 8 - (int(Fen[index + 1]) - int('0'));
		en_passant = (rank * 8) + file;
	}

	for (int i = 0; i < 6; i++) {
		Occupancies[0] |= Bitboards[i];
	}
	for (int i = 6; i < 12; i++) {
		Occupancies[1] |= Bitboards[i];
	}
	Occupancies[2] = (Occupancies[0] | Occupancies[1]);
}


Board_State::Board_State() {

	for (int i = 0; i < 12; i++) Bitboards[i] = 0ULL;

	for (int i = 0; i < 3; i++) Occupancies[i] = 0ULL;

	side = 0;
	castling_rights = 15;
	en_passant = 64;

	Bitboards[0] = 71776119061217280;
	Bitboards[1] = 4755801206503243776;
	Bitboards[2] = 2594073385365405696;
	Bitboards[3] = 9295429630892703744;
	Bitboards[4] = 576460752303423488;
	Bitboards[5] = 1152921504606846976;
	Bitboards[6] = 65280;
	Bitboards[7] = 66;
	Bitboards[8] = 36;
	Bitboards[9] = 129;
	Bitboards[10] = 8;
	Bitboards[11] = 16;

	for (int i = 0; i < 6; i++) Occupancies[0] |= Bitboards[i];

	for (int i = 6; i < 12; i++) Occupancies[1] |= Bitboards[i];

	Occupancies[2] = (Occupancies[0] | Occupancies[1]);
}

Board_State::~Board_State() {

}
