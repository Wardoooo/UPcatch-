/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/video_stream_player.hpp>
#include <Godot/classes/scene_tree.hpp>

using namespace godot;
using namespace jenova::sdk;

static VideoStreamPlayer* self = nullptr;
static bool changed_scene = false;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	self = GetSelf<VideoStreamPlayer>(instance);
	changed_scene = false;
}

void OnDestroy(Caller* instance)
{
	self = nullptr;
}

void OnReady(Caller* instance)
{
	if (self)
		self->play();
}

void OnProcess(Caller* instance, double delta)
{
	if (!self || changed_scene)
		return;

	if (!self->is_playing())
	{
		changed_scene = true;
		self->get_tree()->change_scene_to_file("res://menu.tscn");
	}
}

JENOVA_SCRIPT_END
