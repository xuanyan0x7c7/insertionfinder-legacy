#include <iostream>
#include "Algorithm.h"
using std::array;
using std::endl;
using std::istream;
using std::move;
using std::ostream;
using std::vector;


Algorithm::Algorithm(const Algorithm&) = default;
Algorithm::Algorithm(Algorithm&&) = default;
Algorithm& Algorithm::operator=(const Algorithm&) = default;
Algorithm& Algorithm::operator=(Algorithm&&) = default;
Algorithm::~Algorithm() = default;

Algorithm::Algorithm() {};

Algorithm::Algorithm(const Cube &state): state(state), mask(state.Mask()),
	change_corner(mask & 0xff000), change_edge(mask & 0x00fff) {}

istream& operator>>(istream &in, Algorithm &alg) {
	in >> alg.state;
	alg.mask = alg.state.Mask();
	alg.change_corner = alg.mask & 0xff000;
	alg.change_edge = alg.mask & 0x00fff;
	size_t size;
	in >> size;
	alg.formula.resize(size);
	for (size_t i = 0; i < size; ++i) {
		in >> alg.formula[i];
	}
	return in;
}

void Algorithm::AddFormula(const Formula &f) {
	formula.push_back(f);
}

void Algorithm::AddFormula(Formula &&f) {
	formula.push_back(move(f));
}

const vector<Formula>& Algorithm::GetFormula() const {
	return formula;
}

void Algorithm::CopyFormula(const Algorithm &alg) {
	for (const Formula f: alg.formula) {
		this->formula.push_back(f);
	}
}

void Algorithm::CopyFormula(Algorithm &alg) {
	this->formula.swap(alg.formula);
}
