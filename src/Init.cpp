#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "Algorithm.h"
#include "Init.h"
using std::endl;
using std::ifstream;
using std::map;
using std::move;
using std::ofstream;
using std::string;
using std::vector;


void InitAlgorithmSystem() {
	static const string alg_file[8] = {
		"3CP", "3EP", "2x2CP", "2x2EP", "CO", "EO", "Edges", "Others"};
	static constexpr int file_index[8] =
		{0, 3, 1, 4, 2, 5, 6, 7};
	map<Cube, vector<Formula>> algorithm_set[8];

	for (int i = 0; i < 8; ++i) {
		map<Cube, vector<Formula>> &s = algorithm_set[file_index[i]];
		ifstream file("Algorithm/" + alg_file[i]);
		string str;
		while (getline(file, str)) {
			Formula f(str);
			f.shrink_to_fit();
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
