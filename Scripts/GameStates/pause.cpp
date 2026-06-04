/* Jenova C++ Node Base Script (Meteora) */
/* Attach this to: CanvasLayer */
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/canvas_layer.hpp>
#include <Godot/classes/control.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

CanvasLayer* pm_self        = nullptr;
Control*     pm_pause_menu  = nullptr;
Button*      pm_resume_btn  = nullptr;
Button*      pm_quit_btn    = nullptr;
bool         pm_initialized = false;
bool         pm_esc_held    = false;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	pm_self = GetSelf<CanvasLayer>(instance);

	// Only THIS node is ALWAYS — everything else stays Pausable
	pm_self->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	pm_self->set_process(true);
}

void OnReady(Caller* instance)
{
	pm_pause_menu = pm_self->get_node<Control>("PauseMenu");
	if (!pm_pause_menu)
	{
		UtilityFunctions::print("[PauseMenu] ERROR: PauseMenu not found under CanvasLayer!");
		return;
	}

	// PauseMenu and buttons must also be ALWAYS to stay interactive while paused
	pm_pause_menu->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	pm_pause_menu->set_visible(false);

	pm_resume_btn = pm_pause_menu->get_node<Button>("VBoxContainer/Resume");
	pm_quit_btn   = pm_pause_menu->get_node<Button>("VBoxContainer/Quit");

	if (pm_resume_btn)
		pm_resume_btn->connect("pressed", Callable(pm_self, "OnResumePressed"));
	else
		UtilityFunctions::print("[PauseMenu] WARNING: Resume button not found.");

	if (pm_quit_btn)
		pm_quit_btn->connect("pressed", Callable(pm_self, "OnQuitPressed"));
	else
		UtilityFunctions::print("[PauseMenu] WARNING: Quit button not found.");

	pm_initialized = true;
	UtilityFunctions::print("[PauseMenu] Ready.");
}

void OnDestroy(Caller* instance)
{
	pm_self       = nullptr;
	pm_pause_menu = nullptr;
	pm_resume_btn = nullptr;
	pm_quit_btn   = nullptr;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!pm_initialized || !pm_self) return;

	Input* input = Input::get_singleton();
	if (!input) return;

	bool esc_down = input->is_key_pressed(KEY_ESCAPE);

	if (esc_down && !pm_esc_held)
	{
		pm_esc_held = true;

		SceneTree* tree = pm_self->get_tree();
		if (!tree) return;

		bool paused = tree->is_paused();
		tree->set_pause(!paused);
		pm_pause_menu->set_visible(!paused);

		UtilityFunctions::print(!paused ? "[PauseMenu] PAUSED." : "[PauseMenu] RESUMED.");
	}
	else if (!esc_down)
	{
		pm_esc_held = false;
	}
}

void OnResumePressed(Caller* instance)
{
	if (!pm_self) return;
	SceneTree* tree = pm_self->get_tree();
	if (!tree) return;

	tree->set_pause(false);
	if (pm_pause_menu) pm_pause_menu->set_visible(false);
	UtilityFunctions::print("[PauseMenu] Resumed via button.");
}

void OnQuitPressed(Caller* instance)
{
	if (!pm_self) return;
	SceneTree* tree = pm_self->get_tree();
	if (!tree) return;

	tree->set_pause(false);
	tree->quit();
}

JENOVA_SCRIPT_END
