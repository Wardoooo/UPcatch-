/* Jenova C++ Node Base Script (Meteora) */
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

struct BPMPresets_Normal {
	float bpm_100 = 100.0f; float bpm_110 = 110.0f; float bpm_120 = 120.0f;
	float bpm_130 = 130.0f; float bpm_140 = 140.0f; float bpm_150 = 150.0f;
	float bpm_160 = 160.0f; float bpm_170 = 170.0f; float bpm_180 = 180.0f;
	float bpm_190 = 190.0f; float bpm_200 = 200.0f; float bpm_210 = 210.0f;
	float bpm_220 = 220.0f; float bpm_230 = 230.0f; float bpm_240 = 240.0f;
	float bpm_250 = 250.0f; float bpm_260 = 260.0f; float bpm_270 = 270.0f;
	float bpm_280 = 280.0f; float bpm_290 = 290.0f; float bpm_300 = 300.0f;
};


Label* SecondScene_score_label    = nullptr;
Label* SecondScene_combo_label    = nullptr;
Label* SecondScene_accuracy_label = nullptr;


Control*  SecondScene_end_screen           = nullptr;
Label*    SecondScene_final_score_label    = nullptr;
Label*    SecondScene_final_accuracy_label = nullptr;
LineEdit* SecondScene_name_input           = nullptr;
Button*   SecondScene_submit_btn           = nullptr;


unsigned long long SecondScene_score      = 0ULL;
int                SecondScene_multiplier = 0;
int                SecondScene_caught     = 0;
int                SecondScene_total      = 0;

unsigned long long SecondScene_last_score      = ULLONG_MAX;
int                SecondScene_last_multiplier = -1;
float              SecondScene_last_accuracy   = -1.0f;

Node2D* SecondScene_self           = nullptr;
float   SecondScene_timer          = 0.0f;
float   SecondScene_spawn_interval = 0.0f;
bool    SecondScene_initialized    = false;
bool    SecondScene_game_ended     = false;

BPMPresets_Normal SecondScene_bpm_presets;
float SecondScene_current_bpm = 130.0f;
float SecondScene_fall_speed  = 400.0f;

float SecondScene_song_timer        = 0.0f;
const float SecondScene_song_length = 214.0f;

RandomNumberGenerator* SecondScene_rng = nullptr;

const char* SecondScene_fruit_paths[] = {
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
const int SecondScene_fruit_path_count = sizeof(SecondScene_fruit_paths) / sizeof(SecondScene_fruit_paths[0]);

Ref<PackedScene> SecondScene_packed_scenes[sizeof(SecondScene_fruit_paths) / sizeof(SecondScene_fruit_paths[0])];

Node2D* SecondScene_active[256];       
bool    SecondScene_pending_free[256]; 
int     SecondScene_active_count = 0;

const float SecondScene_left_bound  = -1000.0f;
const float SecondScene_right_bound =   100.0f;
const float SecondScene_top_spawn   =  -300.0f;
const float SecondScene_bottom_kill =   691.0f;

} 

JENOVA_SCRIPT_BEGIN



void SaveScore(String player_name, unsigned long long score, float accuracy)
{
	String save_path = "user://leaderboard.json";
	String level_key = "LEVEL2"; 

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
		UtilityFunctions::print("[Spawner] Score saved for: ", player_name);
	}
	else
	{
		UtilityFunctions::print("[Spawner] ERROR: Could not write leaderboard file!");
	}
}



void ShowEndScreen()
{
	if (!SecondScene_end_screen) return;

	if (SecondScene_final_score_label)
		SecondScene_final_score_label->set_text("Score: " + String::num_uint64(SecondScene_score));

	float acc = (SecondScene_total > 0 ? (float)SecondScene_caught / (float)SecondScene_total * 100.0f : 100.0f);
	if (SecondScene_final_accuracy_label)
		SecondScene_final_accuracy_label->set_text("Accuracy: " + String::num_real(acc, false) + "%");

	SecondScene_end_screen->set_visible(true);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree) tree->set_pause(true);
}

void OnSubmitPressed(Caller* instance)
{
	if (!SecondScene_name_input) return;

	String player_name = SecondScene_name_input->get_text().strip_edges();
	if (player_name.is_empty()) player_name = "Anonymous";

	float acc = (SecondScene_total > 0 ? (float)SecondScene_caught / (float)SecondScene_total * 100.0f : 100.0f);
	SaveScore(player_name, SecondScene_score, acc);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree)
	{
		tree->set_pause(false);
		tree->change_scene_to_file("res://menu.tscn");
	}
}



void OnAwake(Caller* instance)
{
	SecondScene_self = GetSelf<Node2D>(instance);
	SecondScene_self->set_process(true);
	SecondScene_self->set_process_mode(Node::PROCESS_MODE_PAUSABLE);

	SecondScene_score        = 0ULL;
	SecondScene_multiplier   = 0;
	SecondScene_caught       = 0;
	SecondScene_total        = 0;
	SecondScene_timer        = 0.0f;
	SecondScene_song_timer   = 0.0f;
	SecondScene_active_count = 0;
	SecondScene_initialized  = false;
	SecondScene_game_ended   = false;

	SecondScene_last_score      = ULLONG_MAX;
	SecondScene_last_multiplier = -1;
	SecondScene_last_accuracy   = -1.0f;

	for (int i = 0; i < 256; i++)
	{
		SecondScene_active[i]       = nullptr;
		SecondScene_pending_free[i] = false;
	}

	SecondScene_rng = memnew(RandomNumberGenerator);
	SecondScene_rng->randomize();
	SecondScene_spawn_interval = 60.0f / SecondScene_current_bpm;
}

