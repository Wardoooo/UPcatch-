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


Label* FirstScene_score_label    = nullptr;
Label* FirstScene_combo_label    = nullptr;
Label* FirstScene_accuracy_label = nullptr;


Control*  FirstScene_end_screen           = nullptr;
Label*    FirstScene_final_score_label    = nullptr;
Label*    FirstScene_final_accuracy_label = nullptr;
LineEdit* FirstScene_name_input           = nullptr;
Button*   FirstScene_submit_btn           = nullptr;


unsigned long long FirstScene_score      = 0ULL;
int                FirstScene_multiplier = 0;
int                FirstScene_caught     = 0;
int                FirstScene_total      = 0;

unsigned long long FirstScene_last_score      = ULLONG_MAX;
int                FirstScene_last_multiplier = -1;
float              FirstScene_last_accuracy   = -1.0f;

Node2D* FirstScene_self           = nullptr;
float   FirstScene_timer          = 0.0f;
float   FirstScene_spawn_interval = 0.0f;
bool    FirstScene_initialized    = false;
bool    FirstScene_game_ended     = false;

BPMPresets_Normal FirstScene_bpm_presets;
float FirstScene_current_bpm = 130.0f;
float FirstScene_fall_speed  = 400.0f;

float FirstScene_song_timer        = 0.0f;
const float FirstScene_song_length = 70.0f;

RandomNumberGenerator* FirstScene_rng = nullptr;

const char* FirstScene_fruit_paths[] = {
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
const int FirstScene_fruit_path_count = sizeof(FirstScene_fruit_paths) / sizeof(FirstScene_fruit_paths[0]);

Ref<PackedScene> FirstScene_packed_scenes[sizeof(FirstScene_fruit_paths) / sizeof(FirstScene_fruit_paths[0])];

Node2D* FirstScene_active[256];       
bool    FirstScene_pending_free[256]; 
int     FirstScene_active_count = 0;

const float FirstScene_left_bound  = -1000.0f;
const float FirstScene_right_bound =   100.0f;
const float FirstScene_top_spawn   =  -300.0f;
const float FirstScene_bottom_kill =   691.0f;

} 

JENOVA_SCRIPT_BEGIN



void SaveScore(String player_name, unsigned long long score, float accuracy)
{
	String save_path = "user://leaderboard.json";
	String level_key = "LEVEL1"; 

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
	if (!FirstScene_end_screen) return;

	if (FirstScene_final_score_label)
		FirstScene_final_score_label->set_text("Score: " + String::num_uint64(FirstScene_score));

	float acc = (FirstScene_total > 0 ? (float)FirstScene_caught / (float)FirstScene_total * 100.0f : 100.0f);
	if (FirstScene_final_accuracy_label)
		FirstScene_final_accuracy_label->set_text("Accuracy: " + String::num_real(acc, false) + "%");

	FirstScene_end_screen->set_visible(true);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree) tree->set_pause(true);
}

void OnSubmitPressed(Caller* instance)
{
	if (!FirstScene_name_input) return;

	String player_name = FirstScene_name_input->get_text().strip_edges();
	if (player_name.is_empty()) player_name = "Anonymous";

	float acc = (FirstScene_total > 0 ? (float)FirstScene_caught / (float)FirstScene_total * 100.0f : 100.0f);
	SaveScore(player_name, FirstScene_score, acc);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree)
	{
		tree->set_pause(false);
		tree->change_scene_to_file("res://menu.tscn");
	}
}



void OnAwake(Caller* instance)
{
	FirstScene_self = GetSelf<Node2D>(instance);
	FirstScene_self->set_process(true);
	FirstScene_self->set_process_mode(Node::PROCESS_MODE_PAUSABLE);

	FirstScene_score        = 0ULL;
	FirstScene_multiplier   = 0;
	FirstScene_caught       = 0;
	FirstScene_total        = 0;
	FirstScene_timer        = 0.0f;
	FirstScene_song_timer   = 0.0f;
	FirstScene_active_count = 0;
	FirstScene_initialized  = false;
	FirstScene_game_ended   = false;

	FirstScene_last_score      = ULLONG_MAX;
	FirstScene_last_multiplier = -1;
	FirstScene_last_accuracy   = -1.0f;

	for (int i = 0; i < 256; i++)
	{
		FirstScene_active[i]       = nullptr;
		FirstScene_pending_free[i] = false;
	}

	FirstScene_rng = memnew(RandomNumberGenerator);
	FirstScene_rng->randomize();
	FirstScene_spawn_interval = 60.0f / FirstScene_current_bpm;
}

