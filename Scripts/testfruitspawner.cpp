/* Jenova osu!catch Fruit Spawner */

#include <Godot/godot.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/window.hpp>

using namespace godot;
using namespace jenova::sdk;

struct SpawnerState
{
	Node2D* self = nullptr;

	Ref<PackedScene> fruitScene;

	float timer = 0.0f;

	float interval = 1.2f;

	float windowWidth = 1280.0f;

	bool initialized = false;
};

SpawnerState* GetState(Caller* instance)
{
	Node2D* self = GetSelf<Node2D>(instance);

	if (!self || !self->has_meta("state"))
		return nullptr;

	return (SpawnerState*)(int64_t)
		self->get_meta("state");
}

void SpawnFruit(SpawnerState* state)
{
	if (!state)
		return;

	if (!state->fruitScene.is_valid())
		return;

	Node2D* fruit =
		Object::cast_to<Node2D>(
			state->fruitScene->instantiate()
		);

	if (!fruit)
		return;

	//------------------------------------------------
	// RANDOM X POSITION
	//------------------------------------------------

	float margin = 50.0f;

	float usableWidth =
		state->windowWidth -
		(margin * 2.0f);

	float randomX =
		margin +
		(float)(rand() % (int)usableWidth);

	//------------------------------------------------

	fruit->set_position(
		Vector2(randomX, -100)
	);

	state->self->add_child(fruit);
}

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	Node2D* self = GetSelf<Node2D>(instance);

	SpawnerState* state =
		new SpawnerState();

	state->self = self;

	srand((unsigned int)time(nullptr));

	self->set_meta(
		"state",
		(int64_t)state
	);
}

void OnReady(Caller* instance)
{
	SpawnerState* state =
		GetState(instance);

	if (!state)
		return;

	//------------------------------------------------
	// LOAD FRUIT SCENE
	//------------------------------------------------

	state->fruitScene =
		ResourceLoader::get_singleton()
		->load("res://LUCKYDAY.tscn");

	//------------------------------------------------
	// GET WINDOW WIDTH
	//------------------------------------------------

	SceneTree* tree =
		state->self->get_tree();

	if (tree)
	{
		Window* root =
			tree->get_root();

		if (root)
		{
			state->windowWidth =
				(float)root->get_size().x;
		}
	}

	state->initialized = true;

	UtilityFunctions::print(
		"Spawner Ready"
	);
}

void OnDestroy(Caller* instance)
{
	Node2D* self =
		GetSelf<Node2D>(instance);

	if (!self || !self->has_meta("state"))
		return;

	SpawnerState* state =
		(SpawnerState*)(int64_t)
		self->get_meta("state");

	delete state;

	self->remove_meta("state");
}

void OnProcess(Caller* instance, double _delta)
{
	SpawnerState* state =
		GetState(instance);

	if (!state)
		return;

	if (!state->initialized)
		return;

	//------------------------------------------------
	// TIMER
	//------------------------------------------------

	state->timer += (float)_delta;

	if (state->timer >= state->interval)
	{
		SpawnFruit(state);

		state->timer = 0.0f;
	}
}

JENOVA_SCRIPT_END
