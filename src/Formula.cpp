#include <boost/regex.hpp>
#include <limits>
#include <list>
#include <sstream>
#include <utility>
#include "Formula.h"
using boost::regex;
using boost::regex_constants::ECMAScript;
using boost::regex_constants::optimize;
using boost::sregex_iterator;
using std::istream;
using std::list;
using std::min;
using std::next;
using std::numeric_limits;
using std::ostream;
using std::prev;
using std::ostringstream;
using std::size_t;
using std::string;
using std::swap;
using std::vector;


namespace {
	const string twist_str[24] = {
		"", "U", "U2", "U'", "", "D", "D2", "D'", "", "R", "R2", "R'",
		"", "L", "L2", "L'", "", "F", "F2", "F'", "", "B", "B2", "B'"};

	constexpr int inv_move[24] = {
		0, 3, 2, 1, 4, 7, 6, 5, 8, 11, 10, 9,
		12, 15, 14, 13, 16, 19, 18, 17, 20, 23, 22, 21};

	void cycle(char &c, const string &type) {
		for (auto iter = type.cbegin(); iter != type.cend(); ++iter) {
			if (c == *iter) {
				c = next(iter) == type.cend() ? type.front() : *(next(iter));
				return;
			}
		}
	}
}

Formula::Formula(const Formula&) = default;
Formula::Formula(Formula&&) = default;
Formula& Formula::operator =(const Formula&) = default;
Formula& Formula::operator =(Formula&&) = default;
Formula::~Formula() = default;

Formula::Formula() = default;

Formula::Formula(const string &str): Formula() {
	static const boost::regex moves_regex(
			string("[UDRLFB](w|w2|2|')?|[udrlfbMESxyz][2']?|\\[[udrlfb][2']?\\]|NISS"),
			ECMAScript | optimize);
	auto begin = sregex_iterator(str.cbegin(), str.cend(), moves_regex);
	auto end = sregex_iterator();
	list<string> procedure;
	for (auto iter = begin; iter != end; ++iter) {
		procedure.push_back(iter->str());
	}

	for (auto iter = procedure.begin(); iter != procedure.end(); ++iter) {
		if (*iter == "Uw") {
			procedure.insert(iter, "D");
			*iter = "y";
		} else if (*iter == "Uw2") {
			procedure.insert(iter, "D2");
			*iter = "y2";
		} else if (*iter == "Uw'") {
			procedure.insert(iter, "D'");
			*iter = "y'";
		} else if (*iter == "Dw") {
			procedure.insert(iter, "U");
			*iter = "y'";
		} else if (*iter == "Dw2") {
			procedure.insert(iter, "U2");
			*iter = "y2";
		} else if (*iter == "Dw'") {
			procedure.insert(iter, "U'");
			*iter = "y";
		} else if (*iter == "Rw") {
			procedure.insert(iter, "L");
			*iter = "x";
		} else if (*iter == "Rw2") {
			procedure.insert(iter, "L2");
			*iter = "x2";
		} else if (*iter == "Rw'") {
			procedure.insert(iter, "L'");
			*iter = "x'";
		} else if (*iter == "Lw") {
			procedure.insert(iter, "R");
			*iter = "x'";
		} else if (*iter == "Lw2") {
			procedure.insert(iter, "R2");
			*iter = "x2";
		} else if (*iter == "Lw'") {
			procedure.insert(iter, "R'");
			*iter = "x";
		} else if (*iter == "Fw") {
			procedure.insert(iter, "B");
			*iter = "z";
		} else if (*iter == "Fw2") {
			procedure.insert(iter, "B2");
			*iter = "z2";
		} else if (*iter == "Fw'") {
			procedure.insert(iter, "B'");
			*iter = "z'";
		} else if (*iter == "Bw") {
			procedure.insert(iter, "F");
			*iter = "z'";
		} else if (*iter == "Bw2") {
			procedure.insert(iter, "F2");
			*iter = "z2";
		} else if (*iter == "Bw'") {
			procedure.insert(iter, "F'");
			*iter = "z";
		}
	}
	
	if (procedure.empty()) {
		return;
	}
	auto iter = prev(procedure.end());

	while (true) {
		bool quit_loop = (iter == procedure.begin());

		if (*iter == "x" || *iter == "[r]" || *iter == "[l']") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "UFDB");
			}
			procedure.erase(iter--);
		} else if (*iter == "x2" || *iter == "[r2]" || *iter == "[l2]") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "UD");
				cycle(iter2->front(), "FB");
			}
			procedure.erase(iter--);
		} else if (*iter == "x'" || *iter == "[r']" || *iter == "[l]") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "UBDF");
			}
			procedure.erase(iter--);
		} else if (*iter == "y" || *iter == "[u]" || *iter == "[d']") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "RBLF");
			}
			procedure.erase(iter--);
		} else if (*iter == "y2" || *iter == "[u2]" || *iter == "[d2]") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "RL");
				cycle(iter2->front(), "FB");
			}
			procedure.erase(iter--);
		} else if (*iter == "y'" || *iter == "[u']" || *iter == "[d]") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "RFLB");
			}
			procedure.erase(iter--);
		} else if (*iter == "z" || *iter == "[f]" || *iter == "[b']") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "ULDR");
			}
			procedure.erase(iter--);
		} else if (*iter == "z2" || *iter == "[f2]" || *iter == "[b2]") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "UD");
				cycle(iter2->front(), "RL");
			}
			procedure.erase(iter--);
		} else if (*iter == "z'" || *iter == "[f']" || *iter == "[b]") {
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2) {
				cycle(iter2->front(), "URDL");
			}
			procedure.erase(iter--);
		} else {
			--iter;
		}

		if (quit_loop) {
			break;
		}
	}

	vector<string> NISS_pos, NISS_neg;
	bool direction = true;
	for (auto iter = procedure.cbegin(); iter != procedure.cend();) {
		auto iter2 = iter;
		while (iter2 != procedure.cend() && *iter2 != "NISS") {
			++iter2;
		}
		if (direction) {
			NISS_pos.insert(NISS_pos.end(), iter, iter2);
		} else {
			NISS_neg.insert(NISS_neg.end(), iter, iter2);
		}
		iter = iter2;
		if (iter2 != procedure.cend()) {
			++iter;
		}
		direction = !direction;
	}
	for (const string &s: NISS_pos) {
		for (size_t i = 0; i < 24; ++i) {
			if (s == twist_str[i]) {
				move.push_back(i);
				break;
			}
		}
	}
	for (auto iter = NISS_neg.crbegin(); iter != NISS_neg.crend(); ++iter) {
		for (size_t i = 0; i < 24; ++i) {
			if (*iter == twist_str[i]) {
				move.push_back(inv_move[i]);
				break;
			}
		}
	}

	CancelMoves();
}

