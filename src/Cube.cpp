#include <fstream>
#include <string>
#include <unistd.h>
#include <utility>
#include "Cube.h"
using std::array;
using std::bitset;
using std::ifstream;
using std::istream;
using std::ostream;
using std::string;


namespace
{
	constexpr int inverse_move[24] =
		{0, 3, 2, 1, 4, 7, 6, 5, 8, 11, 10, 9,
		12, 15, 14, 13, 16, 19, 18, 17, 20, 23, 22, 21};

	template <size_t N>
	inline void swap(array<int, N> &arr, int x1, int x2)
	{
		int temp = arr[x1];
		arr[x1] = arr[x2];
		arr[x2] = temp;
	}

	template <size_t N>
	inline void cycle(array<int, N> &arr, int x1, int x2, int x3, int x4)
	{
		int temp = arr[x1];
		arr[x1] = arr[x4];
		arr[x4] = arr[x3];
		arr[x3] = arr[x2];
		arr[x2] = temp;
	}

	inline void swap(bitset<12> &arr, int x1, int x2)
	{
		bool temp = arr[x1];
		arr.set(x1, arr[x2]);
		arr.set(x2, temp);
	}

	inline void cycle(bitset<12> &arr, int x1, int x2, int x3, int x4)
	{
		bool temp = arr[x1];
		arr.set(x1, arr[x4]);
		arr.set(x4, arr[x3]);
		arr.set(x3, arr[x2]);
		arr.set(x2, temp);
	}
}

Cube::Cube(const Cube&) = default;
Cube::Cube(Cube&&) = default;
Cube& Cube::operator=(const Cube&) = default;
Cube& Cube::operator=(Cube&&) = default;
Cube::~Cube() = default;

Cube::Cube():
	co({{0, 0, 0, 0, 0, 0, 0, 0}}),
	cp({{0, 1, 2, 3, 4, 5, 6, 7}}),
	ep({{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}}) {}

Cube::Cube(const Formula &formula): Cube()
{
	Twist(formula);
}

namespace
{
	array<Cube, 24> InitOneMoveCube()
	{
		array<Cube, 24> cube_array;
		for (int i = 0; i < 24; ++i)
			cube_array[i].Twist(i);
		return cube_array;
	}
	
	array<int, 147456> InitCornerCycleTable()
	{
		array<int, 147456> table;
		Cube identity;
		for (int i = 0; i < 6144; ++i)
		{
			Cube cube = Cube::CornerCycleCube(i);
			if (cube == identity)
			{
				for (int j = 0; j < 24; ++j)
					table[i * 24 + j] = i;
			}
			else
			{
				for (int j = 0; j < 24; ++j)
				{
					if (j & 3)
					{
						Cube cube0(cube);
						cube0.TwistCornerBefore(inverse_move[j]);
						cube0.TwistCorner(j);
						table[i * 24 + j] = cube0.GetCornerCycleIndex();
					}
					else
						table[i * 24 + j] = i;
				}
			}
		}
		return table;
	}
	
	array<int, 245760> InitEdgeCycleTable()
	{
		array<int, 245760> table;
		Cube identity;
		for (int i = 0; i < 10240; ++i)
		{
			Cube cube = Cube::EdgeCycleCube(i);
			if (cube == identity)
			{
				for (int j = 0; j < 24; ++j)
					table[i * 24 + j] = i;
			}
			else
			{
				for (int j = 0; j < 24; ++j)
				{
					if (j & 3)
					{
						Cube cube0(cube);
						cube0.TwistEdgeBefore(inverse_move[j]);
						cube0.TwistEdge(j);
						table[i * 24 + j] = cube0.GetEdgeCycleIndex();
					}
					else
						table[i * 24 + j] = i;
				}
			}
		}
		return table;
	}
}

const array<Cube, 24> Cube::one_move_cube = InitOneMoveCube();
const array<int, 147456> Cube::corner_cycle_table = InitCornerCycleTable();
const array<int, 245760> Cube::edge_cycle_table = InitEdgeCycleTable();

