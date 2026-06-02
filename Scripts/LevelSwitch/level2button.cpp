#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* startwhen_start_button_2 = nullptr;  // unique nam

JENOVA_SCRIPT_BEGIN

void startwhen_OnButtonPressed()
{
	SceneTree* tree = Object::cast_to<SceneTree>(
		Engine::get_singleton()->get_main_loop()
	);
	if (tree)
	{
		tree->change_scene_to_file("res://level2.tscn");
	}
}

void OnAwake(Caller* instance)
{
	startwhen_start_button_2 = GetSelf<Button>(instance);
	if (startwhen_start_button_2)
	{
		startwhen_start_button_2->connect("pressed", callable_mp_static(&startwhen_OnButtonPressed));
	}
}

void OnProcess(Caller* instance, double _delta)
{
}

JENOVA_SCRIPT_END
