/* Jenova C++ Node Base Script (Meteora) - Level 4 Spawner */
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/random_number_generator.hpp>
#include <Godot/classes/label.hpp>
#include <Godot/classes/line_edit.hpp>
#include <Godot/classes/button.hpp>
#include <Godot/classes/control.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/engine.hpp>
#include <Godot/classes/file_access.hpp>
#include <Godot/classes/json.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;


namespace {


Label* SpecialScene_score_label    = nullptr;
Label* SpecialScene_combo_label    = nullptr;
Label* SpecialScene_accuracy_label = nullptr;


Control*  SpecialScene_end_screen           = nullptr;
Label*    SpecialScene_final_score_label    = nullptr;
Label*    SpecialScene_final_accuracy_label = nullptr;
LineEdit* SpecialScene_name_input           = nullptr;
Button*   SpecialScene_submit_btn           = nullptr;


unsigned long long SpecialScene_score      = 0ULL;
int                SpecialScene_multiplier = 0;
int                SpecialScene_caught     = 0;
int                SpecialScene_total      = 0;

unsigned long long SpecialScene_last_score      = ULLONG_MAX;
int                SpecialScene_last_multiplier = -1;
float              SpecialScene_last_accuracy   = -1.0f;

Node2D* SpecialScene_self           = nullptr;
float   SpecialScene_timer          = 0.0f;
float   SpecialScene_spawn_interval = 0.0f;
bool    SpecialScene_initialized    = false;
bool    SpecialScene_game_ended     = false;

float SpecialScene_current_bpm = 298.0f;
float SpecialScene_fall_speed  = 1000.0f;

float       SpecialScene_song_timer  = 0.0f;
const float SpecialScene_song_length = 126.0f;

RandomNumberGenerator* SpecialScene_rng = nullptr;

const char* SpecialScene_fruit_paths[] = {
	"res://24chicken.tscn",
	"res://malunggay.tscn",
	"res://vscode.tscn",
	"res://uplogo.tscn",
	"res://redbull.tscn",
	"res://LUCKYDAY.tscn",
	"res://fruit_matcha.tscn",
	"res://bath.tscn",
	"res://sixseven.tscn"
};
const int SpecialScene_fruit_path_count = sizeof(SpecialScene_fruit_paths) / sizeof(SpecialScene_fruit_paths[0]);

Ref<PackedScene> SpecialScene_packed_scenes[sizeof(SpecialScene_fruit_paths) / sizeof(SpecialScene_fruit_paths[0])];

struct SpecialSceneFruit {
	Node2D* node   = nullptr;
	bool    caught = false;
};

SpecialSceneFruit SpecialScene_active[256]  = {};
int      SpecialScene_active_count = 0;

const float SpecialScene_left_bound  = -1000.0f;
const float SpecialScene_right_bound =   450.0f;
const float SpecialScene_top_spawn   =  -300.0f;
const float SpecialScene_bottom_kill =   691.0f;

} 

JENOVA_SCRIPT_BEGIN



void SaveScore(String player_name, unsigned long long score, float accuracy)
{
	String save_path = "user://leaderboard.json";
	String level_key = "LEVEL4"; 

	Dictionary data;
	if (FileAccess::file_exists(save_path))
	{
		Ref<FileAccess> f = FileAccess::open(save_path, FileAccess::READ);
		if (f.is_valid())
		{
			Variant parsed = JSON::parse_string(f->get_as_text());
			f->close();
			if (parsed.get_type() == Variant::DICTIONARY)
				data = parsed;
		}
	}

	Array entries;
	if (data.has(level_key)) entries = data[level_key];

	Dictionary entry;
	entry["name"]     = player_name;
	entry["score"]    = (int64_t)score;
	entry["accuracy"] = (double)accuracy;
	entries.append(entry);

	for (int i = 1; i < entries.size(); i++)
	{
		Dictionary cur = entries[i];
		int j = i - 1;
		while (j >= 0 && (int64_t)((Dictionary)entries[j])["score"] < (int64_t)cur["score"])
		{
			entries[j + 1] = entries[j];
			j--;
		}
		entries[j + 1] = cur;
	}
	while (entries.size() > 10) entries.resize(10);

	data[level_key] = entries;

	Ref<FileAccess> f = FileAccess::open(save_path, FileAccess::WRITE);
	if (f.is_valid())
	{
		f->store_string(JSON::stringify(data, "  "));
		f->close();
		UtilityFunctions::print("[Spawner4] Score saved for: ", player_name);
	}
	else
	{
		UtilityFunctions::print("[Spawner4] ERROR: Could not write leaderboard file!");
	}
}



