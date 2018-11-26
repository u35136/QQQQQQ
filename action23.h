#pragma once
#include <algorithm>
#include <unordered_map>
#include <string>
#include "board23.h"

class action23 {
public:
	action23(unsigned code = -1u) : code(code) {}
	action23(const action23& a) : code(a.code) {}
	virtual ~action23() {}

	class slide; // create a sliding action23 with board23 opcode
	class place; // create a placing action23 with position and tile

public:
	virtual board23::reward apply(board23& b) const {
		auto proto = entries().find(type());
		if (proto != entries().end()) return proto->second->reinterpret(this).apply(b);
		return -1;
	}
	virtual std::ostream& operator >>(std::ostream& out) const {
		auto proto = entries().find(type());
		if (proto != entries().end()) return proto->second->reinterpret(this) >> out;
		return out << "??";
	}
	virtual std::istream& operator <<(std::istream& in) {
		auto state = in.rdstate();
		for (auto proto = entries().begin(); proto != entries().end(); proto++) {
			if (proto->second->reinterpret(this) << in) return in;
			in.clear(state);
		}
		return in.ignore(2);
	}

public:
	operator unsigned() const { return code; }
	unsigned type() const { return code & type_flag(-1u); }
	unsigned event() const { return code & ~type(); }
	friend std::ostream& operator <<(std::ostream& out, const action23& a) { return a >> out; }
	friend std::istream& operator >>(std::istream& in, action23& a) { return a << in; }

protected:
	static constexpr unsigned type_flag(unsigned v) { return v << 24; }

	typedef std::unordered_map<unsigned, action23*> prototype;
	static prototype& entries() { static prototype m; return m; }
	virtual action23& reinterpret(const action23* a) const { return *new (const_cast<action23*>(a)) action23(*a); }

	unsigned code;
};

class action23::slide : public action23 {
public:
	static constexpr unsigned type = type_flag('s');
	slide(unsigned oper) : action23(slide::type | (oper & 0b11)) {}
	slide(const action23& a = {}) : action23(a) {}
public:
	board23::reward apply(board23& b) const {
		return b.slide(event());
	}
	std::ostream& operator >>(std::ostream& out) const {
		return out << '#' << ("URDL")[event() & 0b11];
	}
	std::istream& operator <<(std::istream& in) {
		if (in.peek() == '#') {
			char v;
			in.ignore(1) >> v;
			const char* opc = "URDL";
			unsigned oper = std::find(opc, opc + 4, v) - opc;
			if (oper < 4) {
				operator= (action23::slide(oper));
				return in;
			}
		}
		in.setstate(std::ios::failbit);
		return in;
	}
protected:
	action23& reinterpret(const action23* a) const { return *new (const_cast<action23*>(a)) slide(*a); }
	static __attribute__((constructor)) void init() { entries()[type_flag('s')] = new slide; }
};

class action23::place : public action23 {
public:
	static constexpr unsigned type = type_flag('p');
	place(unsigned pos, unsigned tile) : action23(place::type | (pos & 0x0f) | (std::min(tile, 35u) << 4)) {}
	place(const action23& a = {}) : action23(a) {}
	unsigned position() const { return event() & 0x0f; }
	unsigned tile() const { return event() >> 4; }
public:
	board23::reward apply(board23& b) const {
		return b.place(position(), tile());
	}
	std::ostream& operator >>(std::ostream& out) const {
		const char* idx = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ?";
		return out << idx[position()] << idx[std::min(tile(), 36u)];
	}
	std::istream& operator <<(std::istream& in) {
		const char* idx = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		char v = in.peek();
		unsigned pos = std::find(idx, idx + 16, v) - idx;
		if (pos < 16) {
			in.ignore(1) >> v;
			unsigned tile = std::find(idx, idx + 36, v) - idx;
			if (tile < 36) {
				operator =(action23::place(pos, tile));
				return in;
			}
		}
		in.setstate(std::ios::failbit);
		return in;
	}
protected:
	action23& reinterpret(const action23* a) const { return *new (const_cast<action23*>(a)) place(*a); }
	static __attribute__((constructor)) void init() { entries()[type_flag('p')] = new place; }
};
