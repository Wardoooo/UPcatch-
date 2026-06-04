/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/random_number_generator.hpp>
#include <Godot/classes/label.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/window.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

// ─── helpers ────────────────────────────────────────────────────────────────

// Finds the first Area2D anywhere in a subtree, no matter how deep
static Area2D* find_area2d_recursive(Node* node)
{
	if (!node) return nullptr;
	Area2D* as_area = Object::cast_to<Area2D>(node);
	if (as_area) return as_area;
	for (int i = 0; i < node->get_child_count(); i++)
	{
		Area2D* found = find_area2d_recursive(node->get_child(i));
		if (found) return found;
	}
	return nullptr;
}

// ─── state ──────────────────────────────────────────────────────────────────

struct BPMPresets {
	float bpm_100 = 100.0f; float bpm_110 = 110.0f; float bpm_120 = 120.0f;
	float bpm_130 = 130.0f; float bpm_140 = 140.0f; float bpm_150 = 150.0f;
	float bpm_160 = 160.0f; float bpm_170 = 170.0f; float bpm_180 = 180.0f;
	float bpm_190 = 190.0f; float bpm_200 = 200.0f; float bpm_210 = 210.0f;
	float bpm_220 = 220.0f; float bpm_230 = 230.0f; float bpm_240 = 240.0f;
	float bpm_250 = 250.0f; float bpm_260 = 260.0f; float bpm_270 = 270.0f;
	float bpm_280 = 280.0f; float bpm_290 = 290.0f; float bpm_300 = 300.0f;
};

Label* spawner_score_label    = nullptr;
Label* spawner_combo_label    = nullptr;
Label* spawner_accuracy_label = nullptr;

unsigned long long spawner_score      = 0ULL;
int                spawner_multiplier = 0;
int                spawner_caught     = 0;
int                spawner_total      = 0;

Node2D* spawner_self           = nullptr;
float   spawner_timer          = 0.0f;
float   spawner_spawn_interval = 0.0f;
bool    spawner_initialized    = false;

BPMPresets spawner_bpm_presets;
float spawner_current_bpm = 67000.0f;
float spawner_fall_speed  = 2000.0f;

float       spawner_song_timer  = 0.0f;
const float spawner_song_length = 237.0f;

RandomNumberGenerator* spawner_rng = nullptr;

const char* spawner_fruit_paths[] = {
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
const int spawner_fruit_path_count = sizeof(spawner_fruit_paths) / sizeof(spawner_fruit_paths[0]);

Ref<PackedScene> spawner_packed_scenes[sizeof(spawner_fruit_paths) / sizeof(spawner_fruit_paths[0])];

Node2D* spawner_active[256]      = {};
bool    spawner_caught_flag[256] = {};
int     spawner_active_count     = 0;

const float spawner_left_bound  = -1300.0f;
const float spawner_right_bound =   -10.0f;
const float spawner_top_spawn   =  -960.0f;
const float spawner_bottom_kill =   691.0f;

static void spawner_remove_at(int i)
{
	spawner_active_count--;
	spawner_active[i]      = spawner_active[spawner_active_count];
	spawner_caught_flag[i] = spawner_caught_flag[spawner_active_count];
	spawner_active[spawner_active_count]      = nullptr;
	spawner_caught_flag[spawner_active_count] = false;
}

// ─── script ─────────────────────────────────────────────────────────────────

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	spawner_self = GetSelf<Node2D>(instance);
	spawner_self->set_process(true);

	spawner_score        = 0ULL;
	spawner_multiplier   = 0;
	spawner_caught       = 0;
	spawner_total        = 0;
	spawner_timer        = 0.0f;
	spawner_song_timer   = 0.0f;
	spawner_active_count = 0;
	spawner_initialized  = false;

	spawner_rng = memnew(RandomNumberGenerator);
	spawner_rng->randomize();
	spawner_spawn_interval = 60.0f / spawner_current_bpm;
}

void OnReady(Caller* instance)
{
	// Load all fruit scenes
	for (int i = 0; i < spawner_fruit_path_count; i++)
	{
		spawner_packed_scenes[i] = ResourceLoader::get_singleton()->load(spawner_fruit_paths[i]);
		if (spawner_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", spawner_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", spawner_fruit_paths[i]);
	}
	spawner_initialized = true;

	// Locate UI labels — walks up to scene root, then into Gameplay
	Node* scene_root = spawner_self->get_tree()->get_current_scene();
	Node* gameplay   = scene_root->get_node_or_null(NodePath("Gameplay"));
	if (!gameplay)
	{
		UtilityFunctions::print("Spawner ERROR: Could not find Gameplay node!");
		return;
	}

	spawner_score_label    = Object::cast_to<Label>(gameplay->get_node_or_null(NodePath("CanvasLayer/VBoxContainer/ScoreLabel")));
	spawner_combo_label    = Object::cast_to<Label>(gameplay->get_node_or_null(NodePath("CanvasLayer/VBoxContainer/MultiplierLabel")));
	spawner_accuracy_label = Object::cast_to<Label>(gameplay->get_node_or_null(NodePath("CanvasLayer/VBoxContainer/AccuracyLabel")));

	if (!spawner_score_label)    UtilityFunctions::print("Spawner WARNING: ScoreLabel not found");
	if (!spawner_combo_label)    UtilityFunctions::print("Spawner WARNING: MultiplierLabel not found");
	if (!spawner_accuracy_label) UtilityFunctions::print("Spawner WARNING: AccuracyLabel not found");
}

