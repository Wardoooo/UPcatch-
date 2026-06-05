/* Jenova C++ Node Base Script (Meteora) - Level 3 Spawner */
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


Label* ThirdScene_score_label    = nullptr;
Label* ThirdScene_combo_label    = nullptr;
Label* ThirdScene_accuracy_label = nullptr;


Control*  ThirdScene_end_screen           = nullptr;
Label*    ThirdScene_final_score_label    = nullptr;
Label*    ThirdScene_final_accuracy_label = nullptr;
LineEdit* ThirdScene_name_input           = nullptr;
Button*   ThirdScene_submit_btn           = nullptr;


unsigned long long ThirdScene_score      = 0ULL;
int                ThirdScene_multiplier = 0;
int                ThirdScene_caught     = 0;
int                ThirdScene_total      = 0;

unsigned long long ThirdScene_last_score      = ULLONG_MAX;
int                ThirdScene_last_multiplier = -1;
float              ThirdScene_last_accuracy   = -1.0f;

Node2D* ThirdScene_self           = nullptr;
float   ThirdScene_timer          = 0.0f;
float   ThirdScene_spawn_interval = 0.0f;
bool    ThirdScene_initialized    = false;
bool    ThirdScene_game_ended     = false;

float ThirdScene_current_bpm = 268.0f;
float ThirdScene_fall_speed  = 1200.0f;

float       ThirdScene_song_timer  = 0.0f;
const float ThirdScene_song_length = 180.0f;

RandomNumberGenerator* ThirdScene_rng = nullptr;

const char* ThirdScene_fruit_paths[] = {
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
const int ThirdScene_fruit_path_count = sizeof(ThirdScene_fruit_paths) / sizeof(ThirdScene_fruit_paths[0]);

Ref<PackedScene> ThirdScene_packed_scenes[sizeof(ThirdScene_fruit_paths) / sizeof(ThirdScene_fruit_paths[0])];

struct ThirdSceneFruit {
	Node2D* node   = nullptr;
	bool    caught = false;
};

ThirdSceneFruit ThirdScene_active[256]  = {};
int      ThirdScene_active_count = 0;

const float ThirdScene_left_bound  = -1000.0f;
const float ThirdScene_right_bound =   100.0f;
const float ThirdScene_top_spawn   =  -300.0f;
const float ThirdScene_bottom_kill =   691.0f;

}

JENOVA_SCRIPT_BEGIN



void SaveScore(String player_name, unsigned long long score, float accuracy)
{
	String save_path = "user://leaderboard.json";
	String level_key = "LEVEL3"; // ← Level 3

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
		UtilityFunctions::print("[Spawner3] Score saved for: ", player_name);
	}
	else
	{
		UtilityFunctions::print("[Spawner3] ERROR: Could not write leaderboard file!");
	}
}



void ShowEndScreen()
{
	if (!ThirdScene_end_screen) return;

	if (ThirdScene_final_score_label)
		ThirdScene_final_score_label->set_text("Score: " + String::num_uint64(ThirdScene_score));

	float acc = (ThirdScene_total > 0 ? (float)ThirdScene_caught / (float)ThirdScene_total * 100.0f : 100.0f);
	if (ThirdScene_final_accuracy_label)
		ThirdScene_final_accuracy_label->set_text("Accuracy: " + String::num_real(acc, false) + "%");

	ThirdScene_end_screen->set_visible(true);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree) tree->set_pause(true);
}

void OnSubmitPressed(Caller* instance)
{
	if (!ThirdScene_name_input) return;

	String player_name = ThirdScene_name_input->get_text().strip_edges();
	if (player_name.is_empty()) player_name = "Anonymous";

	float acc = (ThirdScene_total > 0 ? (float)ThirdScene_caught / (float)ThirdScene_total * 100.0f : 100.0f);
	SaveScore(player_name, ThirdScene_score, acc);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree)
	{
		tree->set_pause(false);
		tree->change_scene_to_file("res://menu.tscn");
	}
}



void OnAwake(Caller* instance)
{
	ThirdScene_self = GetSelf<Node2D>(instance);
	ThirdScene_self->set_process(true);
	ThirdScene_self->set_process_mode(Node::PROCESS_MODE_PAUSABLE);

	ThirdScene_score        = 0ULL;
	ThirdScene_multiplier   = 0;
	ThirdScene_caught       = 0;
	ThirdScene_total        = 0;
	ThirdScene_timer        = 0.0f;
	ThirdScene_song_timer   = 0.0f;
	ThirdScene_active_count = 0;
	ThirdScene_initialized  = false;
	ThirdScene_game_ended   = false;

	ThirdScene_last_score      = ULLONG_MAX;
	ThirdScene_last_multiplier = -1;
	ThirdScene_last_accuracy   = -1.0f;

	ThirdScene_rng = memnew(RandomNumberGenerator);
	ThirdScene_rng->randomize();
	ThirdScene_spawn_interval = 60.0f / ThirdScene_current_bpm;
}

