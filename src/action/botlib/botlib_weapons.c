#include "../g_local.h"
#include "../acesrc/acebot.h"
#include "botlib.h"



// Returns how much ammunition is loaded/chambered in a weapon -- this is *NOT* spare ammo
int BOTLIB_CheckWeaponLoadedAmmo(edict_t* self, int item_num)
{
	switch (item_num)
	{
		case MK23_NUM:
			return self->client->mk23_rds;
		case DUAL_NUM:
			return self->client->dual_rds;
		case MP5_NUM:
			return self->client->mp5_rds;
		case M4_NUM:
			return self->client->m4_rds;
		case M3_NUM:
			return self->client->shot_rds;
		case HC_NUM:
			return self->client->cannon_rds;
		case SNIPER_NUM:
			return self->client->sniper_rds;
		case KNIFE_NUM:
		case GRENADE_NUM:
			return self->client->inventory[ITEM_INDEX(FindItemByNum(item_num))];
		default:
			return 0;
	}

	return 0;
}

// 
qboolean BOTLIB_ChangeSpecialWeapon(edict_t* self, int item_num)
{
	int			ammo_index;
	gitem_t*	ammo_item;
	qboolean	loaded = true;
	qboolean	clips = true;

	gitem_t* item = FindItemByNum(item_num);

	// Check if weapon is in inventory
	if (self->client->inventory[ITEM_INDEX(item)] == 0)
		return false;

	// Do we have spare ammo
	if (item->ammo)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (self->client->inventory[ammo_index] < 1)
			clips = false;
		else
			clips = true;
	}

	// Check loaded ammo
	if (BOTLIB_CheckWeaponLoadedAmmo(self, item_num) == 0)
		loaded = false;


	// see if we're already using it
	if (item == self->client->weapon)
	{
		if (!loaded && !clips) // Drop weapon if we've got no ammo
		{
			DropSpecialWeapon(self);
			return false;
		}
		else if (!loaded) // Reload if we've got spare ammo
		{
			BOTLIB_Reload(self);
			//Cmd_New_Reload_f(self);
			return true;
		}
		else
			return true; // Nothing to do, weapon is good to go
	}

	// No ammo
	if (!loaded && !clips)
	{
		DropSpecialWeapon(self);
		return false;
	}

	// Change to this weapon
	self->client->newweapon = item;
	ChangeWeapon(self);
	return true;
}

// Bot checks its weapons: reload, drop empty, or change weapon... before encountering an enemy
void BOTLIB_ReadyWeapon(edict_t* self)
{
	// Limit how often this is called
	if (self->bot.last_weapon_change_time > level.framenum)
		return;
	self->bot.last_weapon_change_time = level.framenum + 6 * HZ;
	//Com_Printf("%s %s [%d]\n", __func__, self->client->pers.netname, level.framenum);

	/*
	// Random chance to pull out grenades
	if ((rand() % 20) == 0 && BOTLIB_ChangeSpecialWeapon(self, GRENADE_NUM)) //ACEIT_ChangeWeapon(self, FindItemByNum(GRENADE_NUM)))
	{
		self->client->pers.grenade_mode = 2;
		return;
	}
	if (self->client->weapon == FindItemByNum(GRENADE_NUM))
		return;


	// Random chance to just use knives
	if ((rand() % 20) == 0 && (INV_AMMO(self, KNIFE_NUM) >= 2) && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
	{
		self->client->pers.knife_mode = 1; // Throwing knives
		return;
	}
	if (self->client->weapon == FindItemByNum(KNIFE_NUM))
		return;
	*/


	if (BOTLIB_ChangeSpecialWeapon(self, SNIPER_NUM)) return;

	if (INV_AMMO(self, LASER_NUM))
	{
		if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return;
		if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return;
	}

	if (BOTLIB_ChangeSpecialWeapon(self, HC_NUM)) return;
	if (BOTLIB_ChangeSpecialWeapon(self, M3_NUM)) return;
	if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return;
	if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return;
	if (BOTLIB_ChangeSpecialWeapon(self, DUAL_NUM)) return;
	if (BOTLIB_ChangeSpecialWeapon(self, MK23_NUM)) return;
}

