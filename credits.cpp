#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* credits_start_button = nullptr;  // renamed

JENOVA_SCRIPT_BEGIN

void OnCreditsStartPressed()
{
	SceneTree* tree = Object::cast_to<SceneTree>(
		Engine::get_singleton()->get_main_loop()
	);
	if (tree) tree->change_scene_to_file("res://credits.tscn");
}

void OnAwake(Caller* instance)
{
	credits_start_button = GetSelf<Button>(instance);
	if (credits_start_button)
	{
		credits_start_button->connect("pressed", callable_mp_static(&OnCreditsStartPressed));
	}
}

JENOVA_SCRIPT_END
