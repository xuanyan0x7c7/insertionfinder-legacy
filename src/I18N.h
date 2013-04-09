#pragma once
#include <array>
#include <string>


class I18N {
private:
	static std::array<std::string, 13> Init();
	static const std::array<std::string, 13> i18n;
public:
	static std::string Scramble(const std::string&);
	static std::string Solve(const std::string&);
	static std::string TheCubeIsSolved();
	static std::string ThereIsParity();
	static std::string Cycles(int, int);
	static std::string NoProperInsertionsFound();
	static std::string InsertAt(int, const std::string&);
	static std::string
	AfterInsertion(int, const std::string&, const std::string&);
	static std::string Moves(int, int);
	static std::string FinalSolution(const std::string&);
	static std::string Runtime(int64_t, int64_t);
};

