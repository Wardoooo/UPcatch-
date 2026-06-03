/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

Area2D* fruit_self       = nullptr;
float   fruit_fall_speed = 400.0f;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	fruit_self = GetSelf<Area2D>(instance);
	fruit_self->set_process(true);
}

void OnReady(Caller* instance) {}

void OnDestroy(Caller* instance)
{
	fruit_self = nullptr;
}

void OnProcess(Caller* instance, double _delta)
{
	// fruit.cpp does nothing — spawner handles all movement, scoring and cleanup
}

void OnAreaEntered(Caller* instance, Area2D* area)
{
	// spawner handles catch detection
}

JENOVA_SCRIPT_END
