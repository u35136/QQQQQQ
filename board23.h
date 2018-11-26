#pragma once
#include <array>
#include <iostream>
#include <iomanip>
#include <math.h>

/**
 * array-based board23 for 2048
 *
 * index (1-d form):
 *  (0)  (1)  (2)  
 *  (3)  (4)  (5)
 *
 */
class board23 {
public:
	static const int ROW = 2;
	static const int COLUMN = 3;
	static const int POSSIBLE_INDEX = 10;
	
public:
	typedef uint32_t cell;
	typedef std::array<cell, 3> row;
	typedef std::array<row, 2> grid;
	typedef uint64_t data;
	typedef int reward;
public:
	board23() : tile(), attr(0) {}
	board23(const grid& b, data v = 0) : tile(b), attr(v) {}
	board23(const board23& b) = default;
	board23& operator =(const board23& b) = default;

	operator grid&() { return tile; }
	operator const grid&() const { return tile; }
	row& operator [](unsigned i) { return tile[i]; }
	const row& operator [](unsigned i) const { return tile[i]; }
	cell& operator ()(unsigned i) { return tile[i / 3][i % 3]; }
	const cell& operator ()(unsigned i) const { return tile[i / 3][i % 3]; }

	data info() const { return attr; }
	data info(data dat) { data old = attr; attr = dat; return old; }

public:
	bool operator ==(const board23& b) const { return tile == b.tile; }
	bool operator < (const board23& b) const { return tile <  b.tile; }
	bool operator !=(const board23& b) const { return !(*this == b); }
	bool operator > (const board23& b) const { return b < *this; }
	bool operator <=(const board23& b) const { return !(b < *this); }
	bool operator >=(const board23& b) const { return !(*this < b); }

public:

	reward final_score() const {
		reward sum = 0;
		for (int r = 0; r < 2; r++) {
			auto& row = tile[r];
			for (int c = 0; c < 3; c++) {
				int tile = row[c];
				if (tile >= 3) sum += pow(3 , tile - 2);
			}
		}
		return sum;
	}
	/**
	 * place a tile (index value) to the specific position (1-d form index)
	 * return 0 if the action is valid, or -1 if not
	 */
	reward place(unsigned pos, cell tile) {
		if (pos >= 14) return -1;
		if (tile != 1 && tile != 2 && tile != 3) return -1;
		operator()(pos) = tile;
		return 0;
	}

	/**
	 * apply an action to the board23
	 * return the reward of the action, or -1 if the action is illegal
	 */
	reward slide(unsigned opcode) {
		switch (opcode & 0b11) {
		case 0: return slide_up();
		case 1: return slide_right();
		case 2: return slide_down();
		case 3: return slide_left();
		default: return -1;
		}
	}

	reward slide_left() {
		board23 prev = *this;
		reward score = 0;
		reward init_score = final_score();
		for (int r = 0; r < 2; r++) {  
			auto& row = tile[r];
			bool move_flag = false;
			int top = 0, hold = 0;
			for (int c = 0; c < 3; c++) {
				int tile = row[c];
				if (tile == 0) {
					if (!move_flag)	move_flag = true;
					else row[top++] = tile;
					continue;
				}
				row[c] = 0;
				if (((hold == 1 && tile == 2) || (hold == 2 && tile == 1)) && !move_flag) {
					row[top-1] = 3;
					hold = 0;
					move_flag = true;
				}
				else if (hold && hold != 1 && hold != 2 && !move_flag) {
					if (tile == hold) {
						row[top-1] = ++tile;
						hold = 0;
						move_flag = true;
					} else {
						row[top++] = tile;
						hold = tile;
					}
				} else { 
					row[top++] = tile;
					hold = tile;
				}
			}
			//if (hold) tile[r][top] = hold;
		}
		reward after_score = final_score();
		score = after_score - init_score;
		return (*this != prev) ? score : -1;
	}
	reward slide_right() {
		reflect_horizontal();
		reward score = slide_left();
		reflect_horizontal();
		return score;
	}
	reward slide_up() {
		board23 prev = *this;
		reward score = 0;
		reward init_score = final_score();
		auto& row_0 = tile[0];
		auto& row_1 = tile[1];
		for (int c = 0; c < 3; c++)  {  
			int hold = row_0[c];
			int tile = row_1[c];
			if(hold == 0) {
				row_0[c] = tile;
				row_1[c] = 0;
			}
			else if((hold == 1 && tile == 2) || (hold == 2 && tile == 1)) {
				row_0[c] = 3;
				row_1[c]= 0;
			}
			else if(hold && hold != 1 && hold != 2){
				if (tile == hold) {
						row_0[c] = ++tile;
						row_1[c] = 0;
					} 
			}
			else {
				row_0[c] = hold;
				row_1[c] = tile;
			}
		}
		reward after_score = final_score();
		score = after_score - init_score;
		return (*this != prev) ? score : -1;
	}
	reward slide_down() {
		reflect_vertical();
		int score = slide_up();
		reflect_vertical();
		return score;
	}

	void reflect_horizontal() {
		std::swap(tile[0][0], tile[0][2]);
		std::swap(tile[1][0], tile[1][2]);
	}

	void reflect_vertical() {
		std::swap(tile[0], tile[1]);
	}

	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
	friend std::ostream& operator <<(std::ostream& out, const board23& b) {
		//std::cout << std::endl << "oo>> " << std::flush;
		for (int i = 0; i < 6; i++) {
			out << " " << b(i);
		}
		return out;
		
		/*	out << std::setw(6) << tile_type[(int)t];
		int tile_type[15] = {0, 1, 2, 3, 6, 12, 24, 48, 96, 192, 384, 768, 1536, 3072, 6144};
		out << "+------------------------+" << std::endl;
		for (auto& row : b.tile) {
			out << "|" << std::dec;
			for (auto t : row )	out << std::setw(6) << t;
			out << "|" << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;*/
	}
	friend std::istream& operator >>(std::istream& in, board23& b) {
		for (int i = 0; i < 6; i++) {
			while (!std::isdigit(in.peek()) && in.good()) in.ignore(1);
			in >> b(i);
			//b(i) = log2(b(i));
		}
		std::cout << b;
		return in;
	}


private:
	grid tile;
	data attr;
};
