#pragma once
#include <array>
#include <string>
#include <vector>


class Formula
{
	friend class Algorithm;
private:
	std::vector<int> move;
	static const std::array<std::string, 24> twist_str;
public:
	Formula(const Formula&);
	Formula(Formula&&);
	Formula& operator=(const Formula&);
	Formula& operator=(Formula&&);
	~Formula();
public:
	Formula();
	Formula(const std::string &str);
private:
	friend std::istream& operator>>(std::istream&, Formula&);
	friend std::ostream& operator<<(std::ostream&, const Formula&);
public:
	size_t length() const;
	int& operator[](size_t);
	int operator[](size_t) const;
	size_t CancelMoves();
	size_t Insert(const Formula&, size_t);
	void SwapAdjacentMove(size_t);
	std::string str() const;
	std::string str(size_t, size_t) const;
	void Resize();
};

