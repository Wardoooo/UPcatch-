/* Jenova C++ Node Base Script (Meteora) */
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/packed_scene.hpp>
#include <Godot/classes/resource_loader.hpp>
#include <Godot/classes/random_number_generator.hpp>
#include <Godot/variant/utility_functions.hpp>

using namespace godot;
using namespace jenova::sdk;

struct BPMPresets {
	float bpm_100 = 100.0f;
	float bpm_110 = 110.0f;
	float bpm_120 = 120.0f;
	float bpm_130 = 130.0f;
	float bpm_140 = 140.0f;
	float bpm_150 = 150.0f;
	float bpm_160 = 160.0f;
	float bpm_170 = 170.0f;
	float bpm_180 = 180.0f;
	float bpm_190 = 190.0f;
	float bpm_200 = 200.0f;
	float bpm_210 = 210.0f;
	float bpm_220 = 220.0f;
	float bpm_230 = 230.0f;
	float bpm_240 = 240.0f;
	float bpm_250 = 250.0f;
	float bpm_260 = 260.0f;
	float bpm_270 = 270.0f;
	float bpm_280 = 280.0f;
	float bpm_290 = 290.0f;
	float bpm_300 = 300.0f;
};

// score state
int spawner_score      = 0;
int spawner_multiplier = 0;
int spawner_caught     = 0;
int spawner_total      = 0;

Node2D* spawner_self           = nullptr;
float   spawner_timer          = 0.0f;
float   spawner_spawn_interval = 0.0f;
bool    spawner_initialized    = false;

BPMPresets spawner_bpm_presets;
float spawner_current_bpm = 130.0f;
float spawner_fall_speed  = 400.0f;

RandomNumberGenerator* spawner_rng = nullptr;

// fruit scene paths
const char* spawner_fruit_paths[] = {
	"res://24chicken.tscn",
	"res://malunggay.tscn",
	"res://vscode.tscn",
	"res://uplogo.tscn",
	"res://redbull.tscn",
    "res://LUCKYDAY.tscn"
};
const int spawner_fruit_path_count = 6;

// preloaded packed scenes
Ref<PackedScene> spawner_packed_scenes[6];

// active falling fruits
Node2D* spawner_active[256] = {};
int     spawner_active_count = 0;

JENOVA_SCRIPT_BEGIN

void OnAwake(Caller* instance)
{
	spawner_self = GetSelf<Node2D>(instance);
	spawner_self->set_process(true);
	spawner_rng  = memnew(RandomNumberGenerator);
	spawner_rng->randomize();
	spawner_spawn_interval = 60.0f / spawner_current_bpm;
}

void OnReady(Caller* instance)
{
	// preload all fruit scenes
	for (int i = 0; i < spawner_fruit_path_count; i++)
	{
		spawner_packed_scenes[i] = ResourceLoader::get_singleton()->load(spawner_fruit_paths[i]);
		if (spawner_packed_scenes[i].is_valid())
			UtilityFunctions::print("Loaded: ", spawner_fruit_paths[i]);
		else
			UtilityFunctions::print("FAILED to load: ", spawner_fruit_paths[i]);
	}
	spawner_initialized = true;
}

void OnDestroy(Caller* instance)
{
	if (spawner_rng) memdelete(spawner_rng);
	spawner_self = nullptr;
}

void OnProcess(Caller* instance, double _delta)
{
	if (!spawner_self || !spawner_initialized) return;

	// move all active fruits down
	for (int i = spawner_active_count - 1; i >= 0; i--)
	{
		Node2D* fruit = spawner_active[i];
		if (!fruit || !fruit->is_inside_tree())
		{
			spawner_active[i] = spawner_active[--spawner_active_count];
			continue;
		}

		Vector2 pos = fruit->get_position();
		pos.y += spawner_fall_speed * (float)_delta;
		fruit->set_position(pos);

		// check player catch
		Area2D* fruit_area = static_cast<Area2D*>((Object*)fruit);
		if (fruit_area)
		{
			TypedArray<Area2D> overlapping = fruit_area->get_overlapping_areas();
			for (int j = 0; j < overlapping.size(); j++)
			{
				Area2D* other = Object::cast_to<Area2D>(overlapping[j]);
				if (other && other->is_in_group("player_catcher"))
				{
					spawner_total++;
					spawner_caught++;
					spawner_multiplier++;
					spawner_score += 300 * spawner_multiplier;
					float acc = (float)spawner_caught / spawner_total * 100.0f;
					UtilityFunctions::print("Caught! Score: ", spawner_score, " x", spawner_multiplier, " Acc: ", acc, "%");
					fruit->queue_free();
					spawner_active[i] = spawner_active[--spawner_active_count];
					goto next_fruit;
				}
			}
		}

		// hit ground
		if (pos.y > 600.0f)
		{
			spawner_total++;
			spawner_multiplier = 0;
			float acc = (spawner_total > 0 ? (float)spawner_caught / spawner_total * 100.0f : 0.0f);
			UtilityFunctions::print("Miss! Acc: ", acc, "%");
			fruit->queue_free();
			spawner_active[i] = spawner_active[--spawner_active_count];
		}

		next_fruit:;
	}

	// spawn timer
	spawner_timer += (float)_delta;
	if (spawner_timer >= spawner_spawn_interval)
	{
		spawner_timer = 0.0f;

		int pick = spawner_rng->randi_range(0, spawner_fruit_path_count - 1);
		Ref<PackedScene> scene = spawner_packed_scenes[pick];
		if (!scene.is_valid()) return;

		Node* fruit_instance = scene->instantiate();
		if (!fruit_instance) return;

		spawner_self->add_child(fruit_instance);

		Node2D* fruit_node = Object::cast_to<Node2D>(fruit_instance);
		if (fruit_node && spawner_active_count < 256)
		{
			float random_x = spawner_rng->randf_range(-960.0f, 960.0f);
			fruit_node->set_position(Vector2(random_x, -600.0f));
			fruit_node->set_visible(true);
			spawner_active[spawner_active_count++] = fruit_node;
			UtilityFunctions::print("Spawned fruit at X: ", random_x);
		}
	}
}

JENOVA_SCRIPT_END
