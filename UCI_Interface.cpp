
#include "Board_State_Class.h"
#include "Move_Generation.h"
#include "Search.h"
#include "UCI_Interface.h"
#include <string>
#include <nmmintrin.h>
#include <iostream>


#define count_bits(bitboard) int(_mm_popcnt_u64(bitboard))
#define get_move_source(move) (move & 0x3f)
#define get_move_target(move) ((move & 0xfc0) >> 6)
#define get_move_promoted(move) ((move & 0xf0000) >> 16)

const char* Square_to_Human_Readable_Format[] = {
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};


static int Least_Signifigant_Bit_Index(const U64& bitboard) {
	if (bitboard) return int(count_bits((bitboard & (0 - bitboard)) - 1));
	else return -1;
}

int Parse_Move(const Board_State& board, std::string move_string) {

	Moves move_list = Moves();

	Generate_Sudo_Legal_Moves(board, move_list);

	int source_square = int(move_string[0]) - int('a') + ((8 - int(move_string[1] - int('0'))) * 8);

	int target_square = int(move_string[2]) - int('a') + ((8 - int(move_string[3] - int('0'))) * 8);

	bool move_found_flag = false;

	for (int i = 0; i < move_list.count; i++) {

		if (source_square == get_move_source(move_list.moves[i]) && target_square == get_move_target(move_list.moves[i])) {

			int promoted_peice = get_move_promoted(move_list.moves[i]);

			if (promoted_peice == 0) {
				move_found_flag = true;
			}
			else if ((promoted_peice == 4 || promoted_peice == 10) && move_string[4] == 'q') {
				move_found_flag = true;
			}
			else if ((promoted_peice == 3 || promoted_peice == 9) && move_string[4] == 'r') {
				move_found_flag = true;
			}
			else if ((promoted_peice == 1 || promoted_peice == 7) && move_string[4] == 'n') {
				move_found_flag = true;
			}
			else if ((promoted_peice == 2 || promoted_peice == 8) && move_string[4] == 'b') {
				move_found_flag = true;
			}

			if (move_found_flag) {
				Board_State temp_board_state = board;
				Make_Move(temp_board_state, move_list.moves[i]);
				if (board.side == 0 && Is_Square_Attacked(Least_Signifigant_Bit_Index(temp_board_state.Bitboards[5]), 1, temp_board_state)) {
					return 0;
				}
				else if (board.side == 1 && Is_Square_Attacked(Least_Signifigant_Bit_Index(temp_board_state.Bitboards[11]), 0, temp_board_state)) {
					return 0;
				}
				return move_list.moves[i];
			}
		}

	}

	return 0;
}


void Parse_Position(Board_State& board, std::string position) {

	if (position == "position startpos") {
		board = Board_State();
		return;
	}

	std::size_t moves_idx = position.find("moves");
	std::string current_move;

	if (moves_idx == std::string::npos) {
		board = Board_State(position.substr(13));
		return;
	}
	else if (int(moves_idx) == 18) {
		board = Board_State();
	}
	else {
		board = Board_State(position.substr(13, moves_idx-14));
	}

	for (int i = int(moves_idx)+6; i < position.length(); i++) {

		if (position[i] != ' ') current_move += position[i];
		
		else if (!current_move.empty()) {
			int move = Parse_Move(board, current_move);
			if (move) Make_Move(board, move);
			current_move.clear();
		}
	}

	if (!current_move.empty()) {
		int move = Parse_Move(board, current_move);
		if (move) Make_Move(board, move);
		current_move.clear();
	}

}

void Print_Move(int move) {

	if (get_move_promoted(move)) {
		std::string promoted_peices = " NBRQ  nbrq ";
		std::cout << Square_to_Human_Readable_Format[get_move_source(move)] << Square_to_Human_Readable_Format[get_move_target(move)]<<promoted_peices[get_move_promoted(move)];
	}
	else {
		std::cout << Square_to_Human_Readable_Format[get_move_source(move)] << Square_to_Human_Readable_Format[get_move_target(move)];
	}

}

void Parse_Go(Board_State& board, std::string go_command) {
	
	int depth = -1;
	
	if (go_command.substr(0, 8) == "go depth") {
		depth = stoi(go_command.substr(9));
	}

	//will add time controls here. 

	if (depth > 0) {
		Bartholomew(board, depth);
	}
}

void UCI_Loop(Board_State& board) {

	std::string message;
	std::getline(std::cin, message);

	if (message == "uci") {

		std::cout << "id name Bartholomew_3.0" << std::endl;
		std::cout << "id author ConradWarren" << std::endl;
		std::cout << "uciok" << std::endl;

		while (true) {

			std::getline(std::cin, message);

			if (message.substr(0, 7) == "isready") {
				std::cout << "readyok"<<std::endl;
				continue;
			}

			if (message.substr(0, 8) == "position") {
				Parse_Position(board, message);
			}

			else if (message.substr(0, 10) == "ucinewgame") {
				Parse_Position(board, "position startpos");
			}

			else if (message.substr(0, 2) == "go") {
				Parse_Go(board, message);
			}
			
			else if (message.substr(0, 4) == "quit") {
				break;
			}
		}
	}

}

