#include<iostream>

#include "Board_State_Class.h"
#include "Move_Generation.h"
#include <nmmintrin.h>

#define U64 unsigned long long
#define max_ply 64

//Bit Macros
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define count_bits(bitboard) int(_mm_popcnt_u64(bitboard))
#define encode_move(source, target, piece,promoted, capture, double_p, en_passant, castling) ((source) | (target << 6) | (piece << 12) | (promoted << 16) | (capture << 20) | (double_p << 21) | (en_passant << 22) | (castling << 23))
#define get_move_source(move) (move & 0x3f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_piece(move) ((move & 0xf000)>> 12)
#define get_move_promoted(move) ((move & 0xf0000) >> 16)
#define get_move_capture(move) ((move & 0x100000) >> 20)
#define get_move_doublepush(move) ((move & 0x200000) >> 21)		
#define get_move_enpassant(move) ((move & 0x400000) >> 22)
#define get_move_castle(move) ((move & 0x800000) >> 23)

//Constants
const U64 Not_A_File = 18374403900871474942ULL;
const U64 Not_H_File = 9187201950435737471ULL;
const U64 Not_AB_File = 18229723555195321596ULL;
const U64 Not_HG_File = 4557430888798830399ULL;

const U64 Rook_Magic_Numbers[64] = { 0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL,
0x100081001000420ULL, 0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL, 0x208808088000400ULL, 0x2802200800400ULL,
0x2200800100020080ULL, 0x801000060821100ULL, 0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL, 0x100400080208000ULL, 0x2040002120081000ULL,
0x21200680100081ULL, 0x20100080080080ULL, 0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL, 0x14800401802800ULL, 0x2080040080800200ULL,
0x124080204001001ULL, 0x200046502000484ULL, 0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL, 0x101002200408200ULL, 0x40802000401080ULL,
0x4008142004410100ULL, 0x2060820c0120200ULL, 0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL, 0x20030a0244872ULL, 0x12001008414402ULL,
0x2006104900a0804ULL, 0x1004081002402ULL };

const U64 Bishop_Magic_Numbers[64] = { 0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL,
0x108060845042010ULL, 0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL, 0x4050404440404ULL,
0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL, 0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL,
0x801010402020200ULL, 0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL, 0x4002801a02003ULL, 0x85040820080400ULL,
0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL, 0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL,
0x20080002081110ULL, 0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL, 0x1c004001012080ULL, 0x8004200962a00220ULL,
0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL, 0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL,
0x42008c0340209202ULL, 0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL, 0x1840060a44020800ULL, 0x90080104000041ULL,
0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL, 0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL,
0x300000261044000aULL, 0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL, 0x4010801011c04ULL, 0xa010109502200ULL,
0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL, 0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL,
0x4010011029020020ULL };

