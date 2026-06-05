/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/character_body2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/viewport.hpp>
#include <Godot/variant/vector2.hpp>
#include <Godot/variant/utility_functions.hpp>
using namespace godot;
using namespace jenova::sdk;
namespace sn5 {
	CharacterBody2D* self = nullptr;
	float base_speed         = 900.0f;
	float max_dash_speed     = 960.0f * 15.0f;
	float dash_speed_step    = 900.0f * 5.0f;
	float dash_decay_rate    = 3000.0f;  
	float dash_window        = 0.3f;     
	float dash_timer         = 0.0f;
	int   dash_press_count   = 0;
	float current_dash_speed = 0.0f;
	bool  space_was_pressed  = false;
	const float warp_left_edge  = -950.0f;
	const float warp_right_edge =  950.0f;
	const float warp_y          =  478.0f;
}
JENOVA_SCRIPT_BEGIN
void OnAwake(Caller* instance)
{
	sn5::self = GetSelf<CharacterBody2D>(instance);
}
void OnDestroy(Caller* instance)
{
	sn5::self = nullptr;
}
void OnReady(Caller* instance)
{
}
void OnPhysicsProcess(Caller* instance, double _delta)
{
	Input* input = Input::get_singleton();

	
	bool space_pressed = input->is_action_pressed("ui_accept");
	if (space_pressed && !sn5::space_was_pressed)
	{
		
		sn5::dash_press_count++;
		sn5::dash_timer = 0.0f; 

		sn5::current_dash_speed = sn5::dash_speed_step * sn5::dash_press_count;
		if (sn5::current_dash_speed > sn5::max_dash_speed)
			sn5::current_dash_speed = sn5::max_dash_speed;

		
	}
	sn5::space_was_pressed = space_pressed;

	
	sn5::dash_timer += (float)_delta;


	if (sn5::current_dash_speed > 0.0f)
	{
		sn5::current_dash_speed -= sn5::dash_decay_rate * (float)_delta;
		if (sn5::current_dash_speed < 0.0f)
			sn5::current_dash_speed = 0.0f;
	}

	
	if (sn5::dash_timer > sn5::dash_window)
	{
		sn5::dash_press_count = 0;
	}


	float direction = 0.0f;
	if (input->is_action_pressed("ui_left"))  direction -= 1.0f;
	if (input->is_action_pressed("ui_right")) direction += 1.0f;

	float total_speed = sn5::base_speed + sn5::current_dash_speed;
	Vector2 velocity = sn5::self->get_velocity();
	velocity.x = direction * total_speed;
	if (!sn5::self->is_on_floor())
		velocity.y += 960.0f * (float)_delta;
	sn5::self->set_velocity(velocity);
	sn5::self->move_and_slide();

	
	Vector2 pos = sn5::self->get_position();
	if (pos.x > sn5::warp_right_edge) pos = Vector2(sn5::warp_left_edge  + 5.0f, sn5::warp_y);
	if (pos.x < sn5::warp_left_edge)  pos = Vector2(sn5::warp_right_edge - 5.0f, sn5::warp_y);
	sn5::self->set_position(pos);

}
JENOVA_SCRIPT_END
