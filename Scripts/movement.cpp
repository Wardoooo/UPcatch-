/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/character_body2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/viewport.hpp>
#include <Godot/variant/vector2.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

CharacterBody2D* self = nullptr;

JENOVA_SCRIPT_BEGIN

float speed = 900.0f;
float dash_multiplier = 2.0f;

void OnAwake(Caller* instance)
{
	self = GetSelf<CharacterBody2D>(instance);
}

void OnDestroy(Caller* instance)
{
	self = nullptr;
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

	float current_speed = speed;
	if (input->is_action_pressed("ui_accept"))
		current_speed *= dash_multiplier;

	Vector2 velocity = self->get_velocity();
	velocity.x = direction * current_speed;

	if (!self->is_on_floor())
		velocity.y += 980.0f * (float)_delta;

	self->set_velocity(velocity);
	self->move_and_slide();

	Vector2 pos = self->get_position();
	

	if (pos.x > 1200.0f) pos.x = -1100.0f;
	if (pos.x < -1200.0f) pos.x = 1100.0f;

self->set_position(pos);
	self->set_position(pos);
}

JENOVA_SCRIPT_END