istream& operator>>(istream &in, Cube &cube)
{
	for (int &x: cube.co)
		in >> x;
	for (int &x: cube.cp)
		in >> x;
	for (int i = 0; i < 12; ++i)
	{
		int x;
		in >> x;
		cube.eo.set(i, x);
	}
	for (int &x: cube.ep)
		in >> x;
	return in;
}

ostream& operator<<(ostream &out, const Cube &cube)
{
	for (int x: cube.co)
		out << x << ' ';
	for (int x: cube.cp)
		out << x << ' ';
	for (int i = 0; i < 12; ++i)
		out << cube.eo[i] << ' ';
	for (int x: cube.ep)
		out << x << ' ';
	return out;
}

bool operator==(const Cube &lhs, const Cube &rhs)
{
	return lhs.co == rhs.co && lhs.cp == rhs.cp
		   && lhs.eo == rhs.eo && lhs.ep == rhs.ep;
}

bool operator!=(const Cube &lhs, const Cube &rhs)
{
	return !(lhs == rhs);
}

bool operator<(const Cube &lhs, const Cube &rhs)
{
	for (int i = 0; i < 8; ++i)
	{
		if (lhs.co[i] < rhs.co[i])
			return true;
		else if (lhs.co[i] > rhs.co[i])
			return false;
	}
	for (int i = 0; i < 8; ++i)
	{
		if (lhs.cp[i] < rhs.cp[i])
			return true;
		else if (lhs.cp[i] > rhs.cp[i])
			return false;
	}
	for (int i = 0; i < 12; ++i)
	{
		if (lhs.eo[i] < rhs.eo[i])
			return true;
		else if (lhs.eo[i] > rhs.eo[i])
			return false;
	}
	for (int i = 0; i < 12; ++i)
	{
		if (lhs.ep[i] < rhs.ep[i])
			return true;
		else if (lhs.ep[i] > rhs.ep[i])
			return false;
	}
	return false;
}

bool operator>(const Cube &lhs, const Cube &rhs)
{
	return rhs < lhs;
}

void Cube::Reset()
{
	co.fill(0);
	eo.reset();
	for (int i = 0; i < 8; ++i)
		cp[i] = i;
	for (int i = 0; i < 12; ++i)
		ep[i] = i;
}

