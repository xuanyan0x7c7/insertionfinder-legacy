#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include "Init.h"
#include "Algorithm.h"
using std::array;
using std::endl;
using std::ifstream;
using std::ios;
using std::map;
using std::move;
using std::ofstream;
using std::string;
using std::vector;


void InitAlgorithmSystem() {
	static const string alg_file[20] = {
		"3CP", "3EP 6f", "3EP 7f", "3EP 8f", "3EP 9f", "3EP 10f", "2x2CP 10f",
		"2x2CP 11f", "2x2CP 12f", "2x2EP 6f", "2x2EP 7f", "2x2EP 8f",
		"2x2EP 9f", "2x2EP 10f", "2CO", "3CO", "EO", "Edges 6f", "Edges 7f",
		"Others"};
	static constexpr int file_index[20] =
		{0, 3, 3, 3, 3, 3, 1, 1, 1, 4, 4, 4, 4, 4, 2, 2, 5, 6, 6, 7};
	map<Cube, vector<Formula>> algorithm_set[8];

	for (int i = 0; i < 20; ++i) {
		map<Cube, vector<Formula>> &s = algorithm_set[file_index[i]];
		ifstream file("Algorithm/" + alg_file[i]);
		string str;
		while (getline(file, str)) {
			Formula f(str);
			f.Resize();
			Cube cube(f);
			if (!cube.IsParity()) {
				s[cube].push_back(move(f));
			}
		}
		file.close();
	}

	for (int i = 0; i < 8; ++i) {
		static const string num("01234567");
		ofstream file(string("AlgFiles/") + num[i]);
		file << algorithm_set[i].size() << endl;
		for (const auto &alg: algorithm_set[i]) {
			file << alg.first << endl << alg.second.size() << endl;
			for (const Formula &f: alg.second) {
				file << f << endl;
			}
		}
		file.close();
	}
}
