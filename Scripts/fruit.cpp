#include <Godot/godot.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/sprite2d.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/texture2d.hpp>

using namespace godot;
using namespace jenova::sdk;

struct FruitState
{
	Area2D* self = nullptr;

	Sprite2D* sprite = nullptr;

	float speed = 80.0f;

	bool caught = false;

	int frames = 0;

	int fruitIndex = 0;

	Ref<Texture2D> spriteSheet;
};

FruitState* GetState(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);

	if (!self || !self->has_meta("state"))
		return nullptr;

	return (FruitState*)(int64_t)
		self->get_meta("state");
}

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);

	FruitState* state = new FruitState();

	state->self = self;

	//------------------------------------------------
	// GET SPRITE NODE
	//------------------------------------------------

	state->sprite =
		self->get_node<Sprite2D>(
			NodePath("Sprite2D"));

	//------------------------------------------------
	// LOAD SPRITESHEET
	//------------------------------------------------

	state->spriteSheet =
		ResourceLoader::get_singleton()
		->load(
			"res://Assets/Textures/POTAAAAAANGERNAAAA1.png"
		);

	if (state->spriteSheet.is_valid())
	{
		state->sprite->set_texture(
			state->spriteSheet
		);
	}

	//------------------------------------------------
	// ENABLE REGION MODE
	//------------------------------------------------

	state->sprite->set_region_enabled(true);

	//------------------------------------------------
	// SPRITESHEET SETTINGS
	//------------------------------------------------

	int fruitSize = 32;

	int totalFruits = 3;

	//------------------------------------------------
	// RANDOM FRUIT
	//------------------------------------------------

	state->fruitIndex =
		rand() % totalFruits;

	//------------------------------------------------
	// CREATE REGION RECT
	//------------------------------------------------

	Rect2 region(
		state->fruitIndex * fruitSize,
		0,
		fruitSize,
		fruitSize
	);

	//------------------------------------------------
	// APPLY REGION
	//------------------------------------------------

	state->sprite->set_region_rect(region);

	//------------------------------------------------

	state->caught = false;

	state->frames = 0;

	self->set_meta(
		"state",
		(int64_t)state
	);
}

void OnDestroy(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);

	if (!self || !self->has_meta("state"))
		return;

	FruitState* state =
		(FruitState*)(int64_t)
		self->get_meta("state");

	delete state;

	self->remove_meta("state");
}

void OnProcess(Caller* instance, double _delta)
{
	FruitState* state = GetState(instance);

	if (!state || state->caught)
		return;

	//------------------------------------------------
	// MOVE DOWN
	//------------------------------------------------

	Vector2 pos =
		state->self->get_position();

	pos.y +=
		state->speed *
		(float)_delta;

	state->self->set_position(pos);

	//------------------------------------------------
	// DELETE IF OFFSCREEN
	//------------------------------------------------

	if (pos.y > 1400.0f)
	{
		state->caught = true;

		state->self->queue_free();

		return;
	}

	//------------------------------------------------
	// WAIT FOR PHYSICS
	//------------------------------------------------

	state->frames++;

	if (state->frames < 5)
		return;

	//------------------------------------------------
	// CHECK OVERLAPS
	//------------------------------------------------

	Array overlaps =
		state->self->get_overlapping_areas();

	for (int i = 0; i < overlaps.size(); i++)
	{
		Area2D* area =
			Object::cast_to<Area2D>(
				overlaps[i]
			);

		if (!area)
			continue;

		if (
			area->is_in_group(
				"player_catcher"
			)
		)
		{
			state->caught = true;

			state->self->hide();

			state->self->queue_free();

			UtilityFunctions::print(
				"FRUIT CAUGHT"
			);

			return;
		}
	}
}

JENOVA_SCRIPT_END
