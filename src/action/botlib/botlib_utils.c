#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

/*
This file is for common utilties that are used by the botlib functions
*/
void seed_random_number_generator(void) {
#if _MSC_VER >= 1920 && !__INTEL_COMPILER
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    srand((unsigned int)(li.QuadPart));
#else
    #include <sys/time.h>
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec * tv.tv_sec);
#endif
}

void BOTLIB_SKILL_Init(edict_t* bot)
{
    // Initialize random skill levels
	if (!bot_personality->value) {
		bot->bot.skill.overall = ((float)rand() / RAND_MAX) * 2 - 1;
		bot->bot.skill.aim = ((float)rand() / RAND_MAX) * 2 - 1;
		bot->bot.skill.reaction = ((float)rand() / RAND_MAX) * 2 - 1;
		bot->bot.skill.movement = ((float)rand() / RAND_MAX) * 2 - 1;
		bot->bot.skill.teamwork = ((float)rand() / RAND_MAX) * 2 - 1;
		bot->bot.skill.communication = ((float)rand() / RAND_MAX) * 2 - 1;
		bot->bot.skill.map_skill = ((float)rand() / RAND_MAX) * 2 - 1;
		int i;
		for (i = 0; i < WEAPON_COUNT; i++) {
			bot->bot.skill.weapon_skill[i] = ((float)rand() / RAND_MAX) * 2 - 1;
		}
	} else {  // We know what values to provide if we have a personality
		//copy the personality data when it's implemented
	}
	gi.dprintf("%s's bot skills:\noverall: %f\naim: %f\nreaction: %f\nmovement: %f\nteamwork: %f\ncommunication: %f\nmap_skill: %f\n",
		bot->client->pers.netname, bot->bot.skill.overall, bot->bot.skill.aim, bot->bot.skill.reaction, bot->bot.skill.movement, bot->bot.skill.teamwork, bot->bot.skill.communication, bot->bot.skill.map_skill);
	
}

// Returns true or false depending on if the bot passes the skill check
// Higher values of skilltype will have better chance of returning true
qboolean BOTLIB_SkillChance(float skill_level)
{
    // Normalize skill_level from range [-1, 1] to [0, 1]
    float normalized_skill_level = (skill_level + 1) / 2.0;

    // Generate a random float between 0 and 1
    float random_value = (float)rand() / RAND_MAX;

    // Return true if random_value is less than or equal to normalized_skill_level
    return random_value <= normalized_skill_level;
}

// This function will return a multiplier based on the skill level
// Set increase_with_skill to true if you want the multiplier to increase with skill level (ex: aim)
// Set increase_with_skill to false if you want the multiplier to decrease with skill level (ex: reaction time)
float BOTLIB_SkillMultiplier(float skill_level, bool increase_with_skill)
{
    // Seed the random number generator
    seed_random_number_generator();

    // Normalize the skill level to be between 0.0 and 1.0
    if (skill_level < -1.0)
        skill_level = -1.0;
    else if (skill_level > 1.0)
        skill_level = 1.0;

    // Define the base multiplier range
    float min_multiplier = 1.0;
    float max_multiplier = 2.0;

    // Generate a random base multiplier
    float base_multiplier = min_multiplier + ((float)rand() / RAND_MAX) * (max_multiplier - min_multiplier);

    // Adjust the multiplier based on the normalized skill level
    float final_multiplier;
    if (increase_with_skill)
    {
        final_multiplier = base_multiplier * (1.0 + skill_level);
    }
    else
    {
        final_multiplier = base_multiplier * (1.0 - skill_level);
    }

    return final_multiplier;
}

float BOTLIB_SKILL_Reaction(float reaction_skill)
{
    // Normalize skill level to be between 0 and 1
    float normalized_skill = (reaction_skill + 1.0) / 2.0;

    // Use BOTLIB_SkillMultiplier for randomization
    float reaction_multiplier = BOTLIB_SkillMultiplier(normalized_skill, false);

    // Scale to fit within the range of 0.2 to 1
    float floor = 0.2;
    float ceiling = 1.0;
    reaction_multiplier = floor + (reaction_multiplier * (ceiling - floor));

    return reaction_multiplier;
}
// Generic debug function in relation to bots
void BOTLIB_Debug(const char *debugmsg, ...)
{
    if (!bot_debug->value)
        return;
    gi.dprintf(debugmsg);
}
