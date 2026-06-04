/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/engine.hpp>
#include <Godot/classes/window.hpp>
#include <Godot/variant/utility_functions.hpp>
using namespace godot;
using namespace jenova::sdk;

Node2D* charloader_self = nullptr;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	charloader_self = GetSelf<Node2D>(instance);
}

void OnReady(Caller* instance)
{
	UtilityFunctions::print("=== CharLoader OnReady fired ===");

	Node* game_state = charloader_self->get_node<Node>("/root/GameState");
	if (!game_state)
	{
		UtilityFunctions::print("CharLoader ERROR: GameState not found!");
		return;
	}

	String selected = game_state->get("selected_character_scene");
	UtilityFunctions::print("CharLoader: loading = ", selected);

	if (selected == String())
	{
		UtilityFunctions::print("CharLoader ERROR: No character selected!");
		return;
	}

	Ref<PackedScene> char_scene = ResourceLoader::get_singleton()->load(selected);
	if (!char_scene.is_valid())
	{
		UtilityFunctions::print("CharLoader ERROR: Could not load scene: ", selected);
		return;
	}

	Node2D* char_instance = Object::cast_to<Node2D>(char_scene->instantiate());
	if (!char_instance)
	{
		UtilityFunctions::print("CharLoader ERROR: Root is not a Node2D!");
		return;
	}

	Node* scene_root = charloader_self->get_tree()->get_current_scene();
	if (!scene_root)
	{
		UtilityFunctions::print("CharLoader ERROR: scene_root is null!");
		return;
	}

	// Add to scene root so physics world is shared with fruits
	scene_root->add_child(char_instance);

	// Use PlayerSpawn node position so we never hardcode coordinates
	Node2D* spawn_point = Object::cast_to<Node2D>(scene_root->get_node_or_null(NodePath("PlayerSpawn")));
	if (spawn_point)
	{
		Vector2 spawn_pos = spawn_point->get_global_position();
		char_instance->set_global_position(spawn_pos);
		UtilityFunctions::print("CharLoader: spawned at PlayerSpawn pos: ", spawn_pos.x, ", ", spawn_pos.y);
	}
	else
	{
		// Fallback: center of fruit spawn X range, near bottom of screen
		char_instance->set_global_position(Vector2(-655.0f, 400.0f));
		UtilityFunctions::print("CharLoader WARNING: PlayerSpawn not found, using fallback (-655, 400)");
	}

	// Force character visible
	char_instance->set_visible(true);

	// Tag all Area2Ds in the character as player_catcher so spawner can detect them
	TypedArray<Node> areas = char_instance->find_children("*", "Area2D", true, false);
	bool tagged = false;
	for (int i = 0; i < areas.size(); i++)
	{
		Area2D* area = Object::cast_to<Area2D>(areas[i]);
		if (area)
		{
			area->add_to_group("player_catcher");
			UtilityFunctions::print("CharLoader: tagged '", area->get_name(), "' as player_catcher");
			tagged = true;
		}
	}

	if (!tagged)
		UtilityFunctions::print("CharLoader WARNING: No Area2D found in character scene!");

	UtilityFunctions::print("=== CharLoader done ===");
	charloader_self->queue_free();
}

void OnDestroy(Caller* instance)
{
	charloader_self = nullptr;
}

void OnProcess(Caller* instance, double _delta) {}

JENOVA_SCRIPT_END