const int Bishop_Relevant_Bit_Counts[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

const int Rook_Relevant_Bit_Counts[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};
const int update_castling_rights[64] = {
	7, 15, 15,15,3,15,15,11,
	15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,
	15,15,15,15,15,15,15,15,
	13,15,15,15,12,15,15,14
};

const char* square_to_Human_Readable_Format[] = {
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

//Enums
enum {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum { white, black, both };

enum { rook, bishop };

enum { WK = 1, WQ = 2, BK = 4, BQ = 8 };

enum { P, N, B, R, Q, K, p, n, b, r, q, k };

U64 Pawn_Attack_Table[2][64];
U64 Knight_Attack_Table[64];
U64 King_Attack_Table[64];

U64 Bishop_Masks[64];
U64 Rook_Masks[64];

U64 Bishop_Attack_Table[64][512];	
U64 Rook_Attack_Table[64][4096];

void Print_Board_State(const Board_State& Board) {

	char ascii_pieces[] = "PNBRQKpnbrqk";

	for (int rank = 0; rank < 8; rank++) {
		std::cout << (8 - rank) << "  ";
		for (int file = 0; file < 8; file++) {
			int square = (rank * 8) + file;
			int Peice = -1;
			for (int i = 0; i < 12; i++) {
				if (get_bit(Board.Bitboards[i], square)) {
					Peice = i;
					break;
				}
			}
			if (Peice == -1) std::cout << " .";
			else std::cout << " " << ascii_pieces[Peice];
		}
		std::cout << "\n";
	}
	std::cout << "\n    a b c d e f g h\n\n";

	if (Board.side == white) std::cout << "White to move\n";
	else std::cout << "Black to move\n";
	if (Board.en_passant != no_sq) std::cout << "En_Passant: " << square_to_Human_Readable_Format[Board.en_passant] << "\n";

	if (Board.castling_rights) {
		std::cout << "Castiling rights: \n";
		if (Board.castling_rights & WK) std::cout << "   White_Kingside\n";
		if (Board.castling_rights & WQ) std::cout << "   White_Queenside\n";
		if (Board.castling_rights & BK) std::cout << "   Black_Kingside\n";
		if (Board.castling_rights & BQ) std::cout << "   Black_Queenside\n";
	}
}

static inline int Least_Signifigant_Bit_Index(const U64& bitboard) {
	if (bitboard) return int(count_bits((bitboard & (0 - bitboard)) - 1));
	else return -1;
}

U64 Mask_Pawn_Attacks(int square, int side) {

	U64 bitboard = 0ULL;

	U64 attacks = 0ULL;

	set_bit(bitboard, square);

	if (!side) {
		if ((bitboard >> 7ULL) & Not_A_File) attacks |= (bitboard >> 7ULL);
		if ((bitboard >> 9ULL) & Not_H_File) attacks |= (bitboard >> 9ULL);
	}
	else {
		if ((bitboard << 7ULL) & Not_H_File) attacks |= (bitboard << 7ULL);
		if ((bitboard << 9ULL) & Not_A_File) attacks |= (bitboard << 9ULL);
	}

	return attacks;
}

U64 Mask_Knight_Attacks(int square) {

	U64 bitboard = 0ULL;

	U64 attacks = 0ULL;

	set_bit(bitboard, square);


	if ((bitboard >> 17) & Not_H_File) attacks |= (bitboard >> 17);
	if ((bitboard >> 15) & Not_A_File) attacks |= (bitboard >> 15);

	if ((bitboard >> 10) & Not_HG_File) attacks |= (bitboard >> 10);
	if ((bitboard >> 6) & Not_AB_File) attacks |= (bitboard >> 6);


	if ((bitboard << 17) & Not_A_File) attacks |= (bitboard << 17);
	if ((bitboard << 15) & Not_H_File) attacks |= (bitboard << 15);

	if ((bitboard << 10) & Not_AB_File) attacks |= (bitboard << 10);
	if ((bitboard << 6) & Not_HG_File) attacks |= (bitboard << 6);

	return attacks;
}

U64 Mask_King_Attacks(int square) {

	U64 bitboard = 0ULL;

	U64 attacks = 0ULL;

	set_bit(bitboard, square);

	attacks |= (bitboard >> 8);
	if ((bitboard >> 1) & Not_H_File) attacks |= (bitboard >> 1);
	if ((bitboard >> 7) & Not_A_File) attacks |= (bitboard >> 7);
	if ((bitboard >> 9) & Not_H_File) attacks |= (bitboard >> 9);

	attacks |= (bitboard << 8);
	if ((bitboard << 1) & Not_A_File) attacks |= (bitboard << 1);
	if ((bitboard << 7) & Not_H_File) attacks |= (bitboard << 7);
	if ((bitboard << 9) & Not_A_File) attacks |= (bitboard << 9);

	return attacks;
}

U64 Mask_Bishop_Attacks(int square) {

	U64 Attacks = 0ULL;

	int target_rank = int(square / 8);
	int target_file = square % 8;

	for (int rank = target_rank + 1, file = target_file + 1; rank < 7 && file < 7; rank++, file++) Attacks |= (1ULL << ((rank * 8) + file));
	for (int rank = target_rank - 1, file = target_file + 1; rank > 0 && file < 7; rank--, file++) Attacks |= (1ULL << ((rank * 8) + file));
	for (int rank = target_rank - 1, file = target_file - 1; rank > 0 && file > 0; rank--, file--) Attacks |= (1ULL << ((rank * 8) + file));
	for (int rank = target_rank + 1, file = target_file - 1; rank < 7 && file > 0; rank++, file--) Attacks |= (1ULL << ((rank * 8) + file));

	return Attacks;
}

U64 Mask_Rook_Attacks(int square) {

	U64 Attacks = 0ULL;

	int target_rank = int(square / 8);
	int target_file = square % 8;

	for (int rank = target_rank + 1; rank < 7; rank++) Attacks |= (1ULL << ((rank * 8) + target_file));
	for (int rank = target_rank - 1; rank > 0; rank--) Attacks |= (1ULL << ((rank * 8) + target_file));
	for (int file = target_file + 1; file < 7; file++) Attacks |= (1ULL << ((target_rank * 8) + file));
	for (int file = target_file - 1; file > 0; file--) Attacks |= (1ULL << ((target_rank * 8) + file));

	return Attacks;
}

U64 Generate_Bishop_Attacks(int square, const U64& block) {

	U64 Attacks = 0ULL;

	int target_rank = int(square / 8);
	int target_file = square % 8;

	for (int rank = target_rank + 1, file = target_file + 1; rank < 8 && file < 8; rank++, file++) {
		Attacks |= (1ULL << ((rank * 8) + file));
		if ((1ULL << ((rank * 8) + file) & block)) break;
	}
	for (int rank = target_rank - 1, file = target_file + 1; rank >= 0 && file < 8; rank--, file++) {
		Attacks |= (1ULL << ((rank * 8) + file));
		if ((1ULL << ((rank * 8) + file) & block)) break;
	}
	for (int rank = target_rank - 1, file = target_file - 1; rank >= 0 && file >= 0; rank--, file--) {
		Attacks |= (1ULL << ((rank * 8) + file));
		if ((1ULL << ((rank * 8) + file) & block)) break;
	}
	for (int rank = target_rank + 1, file = target_file - 1; rank < 8 && file >= 0; rank++, file--) {
		Attacks |= (1ULL << ((rank * 8) + file));
		if ((1ULL << ((rank * 8) + file) & block)) break;
	}

	return Attacks;
}

U64 Generate_Rook_Attacks(int square, const U64& block) {

	U64 Attacks = 0ULL;

	int target_rank = int(square / 8);
	int target_file = square % 8;

	for (int rank = target_rank + 1; rank < 8; rank++) {
		Attacks |= (1ULL << ((rank * 8) + target_file));
		if ((1ULL << ((rank * 8) + target_file)) & block) break;
	}
	for (int rank = target_rank - 1; rank >= 0; rank--) {
		Attacks |= (1ULL << ((rank * 8) + target_file));
		if ((1ULL << ((rank * 8) + target_file)) & block) break;
	}
	for (int file = target_file + 1; file < 8; file++) {
		Attacks |= (1ULL << ((target_rank * 8) + file));
		if ((1ULL << ((target_rank * 8) + file)) & block) break;
	}
	for (int file = target_file - 1; file >= 0; file--) {
		Attacks |= (1ULL << ((target_rank * 8) + file));
		if ((1ULL << ((target_rank * 8) + file)) & block) break;
	}

	return Attacks;
}

U64 Set_Occupancy(int index, int bits_in_mask, U64 Attack_Mask) {

	U64 Occupancy_Map = 0ULL;

	for (int i = 0; i < bits_in_mask; i++) {
		int square = Least_Signifigant_Bit_Index(Attack_Mask);
		pop_bit(Attack_Mask, square);
		if (index & (1 << i)) Occupancy_Map |= (1ULL << square);
	}
	return Occupancy_Map;
}

void Init_Non_Sliding_Attack_Tables() {

	for (int square = 0; square < 64; square++) {
		Pawn_Attack_Table[white][square] = Mask_Pawn_Attacks(square, white);
		Pawn_Attack_Table[black][square] = Mask_Pawn_Attacks(square, black);
		Knight_Attack_Table[square] = Mask_Knight_Attacks(square);
		King_Attack_Table[square] = Mask_King_Attacks(square);
	}

}
void Init_Sliding_Attack_Tables() {

	for (int i = 0; i < 64; i++) {
		Bishop_Masks[i] = Mask_Bishop_Attacks(i);
		Rook_Masks[i] = Mask_Rook_Attacks(i);

		int Bishop_Relevant_Bits = int(count_bits(Bishop_Masks[i]));
		int Bishop_Occupancy_Indicies = (1 << Bishop_Relevant_Bits);

		int Rook_Relevant_Bits = int(count_bits(Rook_Masks[i]));
		int Rook_Occupancy_Indicies = (1 << Rook_Relevant_Bits);

		for (int idx = 0; idx < Bishop_Occupancy_Indicies; idx++) {
			U64 Occupancy = Set_Occupancy(idx, Bishop_Relevant_Bits, Bishop_Masks[i]);

			U64 magic_index = ((Occupancy * Bishop_Magic_Numbers[i]) >> (64 - Bishop_Relevant_Bit_Counts[i]));

			Bishop_Attack_Table[i][int(magic_index)] = Generate_Bishop_Attacks(i, Occupancy);
		}
		for (int idx = 0; idx < Rook_Occupancy_Indicies; idx++) {

			U64 Occupancy = Set_Occupancy(idx, Rook_Relevant_Bits, Rook_Masks[i]);

			U64 magic_index = ((Occupancy * Rook_Magic_Numbers[i]) >> (64 - Rook_Relevant_Bit_Counts[i]));

			Rook_Attack_Table[i][int(magic_index)] = Generate_Rook_Attacks(i, Occupancy);
		}

	}

}

void Init_Pre_Calculation() {
	Init_Non_Sliding_Attack_Tables();
	Init_Sliding_Attack_Tables();
}

U64 Get_Bishop_Attacks(int square, U64 occupancy) {

	occupancy &= Bishop_Masks[square];
	occupancy *= Bishop_Magic_Numbers[square];

	occupancy >>= (64 - Bishop_Relevant_Bit_Counts[square]);

	return Bishop_Attack_Table[square][occupancy];
}

U64 Get_Rook_Attacks(int square, U64 occupancy) {

	occupancy &= Rook_Masks[square];
	occupancy *= Rook_Magic_Numbers[square];
	occupancy >>= (64 - Rook_Relevant_Bit_Counts[square]);

	return Rook_Attack_Table[square][occupancy];
}

U64 Get_Queen_Attacks(int square,U64 occupancy) {

	return (Get_Bishop_Attacks(square, occupancy) | Get_Rook_Attacks(square, occupancy));
}

bool Is_Square_Attacked(int square, int side, const Board_State& Board) {

	if (side == white) {
		if (Pawn_Attack_Table[black][square] & Board.Bitboards[P]) return true;
		if (Knight_Attack_Table[square] & Board.Bitboards[N]) return true;
		if (King_Attack_Table[square] & Board.Bitboards[K]) return true;
		if (Get_Bishop_Attacks(square, Board.Occupancies[both]) & Board.Bitboards[B]) return true;
		if (Get_Rook_Attacks(square, Board.Occupancies[both]) & Board.Bitboards[R]) return true;
		if (Get_Queen_Attacks(square, Board.Occupancies[both]) & Board.Bitboards[Q]) return true;
	}
	else if (side == black) {
		if (Pawn_Attack_Table[white][square] & Board.Bitboards[p]) return true;
		if (Knight_Attack_Table[square] & Board.Bitboards[n]) return true;
		if (King_Attack_Table[square] & Board.Bitboards[k]) return true;
		if (Get_Bishop_Attacks(square, Board.Occupancies[both]) & Board.Bitboards[b]) return true;
		if (Get_Rook_Attacks(square, Board.Occupancies[both]) & Board.Bitboards[r]) return true;
		if (Get_Queen_Attacks(square, Board.Occupancies[both]) & Board.Bitboards[q]) return true;
	}
	return false;
}

void Add_Move(moves& move_list, int Move) {
	move_list.moves[move_list.count] = Move;
	move_list.count++;
}


void Generate_Sudo_Legal_Moves(const Board_State& Board, moves& move_list) {

	U64 Current_Bitboard, Attacks;

	int Source_sq, Target_sq;

	if (Board.side == white) {
		Current_Bitboard = Board.Bitboards[0];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Target_sq = (Source_sq - 8);
			if (!get_bit(Board.Occupancies[both], Target_sq)) {
				if (Target_sq > 7) {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, 0, 0, 0, 0, 0));
				}
				else {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, Q, 0, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, R, 0, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, N, 0, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, B, 0, 0, 0, 0));
				}
				if (Source_sq > 47 && !get_bit(Board.Occupancies[both], (Target_sq - 8))) {
					Add_Move(move_list, encode_move(Source_sq, (Target_sq - 8), P, 0, 0, 1, 0, 0));
				}
			}
			Attacks = (Pawn_Attack_Table[white][Source_sq] & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				if (Target_sq < 8) {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, Q, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, N, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, R, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, B, 1, 0, 0, 0));
				}
				else {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, 0, 1, 0, 0, 0));
				}
				pop_bit(Attacks, Target_sq);
			}
			if (Board.en_passant != no_sq && get_bit(Pawn_Attack_Table[white][Source_sq], Board.en_passant)) {
				Add_Move(move_list, encode_move(Source_sq, Board.en_passant, P, 0, 1, 0, 1, 0));
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[1];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Knight_Attack_Table[Source_sq] & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, N, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = (Knight_Attack_Table[Source_sq] & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, N, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[2];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Bishop_Attacks(Source_sq, Board.Occupancies[both]) & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, B, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = Get_Bishop_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[black];
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, B, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[3];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Rook_Attacks(Source_sq, Board.Occupancies[both]) & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, R, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = (Get_Rook_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, R, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[4];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Queen_Attacks(Source_sq, Board.Occupancies[both]) & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, Q, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = (Get_Queen_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, Q, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[5];
		if (Current_Bitboard) {
			if ((Board.castling_rights & WK) && !get_bit(Board.Occupancies[both], f1) && !get_bit(Board.Occupancies[both], g1) && !Is_Square_Attacked(e1, black, Board) && !Is_Square_Attacked(f1, black, Board)) {
				Add_Move(move_list, encode_move(e1, g1, K, 0, 0, 0, 0, 1));
			}
			if ((Board.castling_rights & WQ) && !get_bit(Board.Occupancies[both], d1) && !get_bit(Board.Occupancies[both], c1) && !get_bit(Board.Occupancies[both], b1) && !Is_Square_Attacked(e1, black, Board) && !Is_Square_Attacked(d1, black, Board)) {
				Add_Move(move_list, encode_move(e1, c1, K, 0, 0, 0, 0, 1));
			}
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (King_Attack_Table[Source_sq] & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, K, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = (King_Attack_Table[Source_sq] & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, K, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}

		}
	}
	else if (Board.side == black) {
		Current_Bitboard = Board.Bitboards[6];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Target_sq = (Source_sq + 8);
			if (!get_bit(Board.Occupancies[both], Target_sq)) {
				if (Target_sq < 56) {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, 0, 0, 0, 0, 0));
				}
				else {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, q, 0, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, r, 0, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, n, 0, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, b, 0, 0, 0, 0));
				}
				if (Source_sq < 16 && !get_bit(Board.Occupancies[both], (Target_sq + 8))) {
					Add_Move(move_list, encode_move(Source_sq, (Target_sq + 8), p, 0, 0, 1, 0, 0));
				}
			}
			Attacks = (Pawn_Attack_Table[black][Source_sq] & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				if (Target_sq > 55) {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, q, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, r, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, n, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, b, 1, 0, 0, 0));
				}
				else {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, 0, 1, 0, 0, 0));
				}
				pop_bit(Attacks, Target_sq);
			}
			if (Board.en_passant != no_sq && get_bit(Pawn_Attack_Table[black][Source_sq], Board.en_passant)) {
				Add_Move(move_list, encode_move(Source_sq, Board.en_passant, p, 0, 1, 0, 1, 0));
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[7];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Knight_Attack_Table[Source_sq] & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, n, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = (Knight_Attack_Table[Source_sq] & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, n, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[8];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Bishop_Attacks(Source_sq, Board.Occupancies[both]) & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, b, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = Get_Bishop_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[white];
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, b, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[9];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Rook_Attacks(Source_sq, Board.Occupancies[both]) & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, r, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = (Get_Rook_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, r, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[10];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Queen_Attacks(Source_sq, Board.Occupancies[both]) & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, q, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = (Get_Queen_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, q, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[11];
		if (Current_Bitboard) {
			if ((Board.castling_rights & BK) && !get_bit(Board.Occupancies[both], f8) && !get_bit(Board.Occupancies[both], g8) && !Is_Square_Attacked(e8, white, Board) && !Is_Square_Attacked(f8, white, Board)) {
				Add_Move(move_list, encode_move(e8, g8, k, 0, 0, 0, 0, 1));
			}
			if ((Board.castling_rights & BQ) && !get_bit(Board.Occupancies[both], d8) && !get_bit(Board.Occupancies[both], c8) && !get_bit(Board.Occupancies[both], b8) && !Is_Square_Attacked(e8, white, Board) && !Is_Square_Attacked(d8, white, Board)) {
				Add_Move(move_list, encode_move(e8, c8, k, 0, 0, 0, 0, 1));
			}
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (King_Attack_Table[Source_sq] & ~Board.Occupancies[both]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, k, 0, 0, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			Attacks = (King_Attack_Table[Source_sq] & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, k, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}

		}
	}
}

void Generate_Sudo_Legal_Captures(const Board_State& Board, moves& move_list) {

	U64 Current_Bitboard, Attacks;

	int Source_sq, Target_sq;

	if (Board.side == white) {
		Current_Bitboard = Board.Bitboards[0];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Pawn_Attack_Table[white][Source_sq] & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				if (Target_sq < 8) {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, Q, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, N, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, R, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, B, 1, 0, 0, 0));
				}
				else {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, P, 0, 1, 0, 0, 0));
				}
				pop_bit(Attacks, Target_sq);
			}
			if (Board.en_passant != no_sq && get_bit(Pawn_Attack_Table[white][Source_sq], Board.en_passant)) {
				Add_Move(move_list, encode_move(Source_sq, Board.en_passant, P, 0, 1, 0, 1, 0));
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[1];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Knight_Attack_Table[Source_sq] & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, N, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[2];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = Get_Bishop_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[black];
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, B, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[3];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Rook_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, R, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[4];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Queen_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, Q, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[5];
		if (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (King_Attack_Table[Source_sq] & Board.Occupancies[black]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, K, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}

		}
	}
	else if (Board.side == black) {
		Current_Bitboard = Board.Bitboards[6];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Pawn_Attack_Table[black][Source_sq] & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				if (Target_sq > 55) {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, q, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, r, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, n, 1, 0, 0, 0));
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, b, 1, 0, 0, 0));
				}
				else {
					Add_Move(move_list, encode_move(Source_sq, Target_sq, p, 0, 1, 0, 0, 0));
				}
				pop_bit(Attacks, Target_sq);
			}
			if (Board.en_passant != no_sq && get_bit(Pawn_Attack_Table[black][Source_sq], Board.en_passant)) {
				Add_Move(move_list, encode_move(Source_sq, Board.en_passant, p, 0, 1, 0, 1, 0));
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[7];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Knight_Attack_Table[Source_sq] & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, n, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[8];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = Get_Bishop_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[white];
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, b, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}

		Current_Bitboard = Board.Bitboards[9];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Rook_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, r, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[10];
		while (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (Get_Queen_Attacks(Source_sq, Board.Occupancies[both]) & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, q, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}
			pop_bit(Current_Bitboard, Source_sq);
		}
		Current_Bitboard = Board.Bitboards[11];
		if (Current_Bitboard) {
			Source_sq = Least_Signifigant_Bit_Index(Current_Bitboard);
			Attacks = (King_Attack_Table[Source_sq] & Board.Occupancies[white]);
			while (Attacks) {
				Target_sq = Least_Signifigant_Bit_Index(Attacks);
				Add_Move(move_list, encode_move(Source_sq, Target_sq, k, 0, 1, 0, 0, 0));
				pop_bit(Attacks, Target_sq);
			}

		}
	}
}

