/* Jenova C++ Control Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/control.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/input_event.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>

using namespace godot;
using namespace jenova::sdk;

Button* quit_button = nullptr;

JENOVA_SCRIPT_BEGIN

void OnQuitPressed()
{
	SceneTree* tree = Object::cast_to<SceneTree>(
		Engine::get_singleton()->get_main_loop()
	);
	if (tree) tree->quit();
}

void OnAwake(Caller* instance)
{
	quit_button = GetSelf<Button>(instance);
	if (quit_button)
	{
		quit_button->connect("pressed", callable_mp_static(&OnQuitPressed));
	}
}

void OnUserInterfaceInput(Caller* instance, InputEvent* p_event)
{
}

JENOVA_SCRIPT_END
