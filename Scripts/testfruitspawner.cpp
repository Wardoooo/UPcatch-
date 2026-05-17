/* Jenova osu!catch Fruit Spawner */
#include <Godot/godot.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/window.hpp>

using namespace godot;
using namespace jenova::sdk;

Node2D* spawner_self = nullptr;
Ref<PackedScene> fruit_scene;
float timer = 0.0f;
float interval = 1.2f;
float window_width = 1280.0f; // fallback default

void SpawnFruit()
{
	Node2D* fruit =
		Object::cast_to<Node2D>(
			fruit_scene->instantiate()
		);
	if (fruit == nullptr)
		return;

	// Random X across entire window width with small margin
	float margin = 50.0f;
	float x = margin + (float)(rand() % (int)(window_width - margin * 2));

	fruit->set_position(Vector2(x, -100));
	spawner_self->add_child(fruit);
}

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	spawner_self = GetSelf<Node2D>(instance);
	srand(time(nullptr));
}

void OnDestroy(Caller* instance)
{
	spawner_self = nullptr;
}

void OnReady(Caller* instance)
{
	fruit_scene = ResourceLoader::get_singleton()->load("res://Fruit.tscn");

	// Get actual window width at runtime
	SceneTree* tree = spawner_self->get_tree();
	if (tree)
	{
		Window* root = tree->get_root();
		if (root)
			window_width = (float)root->get_size().x;
	}
}

void OnProcess(Caller* instance, double _delta)
{
	timer += _delta;
	if (timer >= interval)
	{
		SpawnFruit();
		timer = 0.0f;
	}
}

JENOVA_SCRIPT_END
