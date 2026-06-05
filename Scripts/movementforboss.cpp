/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/character_body2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/viewport.hpp>
#include <Godot/variant/vector2.hpp>
#include <Godot/variant/utility_functions.hpp>
#include <Godot/classes/collision_shape2d.hpp>
#include <Godot/classes/rectangle_shape2d.hpp>
using namespace godot;
using namespace jenova::sdk;
static CharacterBody2D* self = nullptr;
JENOVA_SCRIPT_BEGIN
CollisionShape2D* catcher_shape   = nullptr;
Vector2           base_shape_size = Vector2(50.0f, 50.0f);
float base_speed         = 900.0f;
float max_dash_speed     = 900.0f * 16.0f;
float dash_speed_step    = 900.0f * 5.0f;
float dash_decay_rate    = 3000.0f;
float dash_window        = 0.3f;
float dash_timer         = 0.0f;
int   dash_press_count   = 0;
float current_dash_speed = 0.0f;
bool  space_was_pressed  = false;

const float warp_right_edge = 950.0f;
const float warp_left_edge  = -950.0f;

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
	catcher_shape = self->get_node<CollisionShape2D>("CollisionShape2D");
	if (catcher_shape)
	{
		Ref<RectangleShape2D> rect = catcher_shape->get_shape();
		if (rect.is_valid()) base_shape_size = rect->get_size();
	}
}
void OnPhysicsProcess(Caller* instance, double _delta)
{
	Input* input = Input::get_singleton();

	bool space_pressed = input->is_action_pressed("ui_accept");
	if (space_pressed && !space_was_pressed)
	{
		if (dash_timer < dash_window)
			dash_press_count++;
		else
			dash_press_count = 1;

		dash_timer = 0.0f;
		current_dash_speed = dash_speed_step * dash_press_count;
		if (current_dash_speed > max_dash_speed)
			current_dash_speed = max_dash_speed;
		UtilityFunctions::print("Dash! presses: ", dash_press_count, " speed: ", current_dash_speed);
	}
	space_was_pressed = space_pressed;

	
	dash_timer += (float)_delta;


	if (current_dash_speed > 0.0f)
	{
		current_dash_speed -= dash_decay_rate * (float)_delta;
		if (current_dash_speed < 0.0f)
			current_dash_speed = 0.0f;
	}

	
	if (dash_timer > dash_window)
		dash_press_count = 0;

	
	float direction = 0.0f;
	if (input->is_action_pressed("ui_left"))  direction -= 1.0f;
	if (input->is_action_pressed("ui_right")) direction += 1.0f;

	float total_speed = base_speed + current_dash_speed;
	Vector2 velocity = self->get_velocity();
	velocity.x = direction * total_speed;
	if (!self->is_on_floor())
		velocity.y += 980.0f * (float)_delta;
	self->set_velocity(velocity);
	self->move_and_slide();


	Vector2 pos = self->get_position();
	if (pos.x > warp_right_edge) pos.x = warp_left_edge  + 5.0f;
	if (pos.x < warp_left_edge)  pos.x = warp_right_edge - 5.0f;
	self->set_position(pos);
}
JENOVA_SCRIPT_END
