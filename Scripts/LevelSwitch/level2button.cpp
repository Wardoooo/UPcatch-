/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>
#include <Godot/classes/window.hpp>
#include <Godot/variant/utility_functions.hpp>
using namespace godot;
using namespace jenova::sdk;

Button* startwhen_start_button_2 = nullptr;

JENOVA_SCRIPT_BEGIN

void startwhen_OnButtonPressed_2()
{
	UtilityFunctions::print("Button pressed!");
	SceneTree* tree = Object::cast_to<SceneTree>(
		Engine::get_singleton()->get_main_loop()
	);
	if (!tree)
	{
		UtilityFunctions::print("ERROR: tree is null!");
		return;
	}
	
	tree->change_scene_to_file("res://LEVEL2.tscn");
}

void OnAwake(Caller* instance)
{
	startwhen_start_button_2 = GetSelf<Button>(instance);
	if (startwhen_start_button_2)
	{
		startwhen_start_button_2->connect("pressed", callable_mp_static(&startwhen_OnButtonPressed_2));
		UtilityFunctions::print("Signal connected!");
	}
	else
	{
		UtilityFunctions::print("ERROR: Could not get button!");
	}
}

void OnProcess(Caller* instance, double _delta)
{
}

JENOVA_SCRIPT_END
