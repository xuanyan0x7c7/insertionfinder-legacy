#pragma once
#include <string>
#include <vector>


class Formula {
protected:
	std::vector<int> move;
public:
	Formula(const Formula&);
	Formula(Formula&&);
	Formula& operator =(const Formula&);
	Formula& operator =(Formula&&);
	virtual ~Formula();
public:
	Formula();
	Formula(const std::string &str);
public:
	friend std::istream& operator >>(std::istream &in, Formula &f);
	friend std::ostream& operator <<(std::ostream &out, const Formula &f);
public:
	std::size_t length() const noexcept;
	int& operator [](std::size_t index);
	int operator [](std::size_t index) const;
	void shrink_to_fit();
	std::size_t CancelMoves();
	std::size_t insert(const Formula &f, std::size_t index);
	void SwapAdjacentMove(std::size_t index);
	std::string str() const;
	std::string str(std::size_t begin, std::size_t end, bool pos) const;
};
