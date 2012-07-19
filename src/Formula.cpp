#include <list>
#include <sstream>
#include "Formula.h"
using std::array;
using std::istream;
using std::list;
using std::max;
using std::min;
using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;


const array<string, 24> Formula::twist_str =
	{{"", "U", "U2", "U'", "", "D", "D2", "D'", "", "R", "R2", "R'",
	  "", "L", "L2", "L'", "", "F", "F2", "F'", "", "B", "B2", "B'"}};

namespace
{
	constexpr int inv_move[24] =
		{0, 3, 2, 1, 4, 7, 6, 5, 8, 11, 10, 9,
		12, 15, 14, 13, 16, 19, 18, 17, 20, 23, 22, 21};
}

Formula::Formula(const Formula&) = default;
Formula::Formula(Formula&&) = default;
Formula& Formula::operator=(const Formula&) = default;
Formula& Formula::operator=(Formula&&) = default;
Formula::~Formula() = default;

Formula::Formula()
{
	move.reserve(100);
};

Formula::Formula(const string &str): Formula()
{
	static const string outer_twist("UDRLFB");
	static const string inner_twist("udrlfbMESxyz");
	static const string NISS("NISS");
	list<string> procedure;

	for (auto iter = str.cbegin(); iter != str.cend();)
	{
		if (outer_twist.find(*iter) != string::npos)
		{
			if (*(iter + 1) == 'w')
			{
				if (*(iter + 2) == '2')
				{
					procedure.push_back(string(iter, iter + 3));

					if (*(iter + 3) == '\'')
						iter += 4;
					else
						iter += 3;
				}
			}
			else if (*(iter + 1) == '2')
			{
				procedure.push_back(string(iter, iter + 2));

				if (*(iter + 2) == '\'')
					iter += 3;
				else
					iter += 2;
			}
			else if (*(iter + 1) == '\'')
			{
				procedure.push_back(string(iter, iter + 2));
				iter += 2;
			}
			else
			{
				procedure.push_back(string(iter, iter + 1));
				++iter;
			}
		}
		else if (inner_twist.find(*iter) != string::npos)
		{
			if (*(iter + 1) == '2')
			{
				procedure.push_back(string(iter, iter + 2));

				if (*(iter + 2) == '\'')
					iter += 3;
				else
					iter += 2;
			}
			else if (*(iter + 1) == '\'')
			{
				procedure.push_back(string(iter, iter + 2));
				iter += 2;
			}
			else
			{
				procedure.push_back(string(iter, iter + 1));
				++iter;
			}
		}
		else if (string(iter, iter + 4) == NISS)
		{
			procedure.push_back(NISS);
			iter += 4;
		}
		else if (*iter == ' ')
			++iter;
		else
			break;
	}

	for (auto iter = procedure.begin(); iter != procedure.end(); ++iter)
	{
		if (*iter == "E" || *iter == "u'" || *iter == "d")
		{
			procedure.insert(iter, "U");
			procedure.insert(iter, "D'");
			*iter = "y'";
		}
		else if (*iter == "E2" || *iter == "u2" || *iter == "d2")
		{
			procedure.insert(iter, "U2");
			procedure.insert(iter, "D2");
			*iter = "y2";
		}
		else if (*iter == "E'" || *iter == "u" || *iter == "d'")
		{
			procedure.insert(iter, "U'");
			procedure.insert(iter, "D");
			*iter = "y";
		}
		else if (*iter == "M" || *iter == "r'" || *iter == "l")
		{
			procedure.insert(iter, "R");
			procedure.insert(iter, "L'");
			*iter = "x'";
		}
		else if (*iter == "M2" || *iter == "r2" || *iter == "l2")
		{
			procedure.insert(iter, "R2");
			procedure.insert(iter, "L2");
			*iter = "x2";
		}
		else if (*iter == "M'" || *iter == "r" || *iter == "l'")
		{
			procedure.insert(iter, "R'");
			procedure.insert(iter, "L");
			*iter = "x";
		}
		else if (*iter == "S" || *iter == "f" || *iter == "b'")
		{
			procedure.insert(iter, "F'");
			procedure.insert(iter, "B");
			*iter = "z";
		}
		else if (*iter == "S2" || *iter == "f2" || *iter == "b2")
		{
			procedure.insert(iter, "F2");
			procedure.insert(iter, "B2");
			*iter = "z2";
		}
		else if (*iter == "S'" || *iter == "f'" || *iter == "b")
		{
			procedure.insert(iter, "F");
			procedure.insert(iter, "B'");
			*iter = "z'";
		}
		else if (*iter == "Uw")
		{
			procedure.insert(iter, "D");
			*iter = "y";
		}
		else if (*iter == "Uw2")
		{
			procedure.insert(iter, "D2");
			*iter = "y2";
		}
		else if (*iter == "Uw'")
		{
			procedure.insert(iter, "D'");
			*iter = "y'";
		}
		else if (*iter == "Dw")
		{
			procedure.insert(iter, "U");
			*iter = "y'";
		}
		else if (*iter == "Dw2")
		{
			procedure.insert(iter, "U2");
			*iter = "y2";
		}
		else if (*iter == "Dw'")
		{
			procedure.insert(iter, "U'");
			*iter = "y";
		}
		else if (*iter == "Rw")
		{
			procedure.insert(iter, "L");
			*iter = "x";
		}
		else if (*iter == "Rw2")
		{
			procedure.insert(iter, "L2");
			*iter = "x2";
		}
		else if (*iter == "Rw'")
		{
			procedure.insert(iter, "L'");
			*iter = "x'";
		}
		else if (*iter == "Lw")
		{
			procedure.insert(iter, "R");
			*iter = "x'";
		}
		else if (*iter == "Lw2")
		{
			procedure.insert(iter, "R2");
			*iter = "x2";
		}
		else if (*iter == "Lw'")
		{
			procedure.insert(iter, "R'");
			*iter = "x";
		}
		else if (*iter == "Fw")
		{
			procedure.insert(iter, "B");
			*iter = "z";
		}
		else if (*iter == "Fw2")
		{
			procedure.insert(iter, "B2");
			*iter = "z2";
		}
		else if (*iter == "Fw'")
		{
			procedure.insert(iter, "B'");
			*iter = "z'";
		}
		else if (*iter == "Bw")
		{
			procedure.insert(iter, "F");
			*iter = "z'";
		}
		else if (*iter == "Bw2")
		{
			procedure.insert(iter, "F2");
			*iter = "z2";
		}
		else if (*iter == "Bw'")
		{
			procedure.insert(iter, "F'");
			*iter = "z";
		}
	}
	
	if (procedure.empty())
		return;
	auto iter = procedure.end();
	--iter;

	while (true)
	{
		bool quit_loop = (iter == procedure.begin());

		if (*iter == "x")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'U')
					(*iter2)[0] = 'F';
				else if ((*iter2)[0] == 'D')
					(*iter2)[0] = 'B';
				else if ((*iter2)[0] == 'F')
					(*iter2)[0] = 'D';
				else if ((*iter2)[0] == 'B')
					(*iter2)[0] = 'U';
			}
			procedure.erase(iter--);
		}
		else if (*iter == "x2")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'U')
					(*iter2)[0] = 'D';
				else if ((*iter2)[0] == 'D')
					(*iter2)[0] = 'U';
				else if ((*iter2)[0] == 'F')
					(*iter2)[0] = 'B';
				else if ((*iter2)[0] == 'B')
					(*iter2)[0] = 'F';
			}
			procedure.erase(iter--);
		}
		else if (*iter == "x'")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'U')
					(*iter2)[0] = 'D';
				else if ((*iter2)[0] == 'D')
					(*iter2)[0] = 'F';
				else if ((*iter2)[0] == 'F')
					(*iter2)[0] = 'U';
				else if ((*iter2)[0] == 'B')
					(*iter2)[0] = 'D';
			}
			procedure.erase(iter--);
		}
		else if (*iter == "y")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'R')
					(*iter2)[0] = 'B';
				else if ((*iter2)[0] == 'L')
					(*iter2)[0] = 'F';
				else if ((*iter2)[0] == 'F')
					(*iter2)[0] = 'R';
				else if ((*iter2)[0] == 'B')
					(*iter2)[0] = 'L';
			}
			procedure.erase(iter--);
		}
		else if (*iter == "y2")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'R')
					(*iter2)[0] = 'L';
				else if ((*iter2)[0] == 'L')
					(*iter2)[0] = 'R';
				else if ((*iter2)[0] == 'F')
					(*iter2)[0] = 'B';
				else if ((*iter2)[0] == 'B')
					(*iter2)[0] = 'F';
			}
			procedure.erase(iter--);
		}
		else if (*iter == "y'")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'R')
					(*iter2)[0] = 'F';
				else if ((*iter2)[0] == 'L')
					(*iter2)[0] = 'B';
				else if ((*iter2)[0] == 'F')
					(*iter2)[0] = 'L';
				else if ((*iter2)[0] == 'B')
					(*iter2)[0] = 'R';
			}
			procedure.erase(iter--);
		}
		else if (*iter == "z")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'U')
					(*iter2)[0] = 'L';
				else if ((*iter2)[0] == 'D')
					(*iter2)[0] = 'R';
				else if ((*iter2)[0] == 'R')
					(*iter2)[0] = 'U';
				else if ((*iter2)[0] == 'L')
					(*iter2)[0] = 'D';
			}
			procedure.erase(iter--);
		}
		else if (*iter == "z2")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'U')
					(*iter2)[0] = 'D';
				else if ((*iter2)[0] == 'D')
					(*iter2)[0] = 'U';
				else if ((*iter2)[0] == 'R')
					(*iter2)[0] = 'L';
				else if ((*iter2)[0] == 'L')
					(*iter2)[0] = 'R';
			}
			procedure.erase(iter--);
		}
		else if (*iter == "z'")
		{
			for (auto iter2 = iter; iter2 != procedure.end(); ++iter2)
			{
				if ((*iter2)[0] == 'U')
					(*iter2)[0] = 'R';
				else if ((*iter2)[0] == 'D')
					(*iter2)[0] = 'L';
				else if ((*iter2)[0] == 'R')
					(*iter2)[0] = 'D';
				else if ((*iter2)[0] == 'L')
					(*iter2)[0] = 'R';
			}
			procedure.erase(iter--);
		}
		else
			--iter;

		if (quit_loop)
			break;
	}

	vector<string> NISS_pos;
	vector<string> NISS_neg;
	bool direction = true;

	for (auto iter = procedure.cbegin(); iter != procedure.cend();)
	{
		auto iter2 = iter;

		while (iter2 != procedure.cend() && *iter2 != NISS)
			++iter2;

		if (direction)
			NISS_pos.insert(NISS_pos.end(), iter, iter2);
		else
			NISS_neg.insert(NISS_neg.end(), iter, iter2);

		iter = iter2;
		direction = !direction;
	}

	for (auto iter = NISS_pos.cbegin(); iter != NISS_pos.cend(); ++iter)
	{
		for (size_t i = 0; i < 24; ++i)
		{
			if (*iter == twist_str[i])
			{
				move.push_back(i);
				break;
			}
		}
	}

	for (auto iter = NISS_neg.crbegin(); iter != NISS_neg.crend(); ++iter)
	{
		for (size_t i = 0; i < 24; ++i)
		{
			if (*iter == twist_str[i])
			{
				move.push_back(inv_move[i]);
				break;
			}
		}
	}

	CancelMoves();
}

