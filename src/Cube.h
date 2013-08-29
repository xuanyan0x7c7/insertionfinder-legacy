#pragma once
#include <array>
#include <bitset>
#include "Formula.h"


class Cube {
private:
	std::array<int, 8> co;
	std::array<int, 8> cp;
	std::bitset<12> eo;
	std::array<int, 12> ep;
public:
	Cube(const Cube&);
	Cube(Cube&&);
	Cube& operator =(const Cube&);
	Cube& operator =(Cube&&);
	~Cube();
private:
	enum {
		U0, U, U2, U3, D0, D, D2, D3, R0, R, R2, R3,
		L0, L, L2, L3, F0, F, F2, F3, B0, B, B2, B3};
public:
	Cube();
	Cube(const Formula&);
public:
	friend std::istream& operator >>(std::istream&, Cube&);
	friend std::ostream& operator <<(std::ostream&, const Cube&);
private:
	static const std::array<Cube, 24> one_move_cube;
public:
	static const std::array<int, 147456> corner_cycle_table;
	static const std::array<int, 245760> edge_cycle_table;
public:
	friend bool operator ==(const Cube&, const Cube&);
	friend bool operator !=(const Cube&, const Cube&);
	friend bool operator <(const Cube&, const Cube&);
	friend bool operator >(const Cube&, const Cube&);
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
