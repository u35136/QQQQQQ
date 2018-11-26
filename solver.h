#pragma once
#include <iostream>
#include <algorithm>
#include <cmath>
#include "board23.h"
#include "action23.h"
#include <numeric>
#include <vector>

static const int max_index = board23::POSSIBLE_INDEX;
static const int hint_type = 4;
static const int row =  board23::ROW;
static const int col = board23::COLUMN;
static const long SIZE = max_index * max_index * max_index * max_index * max_index * max_index;

class state_type {
public:
	enum type : char {
		before  = 'b',
		after   = 'a',
		illegal = 'i'
	};

public:
	state_type() : t(illegal) {}
	state_type(const state_type& st) = default;
	state_type(state_type::type code) : t(code) {}

	friend std::istream& operator >>(std::istream& in, state_type& type) {
		std::string s;
		if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
		return in;
	}

	friend std::ostream& operator <<(std::ostream& out, const state_type& type) {
		return out << char(type.t);
	}

	bool is_before()  const { return t == before; }
	bool is_after()   const { return t == after; }
	bool is_illegal() const { return t == illegal; }

private:
	type t;
};

// do not understand
class state_hint {
public:
	state_hint(const board23& state) : state(const_cast<board23&>(state)) {}

	char type() const { return state.info() ? state.info() + '0' : 'x'; }
	int hint_value() const { return type() - '0';}
	operator board23::cell() const { return state.info(); }

public:
	friend std::istream& operator >>(std::istream& in, state_hint& hint) {
		while (in.peek() != '+' && in.good()) in.ignore(1);
		char v; in.ignore(1) >> v;
		hint.state.info(v != 'x' ? v - '0' : 0);
		return in;
	}
	friend std::ostream& operator <<(std::ostream& out, const state_hint& hint) {
		return out << "+" << hint.type();
	}

private:
	board23& state;
};


class solver {
public:
	typedef double value_t;

public:
	class answer {
	public:
		answer(value_t min = 0.0/0.0, value_t avg = -1.0, value_t max = 0.0/0.0) : min(min), avg(avg), max(max) {}
	    friend std::ostream& operator <<(std::ostream& out, const answer& ans) {
	    	return !std::isnan(ans.avg) ? (out << ans.min << " " << ans.avg << " " << ans.max) : (out << "-1") << std::endl;
		}
	public:
		value_t min, avg, max;
	};

public:
	solver(const std::string& args) {
		// TODO: explore the tree and save the result
		for(int i = 0; i < hint_type; ++i) {
			expects[i] = new answer[SIZE];
		}
		for(int i = 0; i < hint_type; ++i) {
			for(int j = 0; j < 4; ++j) {
				after_expects[i][j] = new answer[SIZE];
			}
		}
		// calculate
		for(int pos = 0; pos < 6; pos++){
			for(int tile = 1; tile < 3; tile++){
				board23 board;
				int bag[3] = {0};
				bag[tile] = -1;
				action23::place(tile, pos).apply(board);
				get_before_expect(board, tile, bag);
			}
		}
		std::cout << "solver is ready." << std::endl;

//		std::cout << "feel free to display some messages..." << std::endl;
	}

	answer solve(const board23& state, state_type type, state_hint& sta_hint) {
		// TODO: find the answer in the lookup table and return it
		//       do NOT recalculate the tree at here

		// to fetch the hint (if type == state_type::after, hint will be 0)
//		board23::cell hint = state_hint(state);
		int hint = sta_hint.hint_value();
		int temp_tile;
		for(int i = 0; i < row; i++){
			for(int j = 0; j < col; j++){
				temp_tile = state[i][j];
				// if it's impossible
				if(temp_tile >= max_index || temp_tile < 0)
					return -1;
			}
		}

		if(type.is_before() && is_legal_before_state(state, hint)){
			return ans_before_expect(state, hint);
		}
		else if(type.is_after() ){//&& is_legal_after_state(state, hint)
			double expect = 0.0;
			double best_expect = -1000;
			bool is_moved = false;
			int best_move = 0;
			for (int op : {3, 2, 1, 0}) {
				board23 b = state;
				int reward = b.slide(op);
				if (reward != -1) {
					//expect = reward;
					expect = ans_after_expect(b, hint, op).avg + reward;
					if (expect > best_expect) {
						is_moved = true;
						best_move = op;
						best_expect = expect;
					}
				}
			}
			if(is_moved) {return ans_after_expect(state, hint, best_move);}
		}
		return {};
		// for a legal state, return its three values.
//		return { min, avg, max };
		// for an illegal state, simply return {}
		//std::cout << state;
		//return {};
	}