void Cube::Twist(int move)
{
	static const bitset<12> flipF("011001000100");
	static const bitset<12> flipB("100100010001");
	switch (move)
	{
		case Cube::U:
			cycle(co, 0, 3, 2, 1);
			cycle(cp, 0, 3, 2, 1);
			cycle(eo, 0, 3, 2, 1);
			cycle(ep, 0, 3, 2, 1);
			break;
		case Cube::U2:
			swap(co, 0, 2);
			swap(co, 1, 3);
			swap(cp, 0, 2);
			swap(cp, 1, 3);
			swap(eo, 0, 2);
			swap(eo, 1, 3);
			swap(ep, 0, 2);
			swap(ep, 1, 3);
			break;
		case Cube::U3:
			cycle(co, 0, 1, 2, 3);
			cycle(cp, 0, 1, 2, 3);
			cycle(eo, 0, 1, 2, 3);
			cycle(ep, 0, 1, 2, 3);
			break;
		case Cube::D:
			cycle(co, 4, 5, 6, 7);
			cycle(cp, 4, 5, 6, 7);
			cycle(eo, 4, 5, 6, 7);
			cycle(ep, 4, 5, 6, 7);
			break;
		case Cube::D2:
			swap(co, 4, 6);
			swap(co, 5, 7);
			swap(cp, 4, 6);
			swap(cp, 5, 7);
			swap(eo, 4, 6);
			swap(eo, 5, 7);
			swap(ep, 4, 6);
			swap(ep, 5, 7);
			break;
		case Cube::D3:
			cycle(co, 4, 7, 6, 5);
			cycle(cp, 4, 7, 6, 5);
			cycle(eo, 4, 7, 6, 5);
			cycle(ep, 4, 7, 6, 5);
			break;
		case Cube::R:
			cycle(co, 2, 3, 7, 6);
			cycle(cp, 2, 3, 7, 6);
			co[2] = (co[2] + 2) % 3;
			co[3] = (co[3] + 1) % 3;
			co[6] = (co[6] + 1) % 3;
			co[7] = (co[7] + 2) % 3;
			cycle(eo, 3, 11, 7, 10);
			cycle(ep, 3, 11, 7, 10);
			break;
		case Cube::R2:
			swap(co, 2, 7);
			swap(co, 3, 6);
			swap(cp, 2, 7);
			swap(cp, 3, 6);
			swap(eo, 3, 7);
			swap(eo, 10, 11);
			swap(ep, 3, 7);
			swap(ep, 10, 11);
			break;
		case Cube::R3:
			cycle(co, 2, 6, 7, 3);
			cycle(cp, 2, 6, 7, 3);
			co[2] = (co[2] + 2) % 3;
			co[3] = (co[3] + 1) % 3;
			co[6] = (co[6] + 1) % 3;
			co[7] = (co[7] + 2) % 3;
			cycle(eo, 3, 10, 7, 11);
			cycle(ep, 3, 10, 7, 11);
			break;
		case Cube::L:
			cycle(co, 0, 1, 5, 4);
			cycle(cp, 0, 1, 5, 4);
			co[0] = (co[0] + 2) % 3;
			co[1] = (co[1] + 1) % 3;
			co[4] = (co[4] + 1) % 3;
			co[5] = (co[5] + 2) % 3;
			cycle(eo, 1, 9, 5, 8);
			cycle(ep, 1, 9, 5, 8);
			break;
		case Cube::L2:
			swap(co, 0, 5);
			swap(co, 1, 4);
			swap(cp, 0, 5);
			swap(cp, 1, 4);
			swap(eo, 1, 5);
			swap(eo, 8, 9);
			swap(ep, 1, 5);
			swap(ep, 8, 9);
			break;
		case Cube::L3:
			cycle(co, 0, 4, 5, 1);
			cycle(cp, 0, 4, 5, 1);
			co[0] = (co[0] + 2) % 3;
			co[1] = (co[1] + 1) % 3;
			co[4] = (co[4] + 1) % 3;
			co[5] = (co[5] + 2) % 3;
			cycle(eo, 1, 8, 5, 9);
			cycle(ep, 1, 8, 5, 9);
			break;
		case Cube::F:
			cycle(co, 1, 2, 6, 5);
			cycle(cp, 1, 2, 6, 5);
			co[1] = (co[1] + 2) % 3;
			co[2] = (co[2] + 1) % 3;
			co[5] = (co[5] + 1) % 3;
			co[6] = (co[6] + 2) % 3;
			cycle(eo, 2, 10, 6, 9);
			cycle(ep, 2, 10, 6, 9);
			eo ^= flipF;
			break;
		case Cube::F2:
			swap(co, 1, 6);
			swap(co, 2, 5);
			swap(cp, 1, 6);
			swap(cp, 2, 5);
			swap(eo, 2, 6);
			swap(eo, 9, 10);
			swap(ep, 2, 6);
			swap(ep, 9, 10);
			break;
		case Cube::F3:
			cycle(co, 1, 5, 6, 2);
			cycle(cp, 1, 5, 6, 2);
			co[1] = (co[1] + 2) % 3;
			co[2] = (co[2] + 1) % 3;
			co[5] = (co[5] + 1) % 3;
			co[6] = (co[6] + 2) % 3;
			cycle(eo, 2, 9, 6, 10);
			cycle(ep, 2, 9, 6, 10);
			eo ^= flipF;
			break;
		case Cube::B:
			cycle(co, 0, 4, 7, 3);
			cycle(cp, 0, 4, 7, 3);
			co[0] = (co[0] + 1) % 3;
			co[3] = (co[3] + 2) % 3;
			co[4] = (co[4] + 2) % 3;
			co[7] = (co[7] + 1) % 3;
			cycle(eo, 0, 8, 4, 11);
			cycle(ep, 0, 8, 4, 11);
			eo ^= flipB;
			break;
		case Cube::B2:
			swap(co, 0, 7);
			swap(co, 3, 4);
			swap(cp, 0, 7);
			swap(cp, 3, 4);
			swap(eo, 0, 4);
			swap(eo, 8, 11);
			swap(ep, 0, 4);
			swap(ep, 8, 11);
			break;
		case Cube::B3:
			cycle(co, 0, 3, 7, 4);
			cycle(cp, 0, 3, 7, 4);
			co[0] = (co[0] + 1) % 3;
			co[3] = (co[3] + 2) % 3;
			co[4] = (co[4] + 2) % 3;
			co[7] = (co[7] + 1) % 3;
			cycle(eo, 0, 11, 4, 8);
			cycle(ep, 0, 11, 4, 8);
			eo ^= flipB;
			break;
		default:
			break;
	}
}

