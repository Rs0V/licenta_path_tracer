#pragma once
#include "fstream"
#include "iostream"
#include "glad.h"
#include "vector"
#include "glm.hpp"
#include "string"
#include "random"
#include "type_traits"
#include "map"
#include "unordered_map"

typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulong;

typedef char ibyte;
typedef long long ilong;

#define id_to_string(name) #name

#define getter(member) \
auto member##_get() const { \
	return this->member; \
}
#define getterr(member) \
auto& member##_getr() { \
	return this->member; \
} \
const auto& member##_getcr() const { \
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

#define pubb(def) public: def; public:
#define pubt(def) public: def; protected:
#define pubv(def) public: def; private:

#define protb(def) protected: def; public:
#define prott(def) protected: def; protected:
#define protv(def) protected: def; private:

#define privb(def) private: def; public:
#define privt(def) private: def; protected:
#define privv(def) private: def; private:



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

static std::string to_lower(const std::string &string) {
	std::string new_str;
	new_str.reserve(string.size());

	for (int i = 0; i < string.size(); i++) {
		if (string[i] > 64 and string[i] < 91) {
			new_str[i] = string[i] + 32;
		}
		else {
			new_str[i] = string[i];
		}
	}
	return new_str;
}

static std::string to_upper(const std::string& string) {
	std::string new_str;
	new_str.reserve(string.size());

	for (int i = 0; i < string.size(); i++) {
		if (string[i] > 96 and string[i] < 123) {
			new_str[i] = string[i] - 32;
		}
		else {
			new_str[i] = string[i];
		}
	}
	return new_str;
}

template<class T> static T random(T min, T max) {
	std::mt19937 rng(std::random_device{}());

	if constexpr (std::is_same<T, int>::value) {
		std::uniform_int_distribution<int> dist(min, max);
		return dist(rng);
	}
	else if constexpr (std::is_same<T, float>::value) {
		std::uniform_real_distribution<float> dist(min, max);
		return dist(rng);
	}

	return 0;
}

inline std::ostream& operator<<(std::ostream& os, glm::vec3 v) {
	os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
	return os;
}