	answer ans_before_expect(board23 board, int hint) {
		long index = get_index(board);
		if(index < SIZE || expects[hint][index].avg > -1)
			return expects[hint][index];
		return {};
	}
	answer ans_after_expect(board23 board, int hint, int op) {
		long index = get_index(board);
		if(index < SIZE || after_expects[hint][op][index].avg > -1)
			return after_expects[hint][op][index];
		return {};
	}

	answer get_before_expect(board23 board, int hint, int bag[]){
		long index = get_index(board);

		if(expects[hint][index].avg > -1)
			return expects[hint][index];

		double expect = 0.0, min_expect = 0.0, max_expect = 0.0;
		double best_expect = -1000, best_min = 1000, best_max = -1000;
		bool is_moved = false;
		//std::cout << "after" << std::flush;
		for(int op: {3, 2, 1, 0}){
			board23 b = board;
			int reward = b.slide(op);
			if(reward != -1){
				//expect = reward;
				//answer temp = answer();
				
				answer temp = get_after_expect(b, hint, op, bag);
				expect = temp.avg + reward;
				min_expect = temp.min + reward;
				max_expect = temp.max + reward;
				if(expect > best_expect){
					is_moved = true;
					best_expect = expect;
					best_min = min_expect;
					best_max = max_expect;
				}
			}
		}

		if(is_moved){
			expects[hint][index].avg = best_expect;
			expects[hint][index].min = best_min;
			expects[hint][index].max = best_max;
		}
		else{
			expects[hint][index].avg = 0;
			expects[hint][index].min = 0;
			expects[hint][index].max = 0;
		}

		return expects[hint][index];
	}

	answer get_after_expect(board23 board, int hint, int last_action, int bag[]){
		//init bag
		for(int i = 0; i < hint_type; ++i) {
			if(bag[i] != -1) {break;}
			else if(i == 2) {
				for(int j = 0; j < hint_type; ++j) {
					bag[j] = 0;
				}
			}
		}
		
		double expect = 0.0;
		double best_min = 1000, best_max = -1000;
		int count = 0;
		for(int i = 0; i < 3; ++i){ 
			if(bag[i] == -1) {continue;}
			double min_expect = 0.0, max_expect = 0.0;
			std::vector <int> pos;
			for(int j = 0; j < 3; j++) {
				if((j == 2) && (last_action == 1 || last_action == 3)) { // left and right 
					break;
				}
				else if(last_action == 0) {// up
					if(board[1][j] == 0) {pos.push_back(j+3);}
				}
				else if(last_action == 1) {// right
					if(board[j][2] == 0) {pos.push_back(2 + (3 * j));}
				}
				else if(last_action == 2) {// down
					if(board[0][j] == 0) {pos.push_back(j);}
				}
				else if(last_action == 3) {// left
					if(board[j][0] == 0) {pos.push_back((3 * j));}
				}
			}
			for(int j = 0; j < pos.size(); j++){//place rule 
				board23 b = board; 
				int mov_result = action23::place(i, pos[j]).apply(b);
				int reward = 0;
				if(i == 3) { reward = 3;}
				if(mov_result != -1){
					int temp_bag[3]={0};
					for(int h = 0; h < hint_type; ++h) {
						temp_bag[h] = bag[h];
					}
					temp_bag[i] = -1;
					//answer temp = answer();
					answer temp = get_before_expect(b, i, temp_bag);
					expect += temp.avg + reward;
					min_expect = temp.min + reward;
					max_expect = temp.max + reward;
					count++;
					if (min_expect < best_min) {
						best_min = min_expect;
					}
					if (max_expect > best_max) {
						best_min = min_expect;
					}
				}
			}
		}
		long index = get_index(board);
		if(count != 0){after_expects[hint][last_action][index].avg = expect / count;}
		else {after_expects[hint][last_action][index].avg = 0;}
		after_expects[hint][last_action][index].min = best_min;
		after_expects[hint][last_action][index].max = best_max;
		return after_expects[hint][last_action][index];
	}

	long get_index(board23 board){
		long index = 0;
		for(int i = 0; i < row; i++){
			for(int j = 0; j < col; j++){
				index *= max_index;
				index += board[i][j];
			}
		}
		return index;
	}

	bool is_legal_after_state(board23 board, int hint, int action){
		long index = get_index(board);
		return after_expects[hint][action][index].avg != -1;
	}

	bool is_legal_before_state(board23 board, int hint){
		long index = get_index(board);
		return expects[hint][index].avg != -1;

	}

	~solver() {
		for(int i = 0; i < hint_type; ++i) {
			delete expects[i];
		}
		for(int i = 0; i < hint_type; ++i) {
			for(int j = 0; j < 4; ++j) {
				delete after_expects[i][j];
			}
		}
	}

private:
	// TODO: place your transposition table here
	answer *expects[hint_type];
	answer *after_expects[hint_type][4];
};