void Make_Move(Board_State& Board, int move) {

	int Peice = get_move_piece(move);
	int Source_sq = get_move_source(move);
	int Target_sq = get_move_target(move);
	int Promoted = get_move_promoted(move);

	if (get_move_capture(move)) {

		pop_bit(Board.Bitboards[Peice], Source_sq);
		set_bit(Board.Bitboards[Peice], Target_sq);


		if (Board.side == white) {
			for (int i = p; i <= k; i++) {
				if (get_bit(Board.Bitboards[i], Target_sq)) {
					pop_bit(Board.Bitboards[i], Target_sq);
					break;
				}
			}
		}
		else {
			for (int i = P; i <= K; i++) {
				if (get_bit(Board.Bitboards[i], Target_sq)) {
					pop_bit(Board.Bitboards[i], Target_sq);
					break;
				}
			}
		}

		if (get_move_enpassant(move)) {
			if (Board.side == white) {
				pop_bit(Board.Bitboards[p], (Target_sq + 8));
			}
			else {
				pop_bit(Board.Bitboards[P], (Target_sq - 8));
			}
		}
		Board.en_passant = no_sq;
	}
	else {

		pop_bit(Board.Bitboards[Peice], Source_sq);

		set_bit(Board.Bitboards[Peice], Target_sq);

		if (get_move_doublepush(move)) {
			if (Board.side == white) {
				Board.en_passant = (Source_sq - 8);
			}
			else {
				Board.en_passant = (Source_sq + 8);
			}
		}
		else {
			Board.en_passant = no_sq;
		}

		if (get_move_castle(move)) {
			if (Source_sq == e1) {
				if (Target_sq == g1) {
					pop_bit(Board.Bitboards[R], h1);
					set_bit(Board.Bitboards[R], f1);
				}
				else {
					pop_bit(Board.Bitboards[R], a1);
					set_bit(Board.Bitboards[R], d1);
				}
			}
			else {
				if (Target_sq == g8) {
					pop_bit(Board.Bitboards[r], h8);
					set_bit(Board.Bitboards[r], f8);

				}
				else {
					pop_bit(Board.Bitboards[r], a8);
					set_bit(Board.Bitboards[r], d8);
				}
			}
		}
	}

	if (Promoted) {
		pop_bit(Board.Bitboards[Peice], Target_sq);
		set_bit(Board.Bitboards[Promoted], Target_sq);
	}

	if (Board.castling_rights) {
		Board.castling_rights &= update_castling_rights[Source_sq];
		Board.castling_rights &= update_castling_rights[Target_sq];
	}

	Board.Occupancies[white] = 0ULL;
	Board.Occupancies[black] = 0ULL;
	Board.Occupancies[both] = 0ULL;
	for (int i = P; i <= K; i++) {
		Board.Occupancies[white] |= Board.Bitboards[i];
	}
	for (int i = p; i <= k; i++) {
		Board.Occupancies[black] |= Board.Bitboards[i];
	}
	Board.Occupancies[both] = (Board.Occupancies[white] | Board.Occupancies[black]);

	Board.side = (Board.side + 1) % 2;
}

