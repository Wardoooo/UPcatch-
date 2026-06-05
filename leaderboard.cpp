#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* LB = nullptr;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	LB = GetSelf<Button>(instance);
}

void OnProcess(Caller* instance, double _delta)
{
	if (!LB) return;

	if (LB->is_pressed())
	{
		SceneTree* tree = Object::cast_to<SceneTree>(
			Engine::get_singleton()->get_main_loop()
		);
		if (tree)
		{
			LB = nullptr; // prevent double trigger
			tree->change_scene_to_file("res://leaderboard.tscn");
		}
	}
}

JENOVA_SCRIPT_END
