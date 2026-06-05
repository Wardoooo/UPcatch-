#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* TREN = nullptr;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	TREN = GetSelf<Button>(instance);
}

void OnProcess(Caller* instance, double _delta)
{
	if (!TREN) return;

	if (TREN->is_pressed())
	{
		SceneTree* tree = Object::cast_to<SceneTree>(
			Engine::get_singleton()->get_main_loop()
		);
		if (tree)
		{
			TREN = nullptr;
			tree->change_scene_to_file("res://TUTORIAL.tscn");
		}
	}
}

JENOVA_SCRIPT_END
