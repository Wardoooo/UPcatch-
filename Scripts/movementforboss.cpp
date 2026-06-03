/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/character_body2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/viewport.hpp>
#include <Godot/variant/vector2.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

static CharacterBody2D* self = nullptr;

JENOVA_SCRIPT_BEGIN

float speed            = 900.0f;
float base_speed       = 900.0f;
float max_dash_speed   = 900.0f * 8.0f;  // cap at 8x speed
float dash_speed_step  = 900.0f * 2.0f;  // each press adds 2x base speed
float dash_decay_rate  = 3000.0f;         // how fast speed decays per second
float dash_window      = 0.3f;            // time window to count rapid presses (seconds)

float dash_timer       = 0.0f;            // time since last space press
int   dash_press_count = 0;               // presses within window
float current_dash_speed = 0.0f;          // extra speed from dashing

bool space_was_pressed = false;           // edge detection

void OnAwake(Caller* instance)
{
	self = GetSelf<CharacterBody2D>(instance);
}

void OnDestroy(Caller* instance)
{
	self = nullptr;
}

void OnReady(Caller* instance) {}

void OnPhysicsProcess(Caller* instance, double _delta)
{
	Input* input = Input::get_singleton();

	// detect single space press (not held)
	bool space_pressed = input->is_action_pressed("ui_accept");
	if (space_pressed && !space_was_pressed)
	{
		// new press detected
		if (dash_timer < dash_window)
		{
			// within window — stack speed
			dash_press_count++;
		}
		else
		{
			// outside window — reset count
			dash_press_count = 1;
		}

		dash_timer = 0.0f;
		current_dash_speed = dash_speed_step * dash_press_count;
		if (current_dash_speed > max_dash_speed)
			current_dash_speed = max_dash_speed;

		UtilityFunctions::print("Dash! presses: ", dash_press_count, " speed: ", current_dash_speed);
	}
	space_was_pressed = space_pressed;

	// tick dash window timer
	dash_timer += (float)_delta;

	// decay dash speed back to 0 over time
	if (current_dash_speed > 0.0f)
	{
		current_dash_speed -= dash_decay_rate * (float)_delta;
		if (current_dash_speed < 0.0f)
			current_dash_speed = 0.0f;
	}

	// reset press count if window expired
	if (dash_timer > dash_window)
		dash_press_count = 0;

	// movement
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

	// screen warp
	Vector2 pos = self->get_position();
	if (pos.x > -50.0f)   pos.x = -1500.0f;
	if (pos.x < -1500.0f) pos.x =  0.0f;
	self->set_position(pos);
}

JENOVA_SCRIPT_END
