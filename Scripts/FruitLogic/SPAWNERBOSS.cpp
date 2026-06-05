/* Jenova C++ Node Base Script (Meteora) - Boss Level Spawner */
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


Label* BossScene_score_label    = nullptr;
Label* BossScene_combo_label    = nullptr;
Label* BossScene_accuracy_label = nullptr;


Control*  BossScene_end_screen           = nullptr;
Label*    BossScene_final_score_label    = nullptr;
Label*    BossScene_final_accuracy_label = nullptr;
LineEdit* BossScene_name_input           = nullptr;
Button*   BossScene_submit_btn           = nullptr;


unsigned long long BossScene_score      = 0ULL;
int                BossScene_multiplier = 0;
int                BossScene_caught     = 0;
int                BossScene_total      = 0;

unsigned long long BossScene_last_score      = ULLONG_MAX;
int                BossScene_last_multiplier = -1;
float              BossScene_last_accuracy   = -1.0f;

Node2D* BossScene_self           = nullptr;
float   BossScene_timer          = 0.0f;
float   BossScene_spawn_interval = 0.0f;
bool    BossScene_initialized    = false;
bool    BossScene_game_ended     = false;

float BossScene_current_bpm = 6700.0f; //FUNNYY 
float BossScene_fall_speed  = 1500.0f; //and fast

float       BossScene_song_timer  = 0.0f;
const float BossScene_song_length = 228.0f;

RandomNumberGenerator* BossScene_rng = nullptr;

const char* BossScene_fruit_paths[] = {
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
const int BossScene_fruit_path_count = sizeof(BossScene_fruit_paths) / sizeof(BossScene_fruit_paths[0]);

Ref<PackedScene> BossScene_packed_scenes[sizeof(BossScene_fruit_paths) / sizeof(BossScene_fruit_paths[0])];

struct BossSceneFruit {
	Node2D* node   = nullptr;
	bool    caught = false;
};

BossSceneFruit BossScene_active[256]  = {};
int      BossScene_active_count = 0;

const float BossScene_left_bound  = -1000.0f;
const float BossScene_right_bound =   450.0f;
const float BossScene_top_spawn   =  -300.0f;
const float BossScene_bottom_kill =   691.0f;

} 
JENOVA_SCRIPT_BEGIN



void SaveScore(String player_name, unsigned long long score, float accuracy)
{
	String save_path = "user://leaderboard.json";
	String level_key = "bosslevel"; 

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
		UtilityFunctions::print("[SpawnerBoss] Score saved for: ", player_name);
	}
	else
	{
		UtilityFunctions::print("[SpawnerBoss] ERROR: Could not write leaderboard file!");
	}
}



void ShowEndScreen()
{
	if (!BossScene_end_screen) return;

	if (BossScene_final_score_label)
		BossScene_final_score_label->set_text("Score: " + String::num_uint64(BossScene_score));

	float acc = (BossScene_total > 0 ? (float)BossScene_caught / (float)BossScene_total * 100.0f : 100.0f);
	if (BossScene_final_accuracy_label)
		BossScene_final_accuracy_label->set_text("Accuracy: " + String::num_real(acc, false) + "%");

	BossScene_end_screen->set_visible(true);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree) tree->set_pause(true);
}

void OnSubmitPressed(Caller* instance)
{
	if (!BossScene_name_input) return;

	String player_name = BossScene_name_input->get_text().strip_edges();
	if (player_name.is_empty()) player_name = "Anonymous";

	float acc = (BossScene_total > 0 ? (float)BossScene_caught / (float)BossScene_total * 100.0f : 100.0f);
	SaveScore(player_name, BossScene_score, acc);

	SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree)
	{
		tree->set_pause(false);
		tree->change_scene_to_file("res://menu.tscn");
	}
}



void OnAwake(Caller* instance)
{
	BossScene_self = GetSelf<Node2D>(instance);
	BossScene_self->set_process(true);
	BossScene_self->set_process_mode(Node::PROCESS_MODE_PAUSABLE);

	BossScene_score        = 0ULL;
	BossScene_multiplier   = 0;
	BossScene_caught       = 0;
	BossScene_total        = 0;
	BossScene_timer        = 0.0f;
	BossScene_song_timer   = 0.0f;
	BossScene_active_count = 0;
	BossScene_initialized  = false;
	BossScene_game_ended   = false;

	BossScene_last_score      = ULLONG_MAX;
	BossScene_last_multiplier = -1;
	BossScene_last_accuracy   = -1.0f;

	BossScene_rng = memnew(RandomNumberGenerator);
	BossScene_rng->randomize();
	BossScene_spawn_interval = 60.0f / BossScene_current_bpm;
}

