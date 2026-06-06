/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/video_stream_player.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/input.hpp>

using namespace godot;
using namespace jenova::sdk;

static VideoStreamPlayer* TUTVID = nullptr;
static bool changed_scene = false;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	TUTVID = GetSelf<VideoStreamPlayer>(instance);
	changed_scene = false;
}

void OnDestroy(Caller* instance)
{
	TUTVID = nullptr;
}

void OnReady(Caller* instance)
{
	if (TUTVID)
		TUTVID->play();
}

void GoToMenu()
{
	if (!TUTVID || changed_scene)
		return;

	changed_scene = true;
	TUTVID->stop();
	TUTVID->get_tree()->change_scene_to_file("res://menu.tscn");
}

void OnProcess(Caller* instance, double delta)
{
	if (!TUTVID || changed_scene)
		return;


	if (Input::get_singleton()->is_key_pressed(Key::KEY_SPACE))
	{
		GoToMenu();
		return;
	}

	
	if (TUTVID->get_stream_position() >= 74.0)
	{
		GoToMenu();
	}
}

JENOVA_SCRIPT_END