void Perft_Driver(const Board_State& Board, int depth, long long& nodes) {

	if (depth == 0) {
		nodes++;
		return;
	}

	moves Move_List;
	Move_List.count = 0;
	Generate_Sudo_Legal_Moves(Board, Move_List);



	Board_State Temp_Board = Board;
	for (int Move = 0; Move < Move_List.count; Move++) {
		Make_Move(Temp_Board, Move_List.moves[Move]);
		if (Board.side == white && Is_Square_Attacked(Least_Signifigant_Bit_Index(Temp_Board.Bitboards[K]), black, Temp_Board)) {
			Temp_Board = Board;
		}
		else if (Board.side == black && Is_Square_Attacked(Least_Signifigant_Bit_Index(Temp_Board.Bitboards[k]), white, Temp_Board)) {
			Temp_Board = Board;
		}
		else {
			Perft_Driver(Temp_Board, depth - 1, nodes);
			Temp_Board = Board;
		}
	}
}

void Perft_Test(const Board_State& Board, int depth, long long& nodes) {
	char Promoted_Pieces[] = " nbrq  nbrq ";
	moves Move_List;
	Move_List.count = 0;
	Generate_Sudo_Legal_Moves(Board, Move_List);

	Board_State Temp_Board = Board;
	for (int Move = 0; Move < Move_List.count; Move++) {
		Make_Move(Temp_Board, Move_List.moves[Move]);
		if (Board.side == white && Is_Square_Attacked(Least_Signifigant_Bit_Index(Temp_Board.Bitboards[K]), black, Temp_Board)) {
			Temp_Board = Board;
		}
		else if (Board.side == black && Is_Square_Attacked(Least_Signifigant_Bit_Index(Temp_Board.Bitboards[k]), white, Temp_Board)) {
			Temp_Board = Board;
		}
		else {
			long long cummulative_nodes = nodes;
			std::cout << square_to_Human_Readable_Format[get_move_source(Move_List.moves[Move])] << square_to_Human_Readable_Format[get_move_target(Move_List.moves[Move])];
			std::cout << Promoted_Pieces[get_move_promoted(Move_List.moves[Move])] << ":";
			Perft_Driver(Temp_Board, depth - 1, nodes);
			std::cout << " " << nodes - cummulative_nodes << "\n";
			Temp_Board = Board;
		}
	}

}

