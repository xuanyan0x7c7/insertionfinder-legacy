#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "CommandLineResolver.h"
#include "Find.h"
#include "Init.h"
using namespace std;
using namespace chrono;


namespace {
	string language("zh_cn");
	
	void Solve(const string &file) {
		ifstream in(file);
		string str;
		getline(in, str);
		Formula scramble(str);
		getline(in, str);
		Formula solve(str);
		int include;
		in >> include;
		in.close();
		Cube cube(scramble);
		cube.Twist(solve);
		if (cube.Mask() == 0) {
			if (language == "zh_cn") {
				cout << "已经还原" << endl;
			} else {
				cout << "The cube is solved" << endl;
			}
		} else if (cube.IsParity()) {
			if (language == "zh_cn") {
				cout << "有Parity" << endl;
			} else {
				cout << "There's parity" << endl;
			}
		} else {
			int corner_cycles = cube.CornerCycles();
			int edge_cycles = cube.EdgeCycles();
			if (language == "zh_cn") {
				cout << "需要插入";
				if (corner_cycles == 0) {
					cout << edge_cycles << "个棱块三循环";
				} else {
					cout << corner_cycles << "个角块三循环";
					if (edge_cycles > 0) {
						cout << "和" << edge_cycles << "个棱块三循环";
					}
				}
				cout << endl;
			} else {
				if (corner_cycles == 0) {
					if (edge_cycles == 1) {
						cout << "1 edge 3-cycle";
					} else {
						cout << edge_cycles << " edge 3-cycles";
					}
				} else {
					if (corner_cycles == 1) {
						cout << "1 corner 3-cycle";
					} else {
						cout << corner_cycles << " corner 3-cycles";
					}
					if (edge_cycles > 0) {
						cout << " and ";
						if (edge_cycles == 1) {
							cout << "1 edge 3-cycle";
						} else {
							cout << edge_cycles << " edge 3-cycles";
						}
					}
				}
				cout << " needed" << endl;
			}
			auto start = steady_clock::now();
			auto result = Find::Solve(scramble, solve, include, language);
			auto end = steady_clock::now();
			if (language == "zh_cn") {
				cout << result.first << endl
					<< "用时" << result.second << '/'
					<< duration_cast<microseconds>(end - start).count()
					<< "微秒" << endl;
			} else {
				cout << result.first << endl
					<< "Uses " << result.second << '/'
					<< duration_cast<microseconds>(end - start).count()
					<< " microseconds" << endl;
			}
		}
	}

	void Verify(const string &file)
	{
		ifstream in(file);
		string str;
		getline(in, str);
		Formula scramble(str);
		getline(in, str);
		Formula solve(str);
		in.close();
		Cube cube(scramble);
		cube.Twist(solve);
		if (cube.Mask() == 0) {
			if (language == "zh_cn") {
				cout << "已经还原" << endl;
			} else {
				cout << "The cube is solved" << endl;
			}
		} else if (cube.IsParity()) {
			if (language == "zh_cn") {
				cout << "有Parity" << endl;
			} else {
				cout << "There's parity" << endl;
			}
		} else {
			int corner_cycles = cube.CornerCycles();
			int edge_cycles = cube.EdgeCycles();
			if (language == "zh_cn") {
				cout << "需要插入";
				if (corner_cycles == 0) {
					cout << edge_cycles << "个棱块三循环";
				} else {
					cout << corner_cycles << "个角块三循环";
					if (edge_cycles > 0) {
						cout << "和" << edge_cycles << "个棱块三循环";
					}
				}
				cout << endl;
			} else {
				if (corner_cycles == 0) {
					if (edge_cycles == 1) {
						cout << "1 edge 3-cycle";
					} else {
						cout << edge_cycles << " edge 3-cycles";
					}
				} else {
					if (corner_cycles == 1) {
						cout << "1 corner 3-cycle";
					} else {
						cout << corner_cycles << " corner 3-cycles";
					}
					if (edge_cycles > 0) {
						cout << " and ";
						if (edge_cycles == 1) {
							cout << "1 edge 3-cycle";
						} else {
							cout << edge_cycles << " edge 3-cycles";
						}
					}
				}
				cout << " needed" << endl;
			}
		}
	}
}

int main(int argc, char **argv) {
	auto command = Resolve(argc, argv);
	vector<string> files;
	int type = -1;
	for (const auto &com: command) {
		if (com.first == "init") {
			InitAlgorithmSystem();
		} else if (com.first == "") {
			files.push_back(com.second);
		} else if (com.first == "language") {
			language = com.second;
		} else if (com.first == "s" || com.first == "solve") {
			type = 0;
		} else if (com.first == "v" || com.first == "verify") {
			type = 1;
		}
	}
	if (type == 0) {
		for (const string &file: files) {
			Solve(file);
		}
	} else if (type == 1) {
		for (const string &file: files) {
			Verify(file);
		}
	}
	return 0;
}