void OnReady(Caller* instance)
{
	for (int i = 0; i < SecondScene_fruit_path_count; i++)
	{
		SecondScene_packed_scenes[i] = ResourceLoader::get_singleton()->load(SecondScene_fruit_paths[i]);
		if (SecondScene_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", SecondScene_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", SecondScene_fruit_paths[i]);
	}
	SecondScene_initialized = true;

	SecondScene_score_label    = SecondScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Score");
	SecondScene_combo_label    = SecondScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Combo");
	SecondScene_accuracy_label = SecondScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Accuracy");

	SecondScene_end_screen           = SecondScene_self->get_node<Control>("../CanvasLayer/EndScreen");
	SecondScene_final_score_label    = SecondScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalScore");
	SecondScene_final_accuracy_label = SecondScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalAccuracy");
	SecondScene_name_input           = SecondScene_self->get_node<LineEdit>("../CanvasLayer/EndScreen/NameInput");
	SecondScene_submit_btn           = SecondScene_self->get_node<Button>("../CanvasLayer/EndScreen/SubmitButton");

	if (SecondScene_end_screen)
	{
		SecondScene_end_screen->set_visible(false);
		SecondScene_end_screen->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	}
	else
		UtilityFunctions::print("[EndScreen] ERROR: EndScreen not found!");

	if (SecondScene_submit_btn)
		SecondScene_submit_btn->connect("pressed", Callable(SecondScene_self, "OnSubmitPressed"));
	else
		UtilityFunctions::print("[EndScreen] WARNING: SubmitButton not found!");
}

void OnDestroy(Caller* instance)
{
	if (SecondScene_rng) memdelete(SecondScene_rng);
	SecondScene_rng  = nullptr;
	SecondScene_self = nullptr;
}

void RemoveActive(int i)
{
	int last = SecondScene_active_count - 1;
	SecondScene_active[i]       = SecondScene_active[last];
	SecondScene_pending_free[i] = SecondScene_pending_free[last];
	SecondScene_active[last]       = nullptr;
	SecondScene_pending_free[last] = false;
	SecondScene_active_count--;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!SecondScene_self || !SecondScene_initialized) return;
	if (SecondScene_game_ended) return;

	for (int i = SecondScene_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = SecondScene_active[i];

		if (!fruit || !fruit->is_inside_tree())
		{
			SecondScene_total++;
			SecondScene_caught++;
			SecondScene_multiplier++;
			SecondScene_score += 300ULL * (unsigned long long)SecondScene_multiplier;
			RemoveActive(i);
			continue;
		}

		Vector2 pos = fruit->get_position();
		pos.y += SecondScene_fall_speed * (float)_delta;
		fruit->set_position(pos);

		if (pos.y > SecondScene_bottom_kill)
		{
			SecondScene_total++;
			SecondScene_multiplier = 0;
			fruit->queue_free();
			RemoveActive(i);
		}
	}

	if (SecondScene_song_timer < SecondScene_song_length)
	{
		SecondScene_song_timer += (float)_delta;
		SecondScene_timer      += (float)_delta;

		if (SecondScene_timer >= SecondScene_spawn_interval)
		{
			SecondScene_timer = 0.0f;

			int pick = SecondScene_rng->randi_range(0, SecondScene_fruit_path_count - 1);
			Ref<PackedScene> scene = SecondScene_packed_scenes[pick];
			if (!scene.is_valid()) return;

			Node* fruit_instance = scene->instantiate();
			if (!fruit_instance) return;

			SecondScene_self->add_child(fruit_instance);

			Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
			if (fruit_node && SecondScene_active_count < 256)
			{
				float random_x = SecondScene_rng->randf_range(SecondScene_left_bound, SecondScene_right_bound);
				fruit_node->set_position(Vector2(random_x, SecondScene_top_spawn));
				fruit_node->set_visible(true);
				SecondScene_active[SecondScene_active_count]       = fruit_node;
				SecondScene_pending_free[SecondScene_active_count] = false;
				SecondScene_active_count++;
			}
		}
	}
	else if (SecondScene_active_count == 0)
	{
		SecondScene_game_ended = true;
		ShowEndScreen();
	}

	if (SecondScene_score_label && SecondScene_score != SecondScene_last_score)
	{
		SecondScene_score_label->set_text("Score: " + String::num_uint64(SecondScene_score));
		SecondScene_last_score = SecondScene_score;
	}
	if (SecondScene_combo_label && SecondScene_multiplier != SecondScene_last_multiplier)
	{
		SecondScene_combo_label->set_text("x" + String::num_int64(SecondScene_multiplier));
		SecondScene_last_multiplier = SecondScene_multiplier;
	}
	if (SecondScene_accuracy_label)
	{
		float acc = (SecondScene_total > 0 ? (float)SecondScene_caught / (float)SecondScene_total * 100.0f : 100.0f);
		if (acc != SecondScene_last_accuracy)
		{
			SecondScene_accuracy_label->set_text("Acc: " + String::num_real(acc, false) + "%");
			SecondScene_last_accuracy = acc;
		}
	}
}

JENOVA_SCRIPT_END