void OnReady(Caller* instance)
{
	for (int i = 0; i < ThirdScene_fruit_path_count; i++)
	{
		ThirdScene_packed_scenes[i] = ResourceLoader::get_singleton()->load(ThirdScene_fruit_paths[i]);
		if (ThirdScene_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", ThirdScene_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", ThirdScene_fruit_paths[i]);
	}
	ThirdScene_initialized = true;

	ThirdScene_score_label    = ThirdScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Score");
	ThirdScene_combo_label    = ThirdScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Combo");
	ThirdScene_accuracy_label = ThirdScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Accuracy");

	ThirdScene_end_screen           = ThirdScene_self->get_node<Control>("../CanvasLayer/EndScreen");
	ThirdScene_final_score_label    = ThirdScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalScore");
	ThirdScene_final_accuracy_label = ThirdScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalAccuracy");
	ThirdScene_name_input           = ThirdScene_self->get_node<LineEdit>("../CanvasLayer/EndScreen/NameInput");
	ThirdScene_submit_btn           = ThirdScene_self->get_node<Button>("../CanvasLayer/EndScreen/SubmitButton");

	if (ThirdScene_end_screen)
	{
		ThirdScene_end_screen->set_visible(false);
		ThirdScene_end_screen->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	}
	else
		UtilityFunctions::print("[EndScreen] ERROR: EndScreen not found!");

	if (ThirdScene_submit_btn)
		ThirdScene_submit_btn->connect("pressed", Callable(ThirdScene_self, "OnSubmitPressed"));
	else
		UtilityFunctions::print("[EndScreen] WARNING: SubmitButton not found!");
}

void OnDestroy(Caller* instance)
{
	if (ThirdScene_rng) memdelete(ThirdScene_rng);
	ThirdScene_rng  = nullptr;
	ThirdScene_self = nullptr;
}

void RemoveActive3(int i)
{
	int last = ThirdScene_active_count - 1;
	ThirdScene_active[i]    = ThirdScene_active[last];
	ThirdScene_active[last] = {};
	ThirdScene_active_count--;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!ThirdScene_self || !ThirdScene_initialized) return;
	if (ThirdScene_game_ended) return;

	for (int i = ThirdScene_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = ThirdScene_active[i].node;
		if (!fruit) { RemoveActive3(i); continue; }

		if (!fruit->is_inside_tree())
		{
			ThirdScene_total++;
			ThirdScene_caught++;
			ThirdScene_multiplier++;
			ThirdScene_score += 300ULL * (unsigned long long)ThirdScene_multiplier;
			RemoveActive3(i);
			continue;
		}

		Vector2 pos = fruit->get_position();
		pos.y += ThirdScene_fall_speed * (float)_delta;
		fruit->set_position(pos);

		if (pos.y > ThirdScene_bottom_kill)
		{
			ThirdScene_total++;
			ThirdScene_multiplier = 0;
			fruit->queue_free();
			RemoveActive3(i);
		}
	}

	if (ThirdScene_song_timer < ThirdScene_song_length)
	{
		ThirdScene_song_timer += (float)_delta;
		ThirdScene_timer      += (float)_delta;

		if (ThirdScene_timer >= ThirdScene_spawn_interval && ThirdScene_active_count < 256)
		{
			ThirdScene_timer = 0.0f;

			int pick = ThirdScene_rng->randi_range(0, ThirdScene_fruit_path_count - 1);
			Ref<PackedScene> scene = ThirdScene_packed_scenes[pick];
			if (!scene.is_valid()) return;

			Node* fruit_instance = scene->instantiate();
			if (!fruit_instance) return;

			ThirdScene_self->add_child(fruit_instance);

			Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
			if (fruit_node)
			{
				float random_x = ThirdScene_rng->randf_range(ThirdScene_left_bound, ThirdScene_right_bound);
				fruit_node->set_position(Vector2(random_x, ThirdScene_top_spawn));
				fruit_node->set_visible(true);
				ThirdScene_active[ThirdScene_active_count].node   = fruit_node;
				ThirdScene_active[ThirdScene_active_count].caught = false;
				ThirdScene_active_count++;
			}
		}
	}
	else if (ThirdScene_active_count == 0)
	{
		ThirdScene_game_ended = true;
		ShowEndScreen();
	}

	if (ThirdScene_score_label && ThirdScene_score != ThirdScene_last_score)
	{
		ThirdScene_score_label->set_text("Score: " + String::num_uint64(ThirdScene_score));
		ThirdScene_last_score = ThirdScene_score;
	}
	if (ThirdScene_combo_label && ThirdScene_multiplier != ThirdScene_last_multiplier)
	{
		ThirdScene_combo_label->set_text("x" + String::num_int64(ThirdScene_multiplier));
		ThirdScene_last_multiplier = ThirdScene_multiplier;
	}
	if (ThirdScene_accuracy_label)
	{
		float acc = (ThirdScene_total > 0 ? (float)ThirdScene_caught / (float)ThirdScene_total * 100.0f : 100.0f);
		if (acc != ThirdScene_last_accuracy)
		{
			ThirdScene_accuracy_label->set_text("Acc: " + String::num_real(acc, false) + "%");
			ThirdScene_last_accuracy = acc;
		}
	}
}

JENOVA_SCRIPT_END