istream& operator>>(istream &in, Formula &f) {
	int size;
	in >> size;
	for (int i = 0; i < size; ++i) {
		int m;
		in >> m;
		f.move.push_back(m);
	}
	return in;
}

ostream& operator<<(ostream &out, const Formula &f) {
	out << f.move.size();
	for (int m: f.move) {
		out << ' ' << m;
	}
	return out;
}

size_t Formula::length() const noexcept {
	return move.size();
}

int& Formula::operator [](size_t index) {
	return move[index];
}

int Formula::operator [](size_t index) const {
	return move[index];
}

void Formula::shrink_to_fit() {
	move.shrink_to_fit();
}

size_t Formula::CancelMoves() {
	int i = 0, j = -1;
	int index = numeric_limits<int>::max();
	int size = move.size();

	while (i < size) {
		if (j < 0 || move[i] >> 3 != move[j] >> 3) {
			if (++j != i++) {
				move[j] = move[i - 1];
			}
		} else if (move[i] >> 2 != move[j] >> 2) {
			if (j > 0 && move[j - 1] >> 3 == move[j] >> 3) {
				int po = (move[j - 1] + move[i++]) & 3;
				if (index > j) {
					index = j;
				}
				if (po == 0) {
					move[j - 1] = move[j];
					--j;
				} else {
					move[j - 1] = (move[j - 1] & ~3) + po;
				}
			} else {
				move[++j] = move[i++];
			}
		} else {
			int po = (move[j] + move[i++]) & 3;
			if (po == 0) {
				--j;
			} else {
				move[j] = (move[j] & ~3) + po;
			}
			if (index > j + 1) {
				index = j + 1;
			}
		}
	}

	move.resize(j + 1);
	return index;
}

size_t Formula::insert(const Formula &f, size_t index) {
	this->move.insert(this->move.begin() + index,
			f.move.cbegin(), f.move.cend());
	return min(this->CancelMoves(), index + 1);
}

void Formula::SwapAdjacentMove(size_t index) {
	swap(move[index - 1], move[index]);
}

string Formula::str() const {
	if (move.empty()) {
		return "";
	}
	ostringstream out;
	out << twist_str[move.front()];
	for (auto iter = next(move.begin()); iter != move.end(); ++iter) {
		out << ' ' << twist_str[*iter];
	}
	return out.str();
}

string Formula::str(size_t start, size_t end, bool pos) const {
	if (start >= end) {
		return "";
	}
	ostringstream out;
	if (pos) {
		for (size_t k = start; k < end; ++k) {
			out << twist_str[move[k]] << ' ';
		}
	} else {
		for (size_t k = start; k < end; ++k) {
			out << ' ' << twist_str[move[k]];
		}
	}
	return out.str();
}