void ShowEndScreen()
{
	if (!SpecialScene_end_screen) return;

	if (SpecialScene_final_score_label)
		SpecialScene_final_score_label->set_text("Score: " + String::num_uint64(SpecialScene_score));

	float acc = (SpecialScene_total > 0 ? (float)SpecialScene_caught / (float)SpecialScene_total * 100.0f : 100.0f);
	if (SpecialScene_final_accuracy_label)
		SpecialScene_final_accuracy_label->set_text("Accuracy: " + String::num_real(acc, false) + "%");

	SpecialScene_end_screen->set_visible(true);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree) tree->set_pause(true);
}

void OnSubmitPressed(Caller* instance)
{
	if (!SpecialScene_name_input) return;

	String player_name = SpecialScene_name_input->get_text().strip_edges();
	if (player_name.is_empty()) player_name = "Anonymous";

	float acc = (SpecialScene_total > 0 ? (float)SpecialScene_caught / (float)SpecialScene_total * 100.0f : 100.0f);
	SaveScore(player_name, SpecialScene_score, acc);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree)
	{
		tree->set_pause(false);
		tree->change_scene_to_file("res://menu.tscn");
	}
}



void OnAwake(Caller* instance)
{
	SpecialScene_self = GetSelf<Node2D>(instance);
	SpecialScene_self->set_process(true);
	SpecialScene_self->set_process_mode(Node::PROCESS_MODE_PAUSABLE);

	SpecialScene_score        = 0ULL;
	SpecialScene_multiplier   = 0;
	SpecialScene_caught       = 0;
	SpecialScene_total        = 0;
	SpecialScene_timer        = 0.0f;
	SpecialScene_song_timer   = 0.0f;
	SpecialScene_active_count = 0;
	SpecialScene_initialized  = false;
	SpecialScene_game_ended   = false;

	SpecialScene_last_score      = ULLONG_MAX;
	SpecialScene_last_multiplier = -1;
	SpecialScene_last_accuracy   = -1.0f;

	SpecialScene_rng = memnew(RandomNumberGenerator);
	SpecialScene_rng->randomize();
	SpecialScene_spawn_interval = 60.0f / SpecialScene_current_bpm;
}

void OnReady(Caller* instance)
{
	for (int i = 0; i < SpecialScene_fruit_path_count; i++)
	{
		SpecialScene_packed_scenes[i] = ResourceLoader::get_singleton()->load(SpecialScene_fruit_paths[i]);
		if (SpecialScene_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", SpecialScene_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", SpecialScene_fruit_paths[i]);
	}
	SpecialScene_initialized = true;

	SpecialScene_score_label    = SpecialScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Score");
	SpecialScene_combo_label    = SpecialScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Combo");
	SpecialScene_accuracy_label = SpecialScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Accuracy");

	SpecialScene_end_screen           = SpecialScene_self->get_node<Control>("../CanvasLayer/EndScreen");
	SpecialScene_final_score_label    = SpecialScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalScore");
	SpecialScene_final_accuracy_label = SpecialScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalAccuracy");
	SpecialScene_name_input           = SpecialScene_self->get_node<LineEdit>("../CanvasLayer/EndScreen/NameInput");
	SpecialScene_submit_btn           = SpecialScene_self->get_node<Button>("../CanvasLayer/EndScreen/SubmitButton");

	if (SpecialScene_end_screen)
	{
		SpecialScene_end_screen->set_visible(false);
		SpecialScene_end_screen->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	}
	else
		UtilityFunctions::print("[EndScreen] ERROR: EndScreen not found!");

	if (SpecialScene_submit_btn)
		SpecialScene_submit_btn->connect("pressed", Callable(SpecialScene_self, "OnSubmitPressed"));
	else
		UtilityFunctions::print("[EndScreen] WARNING: SubmitButton not found!");
}

