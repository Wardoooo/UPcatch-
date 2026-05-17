#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* start_button = nullptr;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	start_button = GetSelf<Button>(instance);
}

void OnProcess(Caller* instance, double _delta)
{
	if (!start_button) return;

	if (start_button->is_pressed())
	{
		SceneTree* tree = Object::cast_to<SceneTree>(
			Engine::get_singleton()->get_main_loop()
		);
		if (tree)
		{
			start_button = nullptr; // prevent double trigger
			tree->change_scene_to_file("res://level_1.tscn");
		}
	}
}

JENOVA_SCRIPT_END
