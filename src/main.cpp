#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include "Find.h"
#include "Init.h"
using namespace std;
using namespace chrono;


int main(int argc, char **argv)
{
	if (argc >= 2)
	{
		if (strcmp(argv[1], "-init") == 0)
			InitAlgorithmSystem();
		else if (strcmp(argv[1], "-verify") == 0 || strcmp(argv[1], "-v") == 0)
		{
			ifstream in(argv[2]);
			string str;
			getline(in, str);
			Formula scramble(str);
			getline(in, str);
			Formula solve(str);
			in.close();
			Cube cube(scramble);
			cube.Twist(solve);
			if (cube.Mask() == 0)
				cout << "已经还原" << endl;
			else if (cube.IsParity())
				cout << "有Parity" << endl;
			else
			{
				cout << "需要插入";
				int corner_cycles = cube.CornerCycles();
				int edge_cycles = cube.EdgeCycles();
				if (corner_cycles == 0)
					cout << edge_cycles << "个棱块三循环";
				else
				{
					cout << corner_cycles << "个角块三循环";
					if (edge_cycles > 0)
						cout << "和" << edge_cycles << "个棱块三循环";
				}
				cout << endl;
			}
		}
		else if (strcmp(argv[1], "-solve") == 0 || strcmp(argv[1], "-s") == 0)
		{
			ifstream in(argv[2]);
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
			if (cube.Mask() == 0)
				cout << "已经还原" << endl;
			else if (cube.IsParity())
				cout << "有Parity" << endl;
			else if (include == 0)
				cout << "没有选择公式集" << endl;
			else
			{
				cout << "需要插入";
				int corner_cycles = cube.CornerCycles();
				int edge_cycles = cube.EdgeCycles();
				if (corner_cycles == 0)
					cout << edge_cycles << "个棱块三循环";
				else
				{
					cout << corner_cycles << "个角块三循环";
					if (edge_cycles > 0)
						cout << "和" << edge_cycles << "个棱块三循环";
				}
				cout << endl;
				auto start = steady_clock::now();
				auto result = Find::Solve(scramble, solve, include);
				auto end = steady_clock::now();
				cout << result.first << endl << "用时" << result.second << '/'
					<< duration_cast<microseconds>(end - start).count()
					<< "微秒" << endl;
			}
		}
	}
	return 0;
}

