#pragma once
#include <array>
#include <bitset>
#include <iostream>
#include "Formula.h"


class Cube
{
	friend class Algorithm;
private:
	std::array<int, 8> co;
	std::array<int, 8> cp;
	std::bitset<12> eo;
	std::array<int, 12> ep;
public:
	Cube(const Cube&);
	Cube(Cube&&);
	Cube& operator=(const Cube&);
	Cube& operator=(Cube&&);
	~Cube();
public:
	static constexpr int U = 1;
	static constexpr int U2 = 2;
	static constexpr int U3 = 3;
	static constexpr int D = 5;
	static constexpr int D2 = 6;
	static constexpr int D3 = 7;
	static constexpr int R = 9;
	static constexpr int R2 = 10;
	static constexpr int R3 = 11;
	static constexpr int L = 13;
	static constexpr int L2 = 14;
	static constexpr int L3 = 15;
	static constexpr int F = 17;
	static constexpr int F2 = 18;
	static constexpr int F3 = 19;
	static constexpr int B = 21;
	static constexpr int B2 = 22;
	static constexpr int B3 = 23;
public:
	Cube();
	Cube(const Formula&);
private:
	friend std::istream& operator>>(std::istream&, Cube&);
	friend std::ostream& operator<<(std::ostream&, const Cube&);
private:
	static const std::array<Cube, 24> one_move_cube;
public:
	static const std::array<int, 147456> corner_cycle_table;
	static const std::array<int, 245760> edge_cycle_table;
public:
	friend bool operator==(const Cube&, const Cube&);
	friend bool operator!=(const Cube&, const Cube&);
	friend bool operator<(const Cube&, const Cube&);
	friend bool operator>(const Cube&, const Cube&);
	void Reset();
	void Twist(int);
	void TwistCorner(int);
	void TwistEdge(int);
	void Twist(const Formula&, bool dir = true);
	void TwistCorner(const Formula&, bool dir = true);
	void TwistEdge(const Formula&, bool dir = true);
	void Twist(const Formula&, size_t, size_t, bool dir = true);
	void TwistCorner(const Formula&, size_t, size_t, bool dir = true);
	void TwistEdge(const Formula&, size_t, size_t, bool dir = true);
	void Twist(const Cube&);
	void TwistCorner(const Cube&);
	void TwistEdge(const Cube&);
	friend bool Twist(const Cube&, const Cube&, Cube&, bool, bool);
	void TwistBefore(int);
	void TwistCornerBefore(int);
	void TwistEdgeBefore(int);
	void TwistBefore(const Cube&);
	void TwistCornerBefore(const Cube&);
	void TwistEdgeBefore(const Cube&);
	size_t Mask() const;
	bool IsParity() const;
	int CornerCycles() const;
	int EdgeCycles() const;
	int GetCornerCycleIndex() const;
	int GetEdgeCycleIndex() const;
	static Cube CornerCycleCube(int);
	static Cube EdgeCycleCube(int);
};

