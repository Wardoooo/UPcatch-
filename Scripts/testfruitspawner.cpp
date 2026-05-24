#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/random_number_generator.hpp>

using namespace godot;
using namespace jenova::sdk;

struct SpawnerState
{
	Node2D* self = nullptr;

	//------------------------------------------------
	// BPM / TIMING
	//------------------------------------------------

	float bpm          = 120.0f;
	float beatInterval = 0.5f;
	float timer        = 0.0f;

	//------------------------------------------------
	// SPAWN BOUNDS (X axis, in screen pixels)
	//------------------------------------------------

	float spawnXMin = 64.0f;
	float spawnXMax = 576.0f;
	float spawnY    = -32.0f;

	//------------------------------------------------
	// SCENE REF
	//------------------------------------------------

	Ref<PackedScene> fruitScene;

	RandomNumberGenerator rng;
};

SpawnerState* GetState(Caller* instance)
{
	Node2D* self = GetSelf<Node2D>(instance);

	if (!self || !self->has_meta("spawner_state"))
		return nullptr;

	return (SpawnerState*)(int64_t)
		self->get_meta("spawner_state");
}

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	Node2D* self = GetSelf<Node2D>(instance);

	SpawnerState* state = new SpawnerState();

	state->self = self;

	//------------------------------------------------
	// CONFIGURE BPM
	//------------------------------------------------

	state->bpm          = 120.0f;
	state->beatInterval = 60.0f / state->bpm;
	state->timer        = state->beatInterval;

	//------------------------------------------------
	// LOAD LUCKYDAY FRUIT SCENE
	//------------------------------------------------

	state->fruitScene =
		ResourceLoader::get_singleton()
		->load("res://LUCKYDAY.tscon");

	if (!state->fruitScene.is_valid())
	{
		UtilityFunctions::printerr(
			"Spawner: failed to load res://LUCKYDAY.tscon"
		);
	}

	//------------------------------------------------
	// SEED RNG
	//------------------------------------------------

	state->rng.randomize();

	self->set_meta(
		"spawner_state",
		(int64_t)state
	);
}

void OnDestroy(Caller* instance)
{
	Node2D* self = GetSelf<Node2D>(instance);

	if (!self || !self->has_meta("spawner_state"))
		return;

	SpawnerState* state =
		(SpawnerState*)(int64_t)
		self->get_meta("spawner_state");

	delete state;

	self->remove_meta("spawner_state");
}

void OnProcess(Caller* instance, double _delta)
{
	SpawnerState* state = GetState(instance);

	if (!state || !state->fruitScene.is_valid())
		return;

	//------------------------------------------------
	// ADVANCE TIMER
	//------------------------------------------------

	state->timer -= (float)_delta;

	if (state->timer > 0.0f)
		return;

	//------------------------------------------------
	// RESET TIMER
	//------------------------------------------------

	state->timer += state->beatInterval;

	//------------------------------------------------
	// INSTANCE FRUIT
	//------------------------------------------------

	Node* node = state->fruitScene->instantiate();

	if (!node)
	{
		UtilityFunctions::printerr(
			"Spawner: instantiate() returned null"
		);
		return;
	}

	Node2D* fruit = Object::cast_to<Node2D>(node);

	if (!fruit)
	{
		node->queue_free();
		return;
	}

	//------------------------------------------------
	// RANDOM X POSITION
	//------------------------------------------------

	float x = state->rng.randf_range(
		state->spawnXMin,
		state->spawnXMax
	);

	fruit->set_position(Vector2(x, state->spawnY));

	//------------------------------------------------
	// ADD TO FRUITS CONTAINER
	//------------------------------------------------

	Node* fruits =
		state->self->get_parent()
		->get_node<Node>(NodePath("Fruits"));

	if (fruits)
		fruits->add_child(fruit);
	else
		state->self->get_parent()->add_child(fruit);

	UtilityFunctions::print(
		"LUCKYDAY spawned at x=", x
	);
}

JENOVA_SCRIPT_END