void Cube::TwistCorner(int move)
{
	switch (move)
	{
		case Cube::U:
			cycle(co, 0, 3, 2, 1);
			cycle(cp, 0, 3, 2, 1);
			break;
		case Cube::U2:
			swap(co, 0, 2);
			swap(co, 1, 3);
			swap(cp, 0, 2);
			swap(cp, 1, 3);
			break;
		case Cube::U3:
			cycle(co, 0, 1, 2, 3);
			cycle(cp, 0, 1, 2, 3);
			break;
		case Cube::D:
			cycle(co, 4, 5, 6, 7);
			cycle(cp, 4, 5, 6, 7);
			break;
		case Cube::D2:
			swap(co, 4, 6);
			swap(co, 5, 7);
			swap(cp, 4, 6);
			swap(cp, 5, 7);
			break;
		case Cube::D3:
			cycle(co, 4, 7, 6, 5);
			cycle(cp, 4, 7, 6, 5);
			break;
		case Cube::R:
			cycle(co, 2, 3, 7, 6);
			cycle(cp, 2, 3, 7, 6);
			co[2] = (co[2] + 2) % 3;
			co[3] = (co[3] + 1) % 3;
			co[6] = (co[6] + 1) % 3;
			co[7] = (co[7] + 2) % 3;
			break;
		case Cube::R2:
			swap(co, 2, 7);
			swap(co, 3, 6);
			swap(cp, 2, 7);
			swap(cp, 3, 6);
			break;
		case Cube::R3:
			cycle(co, 2, 6, 7, 3);
			cycle(cp, 2, 6, 7, 3);
			co[2] = (co[2] + 2) % 3;
			co[3] = (co[3] + 1) % 3;
			co[6] = (co[6] + 1) % 3;
			co[7] = (co[7] + 2) % 3;
			break;
		case Cube::L:
			cycle(co, 0, 1, 5, 4);
			cycle(cp, 0, 1, 5, 4);
			co[0] = (co[0] + 2) % 3;
			co[1] = (co[1] + 1) % 3;
			co[4] = (co[4] + 1) % 3;
			co[5] = (co[5] + 2) % 3;
			break;
		case Cube::L2:
			swap(co, 0, 5);
			swap(co, 1, 4);
			swap(cp, 0, 5);
			swap(cp, 1, 4);
			break;
		case Cube::L3:
			cycle(co, 0, 4, 5, 1);
			cycle(cp, 0, 4, 5, 1);
			co[0] = (co[0] + 2) % 3;
			co[1] = (co[1] + 1) % 3;
			co[4] = (co[4] + 1) % 3;
			co[5] = (co[5] + 2) % 3;
			break;
		case Cube::F:
			cycle(co, 1, 2, 6, 5);
			cycle(cp, 1, 2, 6, 5);
			co[1] = (co[1] + 2) % 3;
			co[2] = (co[2] + 1) % 3;
			co[5] = (co[5] + 1) % 3;
			co[6] = (co[6] + 2) % 3;
			break;
		case Cube::F2:
			swap(co, 1, 6);
			swap(co, 2, 5);
			swap(cp, 1, 6);
			swap(cp, 2, 5);
			break;
		case Cube::F3:
			cycle(co, 1, 5, 6, 2);
			cycle(cp, 1, 5, 6, 2);
			co[1] = (co[1] + 2) % 3;
			co[2] = (co[2] + 1) % 3;
			co[5] = (co[5] + 1) % 3;
			co[6] = (co[6] + 2) % 3;
			break;
		case Cube::B:
			cycle(co, 0, 4, 7, 3);
			cycle(cp, 0, 4, 7, 3);
			co[0] = (co[0] + 1) % 3;
			co[3] = (co[3] + 2) % 3;
			co[4] = (co[4] + 2) % 3;
			co[7] = (co[7] + 1) % 3;
			break;
		case Cube::B2:
			swap(co, 0, 7);
			swap(co, 3, 4);
			swap(cp, 0, 7);
			swap(cp, 3, 4);
			break;
		case Cube::B3:
			cycle(co, 0, 3, 7, 4);
			cycle(cp, 0, 3, 7, 4);
			co[0] = (co[0] + 1) % 3;
			co[3] = (co[3] + 2) % 3;
			co[4] = (co[4] + 2) % 3;
			co[7] = (co[7] + 1) % 3;
			break;
		default:
			break;
	}
}

