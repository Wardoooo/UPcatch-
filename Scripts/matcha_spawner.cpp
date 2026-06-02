#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/random_number_generator.hpp>

using namespace godot;
using namespace jenova::sdk;

struct SpawnState
{
	float bpm          = 160.0f;
	float beatInterval = 60.0f / 160.0f;
	float timer        = 0.0f;
	float spawnXMin    = 64.0f;
	float spawnXMax    = 576.0f;
	float spawnY       = -32.0f;
	Ref<PackedScene> matchaScene;
	RandomNumberGenerator* rng = nullptr;
};

SpawnState* g_state = nullptr;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	UtilityFunctions::print("MatchaSpawner OnAwake called");

	g_state = new SpawnState();

	g_state->beatInterval = 60.0f / g_state->bpm;
	g_state->timer        = g_state->beatInterval;

	g_state->matchaScene =
		ResourceLoader::get_singleton()
		->load("res://fruit_matcha.tscn");

	if (!g_state->matchaScene.is_valid())
	{
		UtilityFunctions::printerr(
			"MatchaSpawner: failed to load res://fruit_matcha.tscn"
		);
	}
	else
	{
		UtilityFunctions::print("MatchaSpawner: scene loaded OK");
	}

	g_state->rng = memnew(RandomNumberGenerator);
	g_state->rng->randomize();
}

void OnDestroy(Caller* instance)
{
	if (g_state)
	{
		if (g_state->rng)
			memdelete(g_state->rng);

		delete g_state;

		g_state = nullptr;
	}
}

void OnProcess(Caller* instance, double _delta)
{
	if (!g_state || !g_state->matchaScene.is_valid())
		return;

	g_state->timer -= (float)_delta;

	if (g_state->timer > 0.0f)
		return;

	g_state->timer += g_state->beatInterval;

	Node2D* self = GetSelf<Node2D>(instance);

	if (!self)
		return;

	Node* node = g_state->matchaScene->instantiate();

	if (!node)
	{
		UtilityFunctions::printerr("instantiate() returned null");
		return;
	}

	Area2D* matcha = Object::cast_to<Area2D>(node);

	if (!matcha)
	{
		node->queue_free();
		return;
	}

	float x = g_state->rng->randf_range(
		g_state->spawnXMin,
		g_state->spawnXMax
	);

	matcha->set_position(Vector2(x, g_state->spawnY));

	Node* fruits = self->get_parent()
		->get_node<Node>(NodePath("Fruits"));

	if (fruits)
		fruits->add_child(matcha);
	else
		self->get_parent()->add_child(matcha);

	UtilityFunctions::print("Matcha spawned at x=", x);
}

JENOVA_SCRIPT_END
