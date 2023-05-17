#pragma once
#define U64 unsigned long long

void Print_Board_State(const Board_State& Board);

void Init_Pre_Calculation();

bool Is_Square_Attacked(int square, int side, const Board_State& Board);

void Generate_Sudo_Legal_Moves(const Board_State& Board, Moves& move_list);

void Generate_Sudo_Legal_Captures(const Board_State& Board, Moves& move_list);

void Make_Move(Board_State& Board, int move);

void Perft_Test(const Board_State& Board, int depth, long long& nodes);

int Evaluate(const Board_State& Board);
