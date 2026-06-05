#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* GOTOLEVEL4_button_endless = nullptr;  

JENOVA_SCRIPT_BEGIN

void startwhen_OnButtonPressed()
{
	SceneTree* tree = Object::cast_to<SceneTree>(
		Engine::get_singleton()->get_main_loop()
	);
	if (tree)
	{
		tree->change_scene_to_file("res://LEVEL4.tscn");
	}
}

void OnAwake(Caller* instance)
{
	GOTOLEVEL4_button_endless = GetSelf<Button>(instance);
	if (GOTOLEVEL4_button_endless)
	{
		GOTOLEVEL4_button_endless->connect("pressed", callable_mp_static(&startwhen_OnButtonPressed));
	}
}

void OnProcess(Caller* instance, double _delta)
{
}

JENOVA_SCRIPT_END
