#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include "Color.h"
#include "I18N.h"
using std::array;
using std::cerr;
using std::endl;
using std::ifstream;
using std::istringstream;
using std::locale;
using std::ostream;
using std::ostringstream;
using std::size_t;
using std::string;


#ifndef LIBRARY_PATH
	#define LIBRARY_PATH ""
#endif

const array<string, 13> I18N::i18n = I18N::Init();

array<string, 13> I18N::Init() {
	string loc = locale("").name();
	ifstream in;
	do {
		in.open(LIBRARY_PATH + string("i18n/") + loc);
		if (in.is_open()) {
			break;
		}
		in.close();
		in.open("~/.insertionfinder/i18n/" + loc);
	} while (false);
	if (!in.is_open()) {
		in.close();
		cerr << "File " << loc << " does not exist, "
			<< "insertionfinder uses en_US.utf8 instead." << endl;
		in.open(LIBRARY_PATH + string("i18n/en_US.utf8"));
	}

	array<string, 13> str;
	for (int i = 0; i < 13; ++i) {
		getline(in, str[i]);
	}
	return str;
}

namespace {
	void Print(ostream &out, istringstream &in) {
		char ch;
		while (true) {
			in.get(ch);
			if (in.eof()) {
				break;
			}
			out.put(ch);
		}
	}

	template <typename T, typename... Types> void
	Print(ostream &out, istringstream &in, const T &arg, const Types& ...args) {
		char ch;
		while (true) {
			in.get(ch);
			if (in.eof()) {
				break;
			}
			if (ch == '$') {
				out << arg;
				break;
			} else {
				out.put(ch);
			}
		}
		Print(out, in, args...);
	}
	
	void ReplaceAll(string &str, const string &prev, const string &now) {
		size_t length = prev.length();
		while (true) {
			size_t pos = str.find(prev);
			if (pos == string::npos) {
				break;
			} else {
				str = str.substr(0, pos) + now + str.substr(pos + length);
			}
		}
	}
}

string I18N::Scramble(const string &scramble) {
	istringstream in(I18N::i18n[0]);
	ostringstream out;
	Print(out, in, scramble);
	string str = out.str();
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::Solve(const string &solve) {
	istringstream in(I18N::i18n[1]);
	ostringstream out;
	Print(out, in, solve);
	string str = out.str();
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::TheCubeIsSolved() {
	string str(I18N::i18n[2]);
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::ThereIsParity() {
	string str(I18N::i18n[3]);
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::Cycles(int corner, int edge) {
	string str;
	if (edge == 0) {
		istringstream in(I18N::i18n[4]);
		ostringstream out;
		Print(out, in, corner);
		str = out.str();
	} else if (corner == 0) {
		istringstream in(I18N::i18n[5]);
		ostringstream out;
		Print(out, in, edge);
		str = out.str();
	} else {
		istringstream in(I18N::i18n[6]);
		ostringstream out;
		Print(out, in, corner, edge);
		str = out.str();
	}
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::NoProperInsertionsFound() {
	string str(I18N::i18n[7]);
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::InsertAt(int n, const string &formula) {
	istringstream in(I18N::i18n[8]);
	ostringstream out;
	Print(out, in, n, formula);
	string str = out.str();
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::AfterInsertion(int n, const string &f1, const string &f2) {
	istringstream in(I18N::i18n[9]);
	ostringstream out;
	Print(out, in, n, f1, n + 1, f2);
	string str = out.str();
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::Moves(int total, int cancel) {
	istringstream in(I18N::i18n[10]);
	ostringstream out;
	Print(out, in, total, cancel);
	string str = out.str();
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::FinalSolution(const string &solution) {
	istringstream in(I18N::i18n[11]);
	ostringstream out;
	Print(out, in, solution);
	string str = out.str();
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

string I18N::Runtime(int64_t t1, int64_t t2) {
	istringstream in(I18N::i18n[12]);
	ostringstream out;
	Print(out, in, t1, t2);
	string str = out.str();
	ReplaceAll(str, "<color>", Color::Green());
	ReplaceAll(str, "</color>", Color::Reset());
	return str;
}

