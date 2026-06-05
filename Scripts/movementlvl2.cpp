/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/character_body2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/viewport.hpp>
#include <Godot/variant/vector2.hpp>
#include <Godot/variant/utility_functions.hpp>
using namespace godot;
using namespace jenova::sdk;
namespace lvl2 {
	CharacterBody2D* self = nullptr;
	float speed = 900.0f;
	float dash_multiplier = 3.0f;
	const float warp_left_edge  = -1005.0f;
	const float warp_right_edge = 1005.0f;
}
JENOVA_SCRIPT_BEGIN
void OnAwake(Caller* instance)
{
	lvl2::self = GetSelf<CharacterBody2D>(instance);
}
void OnDestroy(Caller* instance)
{
	lvl2::self = nullptr;
}
void OnReady(Caller* instance)
{
}
void OnPhysicsProcess(Caller* instance, double _delta)
{
	Input* input = Input::get_singleton();
	float direction = 0.0f;
	if (input->is_action_pressed("ui_left")) direction -= 1.0f;
	if (input->is_action_pressed("ui_right")) direction += 1.0f;
	float current_speed = lvl2::speed;
	if (input->is_action_pressed("ui_accept"))
		current_speed *= lvl2::dash_multiplier;
	Vector2 velocity = lvl2::self->get_velocity();
	velocity.x = direction * current_speed;
	if (!lvl2::self->is_on_floor())
		velocity.y += 980.0f * (float)_delta;
	lvl2::self->set_velocity(velocity);
	lvl2::self->move_and_slide();
	Vector2 pos = lvl2::self->get_position();
	if (pos.x < lvl2::warp_left_edge)  pos.x = lvl2::warp_right_edge - 5.0f;
	if (pos.x > lvl2::warp_right_edge) pos.x = lvl2::warp_left_edge  + 5.0f;
	lvl2::self->set_position(pos);
}
JENOVA_SCRIPT_END
