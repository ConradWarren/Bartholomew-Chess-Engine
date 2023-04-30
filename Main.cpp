#include <iostream>
#include <string>

#include "Board_State_Class.h"
#include "Move_Generation.h"
#include "Search.h"
#include "UCI_Interface.h"

int main(void) {

	Init_Pre_Calculation();

	Board_State Board = Board_State();

	UCI_Loop(Board);

	return 0; 
}
