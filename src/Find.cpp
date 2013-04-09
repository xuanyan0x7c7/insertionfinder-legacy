#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>
#include "Color.h"
#include "Find.h"
#include "I18N.h"
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::ifstream;
using std::function;
using std::int64_t;
using std::lock_guard;
using std::make_pair;
using std::mem_fn;
using std::move;
using std::mutex;
using std::ostringstream;
using std::pair;
using std::ref;
using std::size_t;
using std::string;
using std::thread;
using std::vector;
#ifndef LIBRARY_PATH
	#define LIBRARY_PATH ""
#endif


Find::Find(const Find&) = default;
Find::Find(Find&&) = default;
Find& Find::operator=(const Find&) = default;
Find& Find::operator=(Find&&) = default;
Find::~Find() = default;

size_t Find::fewest_moves;
int Find::cycles_inserted;
Formula Find::best_answer;
std::array<Formula, 20> Find::best_solve;
std::array<Formula, 20> Find::best_insertion;
std::array<size_t, 20> Find::best_insert_place;

Find::Find() {
	corner_cycle_index.fill(-1);
	edge_cycle_index.fill(-1);
}

void Find::ResetFewestMoves() {
	fewest_moves = 0xffffffff;
	cycles_inserted = 0;
}

namespace {
	mutex fm_mutex;
}

void Find::SetFewestMoves(size_t moves, int cycles) {
	if (moves < fewest_moves
			|| (moves == fewest_moves && cycles < cycles_inserted)) {
		lock_guard<mutex> lock(fm_mutex);
		if (moves < fewest_moves
				|| (moves == fewest_moves && cycles < cycles_inserted)) {
			cycles_inserted = cycles;
			best_answer = solve[cycles];
			fewest_moves = moves;
			for (int i = 0; i < cycles; ++i) {
				best_solve[i] = solve[i];
				best_insert_place[i] = insert_place[i];
				best_insertion[i] = insertion[i];
			}
		}
	}
}

pair<string, int64_t>
Find::Solve(const Formula &scramble, const Formula &solve, int include) {
	static const size_t cores = thread::hardware_concurrency();
	vector<Find> finder(cores);
	bool corner_only = (include | 0x7) == 0x7;
	bool edge_only = (include | 0x78) == 0x78;
	for (int i = 0; i < 8; ++i) {
		if (include & (1 << i)) {
			static const string num("01234567");
			ifstream in(LIBRARY_PATH + string("AlgFiles/") + num[i]);
			size_t alg_count;
			in >> alg_count;
			size_t recent_size = finder[0].algorithm.size();
			size_t new_size = recent_size + alg_count;
			finder[0].algorithm.resize(new_size);
			for (size_t i = recent_size; i < new_size; ++i) {
				in >> finder[0].algorithm[i];
			}
			in.close();
		}
	}
	for (size_t i = 0; i < finder[0].algorithm.size(); ++i) {
		const Cube &state = finder[0].algorithm[i].state;
		int corner_cycles = state.CornerCycles();
		int edge_cycles = state.EdgeCycles();
		if (corner_cycles == 1 && edge_cycles == 0) {
			finder[0].corner_cycle_index[state.GetCornerCycleIndex()] = i;
		} else if (corner_cycles == 0 && edge_cycles == 1) {
			finder[0].edge_cycle_index[state.GetEdgeCycleIndex()] = i;
		}
	}

	finder[0].scramble = scramble;
	finder[0].scramble_cube.Twist(scramble);
	finder[0].inv_scramble_cube.Twist(scramble, false);
	finder[0].solve[0] = solve;
	for (size_t i = 1; i < cores; ++i) {
		finder[i] = finder[0];
	}
	
	Cube cube(scramble);
	cube.Twist(solve);
	if ((corner_only && cube.EdgeCycles() > 0)
		|| (edge_only && cube.CornerCycles() > 0)) {
		return make_pair(I18N::NoProperInsertionsFound(), 0);
	}
	int moves = solve.length();
	int cycles[2] = {cube.CornerCycles(), cube.EdgeCycles()};
	ResetFewestMoves();
	vector<int> split(cores + 1);
	split[0] = -1;
	split[cores] = moves;
	for (size_t i = 1; i < cores; ++i) {
		split[cores - i] = static_cast<int>(moves * (1 - pow(static_cast<double>(i) / cores, 0.5)));
	}
	auto start = steady_clock::now();
	vector<thread> t(cores);
	if (corner_only) {
		static function<void(Find&, int, int, int, int)> f = mem_fn(&Find::SearchCorner);
		for (size_t i = 0; i < cores; ++i) {
			t[i] = move(thread(f, ref(finder[i]), 0, cycles[0], split[i] + 1, split[i + 1]));
		}
	} else if (edge_only) {
		static function<void(Find&, int, int, int, int)> f = mem_fn(&Find::SearchEdge);
		for (size_t i = 0; i < cores; ++i) {
			t[i] = move(thread(f, ref(finder[i]), 0, cycles[1], split[i] + 1, split[i + 1]));
		}
	} else {
		static function<void(Find&, int, int, int, int, int)> f = mem_fn(&Find::Search);
		for (size_t i = 0; i < cores; ++i) {
			t[i] = move(thread(f, ref(finder[i]), 0, cycles[0], cycles[1], split[i] + 1, split[i + 1]));
		}
	}
	for (size_t i = 0; i < cores; ++i) {
		t[i].join();
	}
	auto end = steady_clock::now();
	return make_pair(PrintAnswer(), duration_cast<microseconds>(end - start).count());
}

