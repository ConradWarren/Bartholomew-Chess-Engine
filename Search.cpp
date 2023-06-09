#include "Board_State_Class.h"
#include "PV_Class.h"
#include "Search.h"
#include "Move_Generation.h"
#include "Transposition_Table_Class.h"
#include "UCI_Interface.h"

#include <iostream>
#include <nmmintrin.h>

#define U64 unsigned long long
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define count_bits(bitboard) int(_mm_popcnt_u64(bitboard))

#define get_move_source(move) (move & 0x3f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_piece(move) ((move & 0xf000)>> 12)
#define get_move_promoted(move) ((move & 0xf0000) >> 16)
#define get_move_capture(move) ((move & 0x100000) >> 20)
#define get_move_doublepush(move) ((move & 0x200000) >> 21)		
#define get_move_enpassant(move) ((move & 0x400000) >> 22)
#define get_move_castle(move) ((move & 0x800000) >> 23)



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

const int hash_size = 0x400000;
const int no_hash_entry = 1000000;

const int hash_flag_exact = 0;
const int hash_flag_alpha = 1;
const int hash_flag_beta = 2;

const int full_depth_moves = 4;
const int reduction_limit = 3;

const static int mvv_lva[12][12] = {
	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

transpostion_table transpositon_cache[hash_size];

void Clear_Transposition_Cache() {

	transpostion_table empty_hash = transpostion_table();
	
	for (int i = 0; i < hash_size; i++) {
		transpositon_cache[i] = empty_hash;
	}

}

int Read_Table_Entry(int alpha, int beta, int depth, U64 key) {

	transpostion_table* entry = &transpositon_cache[int((key % hash_size))];

	if (entry->key == key && entry->depth == depth) {
		
		if (entry->flag == hash_flag_exact) {
			return entry->value;
		}

		if (entry->flag == hash_flag_alpha && entry->value <= alpha) {
			return alpha;
		}

		if (entry->flag == hash_flag_beta && entry->value >= beta) {
			return beta;
		}
	}

	return no_hash_entry;
}

void Add_Table_Entry(int score, int depth, int hash_flag, U64 key) {

	transpostion_table* entry = &transpositon_cache[int((key % hash_size))];

	entry->key = key;
	entry->value = score;
	entry->depth = depth;
	entry->flag = hash_flag;
}

static int Least_Signifigant_Bit_Index(const U64& bitboard) {
	if (bitboard) return int(count_bits((bitboard & (0 - bitboard)) - 1));
	else return -1;
}

inline void Enable_PV_scoring(const Moves& move_list, PV& pv) {

	pv.follow_pv_flag = false;

	for (int i = 0; i < move_list.count; i++) {
		if (pv.pv_table[0][pv.ply] == move_list.moves[i]) {

			pv.score_pv_flag = true;

			pv.follow_pv_flag = true;

			break;
		}
	}
}

inline int Score_Move(int move, const Board_State& board, PV& pv) {

	if (pv.score_pv_flag && pv.pv_table[0][pv.ply] == move) {
		pv.score_pv_flag = false;
		return 20000;
	}

	if (get_move_capture(move)) {
		for (int i = 0; i < 12; i++) {
			if (get_bit(board.Bitboards[i], get_move_target(move))) {
				return (mvv_lva[get_move_piece(move)][i] + 10000);
			}
		}
		return 10105;
	}
	else {

		if (pv.killer_moves[0][pv.ply] == move) {
			return 9000;
		}
		else if (pv.killer_moves[1][pv.ply] == move) {
			return 8000;
		}
		else {
			return pv.history_moves[get_move_piece(move)][get_move_target(move)];
		}

	}

}

inline void Sort_Moves(Moves& move_list, const Board_State& Board, PV& pv) {

	int move_score[256];

	for (int i = 0; i < move_list.count; i++) {
		move_score[i] = Score_Move(move_list.moves[i], Board, pv);
	}

	//Currently a dual swap sort system, O(n^2) time complexty and an extra 256 integers are being allocated to the stack. 
	//Built in sort functions are few orders of magnitude slower due to extra memory allocation.
	//Will need to be made into a quick sort for performace later

	int Temp_Score, Temp_Move;
	for (int current_move = 0; current_move < move_list.count; current_move++) {
		for (int next_move = current_move + 1; next_move < move_list.count; next_move++) {
			if (move_score[current_move] < move_score[next_move]) {

				Temp_Score = move_score[current_move];
				move_score[current_move] = move_score[next_move];
				move_score[next_move] = Temp_Score;

				Temp_Move = move_list.moves[current_move];
				move_list.moves[current_move] = move_list.moves[next_move];
				move_list.moves[next_move] = Temp_Move;
			}
		}
	}
}
inline int Capture_Negamax_Search(int alpha, int beta, PV& pv, const Board_State& board) {

	pv.nodes++;

	int Eval = Evaluate(board);

	if (Eval >= beta) {
		return beta;
	}

	if (Eval > alpha) {
		alpha = Eval;
	}

	Moves move_list = Moves();
	Generate_Sudo_Legal_Captures(board, move_list);

	Sort_Moves(move_list, board, pv);

	Board_State Temp_Board = board;
	for (int i = 0; i < move_list.count; i++) {
		pv.ply++;
		Make_Move(Temp_Board, move_list.moves[i]);
		if (board.side == white && Is_Square_Attacked(Least_Signifigant_Bit_Index(Temp_Board.Bitboards[K]), black, Temp_Board)) {
			pv.ply--;
			Temp_Board = board;
			continue;
		}
		else if (board.side == black && Is_Square_Attacked(Least_Signifigant_Bit_Index(Temp_Board.Bitboards[k]), white, Temp_Board)) {
			pv.ply--;
			Temp_Board = board;
			continue;
		}

		int score = -Capture_Negamax_Search(-beta, -alpha, pv, Temp_Board);

		pv.ply--;

		Temp_Board = board;

		if (score >= beta) {
			return beta;
		}

		if (score > alpha) {
			alpha = score;
		}
	}
	return alpha;
}

inline int Negamax_Search(int alpha, int beta, int depth, PV& pv, const Board_State& board) {


	int hash_flag = hash_flag_alpha;

	int score = Read_Table_Entry(alpha, beta, depth, board.position_key);
	
	if (score != no_hash_entry) {
		return score;
	}

	pv.pv_length[pv.ply] = pv.ply;

	if (depth == 0) {
		return Capture_Negamax_Search(alpha, beta, pv, board);
	}

	if (pv.ply >= max_ply) {
		return Evaluate(board);
	}

	pv.nodes++;

	bool found_move_flag = false;

	int moves_searched = 0;

	Moves move_list = Moves();

	Generate_Sudo_Legal_Moves(board, move_list);

	if (pv.follow_pv_flag) {
		Enable_PV_scoring(move_list, pv);
	}

	Sort_Moves(move_list, board, pv);

	int legal_moves = 0;
	bool in_check = Is_Square_Attacked((board.side == white) ? Least_Signifigant_Bit_Index(board.Bitboards[K]) : Least_Signifigant_Bit_Index(board.Bitboards[k]), board.side ^ 1, board);

	if (in_check) {
		depth++;
	}

	Board_State temp_board = board;
	for (int i = 0; i < move_list.count; i++) {
		pv.ply++;
		Make_Move(temp_board, move_list.moves[i]);
		if (board.side == white && Is_Square_Attacked(Least_Signifigant_Bit_Index(temp_board.Bitboards[K]), black, temp_board)) {
			pv.ply--;
			temp_board = board;
			continue;
		}
		else if (board.side == black && Is_Square_Attacked(Least_Signifigant_Bit_Index(temp_board.Bitboards[k]), white, temp_board)) {
			pv.ply--;
			temp_board = board;
			continue;
		}
		legal_moves++;

		

		if (found_move_flag) {
			score = -Negamax_Search(-alpha - 1, -alpha, depth - 1, pv, temp_board);
			if ((score > alpha) && (score < beta)) {
				score = -Negamax_Search(-beta, -alpha, depth - 1, pv, temp_board);
			}
		}
		else if (moves_searched == 0) {
			score = -Negamax_Search(-beta, -alpha, depth - 1, pv, temp_board);
		}
		else {
	
			if (moves_searched >= full_depth_moves && depth >= reduction_limit && !in_check && !get_move_capture(move_list.moves[i]) &&!get_move_promoted(move_list.moves[i])) {
				score = -Negamax_Search(-alpha - 1, -alpha, depth - 2, pv, temp_board);
			}
			else {
				score = alpha + 1;
			}

			if (score > alpha) {
				score = -Negamax_Search(-alpha - 1, -alpha, depth - 1, pv, temp_board);
				if ((score > alpha) && (score < beta)) {
					score = -Negamax_Search(-beta, -alpha, depth - 1, pv, temp_board);
				}
			}
		}
		
		pv.ply--;
		temp_board = board;
		moves_searched++;

		if (score >= beta) {

			Add_Table_Entry(beta, depth, hash_flag_beta, temp_board.position_key);

			if (!get_move_capture(move_list.moves[i])) {
				pv.killer_moves[1][pv.ply] = pv.killer_moves[0][pv.ply];
				pv.killer_moves[0][pv.ply] = move_list.moves[i];
			}
			return beta;
		}

		if (score > alpha) {

			hash_flag = hash_flag_exact;

			alpha = score;

			found_move_flag = true;

			if (!get_move_capture(move_list.moves[i])) {
				pv.history_moves[get_move_piece(move_list.moves[i])][get_move_target(move_list.moves[i])] += depth;
			}

			pv.pv_table[pv.ply][pv.ply] = move_list.moves[i];
			for (int next_ply = pv.ply + 1; next_ply < pv.pv_length[pv.ply + 1]; next_ply++) {
				pv.pv_table[pv.ply][next_ply] = pv.pv_table[pv.ply + 1][next_ply];
			}
			pv.pv_length[pv.ply] = pv.pv_length[pv.ply + 1];

		}
	}

	if (legal_moves == 0) {
		if (in_check) {
			return -99000 + pv.ply;
		}
		else {
			return 0;
		}
	}

	Add_Table_Entry(alpha, depth, hash_flag, board.position_key);

	return alpha;
}

void Bartholomew(Board_State& Board, int depth) {

	PV pv = PV();

	Clear_Transposition_Cache();

	int score = 0;
	for (int current_depth = 1; current_depth <= depth; current_depth++) {
		pv.follow_pv_flag = true;
		score = Negamax_Search(-100000, 100000, current_depth, pv, Board);
	}

	if (pv.pv_table[0][0]) {
		Make_Move(Board, pv.pv_table[0][0]);
	}
	for (int i = 0; i < pv.pv_length[0]; i++) {
		Print_Move(pv.pv_table[0][i]);
		std::cout << " ";
	}
	std::cout << '\n';
	if (pv.pv_table[0][0]) {
		std::cout << "bestmove ";
		Print_Move(pv.pv_table[0][0]);
		std::cout << '\n';
	}
	

}