void OnReady(Caller* instance)
{
	for (int i = 0; i < FirstScene_fruit_path_count; i++)
	{
		FirstScene_packed_scenes[i] = ResourceLoader::get_singleton()->load(FirstScene_fruit_paths[i]);
		if (FirstScene_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", FirstScene_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", FirstScene_fruit_paths[i]);
	}
	FirstScene_initialized = true;

	FirstScene_score_label    = FirstScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Score");
	FirstScene_combo_label    = FirstScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Combo");
	FirstScene_accuracy_label = FirstScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Accuracy");

	FirstScene_end_screen           = FirstScene_self->get_node<Control>("../CanvasLayer/EndScreen");
	FirstScene_final_score_label    = FirstScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalScore");
	FirstScene_final_accuracy_label = FirstScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalAccuracy");
	FirstScene_name_input           = FirstScene_self->get_node<LineEdit>("../CanvasLayer/EndScreen/NameInput");
	FirstScene_submit_btn           = FirstScene_self->get_node<Button>("../CanvasLayer/EndScreen/SubmitButton");

	if (FirstScene_end_screen)
	{
		FirstScene_end_screen->set_visible(false);
		FirstScene_end_screen->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	}
	else
		UtilityFunctions::print("[EndScreen] ERROR: EndScreen not found!");

	if (FirstScene_submit_btn)
		FirstScene_submit_btn->connect("pressed", Callable(FirstScene_self, "OnSubmitPressed"));
	else
		UtilityFunctions::print("[EndScreen] WARNING: SubmitButton not found!");
}

void OnDestroy(Caller* instance)
{
	if (FirstScene_rng) memdelete(FirstScene_rng);
	FirstScene_rng  = nullptr;
	FirstScene_self = nullptr;
}

void RemoveActive(int i)
{
	int last = FirstScene_active_count - 1;
	FirstScene_active[i]       = FirstScene_active[last];
	FirstScene_pending_free[i] = FirstScene_pending_free[last];
	FirstScene_active[last]       = nullptr;
	FirstScene_pending_free[last] = false;
	FirstScene_active_count--;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!FirstScene_self || !FirstScene_initialized) return;
	if (FirstScene_game_ended) return;

	for (int i = FirstScene_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = FirstScene_active[i];

		if (!fruit || !fruit->is_inside_tree())
		{
			FirstScene_total++;
			FirstScene_caught++;
			FirstScene_multiplier++;
			FirstScene_score += 300ULL * (unsigned long long)FirstScene_multiplier;
			RemoveActive(i);
			continue;
		}

		Vector2 pos = fruit->get_position();
		pos.y += FirstScene_fall_speed * (float)_delta;
		fruit->set_position(pos);

		if (pos.y > FirstScene_bottom_kill)
		{
			FirstScene_total++;
			FirstScene_multiplier = 0;
			fruit->queue_free();
			RemoveActive(i);
		}
	}

	if (FirstScene_song_timer < FirstScene_song_length)
	{
		FirstScene_song_timer += (float)_delta;
		FirstScene_timer      += (float)_delta;

		if (FirstScene_timer >= FirstScene_spawn_interval)
		{
			FirstScene_timer = 0.0f;

			int pick = FirstScene_rng->randi_range(0, FirstScene_fruit_path_count - 1);
			Ref<PackedScene> scene = FirstScene_packed_scenes[pick];
			if (!scene.is_valid()) return;

			Node* fruit_instance = scene->instantiate();
			if (!fruit_instance) return;

			FirstScene_self->add_child(fruit_instance);

			Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
			if (fruit_node && FirstScene_active_count < 256)
			{
				float random_x = FirstScene_rng->randf_range(FirstScene_left_bound, FirstScene_right_bound);
				fruit_node->set_position(Vector2(random_x, FirstScene_top_spawn));
				fruit_node->set_visible(true);
				FirstScene_active[FirstScene_active_count]       = fruit_node;
				FirstScene_pending_free[FirstScene_active_count] = false;
				FirstScene_active_count++;
			}
		}
	}
	else if (FirstScene_active_count == 0)
	{
		FirstScene_game_ended = true;
		ShowEndScreen();
	}

	if (FirstScene_score_label && FirstScene_score != FirstScene_last_score)
	{
		FirstScene_score_label->set_text("Score: " + String::num_uint64(FirstScene_score));
		FirstScene_last_score = FirstScene_score;
	}
	if (FirstScene_combo_label && FirstScene_multiplier != FirstScene_last_multiplier)
	{
		FirstScene_combo_label->set_text("x" + String::num_int64(FirstScene_multiplier));
		FirstScene_last_multiplier = FirstScene_multiplier;
	}
	if (FirstScene_accuracy_label)
	{
		float acc = (FirstScene_total > 0 ? (float)FirstScene_caught / (float)FirstScene_total * 100.0f : 100.0f);
		if (acc != FirstScene_last_accuracy)
		{
			FirstScene_accuracy_label->set_text("Acc: " + String::num_real(acc, false) + "%");
			FirstScene_last_accuracy = acc;
		}
	}
}

JENOVA_SCRIPT_END
