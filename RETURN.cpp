#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* returnRIGHTNOW = nullptr; 

JENOVA_SCRIPT_BEGIN

void OnCreditsStartPressed()
{
	SceneTree* tree = Object::cast_to<SceneTree>(
		Engine::get_singleton()->get_main_loop()
	);
	if (tree) tree->change_scene_to_file("res://menu.tscn");
}

void OnAwake(Caller* instance)
{
	returnRIGHTNOW = GetSelf<Button>(instance);
	if (returnRIGHTNOW)
	{
		returnRIGHTNOW->connect("pressed", callable_mp_static(&OnCreditsStartPressed));
	}
}

JENOVA_SCRIPT_END
