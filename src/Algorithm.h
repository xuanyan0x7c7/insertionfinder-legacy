#pragma once
#include <vector>
#include "Cube.h"


class Algorithm {
public:
	Cube state;
	size_t mask;
	bool change_corner;
	bool change_edge;
private:
	std::vector<Formula> formula;
public:
	Algorithm();
	Algorithm(const Algorithm&);
	Algorithm(Algorithm&&);
	Algorithm& operator=(const Algorithm&);
	Algorithm& operator=(Algorithm&&);
	~Algorithm();
public:
	Algorithm(const Cube&);
	friend std::istream& operator>>(std::istream&, Algorithm&);
	void AddFormula(const Formula&);
	void AddFormula(Formula&&);
	const std::vector<Formula>& GetFormula() const;
	void CopyFormula(const Algorithm&);
	void CopyFormula(Algorithm&);
};
