#pragma once
#include <array>
#include <chrono>
#include <deque>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "Algorithm.h"


class Find {
private:
	std::size_t algs;
	std::vector<Algorithm> algorithm;
	std::array<int, 6144> corner_cycle_index;
	std::array<int, 10240> edge_cycle_index;
	Formula scramble;
	Cube scramble_cube, inv_scramble_cube;
	class Finder {
	public:
		Find *find;
		int cycles_inserted;
		int cancelled_moves;
		std::vector<Formula> solve;
		std::vector<const Formula*> insertion;
		std::vector<size_t> insert_place;
	public:
		Finder(const Finder&) = default;
		Finder(Finder&&) = default;
		Finder& operator =(const Finder&) = default;
		Finder& operator =(Finder&&) = default;
		~Finder() = default;
	public:
		Finder(Find &find):
			find(&find), solve(20), insertion(20), insert_place(20) {}
		friend bool operator <(const Finder &f1, const Finder &f2) {
			return f1.cancelled_moves < f2.cancelled_moves;
		}
	private:
		bool ContinueSearch(int depth, int cycles);
	public:
		void Search(int depth, int corner, int edge, int begin, int end);
		void SearchCorner(int depth, int corner, int begin, int end);
		void SearchEdge(int depth, int edge, int begin, int end);
	};
	std::size_t fewest_moves;
	std::deque<Finder> answers;
public:
	enum {alg_3CP = 0x01, alg_2x2CP = 0x02, alg_CO = 0x04, alg_3EP = 0x08,
		alg_2x2EP = 0x10, alg_EO = 0x20, alg_edge = 0x40, alg_other = 0x80};
private:
	Find();
public:
	static std::pair<std::string, int64_t>
	Solve(const Formula &scramble, const Formula &solve, int alg_include);
private:
	void SetFewestMoves(Finder &finder, int cycles_inserted);
	std::string PrintAnswer();
};
