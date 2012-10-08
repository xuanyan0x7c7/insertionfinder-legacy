#pragma once
#include <array>
#include <utility>
#include <vector>
#include "Algorithm.h"


class Find {
private:
	std::vector<Algorithm> algorithm;
	std::array<int, 6144> corner_cycle_index;
	std::array<int, 10240> edge_cycle_index;
	size_t fewest_moves = 0xffffffff;
	Formula best_answer;
	std::array<Formula, 20> best_solve;
	std::array<Formula, 20> best_insertion;
	std::array<size_t, 20> best_insert_place;
	Formula scramble;
	Cube scramble_cube, inv_scramble_cube;
	std::array<Formula, 20> solve;
	std::array<Formula, 20> insertion;
	std::array<size_t, 20> insert_place;
	int cycles_inserted = 0;
	const std::string &language;
public:
	enum {alg_3CP = 0x01, alg_2x2CP = 0x02, alg_CO = 0x04, alg_3EP = 0x08,
		alg_2x2EP = 0x10, alg_EO = 0x20, alg_edge = 0x40, alg_other = 0x80};
private:
	Find(const Find&);
	Find(Find&&);
	Find& operator=(const Find&);
	Find& operator=(Find&&);
	~Find();
private:
	Find(const std::string&);
public:
	static std::pair<std::string, long>
	Solve(const Formula&, const Formula&, int alg_include, std::string);
private:
	void Search(int ,int, int, int);
	void SearchCorner(int, int, int);
	void SearchEdge(int, int, int);
	std::string PrintAnswer();
};

