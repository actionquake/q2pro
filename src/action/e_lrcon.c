#include "g_local.h"

zarkext_t zarkext;

void P_ZarkEnt(edict_t* ent)
{
    gi.dprintf("ZarkEnt called by %s\n\n\n\n", ent->client->pers.netname);
}

void whatever_init_function(void)
{
    zarkext.zark_ent = P_ZarkEnt;
}

