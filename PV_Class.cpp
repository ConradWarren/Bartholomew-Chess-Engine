#include "PV_Class.h"

PV::PV() {
	ply = 0;
	follow_pv_flag = false;
	score_pv_flag = false;

	for (int i = 0; i < max_ply; i++) {
		pv_length[i] = 0;
		killer_moves[0][i] = 0;
		killer_moves[1][i] = 0;
		for (int j = 0; j < max_ply; j++) {
			pv_table[i][j] = 0;
		}
	}
	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 64; j++) {
			history_moves[i][j] = 0;
		}
	}
}
