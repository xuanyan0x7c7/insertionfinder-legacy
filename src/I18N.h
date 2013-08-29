#pragma once
#include <array>
#include <string>


class I18N {
private:
	static std::array<std::string, 13> init();
	static const std::array<std::string, 13> i18n;
public:
	static std::string Scramble(const std::string &scramble);
	static std::string Solve(const std::string &solve);
	static std::string TheCubeIsSolved();
	static std::string ThereIsParity();
	static std::string Cycles(int corner, int edge);
	static std::string NoProperInsertionsFound();
	static std::string InsertAt(int n, const std::string &formula);
	static std::string
	AfterInsertion(int n, const std::string &f1, const std::string &f2);
	static std::string Moves(int total, int cancel);
	static std::string FinalSolution(const std::string &solution);
	static std::string Runtime(std::int64_t t1, std::int64_t t2);
};
