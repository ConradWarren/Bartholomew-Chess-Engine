#include <iostream>
#include <string>

#include "Board_State_Class.h"
#include "Move_Generation.h"
#include "Search.h"
#include "UCI_Interface.h"


int main(void) {

	Init_Pre_Calculation();

	Init_Zobrist_Keys();

	Board_State board = Board_State();

	UCI_Loop(board);

	return 0; 
}
