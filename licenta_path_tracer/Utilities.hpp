#pragma once
#include "glad.h"
#include "vector"
#include "glm.hpp"
#include "string"

typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulong;

typedef char ibyte;
typedef long long ilong;

#define getter(member) \
auto member##_get() const { \
	return this->member; \
}
#define getterr(member) \
auto& member##_getr() { \
	return this->member; \
}
#define setter(member) \
void member##_set(auto value) { \
	this->member = value; \
}
#define getset(member) \
getter(member) \
setter(member)

#define interface class
#define none {}

#define is(_var, _class, _symbol) { auto _var##_class = dynamic_cast<_class##_symbol>(_var); if (_var##_class)
#define repeat(iter, times) for (uint iter = times; iter > 0; iter--)
#define rwhile(decl, cond, iter) { decl; for (; cond; iter)
#define rif(decl, cond) { decl; if (cond)
#define relse(cond) else rif (decl, cond)
#define end }
#define rend }}

static std::string padding(std::string word, uint len) {
	return word + std::string(glm::clamp((uint)(len - word.size()), (uint)0, len), ' ');
}

template<class T>
static bool all(const std::vector<T>& v) {
	for (auto& elem : v) {
		if (!elem) {
			return false;
		}
	}
	return true;
}

template<class T>
static bool allp(const std::vector<T>& v) {
	for (auto& elem : v) {
		if (!(*elem)) {
			return false;
		}
	}
	return true;
}

template<class T, class U>
static std::vector<T> get_of_type(const std::vector<U>& v) {
	std::vector<T> rv;
	for (auto& elem : v) {
		is(elem, T) {
			rv.push_back(elemT);
		}end
	}
	return rv;
}