void Cube::TwistEdge(int move)
{
	static const bitset<12> flipF("011001000100");
	static const bitset<12> flipB("100100010001");
	switch (move)
	{
		case Cube::U:
			cycle(eo, 0, 3, 2, 1);
			cycle(ep, 0, 3, 2, 1);
			break;
		case Cube::U2:
			swap(eo, 0, 2);
			swap(eo, 1, 3);
			swap(ep, 0, 2);
			swap(ep, 1, 3);
			break;
		case Cube::U3:
			cycle(eo, 0, 1, 2, 3);
			cycle(ep, 0, 1, 2, 3);
			break;
		case Cube::D:
			cycle(eo, 4, 5, 6, 7);
			cycle(ep, 4, 5, 6, 7);
			break;
		case Cube::D2:
			swap(eo, 4, 6);
			swap(eo, 5, 7);
			swap(ep, 4, 6);
			swap(ep, 5, 7);
			break;
		case Cube::D3:
			cycle(eo, 4, 7, 6, 5);
			cycle(ep, 4, 7, 6, 5);
			break;
		case Cube::R:
			cycle(eo, 3, 11, 7, 10);
			cycle(ep, 3, 11, 7, 10);
			break;
		case Cube::R2:
			swap(eo, 3, 7);
			swap(eo, 10, 11);
			swap(ep, 3, 7);
			swap(ep, 10, 11);
			break;
		case Cube::R3:
			cycle(eo, 3, 10, 7, 11);
			cycle(ep, 3, 10, 7, 11);
			break;
		case Cube::L:
			cycle(eo, 1, 9, 5, 8);
			cycle(ep, 1, 9, 5, 8);
			break;
		case Cube::L2:
			swap(eo, 1, 5);
			swap(eo, 8, 9);
			swap(ep, 1, 5);
			swap(ep, 8, 9);
			break;
		case Cube::L3:
			cycle(eo, 1, 8, 5, 9);
			cycle(ep, 1, 8, 5, 9);
			break;
		case Cube::F:
			cycle(eo, 2, 10, 6, 9);
			cycle(ep, 2, 10, 6, 9);
			eo ^= flipF;
			break;
		case Cube::F2:
			swap(eo, 2, 6);
			swap(eo, 9, 10);
			swap(ep, 2, 6);
			swap(ep, 9, 10);
			break;
		case Cube::F3:
			cycle(eo, 2, 9, 6, 10);
			cycle(ep, 2, 9, 6, 10);
			eo ^= flipF;
			break;
		case Cube::B:
			cycle(eo, 0, 8, 4, 11);
			cycle(ep, 0, 8, 4, 11);
			eo ^= flipB;
			break;
		case Cube::B2:
			swap(eo, 0, 4);
			swap(eo, 8, 11);
			swap(ep, 0, 4);
			swap(ep, 8, 11);
			break;
		case Cube::B3:
			cycle(eo, 0, 11, 4, 8);
			cycle(ep, 0, 11, 4, 8);
			eo ^= flipB;
			break;
		default:
			break;
	}
}