int Evaluate(const Board_State& Board) {

	int Eval = 0;

	Eval += (count_bits(Board.Bitboards[P]) * 100);
	Eval += (count_bits(Board.Bitboards[N]) * 300);
	Eval += (count_bits(Board.Bitboards[B]) * 330);
	Eval += (count_bits(Board.Bitboards[R]) * 500);
	Eval += (count_bits(Board.Bitboards[Q]) * 900);

	Eval -= (count_bits(Board.Bitboards[p]) * 100);
	Eval -= (count_bits(Board.Bitboards[n]) * 300);
	Eval -= (count_bits(Board.Bitboards[b]) * 330);
	Eval -= (count_bits(Board.Bitboards[r]) * 500);
	Eval -= (count_bits(Board.Bitboards[q]) * 900);



	U64 Current_Bitboard = Board.Bitboards[P];
	U64 Attacks = 0ULL;
	int square = no_sq;

	Current_Bitboard &= (Not_A_File & Not_H_File);
	Eval += count_bits(Current_Bitboard);

	Current_Bitboard = Board.Bitboards[N];
	while (Current_Bitboard) {
		square = Least_Signifigant_Bit_Index(Current_Bitboard);
		Attacks = Knight_Attack_Table[square];
		Eval += count_bits(Attacks);
		pop_bit(Current_Bitboard, square);
	}

	Current_Bitboard = Board.Bitboards[B];
	while (Current_Bitboard) {
		square = Least_Signifigant_Bit_Index(Current_Bitboard);
		Attacks = Get_Bishop_Attacks(square, Board.Occupancies[both]);
		Eval += count_bits(Attacks);
		pop_bit(Current_Bitboard, square);
	}

	Current_Bitboard = Board.Bitboards[R];
	while (Current_Bitboard) {
		square = Least_Signifigant_Bit_Index(Current_Bitboard);
		Attacks = Get_Rook_Attacks(square, Board.Occupancies[both]);
		Attacks &= Board.Bitboards[R];
		if (Attacks) Eval += count_bits(8ULL);
		pop_bit(Current_Bitboard, square);
	}

	Current_Bitboard = Board.Bitboards[p];
	Current_Bitboard &= (Not_A_File & Not_H_File);
	Eval += count_bits(Current_Bitboard);

	Current_Bitboard = Board.Bitboards[n];
	while (Current_Bitboard) {
		square = Least_Signifigant_Bit_Index(Current_Bitboard);
		Attacks = Knight_Attack_Table[square];
		Eval -= count_bits(Attacks);
		pop_bit(Current_Bitboard, square);
	}

	Current_Bitboard = Board.Bitboards[b];
	while (Current_Bitboard) {
		square = Least_Signifigant_Bit_Index(Current_Bitboard);
		Attacks = Get_Bishop_Attacks(square, Board.Occupancies[both]);
		Eval -= count_bits(Attacks);
		pop_bit(Current_Bitboard, square);
	}

	Current_Bitboard = Board.Bitboards[r];
	while (Current_Bitboard) {
		square = Least_Signifigant_Bit_Index(Current_Bitboard);
		Attacks = Get_Rook_Attacks(square, Board.Occupancies[both]);
		Attacks &= Board.Bitboards[r];
		if (Attacks) Eval -= count_bits(8ULL);
		pop_bit(Current_Bitboard, square);
	}

	return (Board.side == white) ? Eval : -Eval;
}



