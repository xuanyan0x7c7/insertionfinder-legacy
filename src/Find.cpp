#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <limits>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>
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
using std::numeric_limits;
using std::ostringstream;
using std::pair;
using std::ref;
using std::size_t;
using std::sort;
using std::string;
using std::thread;
using std::vector;
#ifndef LIBRARY_PATH
	#define LIBRARY_PATH ""
#endif


Find::Find() {
	corner_cycle_index.fill(-1);
	edge_cycle_index.fill(-1);
	fewest_moves = numeric_limits<size_t>::max();
}

namespace {
	mutex fm_mutex;
}

pair<string, int64_t>
Find::Solve(const Formula &scramble, const Formula &solve, int include) {
	static const size_t cores = thread::hardware_concurrency();
	Find find;
	bool corner_only = (include | 0x7) == 0x7;
	bool edge_only = (include | 0x78) == 0x78;
	find.algs = 0;
	for (size_t i = 0; i < 8; ++i) {
		if (include & (1 << i)) {
			static const string num("01234567");
			ifstream in(LIBRARY_PATH + string("AlgFiles/") + num[i]);
			size_t alg_count;
			in >> alg_count;
			size_t new_size = find.algs + alg_count;
			find.algorithm.resize(new_size);
			for (size_t i = find.algs; i < new_size; ++i) {
				in >> find.algorithm[i];
			}
			find.algs = new_size;
			in.close();
		}
	}
	for (size_t i = 0; i < find.algs; ++i) {
		const Cube &state = find.algorithm[i].state;
		int corner_cycles = state.CornerCycles();
		int edge_cycles = state.EdgeCycles();
		if (corner_cycles == 1 && edge_cycles == 0) {
			find.corner_cycle_index[state.GetCornerCycleIndex()] = i;
		} else if (corner_cycles == 0 && edge_cycles == 1) {
			find.edge_cycle_index[state.GetEdgeCycleIndex()] = i;
		}
	}
	find.scramble = scramble;
	find.scramble_cube.Twist(scramble);
	find.inv_scramble_cube.Twist(scramble, false);
	vector<Finder> finder;
	for (size_t i = 0; i < cores; ++i) {
		finder.emplace_back(find);
		finder[i].solve[0] = solve;
	}
	
	Cube cube(scramble);
	cube.Twist(solve);
	if ((corner_only && cube.EdgeCycles() > 0)
		|| (edge_only && cube.CornerCycles() > 0)) {
		return make_pair(I18N::NoProperInsertionsFound(), 0);
	}
	int moves = solve.length();
	int cycles[2] = {cube.CornerCycles(), cube.EdgeCycles()};
	vector<int> split(cores + 1);
	split[0] = -1;
	split[cores] = moves;
	for (size_t i = 1; i < cores; ++i) {
		split[cores - i] = static_cast<int>
			(moves * (1 - pow(static_cast<double>(i) / cores, 0.5)));
	}
	auto start = steady_clock::now();
	vector<thread> t(cores);
	if (corner_only) {
		static function<void(Finder&, int, int, int, int)> f =
			mem_fn(&Finder::SearchCorner);
		for (size_t i = 0; i < cores; ++i) {
			t[i] = move(thread(f, ref(finder[i]), 0, cycles[0],
				split[i] + 1, split[i + 1]));
		}
	} else if (edge_only) {
		static function<void(Finder&, int, int, int, int)> f =
			mem_fn(&Finder::SearchEdge);
		for (size_t i = 0; i < cores; ++i) {
			t[i] = move(thread(f, ref(finder[i]), 0, cycles[1],
				split[i] + 1, split[i + 1]));
		}
	} else {
		static function<void(Finder&, int, int, int, int, int)> f =
			mem_fn(&Finder::Search);
		for (size_t i = 0; i < cores; ++i) {
			t[i] = move(thread(f, ref(finder[i]), 0, cycles[0], cycles[1],
				split[i] + 1, split[i + 1]));
		}
	}
	for (size_t i = 0; i < cores; ++i) {
		t[i].join();
	}
	auto end = steady_clock::now();
	sort(find.answers.begin(), find.answers.end());
	return make_pair(find.PrintAnswer(),
		duration_cast<microseconds>(end - start).count());
}

void Find::SetFewestMoves(Finder &finder, int cycles_inserted) {
	lock_guard<mutex> lock(fm_mutex);
	size_t moves = finder.solve[cycles_inserted].length();
	if (moves <= fewest_moves) {
		if (moves < fewest_moves) {
			fewest_moves = moves;
			answers.clear();
		}
		finder.cycles_inserted = cycles_inserted;
		finder.cancelled_moves = finder.solve[0].length()
			- finder.solve[finder.cycles_inserted].length();
		for (int i = 0; i < finder.cycles_inserted; ++i) {
			finder.cancelled_moves += finder.insertion[i]->length();
		}
		answers.push_back(finder);
	}
}

string Find::PrintAnswer() {
	ostringstream out;
	if (answers.empty()) {
		out << I18N::NoProperInsertionsFound() << '\n';
	} else {
		for (const Finder &finder: answers) {
			const auto &solve = finder.solve;
			const auto &insertion = finder.insertion;
			const auto &insert_place = finder.insert_place;
			int cycles_inserted = finder.cycles_inserted;
			out << '\n';
			out << solve[0].str(0, insert_place[0], true) << "[@1]"
				<< solve[0].str(insert_place[0], solve[0].length(), false)
				<< '\n' << I18N::InsertAt(1, insertion[0]->str()) << '\n';
			for (int i = 1; i < cycles_inserted; ++i) {
				out << I18N::AfterInsertion(i,
						solve[i].str(0, insert_place[i], true),
						solve[i].str(insert_place[i], solve[i].length(), false)) << '\n'
					<< I18N::InsertAt(i + 1, insertion[i]->str()) << '\n';
			}
			out << I18N::Moves(fewest_moves, finder.cancelled_moves) << '\n'
				<< I18N::FinalSolution(solve[cycles_inserted].str()) << '\n';
		}
	}
	return out.str();
}