void OnReady(Caller* instance)
{
	for (int i = 0; i < BossScene_fruit_path_count; i++)
	{
		BossScene_packed_scenes[i] = ResourceLoader::get_singleton()->load(BossScene_fruit_paths[i]);
		if (BossScene_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", BossScene_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", BossScene_fruit_paths[i]);
	}
	BossScene_initialized = true;

	BossScene_score_label    = BossScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Score");
	BossScene_combo_label    = BossScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Combo");
	BossScene_accuracy_label = BossScene_self->get_node<Label>("../CanvasLayer/VBoxContainer/Accuracy");

	BossScene_end_screen           = BossScene_self->get_node<Control>("../CanvasLayer/EndScreen");
	BossScene_final_score_label    = BossScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalScore");
	BossScene_final_accuracy_label = BossScene_self->get_node<Label>("../CanvasLayer/EndScreen/FinalAccuracy");
	BossScene_name_input           = BossScene_self->get_node<LineEdit>("../CanvasLayer/EndScreen/NameInput");
	BossScene_submit_btn           = BossScene_self->get_node<Button>("../CanvasLayer/EndScreen/SubmitButton");

	if (BossScene_end_screen)
	{
		BossScene_end_screen->set_visible(false);
		BossScene_end_screen->set_process_mode(Node::PROCESS_MODE_ALWAYS);
	}
	else
		UtilityFunctions::print("[EndScreen] ERROR: EndScreen not found!");

	if (BossScene_submit_btn)
		BossScene_submit_btn->connect("pressed", Callable(BossScene_self, "OnSubmitPressed"));
	else
		UtilityFunctions::print("[EndScreen] WARNING: SubmitButton not found!");
}

void OnDestroy(Caller* instance)
{
	if (BossScene_rng) memdelete(BossScene_rng);
	BossScene_rng  = nullptr;
	BossScene_self = nullptr;
}

void RemoveActive5(int i)
{
	int last = BossScene_active_count - 1;
	BossScene_active[i]    = BossScene_active[last];
	BossScene_active[last] = {};
	BossScene_active_count--;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!BossScene_self || !BossScene_initialized) return;
	if (BossScene_game_ended) return;

	for (int i = BossScene_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = BossScene_active[i].node;
		if (!fruit) { RemoveActive5(i); continue; }

		if (!fruit->is_inside_tree())
		{
			BossScene_total++;
			BossScene_caught++;
			BossScene_multiplier++;
			BossScene_score += 300ULL * (unsigned long long)BossScene_multiplier;
			RemoveActive5(i);
			continue;
		}

		Vector2 pos = fruit->get_position();
		pos.y += BossScene_fall_speed * (float)_delta;
		fruit->set_position(pos);

		if (pos.y > BossScene_bottom_kill)
		{
			BossScene_total++;
			BossScene_multiplier = 0;
			fruit->queue_free();
			RemoveActive5(i);
		}
	}

	if (BossScene_song_timer < BossScene_song_length)
	{
		BossScene_song_timer += (float)_delta;
		BossScene_timer      += (float)_delta;

		if (BossScene_timer >= BossScene_spawn_interval && BossScene_active_count < 256)
		{
			BossScene_timer = 0.0f;

			int pick = BossScene_rng->randi_range(0, BossScene_fruit_path_count - 1);
			Ref<PackedScene> scene = BossScene_packed_scenes[pick];
			if (!scene.is_valid()) return;

			Node* fruit_instance = scene->instantiate();
			if (!fruit_instance) return;

			BossScene_self->add_child(fruit_instance);

			Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
			if (fruit_node)
			{
				float random_x = BossScene_rng->randf_range(BossScene_left_bound, BossScene_right_bound);
				fruit_node->set_position(Vector2(random_x, BossScene_top_spawn));
				fruit_node->set_visible(true);
				BossScene_active[BossScene_active_count].node   = fruit_node;
				BossScene_active[BossScene_active_count].caught = false;
				BossScene_active_count++;
			}
		}
	}
	else if (BossScene_active_count == 0)
	{
		BossScene_game_ended = true;
		ShowEndScreen();
	}

	if (BossScene_score_label && BossScene_score != BossScene_last_score)
	{
		BossScene_score_label->set_text("Score: " + String::num_uint64(BossScene_score));
		BossScene_last_score = BossScene_score;
	}
	if (BossScene_combo_label && BossScene_multiplier != BossScene_last_multiplier)
	{
		BossScene_combo_label->set_text("x" + String::num_int64(BossScene_multiplier));
		BossScene_last_multiplier = BossScene_multiplier;
	}
	if (BossScene_accuracy_label)
	{
		float acc = (BossScene_total > 0 ? (float)BossScene_caught / (float)BossScene_total * 100.0f : 100.0f);
		if (acc != BossScene_last_accuracy)
		{
			BossScene_accuracy_label->set_text("Acc: " + String::num_real(acc, false) + "%");
			BossScene_last_accuracy = acc;
		}
	}
}

JENOVA_SCRIPT_END
