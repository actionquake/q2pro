#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"

// Smarter weapon + item choices
void BOTLIB_SmartWeaponSelection(edict_t *self)
{
	// Let bots adjust its own skill
	// Skill helps dictate the weapon and item the bot will pick
	int weapon_skill_choice = 0;

	if (bot_skill_threshold->value > 0)
		weapon_skill_choice = BOTLIB_AutoAdjustSkill(self);
	else
		self->bot.skill.overall = bot_skill->value;

	// Allowed weapons: ALL
	// Allowed items: ALL 
	if (weapon_skill_choice == 0)
	{
		int weaponchoice = 0; // Weapon choice
		int highchoice = rand() % 10; // 0..9
		if (highchoice < 7) // 70% chance of ONLY primary weapon
			weaponchoice = rand() % 5;
		else // 30% chance of ANY weapon (including akimbos and knives)
			weaponchoice = rand() % 7;
		int itemchoice;
		switch (weaponchoice)
		{
		case 0:
			ACEAI_Cmd_Choose_Weapon_Num(self, MP5_NUM); // MP5
			itemchoice = rand() % 5;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, SIL_NUM); // Silencer
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else if (itemchoice == 3)
				ACEAI_Cmd_Choose_Item_Num(self, LASER_NUM); // Laser
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 1:
			ACEAI_Cmd_Choose_Weapon_Num(self, M4_NUM); // M4
			itemchoice = rand() % 4;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, LASER_NUM); // Laser
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 2:
			ACEAI_Cmd_Choose_Weapon_Num(self, M3_NUM); // M3
			itemchoice = rand() % 4;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, SLIP_NUM); // Slippers
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 3:
			ACEAI_Cmd_Choose_Weapon_Num(self, HC_NUM); // HC
			itemchoice = rand() % 4;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, SLIP_NUM); // Slippers
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 4:
			ACEAI_Cmd_Choose_Weapon_Num(self, SNIPER_NUM); // SSG
			itemchoice = rand() % 4;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, SIL_NUM); // Silencer
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 5:
			ACEAI_Cmd_Choose_Weapon_Num(self, DUAL_NUM); // Dual MK23
			itemchoice = rand() % 3;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 6:
			ACEAI_Cmd_Choose_Weapon_Num(self, KNIFE_NUM); // Knives
			itemchoice = rand() % 3;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		default:
			ACEAI_Cmd_Choose_Weapon_Num(self, 0);  // Random allowed.
			break;
		}
	}


	// Allowed weapons: MP5, M4, M3, Sniper
	// Allowed items: ALL 
	else if (weapon_skill_choice == 1)
	{
		int weaponchoice = rand() % 4; // MP5, M4, M3, Sniper
		int itemchoice;
		switch (weaponchoice)
		{
		case 0:
			ACEAI_Cmd_Choose_Weapon_Num(self, MP5_NUM); // MP5
			itemchoice = rand() % 5;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, SIL_NUM); // Silencer
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else if (itemchoice == 3)
				ACEAI_Cmd_Choose_Item_Num(self, LASER_NUM); // Laser
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 1:
			ACEAI_Cmd_Choose_Weapon_Num(self, M4_NUM); // M4
			itemchoice = rand() % 4;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, LASER_NUM); // Laser
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 2:
			ACEAI_Cmd_Choose_Weapon_Num(self, M3_NUM); // M3
			itemchoice = rand() % 4;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, SLIP_NUM); // Slippers
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		case 3:
			ACEAI_Cmd_Choose_Weapon_Num(self, SNIPER_NUM); // SSG
			itemchoice = rand() % 4;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, SIL_NUM); // Silencer
			else if (itemchoice == 1)
				ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
			else if (itemchoice == 2)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, HELM_NUM); // Helmet
			break;

		default:
			ACEAI_Cmd_Choose_Weapon_Num(self, 0);  // Random allowed.
			break;
		}
	}

	// Allowed weapons: MP5, M4, Sniper
	// Allowed items: Kevlar Vest, Laser
	else if (weapon_skill_choice == 2)
	{
		int weaponchoice = rand() % 3; // MP5, M4, M3, Sniper
		int itemchoice;
		switch (weaponchoice)
		{
		case 0:
			ACEAI_Cmd_Choose_Weapon_Num(self, MP5_NUM); // MP5
			itemchoice = rand() % 2;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, LASER_NUM); // Laser
			break;

		case 1:
			ACEAI_Cmd_Choose_Weapon_Num(self, M4_NUM); // M4
			itemchoice = rand() % 2;
			if (itemchoice == 0)
				ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			else
				ACEAI_Cmd_Choose_Item_Num(self, LASER_NUM); // Laser
			break;

		case 2:
			ACEAI_Cmd_Choose_Weapon_Num(self, SNIPER_NUM); // SSG
			ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			break;

		default:
			ACEAI_Cmd_Choose_Weapon_Num(self, SNIPER_NUM); // SSG
			ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM); // Kevlar Vest
			break;
		}
	}


	//Com_Printf("%s %s wep:%s item:%s\n", __func__, self->client->pers.netname, self->client->pers.chosenWeapon, self->client->pers.chosenItem);

	//self->weaponchoice = 8; // Knife
	//self->equipchoice = 12; // Bandolier

	// Force bots use grenades only
	//ACEAI_Cmd_Choose_Weapon_Num(self, KNIFE_NUM); // Knives
	//ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier

	//ACEAI_Cmd_Choose_Item_Num(self, LASER_NUM); // Laser
	//self->client->pers.chosenItem = FindItemByNum(LASER_NUM);
	//self->client->inventory[ITEM_INDEX(GET_ITEM(LASER_NUM))] = 1;

	//ACEAI_Cmd_Choose_Weapon_Num(self, HC_NUM);
	//ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier

	//ACEAI_Cmd_Choose_Weapon_Num(self, M3_NUM);
	//ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM);

	//ACEAI_Cmd_Choose_Weapon_Num(self, MP5_NUM);
	//ACEAI_Cmd_Choose_Weapon_Num(self, DUAL_NUM);
	//ACEAI_Cmd_Choose_Weapon_Num(self, SNIPER_NUM);
	//ACEAI_Cmd_Choose_Item_Num(self, BAND_NUM); // Bandolier
	//ACEAI_Cmd_Choose_Item_Num(self, KEV_NUM);
}