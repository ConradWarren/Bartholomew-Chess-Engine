#include <string>
#include <unordered_map>
#include <nmmintrin.h>

#include "Board_State_Class.h"
#include "Random_Number_Generation.h"

#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define count_bits(bitboard) int(_mm_popcnt_u64(bitboard))

U64 piece_keys[12][64];
U64 en_passant_keys[64];
U64 castle_keys[16];
U64 side_key;

void Init_Zobrist_Keys() {

	for (int piece = 0; piece <= 11; piece++) {
		for (int square = 0; square < 64; square++) {
			piece_keys[piece][square] = Get_Random_U64_number();
		}
	}

	for (int square = 0; square < 64; square++) {
		en_passant_keys[square] = Get_Random_U64_number();
	}

	for (int i = 0; i < 16; i++) {
		castle_keys[i] = Get_Random_U64_number();
	}
	side_key = Get_Random_U64_number();
}

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

	position_key = Generate_Zobrist_Key();
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

	position_key = 14485368118541779952;
}

Board_State::~Board_State() {

}

U64 Board_State::Generate_Zobrist_Key() {

	U64 zobrist_key = 0ULL;

	U64 zobrist_bitboard = 0ULL;

	for (int i = 0; i < 12; i++) {
		zobrist_bitboard = Bitboards[i];
		while (zobrist_bitboard) {
			int square = int(count_bits((zobrist_bitboard & (0 - zobrist_bitboard)) - 1));
			zobrist_key ^= piece_keys[i][square];
			pop_bit(zobrist_bitboard, square);
		}
	}

	if (en_passant != 64) {
		zobrist_key ^= en_passant_keys[en_passant];
	}

	zobrist_key ^= castle_keys[castling_rights];

	if (side) {
		zobrist_key ^= side_key;
	}

	return zobrist_key;
}
