#include "Find.h"


namespace {
	unsigned BitCount(unsigned n) {
		n -= (n >> 1 & 0x55555555);
		n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
		n = (n + (n >> 4)) & 0x0f0f0f0f;
		n += n >> 8;
		n += n >> 16;
		return n & 0x3f;
	}

	constexpr int inv_move[24] = {
		0, 3, 2, 1, 4, 7, 6, 5, 8, 11, 10, 9,
		12, 15, 14, 13, 16, 19, 18, 17, 20, 23, 22, 21};

	bool NotSearched(const Formula &f, int index, int begin_new,
			bool swappable = false) {
		if (swappable) {
			return begin_new >= index;
		} else {
			if (index < 2 || f[index - 2] >> 3 != f[index - 1] >> 3) {
				return begin_new >= index;
			} else {
				return begin_new >= index - 1;
			}
		}
	}
}

bool Find::Finder::ContinueSearch(int depth, int cycles) {
	return solve[depth + 1].length()/* + cycles*/ <= find->fewest_moves;
}

void Find::Finder::Search(int depth, int corner, int edge, int begin, int end) {
	if (corner == 1 && edge == 0) {
		SearchCorner(depth, 1, begin, end);
	} else if (corner == 0 && edge == 1) {
		SearchEdge(depth, 1, begin, end);
	} else {
		int moves = solve[depth].length();
		Cube state;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				state.Twist(solve[depth], i, moves);
				state.Twist(find->scramble_cube);
				state.Twist(solve[depth], 0, i);
			} else {
				state.TwistBefore(inv_move[solve[depth][i - 1]]);
				state.Twist(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (size_t j = 0; j < find->algs; ++j) {
				if (BitCount(statemask & find->algorithm[j].mask) < 2) {
					continue;
				}
				bool changeC = find->algorithm[j].change_corner;
				bool changeE = find->algorithm[j].change_edge;
				if (!Twist(state, find->algorithm[j].state, cube,
							changeC, changeE)) {
					continue;
				}
				int cornerX = changeC ? cube.CornerCycles() : corner;
				int edgeX = changeE ? cube.EdgeCycles() : edge;
				if (cornerX == 0 && edgeX == 0) {
					insert_place[depth] = i;
					for (const Formula &k: find->algorithm[j].GetFormula()) {
						insertion[depth] = &k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].insert(k, i);
						find->SetFewestMoves(*this, depth + 1);
					}
				} else if (cornerX + edgeX < corner + edge) {
					for (const Formula &k: find->algorithm[j].GetFormula()) {
						solve[depth + 1] = solve[depth];
						int begin_new = solve[depth + 1].insert(k, i);
						if (NotSearched(solve[depth], i, begin_new)
								&& ContinueSearch(depth, cornerX + edgeX)) {
							insertion[depth] = &k;
							insert_place[depth] = i;
							Search(depth + 1, cornerX, edgeX, begin_new,
									solve[depth + 1].length());
						}
					}
				}
			}
			if (i > 0 && i < moves
					&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				Cube state_inv(state);
				state_inv.TwistBefore(solve[depth][i]);
				state_inv.TwistBefore(inv_move[solve[depth][i - 1]]);
				state_inv.Twist(solve[depth][i - 1]);
				state_inv.Twist(inv_move[solve[depth][i]]);
				statemask = state_inv.Mask();
				for (size_t j = 0; j < find->algs; ++j) {
					if (BitCount(statemask & find->algorithm[j].mask) < 2) {
						continue;
					}
					bool changeC = find->algorithm[j].change_corner;
					bool changeE = find->algorithm[j].change_edge;
					if (!Twist(state_inv, find->algorithm[j].state, cube,
								changeC, changeE)) {
						continue;
					}
					int cornerX = changeC ? cube.CornerCycles() : corner;
					int edgeX = changeE ? cube.EdgeCycles() : edge;
					if (cornerX == 0 && edgeX == 0) {
						insert_place[depth] = i;
						for (const Formula &k: find->algorithm[j].GetFormula()) {
							insertion[depth] = &k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].insert(k, i);
							find->SetFewestMoves(*this, depth + 1);
						}
					} else if (cornerX + edgeX < corner + edge) {
						for (const Formula &k: find->algorithm[j].GetFormula()) {
							solve[depth + 1] = solve[depth];
							int begin_new = solve[depth + 1].insert(k, i);
							if (NotSearched(solve[depth], i, begin_new, true)
									&& ContinueSearch(depth, cornerX + edgeX)) {
								insertion[depth] = &k;
								insert_place[depth] = i;
								Search(depth + 1, cornerX, edgeX, begin_new,
										solve[depth + 1].length());
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
}

void Find::Finder::SearchCorner(int depth, int corner, int begin, int end) {
	int moves = solve[depth].length();
	if (corner == 1) {
		int index;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				Cube state;
				state.TwistCorner(solve[depth], 0, i, false);
				state.TwistCorner(find->inv_scramble_cube);
				state.TwistCorner(solve[depth], i, moves, false);
				index = state.GetCornerCycleIndex();
			} else {
				index = Cube::corner_cycle_table[
					index * 24 + solve[depth][i - 1]];
			}
			int x = find->corner_cycle_index[index];
			if (x != -1) {
				insert_place[depth] = i;
				for (const Formula &j: find->algorithm[x].GetFormula()) {
					insertion[depth] = &j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].insert(j, i);
					find->SetFewestMoves(*this, depth + 1);
				}
			}
			if (i > 0 && i < moves
					&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::corner_cycle_table
					[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::corner_cycle_table
					[index_inv * 24 + inv_move[solve[depth][i]]];
				x = find->corner_cycle_index[index_inv];
				if (x != -1) {
					insert_place[depth] = i;
					for (const Formula &j: find->algorithm[x].GetFormula()) {
						insertion[depth] = &j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].insert(j, i);
						find->SetFewestMoves(*this, depth + 1);
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	} else {
		Cube state;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				state.TwistCorner(solve[depth], i, moves);
				state.TwistCorner(find->scramble_cube);
				state.TwistCorner(solve[depth], 0, i);
			} else {
				state.TwistCornerBefore(inv_move[solve[depth][i - 1]]);
				state.TwistCorner(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (size_t j = 0; j < find->algs; ++j) {
				if (BitCount(statemask & find->algorithm[j].mask) < 2
					|| !Twist(state, find->algorithm[j].state, cube,
						true, false)) {
					continue;
				}
				int cornerX = cube.CornerCycles();
				if (cornerX == 0) {
					insert_place[depth] = i;
					for (const Formula &k: find->algorithm[j].GetFormula()) {
						insertion[depth] = &k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].insert(k, i);
						find->SetFewestMoves(*this, depth + 1);
					}
				} else if (cornerX < corner) {
					for (const Formula &k: find->algorithm[j].GetFormula()) {
						solve[depth + 1] = solve[depth];
						int begin_new = solve[depth + 1].insert(k, i);
						if (NotSearched(solve[depth], i, begin_new)
								&& ContinueSearch(depth, cornerX)) {
							insertion[depth] = &k;
							insert_place[depth] = i;
							SearchCorner(depth + 1, cornerX, begin_new,
									solve[depth + 1].length());
						}
					}
				}
			}
			if (i > 0 && i < moves
					&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				Cube state0(state);
				state0.TwistCornerBefore(solve[depth][i]);
				state0.TwistCornerBefore(inv_move[solve[depth][i - 1]]);
				state0.TwistCorner(solve[depth][i - 1]);
				state0.TwistCorner(inv_move[solve[depth][i]]);
				statemask = state0.Mask();
				for (size_t j = 0; j < find->algs; ++j) {
					if (BitCount(statemask & find->algorithm[j].mask) < 2
						|| !Twist(state0, find->algorithm[j].state, cube,
							true, false)) {
						continue;
					}
					int cornerX = cube.CornerCycles();
					if (cornerX == 0) {
						insert_place[depth] = i;
						for (const Formula &k: find->algorithm[j].GetFormula()) {
							insertion[depth] = &k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].insert(k, i);
							find->SetFewestMoves(*this, depth + 1);
						}
					} else if (cornerX < corner) {
						for (const Formula &k: find->algorithm[j].GetFormula()) {
							solve[depth + 1] = solve[depth];
							int begin_new = solve[depth + 1].insert(k, i);
							if (NotSearched(solve[depth], i, begin_new, true)
									&& ContinueSearch(depth, cornerX)) {
								insertion[depth] = &k;
								insert_place[depth] = i;
								SearchCorner(depth + 1, cornerX, begin_new,
										solve[depth + 1].length());
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
}

void Find::Finder::SearchEdge(int depth, int edge, int begin, int end) {
	int moves = solve[depth].length();
	if (edge == 1) {
		int index;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				Cube state;
				state.TwistEdge(solve[depth], 0, i, false);
				state.TwistEdge(find->inv_scramble_cube);
				state.TwistEdge(solve[depth], i, moves, false);
				index = state.GetEdgeCycleIndex();
			} else {
				index = Cube::edge_cycle_table[
					index * 24 + solve[depth][i - 1]];
			}
			int x = find->edge_cycle_index[index];
			if (x != -1) {
				insert_place[depth] = i;
				for (const Formula &j: find->algorithm[x].GetFormula()) {
					insertion[depth] = &j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].insert(j, i);
					find->SetFewestMoves(*this, depth + 1);
				}
			}
			if (i > 0 && i < moves
					&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::edge_cycle_table
					[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::edge_cycle_table
					[index_inv * 24 + inv_move[solve[depth][i]]];
				x = find->edge_cycle_index[index_inv];
				if (x != -1) {
					insert_place[depth] = i;
					for (const Formula &j: find->algorithm[x].GetFormula()) {
						insertion[depth] = &j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].insert(j, i);
						find->SetFewestMoves(*this, depth + 1);
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	} else {
		Cube state;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				state.TwistEdge(solve[depth], i, moves);
				state.TwistEdge(find->scramble_cube);
				state.TwistEdge(solve[depth], 0, i);
			} else {
				state.TwistEdgeBefore(inv_move[solve[depth][i - 1]]);
				state.TwistEdge(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (size_t j = 0; j < find->algs; ++j) {
				if (BitCount(statemask & find->algorithm[j].mask) < 2
					|| !Twist(state, find->algorithm[j].state, cube,
						false, true)) {
					continue;
				}
				int edgeX = cube.EdgeCycles();
				if (edgeX == 0) {
					insert_place[depth] = i;
					for (const Formula &k: find->algorithm[j].GetFormula()) {
						insertion[depth] = &k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].insert(k, i);
						find->SetFewestMoves(*this, depth + 1);
					}
				} else if (edgeX < edge) {
					for (const Formula &k: find->algorithm[j].GetFormula()) {
						solve[depth + 1] = solve[depth];
						int begin_new = solve[depth + 1].insert(k, i);
						if (NotSearched(solve[depth], i, begin_new)
								&& ContinueSearch(depth, edgeX)) {
							insertion[depth] = &k;
							insert_place[depth] = i;
							SearchEdge(depth + 1, edgeX, begin_new,
									solve[depth + 1].length());
						}
					}
				}
			}
			if (i > 0 && i < moves
					&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				Cube state0(state);
				state0.TwistEdgeBefore(solve[depth][i]);
				state0.TwistEdgeBefore(inv_move[solve[depth][i - 1]]);
				state0.TwistEdge(solve[depth][i - 1]);
				state0.TwistEdge(inv_move[solve[depth][i]]);
				statemask = state0.Mask();
				for (size_t j = 0; j < find->algs; ++j) {
					if (BitCount(statemask & find->algorithm[j].mask) < 2
						|| !Twist(state0, find->algorithm[j].state, cube,
							false, true)) {
						continue;
					}
					int edgeX = cube.EdgeCycles();
					if (edgeX == 0) {
						insert_place[depth] = i;
						for (const Formula &k: find->algorithm[j].GetFormula()) {
							insertion[depth] = &k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].insert(k, i);
							find->SetFewestMoves(*this, depth + 1);
						}
					} else if (edgeX < edge) {
						for (const Formula &k: find->algorithm[j].GetFormula()) {
							solve[depth + 1] = solve[depth];
							int begin_new = solve[depth + 1].insert(k, i);
							if (NotSearched(solve[depth], i, begin_new, true)
									&& ContinueSearch(depth, edgeX)) {
								insertion[depth] = &k;
								insert_place[depth] = i;
								SearchEdge(depth + 1, edgeX, begin_new,
										solve[depth + 1].length());
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
}