void Cube::Twist(const Formula &f, bool dir)
{
	Twist(f, 0, f.length(), dir);
}

void Cube::TwistCorner(const Formula &f, bool dir)
{
	TwistCorner(f, 0, f.length(), dir);
}

void Cube::TwistEdge(const Formula &f, bool dir)
{
	TwistEdge(f, 0, f.length(), dir);
}

void Cube::Twist(const Formula &f, size_t start, size_t end, bool dir)
{
	if (dir)
		for (size_t i = start; i < end; ++i)
			Twist(f[i]);
	else
		for (size_t i = end; i > start; --i)
			Twist(inverse_move[f[i - 1]]);
}

void Cube::TwistCorner(const Formula &f, size_t start, size_t end, bool dir)
{
	if (dir)
		for (size_t i = start; i < end; ++i)
			TwistCorner(f[i]);
	else
		for (size_t i = end; i > start; --i)
			TwistCorner(inverse_move[f[i - 1]]);
}

void Cube::TwistEdge(const Formula &f, size_t start, size_t end, bool dir)
{
	if (dir)
		for (size_t i = start; i < end; ++i)
			TwistEdge(f[i]);
	else
		for (size_t i = end; i > start; --i)
			TwistEdge(inverse_move[f[i - 1]]);
}

void Cube::Twist(const Cube &cube)
{
	static Cube result;
	for (int i = 0; i < 8; ++i)
	{
		int j = cube.cp[i];
		result.cp[i] = this->cp[j];
		result.co[i] = (this->co[j] + cube.co[i]) % 3;
	}
	for (int i = 0; i < 12; ++i)
	{
		int j = cube.ep[i];
		result.ep[i] = this->ep[j];
		result.eo.set(i, this->eo[j] ^ cube.eo[i]);
	}
	*this = result;
}

void Cube::TwistCorner(const Cube &cube)
{
	static int ori[8], perm[8];
	for (int i = 0; i < 8; ++i)
	{
		int j = cube.cp[i];
		perm[i] = this->cp[j];
		ori[i] = (this->co[j] + cube.co[i]) % 3;
	}
	for (int i = 0; i < 8; ++i)
	{
		this->co[i] = ori[i];
		this->cp[i] = perm[i];
	}
}

void Cube::TwistEdge(const Cube &cube)
{
	static bitset<12> ori;
	static int perm[12];
	for (int i = 0; i < 12; ++i)
	{
		int j = cube.ep[i];
		perm[i] = this->ep[j];
		ori.set(i, this->eo[j] ^ cube.eo[i]);
	}
	this->eo = ori;
	for (int i = 0; i < 12; ++i)
		this->ep[i] = perm[i];
}

bool Twist(const Cube &lhs, const Cube &rhs, Cube &result,
		   bool cornerchanged, bool edgechanged)
{
	if (cornerchanged)
	{
		for (int i = 0; i < 8; ++i)
		{
			int j = rhs.cp[i];
			result.cp[i] = lhs.cp[j];
			result.co[i] = (lhs.co[j] + rhs.co[i]) % 3;
			if (result.cp[i] == i && result.co[i] && lhs.cp[i] != i)
				return false;
		}
	}
	if (edgechanged)
	{
		for (int i = 0; i < 12; ++i)
		{
			int j = rhs.ep[i];
			result.ep[i] = lhs.ep[j];
			result.eo.set(i, lhs.eo[j] ^ rhs.eo[i]);
			if (result.ep[i] == i && result.eo[i] && lhs.ep[i] != i)
				return false;
		}
	}
	return true;
}

void Cube::TwistBefore(int move)
{
	TwistBefore(Cube::one_move_cube[move]);
}

void Cube::TwistCornerBefore(int move)
{
	TwistCornerBefore(Cube::one_move_cube[move]);
}

