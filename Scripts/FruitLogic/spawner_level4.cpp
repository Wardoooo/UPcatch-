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


Label* FourthScene_score_label    = nullptr;
Label* FourthScene_combo_label    = nullptr;
Label* FourthScene_accuracy_label = nullptr;


Control*  FourthScene_end_screen           = nullptr;
Label*    FourthScene_final_score_label    = nullptr;
Label*    FourthScene_final_accuracy_label = nullptr;
LineEdit* FourthScene_name_input           = nullptr;
Button*   FourthScene_submit_btn           = nullptr;


unsigned long long FourthScene_score      = 0ULL;
int                FourthScene_multiplier = 0;
int                FourthScene_caught     = 0;
int                FourthScene_total      = 0;

unsigned long long FourthScene_last_score      = ULLONG_MAX;
int                FourthScene_last_multiplier = -1;
float              FourthScene_last_accuracy   = -1.0f;

Node2D* FourthScene_self           = nullptr;
float   FourthScene_timer          = 0.0f;
float   FourthScene_spawn_interval = 0.0f;
bool    FourthScene_initialized    = false;
bool    FourthScene_game_ended     = false;

float FourthScene_current_bpm = 344.0f;
float FourthScene_fall_speed  = 1500.0f;

float       FourthScene_song_timer  = 0.0f;
const float FourthScene_song_length = 129.0f;

RandomNumberGenerator* FourthScene_rng = nullptr;

const char* FourthScene_fruit_paths[] = {
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
const int FourthScene_fruit_path_count = sizeof(FourthScene_fruit_paths) / sizeof(FourthScene_fruit_paths[0]);

Ref<PackedScene> FourthScene_packed_scenes[sizeof(FourthScene_fruit_paths) / sizeof(FourthScene_fruit_paths[0])];

struct FourthSceneFruit {
	Node2D* node   = nullptr;
	bool    caught = false;
};

FourthSceneFruit FourthScene_active[256]  = {};
int      FourthScene_active_count = 0;

const float FourthScene_left_bound  = -1000.0f;
const float FourthScene_right_bound =   450.0f;
const float FourthScene_top_spawn   =  -300.0f;
const float FourthScene_bottom_kill =   691.0f;

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
	if (!FourthScene_end_screen) return;

	if (FourthScene_final_score_label)
		FourthScene_final_score_label->set_text("Score: " + String::num_uint64(FourthScene_score));

	float acc = (FourthScene_total > 0 ? (float)FourthScene_caught / (float)FourthScene_total * 100.0f : 100.0f);
	if (FourthScene_final_accuracy_label)
		FourthScene_final_accuracy_label->set_text("Accuracy: " + String::num_real(acc, false) + "%");

	FourthScene_end_screen->set_visible(true);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree) tree->set_pause(true);
}

void OnSubmitPressed(Caller* instance)
{
	if (!FourthScene_name_input) return;

	String player_name = FourthScene_name_input->get_text().strip_edges();
	if (player_name.is_empty()) player_name = "Anonymous";

	float acc = (FourthScene_total > 0 ? (float)FourthScene_caught / (float)FourthScene_total * 100.0f : 100.0f);
	SaveScore(player_name, FourthScene_score, acc);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree)
	{
		tree->set_pause(false);
		tree->change_scene_to_file("res://menu.tscn");
	}
}



void OnAwake(Caller* instance)
{
	FourthScene_self = GetSelf<Node2D>(instance);
	FourthScene_self->set_process(true);
	FourthScene_self->set_process_mode(Node::PROCESS_MODE_PAUSABLE);

	FourthScene_score        = 0ULL;
	FourthScene_multiplier   = 0;
	FourthScene_caught       = 0;
	FourthScene_total        = 0;
	FourthScene_timer        = 0.0f;
	FourthScene_song_timer   = 0.0f;
	FourthScene_active_count = 0;
	FourthScene_initialized  = false;
	FourthScene_game_ended   = false;

	FourthScene_last_score      = ULLONG_MAX;
	FourthScene_last_multiplier = -1;
	FourthScene_last_accuracy   = -1.0f;

	FourthScene_rng = memnew(RandomNumberGenerator);
	FourthScene_rng->randomize();
	FourthScene_spawn_interval = 60.0f / FourthScene_current_bpm;
}

