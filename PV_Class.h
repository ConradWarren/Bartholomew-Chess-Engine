#pragma once
#define max_ply 64

class PV {
public:
	PV();
	int nodes;
	int ply;
	int killer_moves[2][max_ply];
	int history_moves[12][64];
	int pv_length[max_ply];
	int pv_table[max_ply][max_ply];
	bool follow_pv_flag;
	bool score_pv_flag;
};