void OnDestroy(Caller* instance)
{
	if (spawner_rng) memdelete(spawner_rng);
	spawner_rng  = nullptr;
	spawner_self = nullptr;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!spawner_self || !spawner_initialized) return;

	// ── move fruits, check catches, check misses ───────────────────────────
	for (int i = spawner_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = spawner_active[i];

		if (!fruit || !fruit->is_inside_tree())
		{
			spawner_remove_at(i);
			continue;
		}

		if (spawner_caught_flag[i])
		{
			spawner_remove_at(i);
			continue;
		}

		// Move fruit downward
		Vector2 gpos = fruit->get_global_position();
		gpos.y += spawner_fall_speed * (float)_delta;
		fruit->set_global_position(gpos);

		// ── catch check: use recursive search so ANY Area2D depth works ───
		Area2D* fruit_area = find_area2d_recursive(fruit);
		if (fruit_area)
		{
			TypedArray<Area2D> overlapping = fruit_area->get_overlapping_areas();
			for (int j = 0; j < overlapping.size(); j++)
			{
				Area2D* other = Object::cast_to<Area2D>(overlapping[j]);
				// CharLoader already added the character's Area2D to "player_catcher"
				if (other && other->is_in_group("player_catcher"))
				{
					spawner_caught_flag[i] = true;
					spawner_total++;
					spawner_caught++;
					spawner_multiplier++;
					spawner_score += 300ULL * (unsigned long long)spawner_multiplier;

					float acc = (float)spawner_caught / (float)spawner_total * 100.0f;
					UtilityFunctions::print(
						"Caught! Score: ", (int64_t)spawner_score,
						" x", spawner_multiplier,
						" Acc: ", acc, "%"
					);

					fruit->queue_free();
					spawner_remove_at(i);
					goto next_fruit;
				}
			}
		}

		// ── miss — fell off bottom ─────────────────────────────────────────
		if (gpos.y > spawner_bottom_kill)
		{
			spawner_total++;
			spawner_multiplier = 0;
			float acc = (spawner_total > 0
				? (float)spawner_caught / (float)spawner_total * 100.0f
				: 0.0f);
			UtilityFunctions::print("Miss! Acc: ", acc, "%");

			fruit->queue_free();
			spawner_remove_at(i);
		}

		next_fruit:;
	}

	// ── spawn ──────────────────────────────────────────────────────────────
	if (spawner_song_timer < spawner_song_length)
	{
		spawner_song_timer += (float)_delta;
		spawner_timer      += (float)_delta;

		if (spawner_timer >= spawner_spawn_interval)
		{
			spawner_timer = 0.0f;

			int pick = spawner_rng->randi_range(0, spawner_fruit_path_count - 1);
			Ref<PackedScene> scene = spawner_packed_scenes[pick];
			if (!scene.is_valid()) return;

			Node* fruit_instance = scene->instantiate();
			if (!fruit_instance) return;

			// Fruits also go to scene root — same world as the character
			Node* scene_root = spawner_self->get_tree()->get_current_scene();
			scene_root->add_child(fruit_instance);

			Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
			if (fruit_node && spawner_active_count < 256)
			{
				float random_x = spawner_rng->randf_range(spawner_left_bound, spawner_right_bound);
				fruit_node->set_global_position(Vector2(random_x, spawner_top_spawn));
				fruit_node->set_visible(true);
				spawner_active[spawner_active_count]      = fruit_node;
				spawner_caught_flag[spawner_active_count] = false;
				spawner_active_count++;
				UtilityFunctions::print("Spawned fruit at X: ", random_x);
			}
		}
	}
	else if (spawner_active_count == 0)
	{
		UtilityFunctions::print("Song ended, all fruits cleared!");
	}

	// ── update UI ──────────────────────────────────────────────────────────
	if (spawner_score_label)
		spawner_score_label->set_text("Score: " + String::num_uint64(spawner_score));
	if (spawner_combo_label)
		spawner_combo_label->set_text("x" + String::num_int64(spawner_multiplier));
	if (spawner_accuracy_label)
	{
		float acc = (spawner_total > 0
			? (float)spawner_caught / (float)spawner_total * 100.0f
			: 100.0f);
		spawner_accuracy_label->set_text("Acc: " + String::num_real(acc, false) + "%");
	}
}

JENOVA_SCRIPT_END
