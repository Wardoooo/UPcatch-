/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>
#include <Godot/classes/window.hpp>
#include <Godot/variant/utility_functions.hpp>
using namespace godot;
using namespace jenova::sdk;

Button* startwhen_start_button = nullptr;

JENOVA_SCRIPT_BEGIN

void startwhen_OnButtonPressed()
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

	Node* game_state = tree->get_root()->get_node<Node>("GameState");
	if (!game_state)
	{
		UtilityFunctions::print("ERROR: GameState not found at /root/GameState!");
		return;
	}

	game_state->set("selected_level_scene", String("res://1level_1.tscn"));
	UtilityFunctions::print("Level scene set to res://1level_1.tscn");

	tree->change_scene_to_file("res://character_select.tscn");
}

void OnAwake(Caller* instance)
{
	startwhen_start_button = GetSelf<Button>(instance);
	if (startwhen_start_button)
	{
		startwhen_start_button->connect("pressed", callable_mp_static(&startwhen_OnButtonPressed));
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