void OnReady(Caller* instance)
{
	for (int i = 0; i < FourthScene_fruit_path_count; i++)
	{
		FourthScene_packed_scenes[i] = ResourceLoader::get_singleton()->load(FourthScene_fruit_paths[i]);
		if (FourthScene_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", FourthScene_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", FourthScene_fruit_paths[i]);
	}
	FourthScene_initialized = true;

	FourthScene_score_label    = FourthScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Score");
	FourthScene_combo_label    = FourthScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Combo");
	FourthScene_accuracy_label = FourthScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Accuracy");

	FourthScene_end_screen           = FourthScene_self->get_node<Control>("../CanvasLayer/EndScreen");
	FourthScene_final_score_label    = FourthScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalScore");
	FourthScene_final_accuracy_label = FourthScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalAccuracy");
	FourthScene_name_input           = FourthScene_self->get_node<LineEdit>("../CanvasLayer/EndScreen/NameInput");
	FourthScene_submit_btn           = FourthScene_self->get_node<Button>("../CanvasLayer/EndScreen/SubmitButton");

	if (FourthScene_end_screen)
	{
		FourthScene_end_screen->set_visible(false);
		FourthScene_end_screen->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	}
	else
		UtilityFunctions::print("[EndScreen] ERROR: EndScreen not found!");

	if (FourthScene_submit_btn)
		FourthScene_submit_btn->connect("pressed", Callable(FourthScene_self, "OnSubmitPressed"));
	else
		UtilityFunctions::print("[EndScreen] WARNING: SubmitButton not found!");
}

void OnDestroy(Caller* instance)
{
	if (FourthScene_rng) memdelete(FourthScene_rng);
	FourthScene_rng  = nullptr;
	FourthScene_self = nullptr;
}

void RemoveActive4(int i)
{
	int last = FourthScene_active_count - 1;
	FourthScene_active[i]    = FourthScene_active[last];
	FourthScene_active[last] = {};
	FourthScene_active_count--;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!FourthScene_self || !FourthScene_initialized) return;
	if (FourthScene_game_ended) return;

	for (int i = FourthScene_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = FourthScene_active[i].node;
		if (!fruit) { RemoveActive4(i); continue; }

		if (!fruit->is_inside_tree())
		{
			FourthScene_total++;
			FourthScene_caught++;
			FourthScene_multiplier++;
			FourthScene_score += 300ULL * (unsigned long long)FourthScene_multiplier;
			RemoveActive4(i);
			continue;
		}

		Vector2 pos = fruit->get_position();
		pos.y += FourthScene_fall_speed * (float)_delta;
		fruit->set_position(pos);

		if (pos.y > FourthScene_bottom_kill)
		{
			FourthScene_total++;
			FourthScene_multiplier = 0;
			fruit->queue_free();
			RemoveActive4(i);
		}
	}

	if (FourthScene_song_timer < FourthScene_song_length)
	{
		FourthScene_song_timer += (float)_delta;
		FourthScene_timer      += (float)_delta;

		if (FourthScene_timer >= FourthScene_spawn_interval && FourthScene_active_count < 256)
		{
			FourthScene_timer = 0.0f;

			int pick = FourthScene_rng->randi_range(0, FourthScene_fruit_path_count - 1);
			Ref<PackedScene> scene = FourthScene_packed_scenes[pick];
			if (!scene.is_valid()) return;

			Node* fruit_instance = scene->instantiate();
			if (!fruit_instance) return;

			FourthScene_self->add_child(fruit_instance);

			Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
			if (fruit_node)
			{
				float random_x = FourthScene_rng->randf_range(FourthScene_left_bound, FourthScene_right_bound);
				fruit_node->set_position(Vector2(random_x, FourthScene_top_spawn));
				fruit_node->set_visible(true);
				FourthScene_active[FourthScene_active_count].node   = fruit_node;
				FourthScene_active[FourthScene_active_count].caught = false;
				FourthScene_active_count++;
			}
		}
	}
	else if (FourthScene_active_count == 0)
	{
		FourthScene_game_ended = true;
		ShowEndScreen();
	}

	if (FourthScene_score_label && FourthScene_score != FourthScene_last_score)
	{
		FourthScene_score_label->set_text("Score: " + String::num_uint64(FourthScene_score));
		FourthScene_last_score = FourthScene_score;
	}
	if (FourthScene_combo_label && FourthScene_multiplier != FourthScene_last_multiplier)
	{
		FourthScene_combo_label->set_text("x" + String::num_int64(FourthScene_multiplier));
		FourthScene_last_multiplier = FourthScene_multiplier;
	}
	if (FourthScene_accuracy_label)
	{
		float acc = (FourthScene_total > 0 ? (float)FourthScene_caught / (float)FourthScene_total * 100.0f : 100.0f);
		if (acc != FourthScene_last_accuracy)
		{
			FourthScene_accuracy_label->set_text("Acc: " + String::num_real(acc, false) + "%");
			FourthScene_last_accuracy = acc;
		}
	}
}

JENOVA_SCRIPT_END
