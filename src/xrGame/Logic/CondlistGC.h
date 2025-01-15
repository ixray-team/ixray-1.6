#pragma once
#include "script_utility.h"
#include "GameObject.h"

class CCondlistGarbageCollector final
{
public:
	CCondlistGarbageCollector() = default;
	~CCondlistGarbageCollector() = default;

	const char* Registry(shared_str Value);

private:
	xr_set<shared_str> Storage;
};

extern CCondlistGarbageCollector* GCondlistGC;