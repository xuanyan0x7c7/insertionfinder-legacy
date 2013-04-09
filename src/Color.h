#pragma once
#include <ostream>
#include <string>


struct Color {
	static bool print_color;
	static std::string Reset();
	static std::string Black();
	static std::string Red();
	static std::string Green();
	static std::string Yellow();
	static std::string Blue();
	static std::string Purple();
	static std::string Darkgreen();
	static std::string White();
};

std::ostream& reset(std::ostream&);
std::ostream& black(std::ostream&);
std::ostream& red(std::ostream&);
std::ostream& green(std::ostream&);
std::ostream& yellow(std::ostream&);
std::ostream& blue(std::ostream&);
std::ostream& purple(std::ostream&);
std::ostream& darkgreen(std::ostream&);
std::ostream& white(std::ostream&);

