#pragma once
#include <climits>

struct LevelResult
{
	int                level      = 0;
	unsigned long long score      = 0ULL;
	int                multiplier = 0;
	int                caught     = 0;
	int                total      = 0;
};

struct GameData
{
	static LevelResult results[5];
	static int         result_count;

	static void save_level(int level, unsigned long long score, int multiplier, int caught, int total)
	{
	
		for (int i = 0; i < result_count; i++)
		{
			if (results[i].level == level)
			{
				results[i] = { level, score, multiplier, caught, total };
				return;
			}
		}
		if (result_count < 5)
		{
			results[result_count++] = { level, score, multiplier, caught, total };
		}
	}

	static unsigned long long total_score()
	{
		unsigned long long sum = 0ULL;
		for (int i = 0; i < result_count; i++) sum += results[i].score;
		return sum;
	}

	static float average_accuracy()
	{
		int caught = 0, total = 0;
		for (int i = 0; i < result_count; i++)
		{
			caught += results[i].caught;
			total  += results[i].total;
		}
		return total > 0 ? (float)caught / (float)total * 100.0f : 100.0f;
	}
};
