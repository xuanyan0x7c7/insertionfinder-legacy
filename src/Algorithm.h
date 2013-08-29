#pragma once
#include <vector>
#include "Cube.h"


class Algorithm {
public:
	Cube state;
	std::size_t mask;
	bool change_corner;
	bool change_edge;
private:
	std::vector<Formula> formula;
public:
	Algorithm(const Algorithm&);
	Algorithm(Algorithm&&);
	Algorithm& operator =(const Algorithm&);
	Algorithm& operator =(Algorithm&&);
	~Algorithm();
public:
	Algorithm();
	Algorithm(const Cube &state);
	friend std::istream& operator >>(std::istream &in, Algorithm &alg);
	void AddFormula(const Formula &f);
	void AddFormula(Formula &&f);
	const std::vector<Formula>& GetFormula() const noexcept;
	void CopyFormula(const Algorithm &alg);
	void CopyFormula(Algorithm &alg);
};
