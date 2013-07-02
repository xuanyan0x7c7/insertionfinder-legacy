#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "CommandLineParser.h"
#include "Find.h"
#include "I18N.h"
#include "Init.h"
using namespace std;
using namespace chrono;


void Verify(const Formula &scramble, const Formula &solve) {
	Cube cube(scramble);
	cube.Twist(solve);
	cout << I18N::Scramble(scramble.str()) << endl;
	cout << I18N::Solve(solve.str()) << endl;
	if (cube.Mask() == 0) {
		cout << I18N::TheCubeIsSolved() << endl;
	} else if (cube.IsParity()) {
		cout << I18N::ThereIsParity() << endl;
	} else {
		int corner_cycles = cube.CornerCycles();
		int edge_cycles = cube.EdgeCycles();
		cout << I18N::Cycles(corner_cycles, edge_cycles) << endl;
	}
}

void Solve(const Formula &scramble, const Formula &solve, int include) {
	Verify(scramble, solve);
	Cube cube(scramble);
	cube.Twist(solve);
	if (cube.Mask() != 0 && !cube.IsParity()) {
		auto start = steady_clock::now();
		auto result = Find::Solve(scramble, solve, include);
		auto end = steady_clock::now();
		cout << result.first << endl
			<< I18N::Runtime(result.second,
				duration_cast<microseconds>(end - start).count()) << endl;
	}
}

void Verify(istream &in) {
	string str;
	getline(in, str);
	Formula scramble(str);
	getline(in, str);
	Formula solve(str);
	Verify(scramble, solve);
}

void VerifyMulti(istream &in) {
	string str;
	getline(in, str);
	Formula scramble(str);
	while (getline(in, str)) {
		Formula solve(str);
		Verify(scramble, solve);
	}
}

void Solve(istream &in) {
	string str;
	getline(in, str);
	Formula scramble(str);
	getline(in, str);
	Formula solve(str);
	int include;
	in >> include;
	Solve(scramble, solve, include);
}

void SolveMulti(istream &in) {
	string str;
	getline(in, str);
	Formula scramble(str);
	while (getline(in, str)) {
		int include;
		in >> include;
		Formula solve(str);
		Solve(scramble, solve, include);
	}
}

int main(int argc, char **argv) {
	auto command = Parse(argc, argv);
	vector<string> files;
	int type = -1;
	for (const auto &com: command) {
		if (com.first == "init") {
			InitAlgorithmSystem();
		} else if (com.first == "") {
			files.push_back(com.second);
		} else if (com.first == "s" || com.first == "solve") {
			type = 0;
		} else if (com.first == "v" || com.first == "verify") {
			type = 1;
		} else if (com.first == "m" || com.first == "multi") {
			if (type >= 0) {
				type += 2;
			}
		}
	}
	if (type == 0) {
		if (files.empty()) {
			Solve(cin);
		} else {
			for (const string &file: files) {
				ifstream in(file);
				Solve(in);
			}
		}
	} else if (type == 1) {
		if (files.empty()) {
			Verify(cin);
		} else {
			for (const string &file: files) {
				ifstream in(file);
				Verify(in);
			}
		}
	} else if (type == 2) {
		if (files.empty()) {
			Solve(cin);
		} else {
			for (const string &file: files) {
				ifstream in(file);
				SolveMulti(in);
			}
		}
	} else if (type == 3) {
		if (files.empty()) {
			Verify(cin);
		} else {
			for (const string &file: files) {
				ifstream in(file);
				VerifyMulti(in);
			}
		}
	}
	return 0;
}
