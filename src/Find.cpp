#include <chrono>
#include <fstream>
#include <sstream>
#include "Find.h"
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::endl;
using std::ifstream;
using std::move;
using std::ostringstream;
using std::pair;
using std::string;


Find::Find(const Find&) = delete;
Find::Find(Find&&) = delete;
Find& Find::operator=(const Find&) = delete;
Find& Find::operator=(Find&&) = delete;
Find::~Find() = default;

Find::Find()
{
	corner_cycle_index.fill(-1);
	edge_cycle_index.fill(-1);
}

pair<string, long>
Find::Solve(const Formula &scramble, const Formula &solve, int include)
{
	Find finder;

	bool corner_only = false, edge_only = false;
	if ((include | 0x7) == 0x7)
		corner_only = true;
	else if ((include | 0x78) == 0x78)
		edge_only = true;
	for (int i = 0; i < 8; ++i)
	{
		if (include & (1 << i))
		{
			static const string num("01234567");
			ifstream in((string("/usr/local/bin/insertionfinder/AlgFiles/")
						+ num[i]).c_str());
			size_t alg_count;
			in >> alg_count;
			size_t recent_size = finder.algorithm.size();
			size_t new_size = recent_size + alg_count;
			finder.algorithm.resize(new_size);
			for (size_t i = recent_size; i < new_size; ++i)
				in >> finder.algorithm[i];
			in.close();
		}
	}
	for (size_t i = 0; i < finder.algorithm.size(); ++i)
	{
		const Cube &state = finder.algorithm[i].state;
		int corner_cycles = state.CornerCycles();
		int edge_cycles = state.EdgeCycles();
		if (corner_cycles == 1 && edge_cycles == 0)
			finder.corner_cycle_index[state.GetCornerCycleIndex()] = i;
		else if (corner_cycles == 0 && edge_cycles == 1)
			finder.edge_cycle_index[state.GetEdgeCycleIndex()] = i;
	}

	finder.scramble = scramble;
	finder.scramble_cube.Twist(scramble);
	finder.inv_scramble_cube.Twist(scramble, false);
	finder.solve[0] = solve;
	
	Cube cube(finder.scramble_cube);
	cube.Twist(finder.solve[0]);
	if ((corner_only && cube.EdgeCycles() > 0)
		|| (edge_only && cube.CornerCycles() > 0))
		return make_pair(string("找不到给力的插入"), 0l);
	auto start = steady_clock::now();
	if (corner_only)
		finder.SearchCorner(0, cube.CornerCycles(), 0);
	else if (edge_only)
		finder.SearchEdge(0, cube.EdgeCycles(), 0);
	else
		finder.Search(0, cube.CornerCycles(), cube.EdgeCycles(), 0);
	auto end = steady_clock::now();
	return make_pair(finder.PrintAnswer(),
					duration_cast<microseconds>(end - start).count());
}

namespace
{
	unsigned BitCount(unsigned n)
	{
		n -= (n >> 1) & 0x55555555;
		n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
		n = (n + (n >> 4)) & 0x0f0f0f0f;
		n += n >> 8;
		n += n >> 16;
		return n & 0x3f;
	}
}

namespace
{
	constexpr int inverse_move[24] =
		{0, 3, 2, 1, 4, 7, 6, 5, 8, 11, 10, 9,
		12, 15, 14, 13, 16, 19, 18, 17, 20, 23, 22, 21};
}