void OnDestroy(Caller* instance)
{
	if (SpecialScene_rng) memdelete(SpecialScene_rng);
	SpecialScene_rng  = nullptr;
	SpecialScene_self = nullptr;
}

void RemoveActive4(int i)
{
	int last = SpecialScene_active_count - 1;
	SpecialScene_active[i]    = SpecialScene_active[last];
	SpecialScene_active[last] = {};
	SpecialScene_active_count--;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!SpecialScene_self || !SpecialScene_initialized) return;
	if (SpecialScene_game_ended) return;

	for (int i = SpecialScene_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = SpecialScene_active[i].node;
		if (!fruit) { RemoveActive4(i); continue; }

		if (!fruit->is_inside_tree())
		{
			SpecialScene_total++;
			SpecialScene_caught++;
			SpecialScene_multiplier++;
			SpecialScene_score += 300ULL * (unsigned long long)SpecialScene_multiplier;
			RemoveActive4(i);
			continue;
		}

		Vector2 pos = fruit->get_position();
		pos.y += SpecialScene_fall_speed * (float)_delta;
		fruit->set_position(pos);

		if (pos.y > SpecialScene_bottom_kill)
		{
			SpecialScene_total++;
			SpecialScene_multiplier = 0;
			fruit->queue_free();
			RemoveActive4(i);
		}
	}

	if (SpecialScene_song_timer < SpecialScene_song_length)
	{
		SpecialScene_song_timer += (float)_delta;
		SpecialScene_timer      += (float)_delta;

		if (SpecialScene_timer >=SpecialScene_spawn_interval && SpecialScene_active_count < 256)
		{
			SpecialScene_timer = 0.0f;

			int pick = SpecialScene_rng->randi_range(0, SpecialScene_fruit_path_count - 1);
			Ref<PackedScene> scene = SpecialScene_packed_scenes[pick];
			if (!scene.is_valid()) return;

			Node* fruit_instance = scene->instantiate();
			if (!fruit_instance) return;

			SpecialScene_self->add_child(fruit_instance);

			Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
			if (fruit_node)
			{
				float random_x = SpecialScene_rng->randf_range(SpecialScene_left_bound, SpecialScene_right_bound);
				fruit_node->set_position(Vector2(random_x, SpecialScene_top_spawn));
				fruit_node->set_visible(true);
				SpecialScene_active[SpecialScene_active_count].node   = fruit_node;
				SpecialScene_active[SpecialScene_active_count].caught = false;
				SpecialScene_active_count++;
			}
		}
	}
	else if (SpecialScene_active_count == 0)
	{
		SpecialScene_game_ended = true;
		ShowEndScreen();
	}

	if (SpecialScene_score_label && SpecialScene_score != SpecialScene_last_score)
	{
		SpecialScene_score_label->set_text("Score: " + String::num_uint64(SpecialScene_score));
		SpecialScene_last_score = SpecialScene_score;
	}
	if (SpecialScene_combo_label && SpecialScene_multiplier != SpecialScene_last_multiplier)
	{
		SpecialScene_combo_label->set_text("x" + String::num_int64(SpecialScene_multiplier));
		SpecialScene_last_multiplier = SpecialScene_multiplier;
	}
	if (SpecialScene_accuracy_label)
	{
		float acc = (SpecialScene_total > 0 ? (float)SpecialScene_caught / (float)SpecialScene_total * 100.0f : 100.0f);
		if (acc != SpecialScene_last_accuracy)
		{
			SpecialScene_accuracy_label->set_text("Acc: " + String::num_real(acc, false) + "%");
			SpecialScene_last_accuracy = acc;
		}
	}
}

JENOVA_SCRIPT_END
