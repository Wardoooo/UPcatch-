/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>
#include <Godot/classes/label.hpp>
#include <Godot/classes/sprite2d.hpp>
#include <Godot/classes/window.hpp>
using namespace godot;
using namespace jenova::sdk;

static Node2D* charselect_self = nullptr;

JENOVA_SCRIPT_BEGIN

const char* charselect_paths[] = {
	"res://axelreal.tscn",
	"res://neiljan.tscn",
	"res://neiljanreal.tscn",
	"res://realme.tscn",
	"res://edward.tscn",
	"res://junereal.tscn",
	"res://edward_2.tscn",
    "res://edwardreal_3.tscn"
};

const char* charselect_names[] = {
	"Axel",
	"Neil Jan",
	"Neil Jan Real",
	"Real Me",
	"Edward",
	"June",
	"Edward 2",
    "Edward 3"
};

const int charselect_count = sizeof(charselect_paths) / sizeof(charselect_paths[0]);
int   charselect_index    = 0;

Label*    charselect_name_label  = nullptr;
Sprite2D* charselect_sprites[8]  = {};

void charselect_update_preview()
{
	for (int i = 0; i < charselect_count; i++)
		if (charselect_sprites[i])
			charselect_sprites[i]->set_visible(i == charselect_index);
	if (charselect_name_label)
		charselect_name_label->set_text(charselect_names[charselect_index]);
}

void OnAwake(Caller* instance)
{
	charselect_self  = GetSelf<Node2D>(instance);
	charselect_index = 0;
}

void OnReady(Caller* instance)
{
	charselect_sprites[0] = charselect_self->get_node<Sprite2D>("CanvasLayer/Preview_axelreal");
	charselect_sprites[1] = charselect_self->get_node<Sprite2D>("CanvasLayer/Preview_neiljan");
	charselect_sprites[2] = charselect_self->get_node<Sprite2D>("CanvasLayer/Preview_neiljanreal");
	charselect_sprites[3] = charselect_self->get_node<Sprite2D>("CanvasLayer/Preview_realme");
	charselect_sprites[4] = charselect_self->get_node<Sprite2D>("CanvasLayer/Preview_edward");
	charselect_sprites[5] = charselect_self->get_node<Sprite2D>("CanvasLayer/Preview_junereal");
	charselect_sprites[6] = charselect_self->get_node<Sprite2D>("CanvasLayer/Preview_edward_2");
	charselect_sprites[7] = charselect_self->get_node<Sprite2D>("CanvasLayer/Preview_edwardreal_3");
	charselect_name_label = charselect_self->get_node<Label>("CanvasLayer/NameLabel");
	charselect_update_preview();
}

void OnDestroy(Caller* instance)
{
	charselect_self = nullptr;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!charselect_self) return;

	Input* input = Input::get_singleton();

	bool left    = input->is_action_just_pressed("ui_left");
	bool right   = input->is_action_just_pressed("ui_right");
	bool confirm = input->is_action_just_pressed("ui_accept");

	if (left)
	{
		charselect_index--;
		if (charselect_index < 0) charselect_index = charselect_count - 1;
		charselect_update_preview();
	}

	if (right)
	{
		charselect_index++;
		if (charselect_index >= charselect_count) charselect_index = 0;
		charselect_update_preview();
	}

	if (confirm)
	{
		SceneTree* tree = charselect_self->get_tree();
		if (!tree)
		{
			UtilityFunctions::print("ERROR: tree is null!");
			return;
		}

		Node* game_state = tree->get_root()->get_node<Node>("GameState");
		if (!game_state)
		{
			UtilityFunctions::print("ERROR: GameState not found at /root/GameState!");
			return;
		}

		String level = game_state->get("selected_level_scene");
		UtilityFunctions::print("Level to load: ", level);

		if (level == String())
		{
			UtilityFunctions::print("ERROR: selected_level_scene is empty!");
			return;
		}

		game_state->set("selected_character_scene", String(charselect_paths[charselect_index]));
		UtilityFunctions::print("Character set to: ", charselect_paths[charselect_index]);

		tree->change_scene_to_file(level);
	}
}

JENOVA_SCRIPT_END