void Find::Search(int depth, int corner, int edge, int searched)
{
	int moves = solve[depth].length();
	if (corner == 1 && edge == 0)
	{
		int index;
		for (int i = searched; i <= moves; ++i)
		{
			if (i == searched)
			{
				Cube state;
				state.TwistCorner(solve[depth], 0, i, false);
				state.TwistCorner(inv_scramble_cube);
				state.TwistCorner(solve[depth], i, moves, false);
				index = state.GetCornerCycleIndex();
			}
			else
				index = Cube::corner_cycle_table[index * 24 + solve[depth][i - 1]];
			int x = corner_cycle_index[index];
			if (x != -1)
			{
				insert_place[depth] = i;
				for (const Formula &j: algorithm[x].GetFormula())
				{
					insertion[depth] = j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].Insert(j, i);
					if (solve[depth + 1].length() < fewest_moves
						|| (solve[depth + 1].length() == fewest_moves
							&& depth + 1 < cycles_inserted))
					{
						fewest_moves = (best_answer =
							solve[cycles_inserted = depth + 1]).length();
						for (int k = 0; k <= depth; ++k)
						{
							best_solve[k] = solve[k];
							best_insert_place[k] = insert_place[k];
							best_insertion[k] = insertion[k];
						}
					}
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3)
			{
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::corner_cycle_table
								[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::corner_cycle_table
							[index_inv * 24 + inverse_move[solve[depth][i]]];
				x = corner_cycle_index[index_inv];
				if (x != -1)
				{
					insert_place[depth] = i;
					for (const Formula &j: algorithm[x].GetFormula())
					{
						insertion[depth] = j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(j, i);
						if (solve[depth + 1].length() < fewest_moves
							|| (solve[depth + 1].length() == fewest_moves
								&& depth + 1 < cycles_inserted))
						{
							fewest_moves = (best_answer =
								solve[cycles_inserted = depth + 1]).length();
							for (int k = 0; k <= depth; ++k)
							{
								best_solve[k] = solve[k];
								best_insert_place[k] = insert_place[k];
								best_insertion[k] = insertion[k];
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
	else if (corner == 0 && edge == 1)
	{
		int index;
		for (int i = searched; i <= moves; ++i)
		{
			if (i == searched)
			{
				Cube state;
				state.TwistEdge(solve[depth], 0, i, false);
				state.TwistEdge(inv_scramble_cube);
				state.TwistEdge(solve[depth], i, moves, false);
				index = state.GetEdgeCycleIndex();
			}
			else
				index = Cube::edge_cycle_table[index * 24 + solve[depth][i - 1]];
			int x = edge_cycle_index[index];
			if (x != -1)
			{
				insert_place[depth] = i;
				for (const Formula &j: algorithm[x].GetFormula())
				{
					insertion[depth] = j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].Insert(j, i);
					if (solve[depth + 1].length() < fewest_moves
						|| (solve[depth + 1].length() == fewest_moves
							&& depth + 1 < cycles_inserted))
					{
						fewest_moves = (best_answer =
							solve[cycles_inserted = depth + 1]).length();
						for (int k = 0; k <= depth; ++k)
						{
							best_solve[k] = solve[k];
							best_insert_place[k] = insert_place[k];
							best_insertion[k] = insertion[k];
						}
					}
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3)
			{
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::edge_cycle_table
								[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::edge_cycle_table
							[index_inv * 24 + inverse_move[solve[depth][i]]];
				x = edge_cycle_index[index_inv];
				if (x != -1)
				{
					insert_place[depth] = i;
					for (const Formula &j: algorithm[x].GetFormula())
					{
						insertion[depth] = j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(j, i);
						if (solve[depth + 1].length() < fewest_moves
							|| (solve[depth + 1].length() == fewest_moves
								&& depth + 1 < cycles_inserted))
						{
							fewest_moves = (best_answer =
								solve[cycles_inserted = depth + 1]).length();
							for (int k = 0; k <= depth; ++k)
							{
								best_solve[k] = solve[k];
								best_insert_place[k] = insert_place[k];
								best_insertion[k] = insertion[k];
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
	else
	{
		int algs = algorithm.size();
		Cube state;
		for (int i = searched; i <= moves; ++i)
		{
			if (i == searched)
			{
				state.Twist(solve[depth], i, moves);
				state.Twist(scramble_cube);
				state.Twist(solve[depth], 0, i);
			}
			else
			{
				state.TwistBefore(inverse_move[solve[depth][i - 1]]);
				state.Twist(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (int j = 0; j < algs; ++j)
			{
				if (BitCount(statemask & algorithm[j].mask) < 2)
					continue;
				bool changeC = algorithm[j].change_corner;
				bool changeE = algorithm[j].change_edge;
				if (!Twist(state, algorithm[j].state, cube, changeC, changeE))
					continue;
				int cornerX = changeC ? cube.CornerCycles() : corner;
				int edgeX = changeE ? cube.EdgeCycles() : edge;
				if (cornerX + edgeX >= corner + edge)
					continue;
				if (cornerX == 0 && edgeX == 0)
				{
					for (const Formula &k: algorithm[j].GetFormula())
					{
						insertion[depth] = k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(k, i);
						if (solve[depth + 1].length() < fewest_moves
							|| (solve[depth + 1].length() == fewest_moves
								&& depth + 1 < cycles_inserted))
						{
							fewest_moves = (best_answer =
								solve[cycles_inserted = depth + 1]).length();
							for (int l = 0; l <= depth; ++l)
							{
								best_solve[l] = solve[l];
								best_insert_place[l] = insert_place[l];
								best_insertion[l] = insertion[l];
							}
						}
					}
				}
				else
				{
					for (const Formula &k: algorithm[j].GetFormula())
					{
						solve[depth + 1] = solve[depth];
						int searched_new = solve[depth + 1].Insert(k, i);
						if (i < 2 || solve[depth][i - 2] >> 3
									!= solve[depth][i - 1] >> 3)
						{
							if (searched_new < i)
								continue;
						}
						else if (searched_new < i - 1)
							continue;
						if (solve[depth + 1].length() + cornerX + edgeX
							< fewest_moves)
						{
							insertion[depth] = k;
							insert_place[depth] = i;
							Search(depth + 1, cornerX, edgeX, searched_new);
						}
					}
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3)
			{
				solve[depth].SwapAdjacentMove(i);
				Cube state_inv(state);
				state_inv.TwistBefore(solve[depth][i]);
				state_inv.TwistBefore(inverse_move[solve[depth][i - 1]]);
				state_inv.Twist(solve[depth][i - 1]);
				state_inv.Twist(inverse_move[solve[depth][i]]);
				statemask = state_inv.Mask();
				for (int j = 0; j < algs; ++j)
				{
					if (BitCount(statemask & algorithm[j].mask) < 2)
						continue;
					bool changeC = algorithm[j].change_corner;
					bool changeE = algorithm[j].change_edge;
					if (!Twist(state_inv, algorithm[j].state, cube,
								changeC, changeE))
						continue;
					int cornerX = changeC ? cube.CornerCycles() : corner;
					int edgeX = changeE ? cube.EdgeCycles() : edge;
					if (cornerX + edgeX >= corner + edge)
						continue;
					if (cornerX == 0 && edgeX == 0)
					{
						for (const Formula &k: algorithm[j].GetFormula())
						{
							insertion[depth] = k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].Insert(k, i);
							if (solve[depth + 1].length() < fewest_moves
								|| (solve[depth + 1].length() == fewest_moves
									&& depth + 1 < cycles_inserted))
							{
								fewest_moves = (best_answer = solve
									[cycles_inserted = depth + 1]).length();
								for (int l = 0; l <= depth; ++l)
								{
									best_solve[l] = solve[l];
									best_insert_place[l] = insert_place[l];
									best_insertion[l] = insertion[l];
								}
							}
						}
					}
					else
					{
						for (const Formula &k: algorithm[j].GetFormula())
						{
							solve[depth + 1] = solve[depth];
							int searched_new = solve[depth + 1].Insert(k, i);
							if (searched_new >= i
								&& solve[depth + 1].length() + cornerX + edgeX
									< fewest_moves)
							{
								insertion[depth] = k;
								insert_place[depth] = i;
								Search(depth + 1, cornerX, edgeX, searched_new);
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
}

void Find::SearchCorner(int depth, int corner, int searched)
{
	int moves = solve[depth].length();
	if (corner == 1)
	{
		int index;
		for (int i = searched; i <= moves; ++i)
		{
			if (i == searched)
			{
				Cube state;
				state.TwistCorner(solve[depth], 0, i, false);
				state.TwistCorner(inv_scramble_cube);
				state.TwistCorner(solve[depth], i, moves, false);
				index = state.GetCornerCycleIndex();
			}
			else
				index = Cube::corner_cycle_table[index * 24 + solve[depth][i - 1]];
			int x = corner_cycle_index[index];
			if (x != -1)
			{
				insert_place[depth] = i;
				for (const Formula &j: algorithm[x].GetFormula())
				{
					insertion[depth] = j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].Insert(j, i);
					if (solve[depth + 1].length() < fewest_moves
						|| (solve[depth + 1].length() == fewest_moves
							&& depth + 1 < cycles_inserted))
					{
						fewest_moves = (best_answer =
							solve[cycles_inserted = depth + 1]).length();
						for (int k = 0; k <= depth; ++k)
						{
							best_solve[k] = solve[k];
							best_insert_place[k] = insert_place[k];
							best_insertion[k] = insertion[k];
						}
					}
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3)
			{
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::corner_cycle_table
								[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::corner_cycle_table
							[index_inv * 24 + inverse_move[solve[depth][i]]];
				x = corner_cycle_index[index_inv];
				if (x != -1)
				{
					insert_place[depth] = i;
					for (const Formula &j: algorithm[x].GetFormula())
					{
						insertion[depth] = j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(j, i);
						if (solve[depth + 1].length() < fewest_moves
							|| (solve[depth + 1].length() == fewest_moves
								&& depth + 1 < cycles_inserted))
						{
							fewest_moves = (best_answer =
								solve[cycles_inserted = depth + 1]).length();
							for (int k = 0; k <= depth; ++k)
							{
								best_solve[k] = solve[k];
								best_insert_place[k] = insert_place[k];
								best_insertion[k] = insertion[k];
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
	else
	{
		int algs = algorithm.size();
		Cube state;
		for (int i = searched; i <= moves; ++i)
		{
			if (i == searched)
			{
				state.TwistCorner(solve[depth], i, moves);
				state.TwistCorner(scramble_cube);
				state.TwistCorner(solve[depth], 0, i);
			}
			else
			{
				state.TwistCornerBefore(inverse_move[solve[depth][i - 1]]);
				state.TwistCorner(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (int j = 0; j < algs; ++j)
			{
				if (BitCount(statemask & algorithm[j].mask) < 2
					|| !Twist(state, algorithm[j].state, cube, true, false))
					continue;
				int cornerX = cube.CornerCycles();
				if (cornerX >= corner)
					continue;
				else if (cornerX == 0)
				{
					for (const Formula &k: algorithm[j].GetFormula())
					{
						insertion[depth] = k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(k, i);
						if (solve[depth + 1].length() < fewest_moves
							|| (solve[depth + 1].length() == fewest_moves
								&& depth + 1 < cycles_inserted))
						{
							fewest_moves = (best_answer =
								solve[cycles_inserted = depth + 1]).length();
							for (int l = 0; l <= depth; ++l)
							{
								best_solve[l] = solve[l];
								best_insert_place[l] = insert_place[l];
								best_insertion[l] = insertion[l];
							}
						}
					}
				}
				else
				{
					for (const Formula &k: algorithm[j].GetFormula())
					{
						solve[depth + 1] = solve[depth];
						int searched_new = solve[depth + 1].Insert(k, i);
						if (i < 2 || solve[depth][i - 2] >> 3
									!= solve[depth][i - 1] >> 3)
						{
							if (searched_new < i)
								continue;
						}
						else if (searched_new < i - 1)
							continue;
						if (solve[depth + 1].length() + cornerX < fewest_moves)
						{
							insertion[depth] = k;
							insert_place[depth] = i;
							SearchCorner(depth + 1, cornerX, searched_new);
						}
					}
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3)
			{
				solve[depth].SwapAdjacentMove(i);
				Cube state0(state);
				state0.TwistCornerBefore(solve[depth][i]);
				state0.TwistCornerBefore(inverse_move[solve[depth][i - 1]]);
				state0.TwistCorner(solve[depth][i - 1]);
				state0.TwistCorner(inverse_move[solve[depth][i]]);
				statemask = state0.Mask();
				for (int j = 0; j < algs; ++j)
				{
					if (BitCount(statemask & algorithm[j].mask) < 2
						|| !Twist(state0, algorithm[j].state, cube,
								true, false))
						continue;
					int cornerX = cube.CornerCycles();
					if (cornerX >= corner)
						continue;
					else if (cornerX == 0)
					{
						for (const Formula &k: algorithm[j].GetFormula())
						{
							insertion[depth] = k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].Insert(k, i);
							if (solve[depth + 1].length() < fewest_moves
								|| (solve[depth + 1].length() == fewest_moves
									&& depth + 1 < cycles_inserted))
							{
								fewest_moves = (best_answer = solve
									[cycles_inserted = depth + 1]).length();
								for (int l = 0; l <= depth; ++l)
								{
									best_solve[l] = solve[l];
									best_insert_place[l] = insert_place[l];
									best_insertion[l] = insertion[l];
								}
							}
						}
					}
					else
					{
						for (const Formula &k: algorithm[j].GetFormula())
						{
							solve[depth + 1] = solve[depth];
							int searched_new = solve[depth + 1].Insert(k, i);
							if (searched_new >= i
								&& solve[depth + 1].length() + cornerX
									< fewest_moves)
							{
								insertion[depth] = k;
								insert_place[depth] = i;
								SearchCorner(depth + 1, cornerX, searched_new);
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
}

void Find::SearchEdge(int depth, int edge, int searched)
{
	int moves = solve[depth].length();
	if (edge == 1)
	{
		int index;
		for (int i = searched; i <= moves; ++i)
		{
			if (i == searched)
			{
				Cube state;
				state.TwistEdge(solve[depth], 0, i, false);
				state.TwistEdge(inv_scramble_cube);
				state.TwistEdge(solve[depth], i, moves, false);
				index = state.GetEdgeCycleIndex();
			}
			else
				index = Cube::edge_cycle_table[index * 24 + solve[depth][i - 1]];
			int x = edge_cycle_index[index];
			if (x != -1)
			{
				insert_place[depth] = i;
				for (const Formula &j: algorithm[x].GetFormula())
				{
					insertion[depth] = j;
					solve[depth + 1] = solve[depth];
					solve[depth + 1].Insert(j, i);
					if (solve[depth + 1].length() < fewest_moves
						|| (solve[depth + 1].length() == fewest_moves
							&& depth + 1 < cycles_inserted))
					{
						fewest_moves = (best_answer =
							solve[cycles_inserted = depth + 1]).length();
						for (int k = 0; k <= depth; ++k)
						{
							best_solve[k] = solve[k];
							best_insert_place[k] = insert_place[k];
							best_insertion[k] = insertion[k];
						}
					}
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3)
			{
				solve[depth].SwapAdjacentMove(i);
				int index_inv = Cube::edge_cycle_table
								[index * 24 + solve[depth][i - 1]];
				index_inv = Cube::edge_cycle_table
							[index_inv * 24 + inverse_move[solve[depth][i]]];
				x = edge_cycle_index[index_inv];
				if (x != -1)
				{
					insert_place[depth] = i;
					for (const Formula &j: algorithm[x].GetFormula())
					{
						insertion[depth] = j;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(j, i);
						if (solve[depth + 1].length() < fewest_moves
							|| (solve[depth + 1].length() == fewest_moves
								&& depth + 1 < cycles_inserted))
						{
							fewest_moves = (best_answer =
								solve[cycles_inserted = depth + 1]).length();
							for (int k = 0; k <= depth; ++k)
							{
								best_solve[k] = solve[k];
								best_insert_place[k] = insert_place[k];
								best_insertion[k] = insertion[k];
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
	else
	{
		int algs = algorithm.size();
		Cube state;
		for (int i = searched; i <= moves; ++i)
		{
			if (i == searched)
			{
				state.TwistEdge(solve[depth], i, moves);
				state.TwistEdge(scramble_cube);
				state.TwistEdge(solve[depth], 0, i);
			}
			else
			{
				state.TwistEdgeBefore(inverse_move[solve[depth][i - 1]]);
				state.TwistEdge(solve[depth][i - 1]);
			}
			int statemask = state.Mask();
			Cube cube;
			for (int j = 0; j < algs; ++j)
			{
				if (BitCount(statemask & algorithm[j].mask) < 2
					|| !Twist(state, algorithm[j].state, cube, false, true))
					continue;
				int edgeX = cube.EdgeCycles();
				if (edgeX >= edge)
					continue;
				else if (edgeX == 0)
				{
					for (const Formula &k: algorithm[j].GetFormula())
					{
						insertion[depth] = k;
						solve[depth + 1] = solve[depth];
						solve[depth + 1].Insert(k, i);
						if (solve[depth + 1].length() < fewest_moves
							|| (solve[depth + 1].length() == fewest_moves
								&& depth + 1 < cycles_inserted))
						{
							fewest_moves = (best_answer =
								solve[cycles_inserted = depth + 1]).length();
							for (int l = 0; l <= depth; ++l)
							{
								best_solve[l] = solve[l];
								best_insert_place[l] = insert_place[l];
								best_insertion[l] = insertion[l];
							}
						}
					}
				}
				else
				{
					for (const Formula &k: algorithm[j].GetFormula())
					{
						solve[depth + 1] = solve[depth];
						int searched_new = solve[depth + 1].Insert(k, i);
						if (i < 2 || solve[depth][i - 2] >> 3
									!= solve[depth][i - 1] >> 3)
						{
							if (searched_new < i)
								continue;
						}
						else if (searched_new < i - 1)
							continue;
						if (solve[depth + 1].length() + edgeX < fewest_moves)
						{
							insertion[depth] = k;
							insert_place[depth] = i;
							SearchEdge(depth + 1, edgeX, searched_new);
						}
					}
				}
			}
			if (i > 0 && i < moves
				&& solve[depth][i - 1] >> 3 == solve[depth][i] >> 3)
			{
				solve[depth].SwapAdjacentMove(i);
				Cube state0(state);
				state0.TwistEdgeBefore(solve[depth][i]);
				state0.TwistEdgeBefore(inverse_move[solve[depth][i - 1]]);
				state0.TwistEdge(solve[depth][i - 1]);
				state0.TwistEdge(inverse_move[solve[depth][i]]);
				statemask = state0.Mask();
				for (int j = 0; j < algs; ++j)
				{
					if (BitCount(statemask & algorithm[j].mask) < 2
						|| !Twist(state0, algorithm[j].state, cube,
								false, true))
						continue;
					int edgeX = cube.EdgeCycles();
					if (edgeX >= edge)
						continue;
					else if (edgeX == 0)
					{
						for (const Formula &k: algorithm[j].GetFormula())
						{
							insertion[depth] = k;
							solve[depth + 1] = solve[depth];
							solve[depth + 1].Insert(k, i);
							if (solve[depth + 1].length() < fewest_moves
								|| (solve[depth + 1].length() == fewest_moves
									&& depth + 1 < cycles_inserted))
							{
								fewest_moves = (best_answer = solve
									[cycles_inserted = depth + 1]).length();
								for (int l = 0; l <= depth; ++l)
								{
									best_solve[l] = solve[l];
									best_insert_place[l] = insert_place[l];
									best_insertion[l] = insertion[l];
								}
							}
						}
					}
					else
					{
						for (const Formula &k: algorithm[j].GetFormula())
						{
							solve[depth + 1] = solve[depth];
							int searched_new = solve[depth + 1].Insert(k, i);
							if (searched_new >= i
								&& solve[depth + 1].length() + edgeX
									< fewest_moves)
							{
								insertion[depth] = k;
								insert_place[depth] = i;
								SearchEdge(depth + 1, edgeX, searched_new);
							}
						}
					}
				}
				solve[depth].SwapAdjacentMove(i);
			}
		}
	}
}

string Find::PrintAnswer()
{
	ostringstream str;
	if (fewest_moves == 0xffffffff)
		str << "找不到给力的插入";
	else
	{
		str << best_solve[0].str(0, best_insert_place[0]) << " [@1] "
		    << best_solve[0].str(best_insert_place[0], best_solve[0].length())
			<< endl << "在@1处插入： " << best_insertion[0].str() << endl;
		for (int i = 1; i < cycles_inserted; ++i)
		{
			str << "插入第" << i << "个后： "
			 	<< best_solve[i].str(0, best_insert_place[i])
				<< " [@" << i + 1 << "] "
				<< best_solve[i].str(best_insert_place[i],
									best_solve[i].length())
				<< endl << "在@" << i + 1 << "处插入： "
				<< best_insertion[i].str() << endl;
		}
		int cancelled_moves = solve[0].length() - best_answer.length();
		for (int i = 0; i < cycles_inserted; ++i)
			cancelled_moves += best_insertion[i].length();
		str << "最少步数：" << fewest_moves
			<< "\t消去" << cancelled_moves << "步" << endl
			<< "完整解法：" << best_answer.str();
	}
	return str.str();
}