// Choose the best weapon for the situation
qboolean BOTLIB_ChooseWeapon(edict_t* self)
{
	// Limit how often this is called
	if (self->bot.last_weapon_change_time > level.framenum)
		return true;
	self->bot.last_weapon_change_time = level.framenum + 6 * HZ;
	//Com_Printf("%s %s [%d]\n", __func__, self->client->pers.netname, level.framenum);

	// Don't change if weaapon is busy
	if (self->client->weaponstate == WEAPON_DROPPING || self->client->weaponstate == WEAPON_BUSY)
		return true;


	// If not firing weapon
	if (self->client->weaponstate == WEAPON_READY && self->client->weapon != FindItemByNum(HC_NUM) && self->client->weapon != FindItemByNum(M3_NUM))
	{
		// Random chance to pull out grenades
		if ((rand() % 2) == 0 && self->client->weapon != FindItemByNum(GRENADE_NUM) && BOTLIB_ChangeSpecialWeapon(self, GRENADE_NUM))
		{
			self->client->pers.grenade_mode = 2;
			return true;
		}

		// Random chance to just use knives
		//if ((rand() % 20) == 0 && (INV_AMMO(self, KNIFE_NUM) >= 2) && self->client->weapon != FindItemByNum(KNIFE_NUM) && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
		if (self->enemy && self->bot.enemy_dist <= 512) // More included to use knives when enemy is closer
		{
			if ((rand() % 2) == 0 && self->client->weapon != FindItemByNum(KNIFE_NUM) && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
			{
				self->client->pers.knife_mode = 1; // Throwing knives
				return true;
			}
		}
		else // Less included to use knives when enemy is far
		{
			if ((rand() % 5) == 0 && self->client->weapon != FindItemByNum(KNIFE_NUM) && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
			{
				self->client->pers.knife_mode = 1; // Throwing knives
				return true;
			}
		}
	}


	if (self->enemy && self->bot.enemy_dist <= 1200)
	{
		// Let bots use the grenades if already using
		if (self->client->weapon == FindItemByNum(GRENADE_NUM) && BOTLIB_CheckWeaponLoadedAmmo(self, GRENADE_NUM))
		{
			if (self->bot.enemy_dist > 800)
				self->client->pers.grenade_mode = 2;
			else if (self->bot.enemy_dist > 400)
				self->client->pers.grenade_mode = 1;
			else
				self->client->pers.grenade_mode = 0;
			return true;
		}
		// Let bots use the knives if already using
		if (self->client->weapon == FindItemByNum(KNIFE_NUM) && BOTLIB_CheckWeaponLoadedAmmo(self, KNIFE_NUM))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		// Let bots use the pistol if already using
		if (self->client->weapon == FindItemByNum(MK23_NUM) && self->client->mk23_rds == self->client->mk23_max)
		{
			return true;
		}
	}

	/*
	// Friendly fire after round should be fought with honor.
	if( team_round_countdown && ff_afterround->value )
	{
		if( ACEIT_ChangeWeapon(self,FindItem(KNIFE_NAME)) )
			return true;
		if( ACEIT_ChangeHCSpecialWeapon(self,FindItem(HC_NAME)) )
			return true;
		if( ACEIT_ChangeWeapon(self,FindItem(GRENADE_NAME)) )
		{
			self->client->pers.grenade_mode = (	self->bot.enemy_dist > 500) ? 1 : 0;
			return true;
		}
	}
	*/

	/*
	if (self->client->weapon == FindItemByNum(GRENADE_NUM))
	{
		if (self->client->weaponstate == WEAPON_ACTIVATING || self->client->weaponstate == WEAPON_FIRING)
			return true;
	}
	*/

	// Force bots use grenades only
	/*
	if (ACEIT_ChangeWeapon(self, FindItemByNum(GRENADE_NUM)))
	{
		self->client->pers.grenade_mode = 2;
		return true;
	}
	*/

	if (self->enemy == NULL)
	{
		if (BOTLIB_ChangeSpecialWeapon(self, SNIPER_NUM)) return true;

		if (INV_AMMO(self, LASER_NUM))
		{
			if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return true;
			if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return true;
		}

		if (BOTLIB_ChangeSpecialWeapon(self, HC_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, M3_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, DUAL_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MK23_NUM)) return true;

		return true;
	}
	

	// Extreme range
	if (self->bot.enemy_dist > 1300)
	{
		if (BOTLIB_ChangeSpecialWeapon(self, SNIPER_NUM)) return true;
	}

	// Long range 
	if (self->bot.enemy_dist > 1000)
	{
		/*
		// Random chance to pull out grenades
		if ((rand() % 3) == 0 && BOTLIB_ChangeSpecialWeapon(self, GRENADE_NUM))
		{
			self->client->pers.grenade_mode = 2;
			return true;
		}
		// Random chance to just use knives
		if ((rand() % 6) == 0 && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}
		*/

		if (BOTLIB_ChangeSpecialWeapon(self, SNIPER_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return true;

		// Allow bots that picked throwing knives to use them at a distance
		if (INV_AMMO(self, KNIFE_NUM) >= 2 && self->client->pers.chosenItem == FindItemByNum(KNIFE_NUM) && ACEIT_ChangeWeapon(self, FindItem(KNIFE_NAME)))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		if (INV_AMMO(self, SIL_NUM) && BOTLIB_ChangeSpecialWeapon(self, MK23_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, DUAL_NUM)) return true;

		// If bot has a HC and healthy: 
		// keep HC equipped so the bot can try get closer to its target, instead of pulling out the pistol
		if (self->client->cannon_rds && self->client->weapon == FindItem(HC_NAME)
			&& self->client->leg_damage == 0 && self->client->bleeding == 0
			&& BOTLIB_ChangeSpecialWeapon(self, HC_NUM))
		{
			return true;
		}

		if (BOTLIB_ChangeSpecialWeapon(self, MK23_NUM)) return true;
	}

	// Longer range 
	if (self->bot.enemy_dist > 700)
	{
		/*
		// Random chance to pull out grenades
		if ((rand() % 2) == 0 && BOTLIB_ChangeSpecialWeapon(self, GRENADE_NUM))
		{
			self->client->pers.grenade_mode = 2;
			return true;
		}
		// Random chance to just use knives
		if ((rand() % 3) == 0 && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}
		*/

		if (BOTLIB_ChangeSpecialWeapon(self, SNIPER_NUM)) return true;

		if (INV_AMMO(self, SIL_NUM))
		{
			if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return true;
			if (BOTLIB_ChangeSpecialWeapon(self, MK23_NUM)) return true;
		}

		if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, M3_NUM)) return true;

		// If bot has a HC and healthy: 
		// keep HC equipped so the bot can try get closer to its target, instead of pulling out the pistol
		if (self->client->cannon_rds && self->client->weapon == FindItem(HC_NAME)
			&& self->client->leg_damage == 0 && BOTLIB_ChangeSpecialWeapon(self, HC_NUM))
		{
			return true;
		}

		if ((INV_AMMO(self, KNIFE_NUM) >= 2) && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		if (BOTLIB_ChangeSpecialWeapon(self, DUAL_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MK23_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, HC_NUM)) return true;
	}

	// Long range 
	if (self->bot.enemy_dist > 500)
	{
		/*
		// Random chance to pull out grenades
		if ((rand() % 5) == 0 && BOTLIB_ChangeSpecialWeapon(self, GRENADE_NUM))
		{
			self->client->pers.grenade_mode = 2;
			return true;
		}
		// Random chance to just use knives
		if ((rand() % 5) == 0 && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}
		*/

		if (INV_AMMO(self, LASER_NUM))
		{
			if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return true;
			if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return true;
		}

		if (BOTLIB_ChangeSpecialWeapon(self, M3_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, SNIPER_NUM)) return true;

		// If bot has a HC and healthy: 
		// keep HC equipped so the bot can try get closer to its target, instead of pulling out the pistol
		if (self->client->cannon_rds && self->client->weapon == FindItem(HC_NAME)
			&& self->client->leg_damage == 0 && BOTLIB_ChangeSpecialWeapon(self, HC_NUM))
		{
			return true;
		}

		if ((INV_AMMO(self, KNIFE_NUM) >= 2) && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}

		if (BOTLIB_ChangeSpecialWeapon(self, DUAL_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MK23_NUM)) return true;
	}

	// Medium range 
	if (self->bot.enemy_dist)
	{
		if (BOTLIB_ChangeSpecialWeapon(self, HC_NUM)) return true;

		/*
		// Random chance to pull out grenades
		if ((rand() % 10) == 0 && BOTLIB_ChangeSpecialWeapon(self, GRENADE_NUM))
		{
			self->client->pers.grenade_mode = 2;
			return true;
		}
		// Random chance to just use knives
		if ((rand() % 5) == 0 && BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM))
		{
			self->client->pers.knife_mode = 1; // Throwing knives
			return true;
		}
		*/

		if (BOTLIB_ChangeSpecialWeapon(self, M3_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, M4_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MP5_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, SNIPER_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, KNIFE_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, DUAL_NUM)) return true;
		if (BOTLIB_ChangeSpecialWeapon(self, MK23_NUM)) return true;

		// Should punch as a last resort?
	}

	// We have no weapon available for use.
	//if (debug_mode) gi.bprintf(PRINT_HIGH, "%s: No weapon available...\n", self->client->pers.netname);
	return false;
}

void BOTLIB_Reload(edict_t* self)
{
	int	ammo_index, rounds = 0;
	gitem_t* ammo_item;
	qboolean loaded = true;
	qboolean clips = true;
	qboolean using_special = false; // If bot is using high powered weapon

	// Limit how often this is called
	if (self->bot.last_weapon_reload_time > level.framenum)
		return;
	self->bot.last_weapon_reload_time = level.framenum + 2 * HZ;

	if (self->client->weapon == FindItem(MK23_NAME) || self->client->weapon == FindItem(DUAL_NAME)
		|| self->client->weapon == FindItem(MP5_NAME) || self->client->weapon == FindItem(M4_NAME)
		|| self->client->weapon == FindItem(M3_NAME) || self->client->weapon == FindItem(HC_NAME)
		|| self->client->weapon == FindItem(SNIPER_NAME))
	{
		// Do we have spare ammo
		if (self->client->weapon->ammo)
		{
			ammo_item = FindItem(self->client->weapon->ammo);
			ammo_index = ITEM_INDEX(ammo_item);
			if (self->client->inventory[ammo_index] < 1)
				clips = false;
			else
				clips = true;
		}

		// Check ammo
		if (self->client->weapon == FindItem(MK23_NAME))
		{
			rounds = self->client->mk23_rds;
			if (self->client->weaponstate == WEAPON_END_MAG)
				rounds = 0;
			if (rounds < 5)
				loaded = false;
			if (self->bot.see_enemies && rounds) // Don't reload until empty if facing enemy
				loaded = true;
		}
		else if (self->client->weapon == FindItem(DUAL_NAME))
		{
			rounds = self->client->dual_rds;
			if (self->client->weaponstate == WEAPON_END_MAG)
				rounds = 0;
			if (rounds < 6)
				loaded = false;
			if (self->bot.see_enemies && rounds) // Don't reload until empty if facing enemy
				loaded = true;
		}
		else if (self->client->weapon == FindItem(MP5_NAME))
		{
			using_special = true;
			rounds = self->client->mp5_rds;
			if (rounds < 5)
				loaded = false;
			if (self->bot.see_enemies && rounds) // Don't reload until empty if facing enemy
				loaded = true;
		}
		else if (self->client->weapon == FindItem(M4_NAME))
		{
			using_special = true;
			rounds = self->client->m4_rds;
			if (rounds < 5)
				loaded = false;
			if (self->bot.see_enemies && rounds) // Don't reload until empty if facing enemy
				loaded = true;
		}
		else if (self->client->weapon == FindItem(M3_NAME))
		{
			//using_special = true;
			rounds = self->client->shot_rds;
			if (rounds < self->client->shot_max)
				loaded = false;
			if (self->bot.see_enemies && rounds) // Don't reload until empty if facing enemy
				loaded = true;
		}
		else if (self->client->weapon == FindItem(HC_NAME))
		{
			//using_special = true;
			rounds = self->client->cannon_rds;
			if (hc_single->value && self->client->pers.hc_mode) // Single barrel fire mode
			{
				if (rounds < 1)
					loaded = false;
			}
			else if (rounds < 2) // Double barrel fire mode
				loaded = false;
		}
		else if (self->client->weapon == FindItem(SNIPER_NAME))
		{
			using_special = true;
			rounds = self->client->sniper_rds;
			if (rounds < self->client->sniper_max)
				loaded = false;
			if (self->bot.see_enemies && rounds) // Don't reload until empty if facing enemy
				loaded = true;
		}

		//Com_Printf("%s %s inv_ammo[%d] rnds[%d] loaded[%d] clips[%d]\n", __func__, self->client->pers.netname, self->client->inventory[ammo_index], rounds, loaded, clips);

		// No ammo - drop weapon
		if (!loaded && !clips)
		{
			DropSpecialWeapon(self);
		}
		// Reload
		else if (!loaded)
		{
			// If using high powered weapon that is out of ammo and bot has a full pistol, switch to pistol
			if (rounds == 0 && using_special && self->bot.enemy_dist < 512 && self->client->mk23_rds == self->client->mk23_max)// && self->bot.see_enemies)
			{
				// Try switch to pistol instead of reloading current weapon
				BOTLIB_ChangeSpecialWeapon(self, MK23_NUM);
				return;
			}

			self->client->reload_attempts += 2; // Reload twice in one attempt

			//Cmd_New_Reload_f(self);
		}
	}
}

// If have sniper and not zoomed in, zoom in and return true.
// Returns false otherwise
qboolean BOTLIB_SniperZoom(edict_t* self)
{
	if (self->bot.last_sniper_zoom_time > level.framenum)
	{
		return true;
	}

	if (self->client->weapon == FindItem(SNIPER_NAME) && (self->client->resp.sniper_mode != SNIPER_2X || self->client->desired_fov != SNIPER_FOV2))
	{
		if (self->client->weaponstate != WEAPON_FIRING && self->client->weaponstate != WEAPON_BUSY && self->client->weaponstate != WEAPON_RELOADING
			&& !self->client->bandaging && !self->client->bandage_stopped)
		{
			//Com_Printf("%s %s sniper zoom X2 [%d]\n", __func__, self->client->pers.netname, level.framenum);
			self->bot.last_sniper_zoom_time = level.framenum + 1 * HZ;
			_SetSniper(self, 2);
			return true;
		}
	}

	return false;
}

//rekkie -- collecting weapons, items, ammo -- s

// Checks if the item is on the map and can be picked up by the bot
// Instances where it cannot be picked up: it's been taken by another player, or it's just not on the map
qboolean BOTLIB_CanTouchItem(edict_t* self, int typeNum)
{
	int base = 1 + game.maxclients + BODY_QUEUE_SIZE;
	edict_t* ent = g_edicts + base;

	for (int i = base; i < globals.num_edicts; i++, ent++)
	{
		if (ent->inuse == false) continue;
		if (!ent->classname) continue;
		if (ent->solid == SOLID_NOT) continue;
		//Com_Printf("%s %s typeNum[%d] dist[%f]\n", __func__, ent->classname, ent->typeNum, VectorDistance(self->s.origin, ent->s.origin));
		if (ent->typeNum == typeNum)
			return true;
	}

	return false;
}

// Do we need a primary weapon?
// Pass parameters: primary_weapon and secondary_weapon to get a return on the weapons the bot is carrying
// The secondary_weapon only returns if the bot is using a bandolier
qboolean BOTLIB_Need_Weapons(edict_t* self, int* primary_weapon, int* secondary_weapon)
{
	int weapon_count = 0;
	qboolean band = INV_AMMO(self, BAND_NUM) ? 1 : 0;

	// Figure out what weapons we have
	for (int i = 0; i < game.num_items; i++)
	{
		if (self->client->inventory[i] > 0)
		{
			switch (itemlist[i].typeNum)
			{
			case MP5_NUM:
				if (*primary_weapon == 0)
					*primary_weapon = MP5_NUM;
				else
					*secondary_weapon = MP5_NUM;
				weapon_count++;
				break;

			case M4_NUM:
				if (*primary_weapon == 0)
					*primary_weapon = M4_NUM;
				else
					*secondary_weapon = M4_NUM;
				weapon_count++;
				break;

			case M3_NUM:
				if (*primary_weapon == 0)
					*primary_weapon = M3_NUM;
				else
					*secondary_weapon = M3_NUM;
				weapon_count++;
				break;

			case HC_NUM:
				if (*primary_weapon == 0)
					*primary_weapon = HC_NUM;
				else
					*secondary_weapon = HC_NUM;
				weapon_count++;
				break;

			case SNIPER_NUM:
				if (*primary_weapon == 0)
					*primary_weapon = SNIPER_NUM;
				else
					*secondary_weapon = SNIPER_NUM;
				weapon_count++;
				break;
			}

			//Com_Printf("%s inventory[%d] = %d [%s]\n", __func__, i, self->client->inventory[i], itemlist[i].classname);
		}
	}

	if (band && weapon_count < 2) // Bandolier: desire two primary weapons
		return true;
	if (weapon_count < 1) // Desire at least one primary weapon
		return true;

	return false; // Don't need a primary
}

// Do we need a grendes?
qboolean BOTLIB_Need_Grenades(edict_t* self)
{
	int grens_count = 0;
	qboolean band = INV_AMMO(self, BAND_NUM) ? 1 : 0;

	// Figure out what weapons we have
	for (int i = 0; i < game.num_items; i++)
	{
		if (self->client->inventory[i] > 0)
		{
			switch (itemlist[i].typeNum)
			{
			case GRENADE_NUM:
				grens_count = self->client->inventory[i];
				break;
			}
		}
	}

	if (band && grens_count < 4) // Bandolier: desire up to four greandes
		return true;
	if (grens_count < 1) // Desire at least one grenade
		return true;

	return false; // Don't need grenades
}

// Do we need a knives?
qboolean BOTLIB_Need_Knives(edict_t* self)
{
	int knives_count = 0;
	qboolean band = INV_AMMO(self, BAND_NUM) ? 1 : 0;

	// Figure out what weapons we have
	for (int i = 0; i < game.num_items; i++)
	{
		if (self->client->inventory[i] > 0)
		{
			switch (itemlist[i].typeNum)
			{
			case KNIFE_NUM:
				knives_count = self->client->inventory[i];
				break;
			}
		}
	}

	if (band && knives_count < 20) // Bandolier: desire up to 20 knives
		return true;
	if (knives_count < 4) // Desire at least a few knives
		return true;

	return false; // Don't need knives
}

// Do we need a dual pistols?
qboolean BOTLIB_Need_Dual_MK23(edict_t* self)
{
	// Figure out what weapons we have
	for (int i = 0; i < game.num_items; i++)
	{
		if (self->client->inventory[i] > 0)
		{
			switch (itemlist[i].typeNum)
			{
			case DUAL_NUM:
				return false; // Don't need another pistol
				break;
			}
		}
	}

	return true; // Desire a pair of mk23 pistols
}

// Do we need ammo for weapons we carry: mk23, primary, or secondary (if any)
// Pass parameters input: primary_weapon and secondary_weapon
// Returns ammo type
int BOTLIB_Need_Ammo(edict_t* self, int* primary_weapon, int* secondary_weapon)
{
	gitem_t* weapon;
	gitem_t* ammo_item;
	int ammo_index = 0;

	if (*primary_weapon == MP5_NUM || *secondary_weapon == MP5_NUM)
	{
		weapon = FindItem(MP5_NAME);
		ammo_item = FindItem(weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (self->client->inventory[ammo_index] < self->client->max_mp5mags)
		{
			// Check there's ammo for this weapon ready for pickup on the map
			if (BOTLIB_CanTouchItem(self, MP5_ANUM))
				return MP5_ANUM;
		}
	}
	if (*primary_weapon == M4_NUM || *secondary_weapon == M4_NUM)
	{
		weapon = FindItem(M4_NAME);
		ammo_item = FindItem(weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (self->client->inventory[ammo_index] < self->client->max_m4mags)
		{
			if (BOTLIB_CanTouchItem(self, M4_ANUM))
				return M4_ANUM;
		}
	}
	if (*primary_weapon == M3_NUM || *secondary_weapon == M3_NUM)
	{
		weapon = FindItem(M3_NAME);
		ammo_item = FindItem(weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (self->client->inventory[ammo_index] < self->client->max_shells)
		{
			if (BOTLIB_CanTouchItem(self, SHELL_ANUM))
				return SHELL_ANUM;
		}
	}
	if (*primary_weapon == HC_NUM || *secondary_weapon == HC_NUM)
	{
		weapon = FindItem(HC_NAME);
		ammo_item = FindItem(weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (self->client->inventory[ammo_index] < self->client->max_shells)
		{
			if (BOTLIB_CanTouchItem(self, SHELL_ANUM))
				return SHELL_ANUM;
		}
	}
	if (*primary_weapon == SNIPER_NUM || *secondary_weapon == SNIPER_NUM)
	{
		weapon = FindItem(SNIPER_NAME);
		ammo_item = FindItem(weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (self->client->inventory[ammo_index] < self->client->max_sniper_rnds)
		{
			if (BOTLIB_CanTouchItem(self, SNIPER_ANUM))
				return SNIPER_ANUM;
		}
	}

	// Dual MK23
	weapon = FindItem(DUAL_NAME);
	if (weapon)
	{
		ammo_item = FindItem(weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (self->client->inventory[ammo_index] < self->client->max_pistolmags)
			return DUAL_NUM;
	}

	// MK23
	weapon = FindItem(MK23_NAME);
	if (weapon)
	{
		ammo_item = FindItem(weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if (self->client->inventory[ammo_index] < self->client->max_pistolmags)
			return MK23_ANUM;
	}

	return 0;
}

// Do we need a primary weapon?
// Pass parameters input: primary_weapon and secondary_weapon
// Returns a special best fit for the weapons used
int BOTLIB_Need_Special(edict_t* self, int* primary_weapon, int* secondary_weapon)
{
	int special = 0;
	int random = 0;

	// Figure out what special items we have
	for (int i = 0; i < game.num_items; i++)
	{
		if (special)
			break;

		if (self->client->inventory[i] > 0)
		{
			switch (itemlist[i].typeNum)
			{
			case SIL_NUM:
				special = SIL_NUM;
				break;
			case SLIP_NUM:
				special = SLIP_NUM;
				break;
			case BAND_NUM:
				special = BAND_NUM;
				break;
			case KEV_NUM:
				special = KEV_NUM;
				break;
			case LASER_NUM:
				special = LASER_NUM;
				break;
			case HELM_NUM:
				special = HELM_NUM;
				break;
			}
		}
	}

	// No special items, so lets pick one based on our weapons in inventory
	if (special == 0)
	{
		if (*primary_weapon == MP5_NUM || *secondary_weapon == MP5_NUM)
		{
			random = rand() % 5;
			if (random == 0)
				return SIL_NUM;
			else if (random == 1)
				return BAND_NUM;
			else if (random == 2)
				return KEV_NUM;
			else if (random == 3)
				return LASER_NUM;
			else
				return HELM_NUM;
		}
		else if (*primary_weapon == M4_NUM || *secondary_weapon == M4_NUM)
		{
			random = rand() % 5;
			if (random == 1)
				return BAND_NUM;
			else if (random == 2)
				return KEV_NUM;
			else if (random == 3)
				return LASER_NUM;
			else
				return HELM_NUM;
		}
		else if (*primary_weapon == M3_NUM || *secondary_weapon == M3_NUM)
		{
			random = rand() % 4;
			if (random == 0)
				return SLIP_NUM;
			else if (random == 1)
				return BAND_NUM;
			else if (random == 2)
				return KEV_NUM;
			else
				return HELM_NUM;
		}
		else if (*primary_weapon == HC_NUM || *secondary_weapon == HC_NUM)
		{
			random = rand() % 4;
			if (random == 0)
				return SLIP_NUM;
			else if (random == 1)
				return BAND_NUM;
			else if (random == 2)
				return KEV_NUM;
			else
				return HELM_NUM;
		}
		else if (*primary_weapon == SNIPER_NUM || *secondary_weapon == SNIPER_NUM)
		{
			random = rand() % 4;
			if (random == 0)
				return SIL_NUM;
			else if (random == 1)
				return BAND_NUM;
			else if (random == 2)
				return KEV_NUM;
			else
				return HELM_NUM;
		}
		else // Just get a random item
		{
			random = rand() % 6;
			if (random == 0)
				return SIL_NUM;
			else if (random == 1)
				return SLIP_NUM;
			else if (random == 2)
				return BAND_NUM;
			else if (random == 3)
				return KEV_NUM;
			else if (random == 4)
				return LASER_NUM;
			else
				return HELM_NUM;
		}
	}

	return 0; // Don't need a special
}

// Locate weapons and items
int BOTLIB_LocateFloorItem(edict_t* self, int* items_to_get, int items_counter)
{
	qboolean found = false;
	edict_t* items[32]; // Items we find
	int item_nodes[32]; // Keep track of the nodes near those items
	int item_count = 0; // Total items we found
	int base = 1 + game.maxclients + BODY_QUEUE_SIZE;
	edict_t* ent = g_edicts + base;

	vec3_t mins = { -16, -16, -24 };
	vec3_t maxs = { 16, 16, 32 };
	vec3_t bmins = { 0,0,0 };
	vec3_t bmaxs = { 0,0,0 };
	//qboolean outer_break = false; // Flag to break the outer loop

	//for (int i = base; i < globals.num_edicts && !outer_break; i++, ent++)
	for (int i = base; i < globals.num_edicts; i++, ent++)
	{
		if (ent->inuse == false) continue;
		if (!ent->classname) continue;

		switch (ent->typeNum) {
		case MK23_NUM:
		case MP5_NUM:
		case M4_NUM:
		case M3_NUM:
		case HC_NUM:
		case SNIPER_NUM:
		case DUAL_NUM:
		case KNIFE_NUM:
		case GRENADE_NUM:
		case SIL_NUM:
		case SLIP_NUM:
		case BAND_NUM:
		case KEV_NUM:
		case LASER_NUM:
		case HELM_NUM:
		case MK23_ANUM:
		case MP5_ANUM:
		case M4_ANUM:
		case SHELL_ANUM:
		case SNIPER_ANUM:

			// If the item is in our list
			found = false;
			for (int j = 0; j < items_counter; j++)
			{
				if (ent->typeNum == items_to_get[j])
					found = true;
			}

			// Only look at items we want
			if (found && item_count < 32)
			{
				for (int j = 0; j < numnodes; j++)
				{
					VectorAdd(ent->s.origin, mins, bmins); // Update absolute box min/max in the world
					VectorAdd(ent->s.origin, maxs, bmaxs); // Update absolute box min/max in the world

					// If ent is touching a node
					//if (BOTLIB_BoxIntersection(ent->absmin, ent->absmax, nodes[i].absmin, nodes[i].absmax))
					if (BOTLIB_BoxIntersection(bmins, bmaxs, nodes[j].absmin, nodes[j].absmax))
					{
						//trace_t tr = gi.trace(nodes[j].origin, mins, maxs, tv(ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + 0), NULL, MASK_PLAYERSOLID);
						trace_t tr = gi.trace(nodes[j].origin, NULL, NULL, tv(ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] + 0), NULL, MASK_PLAYERSOLID);
						if (tr.fraction == 1.0 && (self->bot.state == BOT_MOVE_STATE_NAV || self->bot.state == BOT_MOVE_STATE_NONE) && BOTLIB_CanGotoNode(self, nodes[j].nodenum, false)) // Check if bot can nav to item
						{
							items[item_count] = ent;
							item_nodes[item_count] = nodes[j].nodenum;
							item_count++;
							//outer_break = true; // Set flag to break the outer loop
							break;
						}
						else
						{
							//Com_Printf("%s %s could not visit node [%d] for item %s [%f %f %f]\n", __func__, self->client->pers.netname, nodes[j].nodenum, ent->classname, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
							//Com_Printf("%s %s could not visit node [%d] for item %s dist[%f]\n", __func__, self->client->pers.netname, nodes[j].nodenum, ent->classname, VectorDistance(self->s.origin, ent->s.origin));
						}
					}
				}
			}
			break;

		default:
			break;
		}
	}

	//Com_Printf("%s %s -----\n\n", __func__, self->client->pers.netname);

	// Find the closest item
	float closest = 999999;
	float dist = 0;
	int node = INVALID;
	ent = NULL;
	for (int i = 0; i < item_count; i++)
	{
		dist = VectorDistance(self->s.origin, items[i]->s.origin);
		if (dist < closest)
		{
			closest = dist;
			ent = items[i];
			node = item_nodes[i];

		}
	}
	if (node != INVALID && ent)
	{
		self->bot.get_item = ent;
		//Com_Printf("%s %s [%d] %s [%f %f %f]\n", __func__, self->client->pers.netname, nodes[node].nodenum, ent->classname, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
		//Com_Printf("%s %s goal item [%s] at node [%d] dist[%f]\n", __func__, self->client->pers.netname, ent->classname, nodes[node].nodenum, VectorDistance(self->s.origin, ent->s.origin));
		return nodes[node].nodenum;
	}
	else
	{
		self->bot.get_item = NULL;
		return INVALID;
	}

	return INVALID;
}

// Does bot need weapons, items, ammo? If so find ground based items that are close.
// Returns the node of the item and sets the item for the bot to walk toward when its nearby
int BOTLIB_GetEquipment(edict_t* self)
{
	int primary_weapon = 0;
	int secondary_weapon = 0;
	//qboolean need_weapon = BOTLIB_Need_Weapons(self, &primary_weapon, &secondary_weapon);
	int ammo_type = BOTLIB_Need_Ammo(self, &primary_weapon, &secondary_weapon);
	int special = BOTLIB_Need_Special(self, &primary_weapon, &secondary_weapon);
	qboolean need_grenades = BOTLIB_Need_Grenades(self);
	qboolean need_dual_mk23 = BOTLIB_Need_Dual_MK23(self);
	//qboolean need_knives = BOTLIB_Need_Knives(self);
	//Com_Printf("%s wep[%d][%d][%d] gren[%d] knife[%d] dual[%d] ammo[%d] special[%d]\n", __func__, need_weapon, primary_weapon, secondary_weapon, need_grenades, need_knives, need_dual_mk23, ammo_type, special);

	int items_to_get[32];
	int items_counter = 0;

	// Acceptable primary weapon choices
	if (primary_weapon == 0)
	{
		items_to_get[0] = MP5_NUM;
		items_to_get[1] = M4_NUM;
		items_to_get[2] = M3_NUM;
		items_to_get[3] = HC_NUM;
		items_to_get[4] = SNIPER_NUM;
		items_counter += 5;
	}
	// Acceptable secondary weapon choices (if bandolier equipped)
	qboolean band = INV_AMMO(self, BAND_NUM) ? 1 : 0;
	if (band && primary_weapon && secondary_weapon == 0)
	{
		// Only add a weapon if not already in our inventory
		if (primary_weapon != MP5_NUM)
		{
			items_to_get[items_counter] = MP5_NUM;
			items_counter++;
		}
		if (primary_weapon != M4_NUM)
		{
			items_to_get[items_counter] = M4_NUM;
			items_counter++;
		}
		if (primary_weapon != M3_NUM)
		{
			items_to_get[items_counter] = M3_NUM;
			items_counter++;
		}
		if (primary_weapon != HC_NUM)
		{
			items_to_get[items_counter] = HC_NUM;
			items_counter++;
		}
		if (primary_weapon != SNIPER_NUM)
		{
			items_to_get[items_counter] = SNIPER_NUM;
			items_counter++;
		}
	}
	if (ammo_type)
	{
		items_to_get[items_counter] = ammo_type;
		items_counter++;

		// Because MK23 ammo can come from clips or pistols, try look for both
		if (ammo_type == MK23_ANUM)// && BOTLIB_CanTouchItem(self, DUAL_NUM))
		{
			items_to_get[items_counter] = DUAL_NUM;
			items_counter++;
		}
		if (ammo_type == DUAL_NUM)// && BOTLIB_CanTouchItem(self, MK23_ANUM))
		{
			items_to_get[items_counter] = MK23_ANUM;
			items_counter++;
		}

	}
	if (special)
	{
		items_to_get[items_counter] = special;
		items_counter++;
	}
	if (need_grenades)
	{
		items_to_get[items_counter] = GRENADE_NUM;
		items_counter++;
	}
	if (need_dual_mk23)
	{
		items_to_get[items_counter] = DUAL_NUM;
		items_counter++;
	}

	// Debug output
	if (0 && items_counter)
	{
		Com_Printf("%s items_to_get [", __func__);
		for (int i = 0; i < items_counter; i++)
		{
			gitem_t* item = GET_ITEM(items_to_get[i]);
			Com_Printf(" %s ", item->classname);
		}
		Com_Printf("]\n");
	}

	return BOTLIB_LocateFloorItem(self, items_to_get, items_counter);
}
//rekkie -- collecting weapons, items, ammo -- e
