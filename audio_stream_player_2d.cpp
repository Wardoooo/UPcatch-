/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/audio_stream_player.hpp>

using namespace godot;
using namespace jenova::sdk;

AudioStreamPlayer* music = nullptr;
bool first_loop_done = false;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	music = GetSelf<AudioStreamPlayer>(instance);
}

void OnReady(Caller* instance)
{
	if (music)
	{
		// MAG START SA 19 SECS AHG SONG
		music->play(19.0);
	}
}

void OnProcess(Caller* instance, double _delta)
{
	if (!music)
		return;
		
	if (!music->is_playing())
	{
		if (!first_loop_done)
		{
			first_loop_done = true;

			//LOOP TA
			music->play(0.0);
		}
		else
		{
			
			music->play(0.0);
		}
	}
}

JENOVA_SCRIPT_END
