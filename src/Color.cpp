#include "Color.h"
using std::ostream;
using std::string;


bool Color::print_color = false;

string Color::Reset() {
	return print_color ? "\033[0m" : "";
}

string Color::Black() {
	return print_color ? "\033[30m" : "";
}

string Color::Red() {
	return print_color ? "\033[31m" : "";
}

string Color::Green() {
	return print_color ? "\033[32m" : "";
}

string Color::Yellow() {
	return print_color ? "\033[33m" : "";
}

string Color::Blue() {
	return print_color ? "\033[34m" : "";
}

string Color::Purple() {
	return print_color ? "\033[35m" : "";
}

string Color::Darkgreen() {
	return print_color ? "\033[36m" : "";
}

string Color::White() {
	return print_color ? "\033[37m" : "";
}

ostream& reset(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[0m";
	} else {
		return ostr;
	}
}

ostream& black(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[30m";
	} else {
		return ostr;
	}
}

ostream& red(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[31m";
	} else {
		return ostr;
	}
}

ostream& green(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[32m";
	} else {
		return ostr;
	}
}

ostream& yellow(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[33m";
	} else {
		return ostr;
	}
}

ostream& blue(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[34m";
	} else {
		return ostr;
	}
}

ostream& purple(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[35m";
	} else {
		return ostr;
	}
}

ostream& darkgreen(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[36m";
	} else {
		return ostr;
	}
}

ostream& white(ostream &ostr) {
	if (Color::print_color) {
		return ostr << "\033[37m";
	} else {
		return ostr;
	}
}

