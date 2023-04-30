#pragma once
int Parse_Move(const Board_State& board, std::string move_string);

void Parse_Position(Board_State& board, std::string position);

void Print_Move(int move);

void Parse_Go(Board_State& board, std::string go_command);

void UCI_Loop(Board_State& board);
