#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* startwhen_start_button_boss = nullptr;  // unique nam

JENOVA_SCRIPT_BEGIN

void startwhen_OnButtonPressed()
{
	SceneTree* tree = Object::cast_to<SceneTree>(
		Engine::get_singleton()->get_main_loop()
	);
	if (tree)
	{
		tree->change_scene_to_file("res://bosslvl.tscn");
	}
}

void OnAwake(Caller* instance)
{
	startwhen_start_button_boss = GetSelf<Button>(instance);
	if (startwhen_start_button_boss)
	{
		startwhen_start_button_boss->connect("pressed", callable_mp_static(&startwhen_OnButtonPressed));
	}
}

void OnProcess(Caller* instance, double _delta)
{
}

JENOVA_SCRIPT_END
