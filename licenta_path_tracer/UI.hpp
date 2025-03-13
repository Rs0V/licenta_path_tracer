#pragma once
#include "Utilities.hpp"


interface UI {
public:
	virtual ~UI() = 0;

	static std::vector<void(*)()> uiGenFuncs;

	static std::string buffer;
	static float value;
	static void CreateUI() {
		for (auto &uiGenFunc : uiGenFuncs) {
			uiGenFunc();
		}
	}
};

inline UI::~UI() {}