void Cube::TwistEdgeBefore(int move)
{
	TwistEdgeBefore(Cube::one_move_cube[move]);
}

void Cube::TwistBefore(const Cube &cube)
{
	for (int i = 0; i < 8; ++i)
	{
		int j = this->cp[i];
		this->cp[i] = cube.cp[j];
		this->co[i] = (this->co[i] + cube.co[j]) % 3;
	}
	for (int i = 0; i < 12; ++i)
	{
		int j = this->ep[i];
		this->ep[i] = cube.ep[j];
		if (cube.eo[j])
			this->eo.flip(i);
	}
}

void Cube::TwistCornerBefore(const Cube &cube)
{
	for (int i = 0; i < 8; ++i)
	{
		int j = this->cp[i];
		this->cp[i] = cube.cp[j];
		this->co[i] = (this->co[i] + cube.co[j]) % 3;
	}
}

void Cube::TwistEdgeBefore(const Cube &cube)
{
	for (int i = 0; i < 12; ++i)
	{
		int j = this->ep[i];
		this->ep[i] = cube.ep[j];
		if (cube.eo[j])
			this->eo.flip(i);
	}
}

size_t Cube::Mask() const
{
	size_t mask = 0;
	for (int i = 0; i < 8; ++i)
	{
		mask <<= 1;
		if (co[i] || cp[i] != i)
			mask |= 1;
	}
	for (int i = 0; i < 12; ++i)
	{
		mask <<= 1;
		if (eo[i] || ep[i] != i)
			mask |= 1;
	}
	return mask;
}

bool Cube::IsParity() const
{
	static bitset<8> visited;
	int length;
	bool ok = false;
	visited.reset();

	for (int i = 0; i < 8; ++i)
	{
		if (!visited.test(i))
		{
			if (cp[i] != i)
			{
				length = 0;
				int j = i;
				while (cp[j] != i)
				{
					j = cp[j];
					visited.set(j);
					length ^= 1;
				}
				if (length)
					ok = !ok;
			}
		}
	}

	return ok;
}

namespace
{
	array<int, 20736> InitNextState()
	{
		array<int, 20736> table;
		static constexpr int next[6][2] =
			{{1, 2}, {3, 4}, {4, 5}, {0, 1}, {1, 2}, {2, 0}};
		for (int i = 0; i < 6 * 144; ++i)
		{
			for (int j = 0; j < 8 * 3; ++j)
			{
				int count = i / 144;
				int type1 = i % 144 / 24;
				int type2 = i % 24 / 6;
				int type3 = i % 6;
				int len = j / 3;
				int o = j % 3;
				while (len >= 3)
				{
					len -= 2;
					++count;
				}
				len %= 3;
				if (len == 0 && o != 0)
				{
					if (type1 >= 3)
					{
						++count;
						o = next[type1][o - 1];
						len = 2;
						type1 = 0;
					}
					else
						type1 = next[type1][o - 1];
				}
				if (len == 1)
				{
					if (type2 == 0)
						type2 = o + 1;
					else
					{
						o = (type2 - 1 + o) % 3;
						++count;
						len = 2;
						type2 = 0;
					}
				}
				if (len == 2)
				{
					if (o == 0)
						++count;
					else if (type3 <= 3)
						type3 = next[type3][o - 1];
				}
				table[i * 24 + j] = count * 144 + type1 * 24
									+ type2 * 6 + type3;
			}
		}
		return table;
	}
	
	array<int, 864> InitOutputState()
	{
		array<int, 864> table;
		static constexpr int spl[6][2] =
			{{0, 0}, {1, 0}, {0, 1}, {2, 0}, {1, 1}, {0, 2}};
		for (int i = 0; i < 6 * 144; i++)
		{
			int count = i / 144;
			int type1 = i % 144 / 24;
			int type2 = i % 24 / 6;
			int type3 = i % 6;
			if (type2 != 0)
				table[i] = -1;
			int t0 = spl[type1][0];
			int t1 = spl[type1][1];
			int t2 = spl[type3][0];
			int t3 = spl[type3][1];
			int min = std::min(t0, t3);
			count += 2 * min;
			t0 -= min;
			t3 -= min;
			min = std::min(t1, t2);
			count += 2 * min;
			t1 -= min;
			t2 -= min;
			if (t0 && t1)
				table[i] = count + t0 + 1;
			else if (t0 || t1)
				table[i] = count + 2 + t2 + t3;
			else
				table[i] = count + 3 * t2;
		}
		return table;
	}
	
