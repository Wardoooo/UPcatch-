/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

// -- score state (shared across all fruit instances via globals)
int fruit_score      = 0;
int fruit_multiplier = 0;
int fruit_caught     = 0;
int fruit_total      = 0;

Area2D* fruit_self           = nullptr;
float   fruit_fall_speed     = 400.0f;
bool    fruit_is_template    = true; // templates don't fall or score

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	fruit_self = GetSelf<Area2D>(instance);
}

void OnDestroy(Caller* instance)
{
	fruit_self = nullptr;
}

void OnReady(Caller* instance) {}



void OnProcess(Caller* instance, double _delta)
{
	if (!fruit_self) return;
	if (!fruit_self->is_in_group("active_fruit")) return; // only fall if active

	Vector2 pos = fruit_self->get_position();
	pos.y += fruit_fall_speed * (float)_delta;
	fruit_self->set_position(pos);

	if (pos.y > 600.0f)
	{
		fruit_total++;
		fruit_multiplier = 0;
		UtilityFunctions::print("Miss! Accuracy: ",
			(fruit_total > 0 ? (float)fruit_caught / fruit_total * 100.0f : 0.0f), "%");
		fruit_self->queue_free();
	}
}

void OnAreaEntered(Caller* instance, Area2D* area)
{
	if (!fruit_self) return;
	if (!fruit_self->is_in_group("active_fruit")) return; // only catch if active

	if (area && area->is_in_group("player_catcher"))
	{
		fruit_total++;
		fruit_caught++;
		fruit_multiplier++;
		fruit_score += 300 * fruit_multiplier;

		float accuracy = (fruit_total > 0)
			? (float)fruit_caught / fruit_total * 100.0f
			: 0.0f;

		UtilityFunctions::print(
			"Caught! Score: ", fruit_score,
			" | Multiplier: x", fruit_multiplier,
			" | Accuracy: ", accuracy, "%"
		);
		fruit_self->queue_free();
	}
}

JENOVA_SCRIPT_END
