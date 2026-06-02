/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

Node* score_manager_self = nullptr;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	score_manager_self = GetSelf<Node>(instance);
}

void OnDestroy(Caller* instance)
{
	score_manager_self = nullptr;
}

void OnReady(Caller* instance) {}

void OnProcess(Caller* instance, double _delta)
{
	// hook to UI labels later
}

JENOVA_SCRIPT_END