	const array<int, 20736> next_table = InitNextState();
	const array<int, 864> output_table = InitOutputState();
}

int Cube::CornerCycles() const
{
	static bitset<8> visited;
	int state = 0;
	visited.reset();

	for (int i = 0; i < 8; ++i)
	{
		if (!visited.test(i))
		{
			if (cp[i] != i)
			{
				int len = 0;
				int ori_sum = co[i];
				int j = i;
				do
				{
					j = cp[j];
					visited.set(j);
					ori_sum += co[j];
					++len;
				}
				while (cp[j] != i);
				state = next_table[state * 24 + len * 3 + ori_sum % 3];
			}
			else if (co[i])
				state = next_table[state * 24 + co[i]];
		}
	}

	return output_table[state];
}

int Cube::EdgeCycles() const
{
	static constexpr array<int, 7> edge_add = {{0, 2, 3, 5, 6, 8, 9}};
	static bitset<12> visited;
	visited.reset();
	int count = 0, oricount = 0;

	for (int i = 0; i < 12; ++i)
	{
		if (!visited.test(i))
		{
			if (ep[i] != i)
			{
				int ori_sum = eo[i];
				int len = 1;
				int j = i;
				do
				{
					j = ep[j];
					visited.set(j);
					ori_sum ^= eo[j];
					++len;
				}
				while (ep[j] != i);
				count += len >> 1;
				if (len & ori_sum)
				{
					++count;
					--oricount;
				}
			}
			else if (eo[i])
				++oricount;
		}
	}

	if (oricount & 1)
	{
		++count;
		--oricount;
	}

	return count + (oricount ? edge_add[oricount >> 1] : oricount >> 1);
}

int Cube::GetCornerCycleIndex() const
{
	for (int i = 0; i < 6; ++i)
	{
		if (cp[i] != i)
		{
			int j = cp[i];
			int k = cp[j];
			return (i << 10) | (j << 7) | (k << 4) | (co[i] << 2) | co[j];
		}
	}
	return -1;
}

int Cube::GetEdgeCycleIndex() const
{
	for (int i = 0; i < 10; ++i)
	{
		if (ep[i] != i)
		{
			int j = ep[i];
			int k = ep[j];
			return (i << 10) | (j << 6) | (k << 2) | (eo[i] << 1) | eo[j];
		}
	}
	return -1;
}

Cube Cube::CornerCycleCube(int index)
{
	Cube cube;
	int perm = index >> 4;
	int ori = index & 0xf;
	int i = perm >> 6, j = (perm >> 3) & 7, k = perm & 7;
	int ori_i = ori >> 2, ori_j = ori & 3;
	if (i != j && i != k && j != k && ori_i < 3 && ori_j < 3)
	{
		cube.cp[i] = j;
		cube.cp[j] = k;
		cube.cp[k] = i;
		cube.co[i] = ori_i;
		cube.co[j] = ori_j;
		cube.co[k] = (6 - ori_i - ori_j) % 3;
	}
	return cube;
}

Cube Cube::EdgeCycleCube(int index)
{
	Cube cube;
	int perm = index >> 2;
	int ori = index & 3;
	int i = perm >> 8, j = (perm >> 4) & 0xf, k = perm & 0xf;
	int ori_i = ori >> 1, ori_j = ori & 1;
	if (j < 12 && k < 12 && i != j && i != k && j != k)
	{
		cube.ep[i] = j;
		cube.ep[j] = k;
		cube.ep[k] = i;
		cube.eo.set(i, ori_i);
		cube.eo.set(j, ori_j);
		cube.eo.set(k, ori_i ^ ori_j);
	}
	return cube;
}