namespace {
	unsigned BitCount(unsigned n) {
		n -= (n >> 1) & 0x55555555;
		n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
		n = (n + (n >> 4)) & 0x0f0f0f0f;
		n += n >> 8;
		n += n >> 16;
		return n & 0x3f;
	}
}

namespace {
	constexpr int inverse_move[24] = {
		0, 3, 2, 1, 4, 7, 6, 5, 8, 11, 10, 9,
		12, 15, 14, 13, 16, 19, 18, 17, 20, 23, 22, 21};
}

void Find::Search(int depth, int corner, int edge, int begin, int end) {
	int moves = solve[depth].length();
	if (corner == 1 && edge == 0) {
		int index;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				Cube state;
				state.TwistCorner(solve[depth], 0, i, false);
				state.TwistCorner(inv_scramble_cube);
				state.TwistCorner(solve[depth], i, moves, false);
				index = state.GetCornerCycleIndex();
			} else {
				index = Cube::corner_cycle_table[index * 24 + solve[depth][i - 1]];
			}
			int x = corner_cycle_index[index];
			if (x != -1) {
				insert_place[depth] = i;
				for (const Formula &j: algorithm[x].GetFormula()) {
					insertion[depth] = j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].Insert(j, i);
					SetFewestMoves(solve[depth + 1].length(), depth + 1);
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::corner_cycle_table
					[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::corner_cycle_table
					[index_inv * 24 + inverse_move[solve[depth][i]]];
				x = corner_cycle_index[index_inv];
				if (x != -1) {
					insert_place[depth] = i;
					for (const Formula &j: algorithm[x].GetFormula()) {
						insertion[depth] = j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(j, i);
						SetFewestMoves(solve[depth + 1].length(), depth + 1);
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	} else if (corner == 0 && edge == 1) {
		int index;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				Cube state;
				state.TwistEdge(solve[depth], 0, i, false);
				state.TwistEdge(inv_scramble_cube);
				state.TwistEdge(solve[depth], i, moves, false);
				index = state.GetEdgeCycleIndex();
			} else {
				index = Cube::edge_cycle_table[index * 24 + solve[depth][i - 1]];
			}
			int x = edge_cycle_index[index];
			if (x != -1) {
				insert_place[depth] = i;
				for (const Formula &j: algorithm[x].GetFormula()) {
					insertion[depth] = j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].Insert(j, i);
					SetFewestMoves(solve[depth + 1].length(), depth + 1);
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::edge_cycle_table
					[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::edge_cycle_table
					[index_inv * 24 + inverse_move[solve[depth][i]]];
				x = edge_cycle_index[index_inv];
				if (x != -1) {
					insert_place[depth] = i;
					for (const Formula &j: algorithm[x].GetFormula()) {
						insertion[depth] = j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(j, i);
						SetFewestMoves(solve[depth + 1].length(), depth + 1);
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	} else {
		int algs = algorithm.size();
		Cube state;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				state.Twist(solve[depth], i, moves);
				state.Twist(scramble_cube);
				state.Twist(solve[depth], 0, i);
			} else {
				state.TwistBefore(inverse_move[solve[depth][i - 1]]);
				state.Twist(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (int j = 0; j < algs; ++j) {
				if (BitCount(statemask & algorithm[j].mask) < 2) {
					continue;
				}
				bool changeC = algorithm[j].change_corner;
				bool changeE = algorithm[j].change_edge;
				if (!Twist(state, algorithm[j].state, cube, changeC, changeE)) {
					continue;
				}
				int cornerX = changeC ? cube.CornerCycles() : corner;
				int edgeX = changeE ? cube.EdgeCycles() : edge;
				if (cornerX + edgeX >= corner + edge) {
					continue;
				}
				if (cornerX == 0 && edgeX == 0) {
					for (const Formula &k: algorithm[j].GetFormula()) {
						insertion[depth] = k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(k, i);
						SetFewestMoves(solve[depth + 1].length(), depth + 1);
					}
				} else {
					for (const Formula &k: algorithm[j].GetFormula()) {
						solve[depth + 1] = solve[depth];
						int begin_new = solve[depth + 1].Insert(k, i);
						if (i < 2 || solve[depth][i - 2] >> 3
							!= solve[depth][i - 1] >> 3) {
							if (begin_new < i) {
								continue;
							}
						} else if (begin_new < i - 1) {
							continue;
						}
						if (solve[depth + 1].length() + cornerX + edgeX
							< fewest_moves) {
							insertion[depth] = k;
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
				state_inv.TwistBefore(inverse_move[solve[depth][i - 1]]);
				state_inv.Twist(solve[depth][i - 1]);
				state_inv.Twist(inverse_move[solve[depth][i]]);
				statemask = state_inv.Mask();
				for (int j = 0; j < algs; ++j) {
					if (BitCount(statemask & algorithm[j].mask) < 2) {
						continue;
					}
					bool changeC = algorithm[j].change_corner;
					bool changeE = algorithm[j].change_edge;
					if (!Twist(state_inv, algorithm[j].state, cube,
						changeC, changeE)) {
						continue;
					}
					int cornerX = changeC ? cube.CornerCycles() : corner;
					int edgeX = changeE ? cube.EdgeCycles() : edge;
					if (cornerX + edgeX >= corner + edge) {
						continue;
					}
					if (cornerX == 0 && edgeX == 0) {
						for (const Formula &k: algorithm[j].GetFormula()) {
							insertion[depth] = k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].Insert(k, i);
							SetFewestMoves(solve[depth + 1].length(), depth + 1);
						}
					} else {
						for (const Formula &k: algorithm[j].GetFormula()) {
							solve[depth + 1] = solve[depth];
							int begin_new = solve[depth + 1].Insert(k, i);
							if (begin_new >= i
								&& solve[depth + 1].length() + cornerX + edgeX
								< fewest_moves) {
								insertion[depth] = k;
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

void Find::SearchCorner(int depth, int corner, int begin, int end) {
	int moves = solve[depth].length();
	if (corner == 1) {
		int index;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				Cube state;
				state.TwistCorner(solve[depth], 0, i, false);
				state.TwistCorner(inv_scramble_cube);
				state.TwistCorner(solve[depth], i, moves, false);
				index = state.GetCornerCycleIndex();
			} else {
				index = Cube::corner_cycle_table[index * 24 + solve[depth][i - 1]];
			}
			int x = corner_cycle_index[index];
			if (x != -1) {
				insert_place[depth] = i;
				for (const Formula &j: algorithm[x].GetFormula()) {
					insertion[depth] = j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].Insert(j, i);
					SetFewestMoves(solve[depth + 1].length(), depth + 1);
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::corner_cycle_table
					[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::corner_cycle_table
					[index_inv * 24 + inverse_move[solve[depth][i]]];
				x = corner_cycle_index[index_inv];
				if (x != -1) {
					insert_place[depth] = i;
					for (const Formula &j: algorithm[x].GetFormula()) {
						insertion[depth] = j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(j, i);
						SetFewestMoves(solve[depth + 1].length(), depth + 1);
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	} else {
		int algs = algorithm.size();
		Cube state;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				state.TwistCorner(solve[depth], i, moves);
				state.TwistCorner(scramble_cube);
				state.TwistCorner(solve[depth], 0, i);
			} else {
				state.TwistCornerBefore(inverse_move[solve[depth][i - 1]]);
				state.TwistCorner(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (int j = 0; j < algs; ++j) {
				if (BitCount(statemask & algorithm[j].mask) < 2
					|| !Twist(state, algorithm[j].state, cube, true, false)) {
					continue;
				}
				int cornerX = cube.CornerCycles();
				if (cornerX >= corner) {
					continue;
				} else if (cornerX == 0) {
					for (const Formula &k: algorithm[j].GetFormula()) {
						insertion[depth] = k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(k, i);
						SetFewestMoves(solve[depth + 1].length(), depth + 1);
					}
				} else {
					for (const Formula &k: algorithm[j].GetFormula()) {
						solve[depth + 1] = solve[depth];
						int begin_new = solve[depth + 1].Insert(k, i);
						if (i < 2 || solve[depth][i - 2] >> 3
							!= solve[depth][i - 1] >> 3) {
							if (begin_new < i) {
								continue;
							}
						} else if (begin_new < i - 1) {
							continue;
						}
						if (solve[depth + 1].length() + cornerX < fewest_moves) {
							insertion[depth] = k;
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
				state0.TwistCornerBefore(inverse_move[solve[depth][i - 1]]);
				state0.TwistCorner(solve[depth][i - 1]);
				state0.TwistCorner(inverse_move[solve[depth][i]]);
				statemask = state0.Mask();
				for (int j = 0; j < algs; ++j) {
					if (BitCount(statemask & algorithm[j].mask) < 2
						|| !Twist(state0, algorithm[j].state, cube,
						true, false)) {
						continue;
					}
					int cornerX = cube.CornerCycles();
					if (cornerX >= corner) {
						continue;
					} else if (cornerX == 0) {
						for (const Formula &k: algorithm[j].GetFormula()) {
							insertion[depth] = k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].Insert(k, i);
							SetFewestMoves(solve[depth + 1].length(), depth + 1);
						}
					} else {
						for (const Formula &k: algorithm[j].GetFormula()) {
							solve[depth + 1] = solve[depth];
							int begin_new = solve[depth + 1].Insert(k, i);
							if (begin_new >= i
								&& solve[depth + 1].length() + cornerX
								< fewest_moves) {
								insertion[depth] = k;
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

void Find::SearchEdge(int depth, int edge, int begin, int end) {
	int moves = solve[depth].length();
	if (edge == 1) {
		int index;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				Cube state;
				state.TwistEdge(solve[depth], 0, i, false);
				state.TwistEdge(inv_scramble_cube);
				state.TwistEdge(solve[depth], i, moves, false);
				index = state.GetEdgeCycleIndex();
			} else {
				index = Cube::edge_cycle_table[index * 24 + solve[depth][i - 1]];
			}
			int x = edge_cycle_index[index];
			if (x != -1) {
				insert_place[depth] = i;
				for (const Formula &j: algorithm[x].GetFormula()) {
					insertion[depth] = j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].Insert(j, i);
					SetFewestMoves(solve[depth + 1].length(), depth + 1);
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3) {
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::edge_cycle_table
					[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::edge_cycle_table
					[index_inv * 24 + inverse_move[solve[depth][i]]];
				x = edge_cycle_index[index_inv];
				if (x != -1) {
					insert_place[depth] = i;
					for (const Formula &j: algorithm[x].GetFormula()) {
						insertion[depth] = j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(j, i);
						SetFewestMoves(solve[depth + 1].length(), depth + 1);
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	} else {
		int algs = algorithm.size();
		Cube state;
		for (int i = begin; i <= end; ++i) {
			if (i == begin) {
				state.TwistEdge(solve[depth], i, moves);
				state.TwistEdge(scramble_cube);
				state.TwistEdge(solve[depth], 0, i);
			} else {
				state.TwistEdgeBefore(inverse_move[solve[depth][i - 1]]);
				state.TwistEdge(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (int j = 0; j < algs; ++j) {
				if (BitCount(statemask & algorithm[j].mask) < 2
					|| !Twist(state, algorithm[j].state, cube, false, true)) {
					continue;
				}
				int edgeX = cube.EdgeCycles();
				if (edgeX >= edge) {
					continue;
				} else if (edgeX == 0) {
					for (const Formula &k: algorithm[j].GetFormula()) {
						insertion[depth] = k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(k, i);
						SetFewestMoves(solve[depth + 1].length(), depth + 1);
					}
				} else {
					for (const Formula &k: algorithm[j].GetFormula()) {
						solve[depth + 1] = solve[depth];
						int begin_new = solve[depth + 1].Insert(k, i);
						if (i < 2 || solve[depth][i - 2] >> 3
							!= solve[depth][i - 1] >> 3) {
							if (begin_new < i) {
								continue;
							}
						} else if (begin_new < i - 1) {
							continue;
						}
						if (solve[depth + 1].length() + edgeX < fewest_moves) {
							insertion[depth] = k;
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
				state0.TwistEdgeBefore(inverse_move[solve[depth][i - 1]]);
				state0.TwistEdge(solve[depth][i - 1]);
				state0.TwistEdge(inverse_move[solve[depth][i]]);
				statemask = state0.Mask();
				for (int j = 0; j < algs; ++j) {
					if (BitCount(statemask & algorithm[j].mask) < 2
						|| !Twist(state0, algorithm[j].state, cube,
						false, true)) {
						continue;
					}
					int edgeX = cube.EdgeCycles();
					if (edgeX >= edge) {
						continue;
					} else if (edgeX == 0) {
						for (const Formula &k: algorithm[j].GetFormula()) {
							insertion[depth] = k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].Insert(k, i);
							SetFewestMoves(solve[depth + 1].length(), depth + 1);
						}
					} else {
						for (const Formula &k: algorithm[j].GetFormula()) {
							solve[depth + 1] = solve[depth];
							int begin_new = solve[depth + 1].Insert(k, i);
							if (begin_new >= i
								&& solve[depth + 1].length() + edgeX
								< fewest_moves) {
								insertion[depth] = k;
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

string Find::PrintAnswer() {
	ostringstream str;
	if (fewest_moves == 0xffffffff) {
		str << I18N::NoProperInsertionsFound();
	} else {
		str << best_solve[0].str(0, best_insert_place[0], true)
			<< green << "[@1]" << reset
			<< best_solve[0].str(best_insert_place[0], best_solve[0].length(),
				false)
			<< '\n' << I18N::InsertAt(1, best_insertion[0].str()) << '\n';
		for (int i = 1; i < cycles_inserted; ++i) {
			str << I18N::AfterInsertion(i, 
					best_solve[i].str(0, best_insert_place[i], true),
					best_solve[i].str(best_insert_place[i],
						best_solve[i].length(), false)) << '\n'
				<< I18N::InsertAt(i + 1, best_insertion[i].str()) << '\n';
		}
		int cancelled_moves = best_solve[0].length() - best_answer.length();
		for (int i = 0; i < cycles_inserted; ++i) {
			cancelled_moves += best_insertion[i].length();
		}
		str << I18N::Moves(fewest_moves, cancelled_moves) << '\n'
			<< I18N::FinalSolution(best_answer.str());
	}
	return str.str();
}

