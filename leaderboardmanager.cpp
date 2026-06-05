/* Jenova C++ Node Base Script (Meteora) */

#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/label.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/v_box_container.hpp>
#include <Godot/classes/file_access.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>
#include <Godot/classes/json.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;


const int LB_MAX_ENTRIES = 35;
const char* LB_SAVE_PATH = "user://leaderboard.json"; 

const char* LB_LEVEL_KEYS[] = {
	"LEVEL1", "LEVEL2", "LEVEL3", "LEVEL4", "bosslevel"
};
const char* LB_LEVEL_LABELS[] = {
	"LEVEL 1", "LEVEL 2", "LEVEL 3", "LEVEL 4", "BOSS LEVEL"
};
const int LB_LEVEL_COUNT = 5;


Node*   lb_self     = nullptr;
Button* lb_back_btn = nullptr;

VBoxContainer* lb_columns[5]; 

JENOVA_SCRIPT_BEGIN



Dictionary LoadRaw()
{
	String path = String(LB_SAVE_PATH);

	if (!FileAccess::file_exists(path))
		return Dictionary();

	Ref<FileAccess> f = FileAccess::open(path, FileAccess::READ);
	if (!f.is_valid()) return Dictionary();

	String text = f->get_as_text();
	f->close();

	Variant parsed = JSON::parse_string(text);
	if (parsed.get_type() == Variant::DICTIONARY)
		return parsed;

	return Dictionary();
}

void SaveRaw(Dictionary data)
{
	String path = String(LB_SAVE_PATH);

	Ref<FileAccess> f = FileAccess::open(path, FileAccess::WRITE);
	if (!f.is_valid())
	{
		UtilityFunctions::print("[Leaderboard] ERROR: Cannot open save file for writing!");
		return;
	}
	f->store_string(JSON::stringify(data, "  "));
	f->close();
	UtilityFunctions::print("[Leaderboard] Saved to ", path);
}



void PopulateColumn(int col_index, Array entries)
{
	VBoxContainer* col = lb_columns[col_index];
	if (!col) return;

	TypedArray<Node> children = col->get_children();
	for (int i = children.size() - 1; i >= 1; i--)
	{
		Node* child = Object::cast_to<Node>(children[i]);
		if (child) child->queue_free();
	}

	for (int rank = 0; rank < LB_MAX_ENTRIES; rank++)
	{
		Label* row = memnew(Label);
		row->add_theme_font_size_override("font_size", 12);

		if (rank < entries.size())
		{
			Dictionary e  = entries[rank];
			String name   = e["name"];
			int64_t score = e["score"];
			double  acc   = e["accuracy"];

			row->set_text(
				String::num_int64(rank + 1) + ". " +
				name + "  " +
				String::num_int64(score) + "  (" +
				String::num_real((float)acc, false) + "%)"
			);
		}
		else
		{
			row->set_text(String::num_int64(rank + 1) + ". ---");
		}

		col->add_child(row);
	}
}

void RefreshDisplay()
{
	Dictionary data = LoadRaw();

	for (int i = 0; i < LB_LEVEL_COUNT; i++)
	{
		Array entries;
		String key = String(LB_LEVEL_KEYS[i]);
		if (data.has(key))
			entries = data[key];
		PopulateColumn(i, entries);
	}
}



void OnAwake(Caller* instance)
{
	lb_self     = GetSelf<Node>(instance);
	lb_back_btn = nullptr;

	
	for (int i = 0; i < 5; i++)
		lb_columns[i] = nullptr;
}

void OnReady(Caller* instance)
{
	lb_columns[0] = lb_self->get_node<VBoxContainer>("LeaderboardColumns/Level1Col");
	lb_columns[1] = lb_self->get_node<VBoxContainer>("LeaderboardColumns/Level2Col");
	lb_columns[2] = lb_self->get_node<VBoxContainer>("LeaderboardColumns/Level3Col");
	lb_columns[3] = lb_self->get_node<VBoxContainer>("LeaderboardColumns/Level4Col");
	lb_columns[4] = lb_self->get_node<VBoxContainer>("LeaderboardColumns/Level5Col"); 

	for (int i = 0; i < LB_LEVEL_COUNT; i++)
	{
		if (!lb_columns[i])
			UtilityFunctions::print("[Leaderboard] WARNING: Column ", i, " not found!");
	}

	lb_back_btn = lb_self->get_node<Button>("Button"); 
	if (lb_back_btn)
		lb_back_btn->connect("pressed", Callable(lb_self, "OnBackPressed"));
	else
		UtilityFunctions::print("[Leaderboard] WARNING: BackButton not found!");

	RefreshDisplay();
}

void OnDestroy(Caller* instance)
{
	lb_self = nullptr;
}

void OnBackPressed(Caller* instance)
{
	SceneTree* tree = Object::cast_to<SceneTree>(
		Engine::get_singleton()->get_main_loop()
	);
	if (tree) tree->change_scene_to_file("res://menu.tscn");
}

void OnProcess(Caller* instance, double _delta) {}

JENOVA_SCRIPT_END
