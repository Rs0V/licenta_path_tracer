#pragma once
#include "Utilities.hpp"
#include "functional"


interface UI {
public:
	virtual ~UI() = 0;

	static std::vector<std::function<void()>> uiGenFuncs;

	static void CreateUI() {
		for (auto &uiGenFunc : uiGenFuncs) {
			uiGenFunc();
		}
	}
};

inline UI::~UI() {}
