#pragma once
#define U64 unsigned long long

void Init_Pre_Calculation();

bool Is_Square_Attacked(int square, int side, const Board_State& Board);

void Generate_Sudo_Legal_Moves(const Board_State& Board, moves& move_list);

void Generate_Sudo_Legal_Captures(const Board_State& Board, moves& move_list);

void Make_Move(Board_State& Board, int move);

void Perft_Test(const Board_State& Board, int depth, long long& nodes);

void Bartholomew(Board_State& Board);