istream& operator>>(istream &in, Formula &f)
{
	int size;
	in >> size;
	for (int i = 0; i < size; ++i)
	{
		int m;
		in >> m;
		f.move.push_back(m);
	}
	return in;
}

ostream& operator<<(ostream &out, const Formula &f)
{
	out << f.move.size();
	for (int m: f.move)
		out << ' ' << m;
	return out;
}

size_t Formula::length() const
{
	return move.size();
}

int& Formula::operator[](size_t index)
{
	return move[index];
}

int Formula::operator[](size_t index) const
{
	return move[index];
}

size_t Formula::CancelMoves()
{
	int i = 0, j = -1;
	int index = 0x7fffffff;
	int size = move.size();

	while (i < size)
	{
		if (j < 0 || move[i] >> 3 != move[j] >> 3)
		{
			if (++j != i++)
				move[j] = move[i - 1];
		}
		else if (move[i] >> 2 != move[j] >> 2)
		{
			if (j > 0 && move[j - 1] >> 3 == move[j] >> 3)
			{
				int po = (move[j - 1] + move[i++]) & 3;
				if (index > j)
					index = j;
				if (po == 0)
				{
					move[j - 1] = move[j];
					--j;
				}
				else
					move[j - 1] = (move[j - 1] & -4) + po;
			}
			else
				move[++j] = move[i++];
		}
		else
		{
			int po = (move[j] + move[i++]) & 3;
			if (po == 0)
				--j;
			else
				move[j] = (move[j] & -4) + po;
			if (index > j + 1)
				index = j + 1;
		}
	}
	
	move.resize(j + 1);
	return index;
}

size_t Formula::Insert(const Formula &f, size_t index)
{
	this->move.insert(this->move.begin() + index,
						f.move.cbegin(), f.move.cend());
	return min(this->CancelMoves(), index + 1);
}

void Formula::SwapAdjacentMove(size_t index)
{
	int temp = move[index];
	move[index] = move[index - 1];
	move[index - 1] = temp;
}

string Formula::str() const
{
	return str(0, length());
}

string Formula::str(size_t start, size_t end) const
{
	if (start >= end)
		return "";

	ostringstream str;
	
	str << twist_str[move[start]];

	for (size_t k = start; ++k < end;)
		str << ' ' << twist_str[move[k]];

	return str.str();
}

void Formula::Resize()
{
	move.shrink_to_fit();
}

