/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/character_body2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/viewport.hpp>
#include <Godot/variant/vector2.hpp>
#include <Godot/variant/utility_functions.hpp>
using namespace godot;
using namespace jenova::sdk;
namespace lvl4 {
	CharacterBody2D* self = nullptr;
	float speed = 1000.0f;
	float dash_multiplier = 2.67f;
	const float warp_left_edge  =  -978.0f;
	const float warp_right_edge =  1002.0f;
	const float warp_y          =   478.0f;
}
JENOVA_SCRIPT_BEGIN
void OnAwake(Caller* instance)
{
	lvl4::self = GetSelf<CharacterBody2D>(instance);
}
void OnDestroy(Caller* instance)
{
	lvl4::self = nullptr;
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
	float current_speed = lvl4::speed;
	if (input->is_action_pressed("ui_accept"))
		current_speed *= lvl4::dash_multiplier;
	Vector2 velocity = lvl4::self->get_velocity();
	velocity.x = direction * current_speed;
	if (!lvl4::self->is_on_floor())
		velocity.y += 960.0f * (float)_delta;
	lvl4::self->set_velocity(velocity);
	lvl4::self->move_and_slide();
	Vector2 pos = lvl4::self->get_position();
	if (pos.x > lvl4::warp_right_edge) pos = Vector2(lvl4::warp_left_edge  + 5.0f, lvl4::warp_y);
	if (pos.x < lvl4::warp_left_edge)  pos = Vector2(lvl4::warp_right_edge - 5.0f, lvl4::warp_y);
	lvl4::self->set_position(pos);
	UtilityFunctions::print("X: ", pos.x, " Y: ", pos.y);
}
JENOVA_SCRIPT_END
