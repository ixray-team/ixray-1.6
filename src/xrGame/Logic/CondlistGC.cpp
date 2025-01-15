#include "stdafx.h"
#include "CondlistGC.h"
#include "script_game_object.h"

CCondlistGarbageCollector* GCondlistGC = nullptr;

const char* CCondlistGarbageCollector::Registry(shared_str Value)
{
	return **Storage.emplace(Value).first;
}