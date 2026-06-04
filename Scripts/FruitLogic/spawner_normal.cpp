/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/random_number_generator.hpp>
#include <Godot/classes/label.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

struct BPMPresets_Normal {
	float bpm_100 = 100.0f; float bpm_110 = 110.0f; float bpm_120 = 120.0f;
	float bpm_130 = 130.0f; float bpm_140 = 140.0f; float bpm_150 = 150.0f;
	float bpm_160 = 160.0f; float bpm_170 = 170.0f; float bpm_180 = 180.0f;
	float bpm_190 = 190.0f; float bpm_200 = 200.0f; float bpm_210 = 210.0f;
	float bpm_220 = 220.0f; float bpm_230 = 230.0f; float bpm_240 = 240.0f;
	float bpm_250 = 250.0f; float bpm_260 = 260.0f; float bpm_270 = 270.0f;
	float bpm_280 = 280.0f; float bpm_290 = 290.0f; float bpm_300 = 300.0f;
};

Label* sn_score_label    = nullptr;
Label* sn_combo_label    = nullptr;
Label* sn_accuracy_label = nullptr;

// score state
unsigned long long sn_score      = 0ULL;
int                sn_multiplier = 0;
int                sn_caught     = 0;
int                sn_total      = 0;

Node2D* sn_self           = nullptr;
float   sn_timer          = 0.0f;
float   sn_spawn_interval = 0.0f;
bool    sn_initialized    = false;

BPMPresets_Normal sn_bpm_presets;
float sn_current_bpm = 130.0f;
float sn_fall_speed  = 200.0f;

// song timer
float sn_song_timer        = 0.0f;
const float sn_song_length = 79.0f; // 3min 57sec

RandomNumberGenerator* sn_rng = nullptr;

const char* sn_fruit_paths[] = {
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
const int sn_fruit_path_count = sizeof(sn_fruit_paths) / sizeof(sn_fruit_paths[0]);

Ref<PackedScene> sn_packed_scenes[sizeof(sn_fruit_paths) / sizeof(sn_fruit_paths[0])];

Node2D* sn_active[256]      = {};
bool    sn_caught_flag[256] = {};
int     sn_active_count     = 0;

const float sn_left_bound  = -1000.0f;
const float sn_right_bound =   100.0f;
const float sn_top_spawn   =  -300.0f;
const float sn_bottom_kill =   691.0f;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	sn_self = GetSelf<Node2D>(instance);
	sn_self->set_process(true);
	sn_self->set_process_mode(Node::PROCESS_MODE_PAUSABLE);

	sn_score        = 0ULL;
	sn_multiplier   = 0;
	sn_caught       = 0;
	sn_total        = 0;
	sn_timer        = 0.0f;
	sn_song_timer   = 0.0f;
	sn_active_count = 0;
	sn_initialized  = false;

	sn_rng = memnew(RandomNumberGenerator);
	sn_rng->randomize();
	sn_spawn_interval = 60.0f / sn_current_bpm;
}

void OnReady(Caller* instance)
{
	for (int i = 0; i < sn_fruit_path_count; i++)
	{
		sn_packed_scenes[i] = ResourceLoader::get_singleton()->load(sn_fruit_paths[i]);
		if (sn_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", sn_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", sn_fruit_paths[i]);
	}
	sn_initialized = true;

	sn_score_label    = sn_self->get_node<Label>("../CanvasLayer/VBoxContainer/Score");
	sn_combo_label    = sn_self->get_node<Label>("../CanvasLayer/VBoxContainer/Combo");
	sn_accuracy_label = sn_self->get_node<Label>("../CanvasLayer/VBoxContainer/Accuracy");
}

void OnDestroy(Caller* instance)
{
	if (sn_rng) memdelete(sn_rng);
	sn_rng  = nullptr;
	sn_self = nullptr;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!sn_self || !sn_initialized) return;

	for (int i = sn_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = sn_active[i];

		// fruit freed itself = caught Edward
		if (!fruit || !fruit->is_inside_tree())
		{
			sn_total++;
			sn_caught++;
			sn_multiplier++;
			sn_score += 300ULL * (unsigned long long)sn_multiplier;
			UtilityFunctions::print("Caught! Score: ", (int64_t)sn_score, " x", sn_multiplier);
			sn_active[i]      = sn_active[sn_active_count - 1];
			sn_caught_flag[i] = sn_caught_flag[sn_active_count - 1];
			sn_active_count--;
			continue;
		}

		Vector2 pos = fruit->get_position();
		pos.y += sn_fall_speed * (float)_delta;
		fruit->set_position(pos);

		// missed — hit bottom
		if (pos.y > sn_bottom_kill)
		{
			sn_total++;
			sn_multiplier = 0;
			UtilityFunctions::print("Miss! Multiplier reset.");
			fruit->queue_free();
			sn_active[i]      = sn_active[sn_active_count - 1];
			sn_caught_flag[i] = sn_caught_flag[sn_active_count - 1];
			sn_active_count--;
		}
	}

	// only spawn while song is still playing
	if (sn_song_timer < sn_song_length)
	{
		sn_song_timer += (float)_delta;
		sn_timer += (float)_delta;

		if (sn_timer >= sn_spawn_interval)
		{
			sn_timer = 0.0f;

			int pick = sn_rng->randi_range(0, sn_fruit_path_count - 1);
			Ref<PackedScene> scene = sn_packed_scenes[pick];
			if (!scene.is_valid()) return;

			Node* fruit_instance = scene->instantiate();
			if (!fruit_instance) return;

			sn_self->add_child(fruit_instance);

			Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
			if (fruit_node && sn_active_count < 256)
			{
				float random_x = sn_rng->randf_range(sn_left_bound, sn_right_bound);
				fruit_node->set_position(Vector2(random_x, sn_top_spawn));
				fruit_node->set_visible(true);
				sn_active[sn_active_count]      = fruit_node;
				sn_caught_flag[sn_active_count] = false;
				sn_active_count++;
			}
		}
	}
	else if (sn_active_count == 0)
	{
		UtilityFunctions::print("Song ended, all fruits cleared!");
	}

	// update UI labels
	if (sn_score_label)
		sn_score_label->set_text("Score: " + String::num_uint64(sn_score));
	if (sn_combo_label)
		sn_combo_label->set_text("x" + String::num_int64(sn_multiplier));
	if (sn_accuracy_label)
	{
		float acc = (sn_total > 0 ? (float)sn_caught / (float)sn_total * 100.0f : 100.0f);
		sn_accuracy_label->set_text("Acc: " + String::num_real(acc, false) + "%");
	}
}

JENOVA_SCRIPT_END
