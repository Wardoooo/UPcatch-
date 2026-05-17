#include <Godot/godot.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/node.hpp>

using namespace godot;
using namespace jenova::sdk;

// Per-instance state — no globals!
struct FruitState
{
	Area2D* self = nullptr;
	float speed = 80.0f;
	bool caught = false;
};

FruitState* GetState(Caller* instance)
{
	// Store state pointer in instance metadata
	Area2D* self = GetSelf<Area2D>(instance);
	if (!self) return nullptr;
	if (!self->has_meta("state"))
		return nullptr;
	return (FruitState*)(int64_t)self->get_meta("state");
}

void OnAreaEntered(Area2D* fruit_area, Area2D* other_area)
{
	FruitState* state = (FruitState*)(int64_t)fruit_area->get_meta("state");
	if (!state || state->caught) return;

	if (other_area->get_name() == StringName("CollisionArea") ||
		other_area->is_in_group("player_catcher"))
	{
		state->caught = true;
		fruit_area->hide();      // instantly invisible
		fruit_area->queue_free(); // then cleaned up next frame
	}
}

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);
	FruitState* state = new FruitState();
	state->self = self;
	state->caught = false;
	self->set_meta("state", (int64_t)state);
}

void OnDestroy(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);
	if (self && self->has_meta("state"))
	{
		FruitState* state = (FruitState*)(int64_t)self->get_meta("state");
		delete state;
		self->remove_meta("state");
	}
}

void OnReady(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);
	self->connect(
		"area_entered",
		Callable(self, "OnAreaEntered")
	);
}

void OnProcess(Caller* instance, double _delta)
{
	FruitState* state = GetState(instance);
	if (!state || state->caught) return;

	Vector2 pos = state->self->get_position();
	pos.y += state->speed * (float)_delta;
	state->self->set_position(pos);

	if (pos.y > 1200.0f)
	{
		state->caught = true;
		state->self->queue_free();
	}
}

JENOVA_SCRIPT_END
