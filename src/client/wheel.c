/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "client.h"
#include "common/loc.h"

static cvar_t *wc_screen_frac_y;
static cvar_t *wc_timeout;
static cvar_t *wc_lock_time;

static int sort_wheel_powerups(const void *a, const void *b)
{
    const cl_wheel_powerup_t *pa = a;
    const cl_wheel_powerup_t *pb = b;

    if (pa->sort_id == pb->sort_id)
        return pa->item_index - pb->item_index;

    return pa->sort_id - pb->sort_id;
}

static int sort_wheel_weapons(const void *a, const void *b)
{
    const cl_wheel_weapon_t *pa = a;
    const cl_wheel_weapon_t *pb = b;

    if (pa->sort_id == pb->sort_id)
        return pa->item_index - pb->item_index;

    return pa->sort_id - pb->sort_id;
}

static inline void set_compressed_integer(size_t bits_per_value, uint16_t *start, uint8_t id, uint16_t count)
{
    uint16_t bit_offset = bits_per_value * id;
    uint16_t byte = bit_offset / 8;
    uint16_t bit_shift = bit_offset % 8;
    uint16_t mask = (BIT(bits_per_value) - 1) << bit_shift;
    uint16_t *base = (uint16_t *) ((uint8_t *) start + byte);
    *base = (*base & ~mask) | ((count << bit_shift) & mask);
}

static inline uint16_t get_compressed_integer(size_t bits_per_value, uint16_t *start, uint8_t id)
{
    uint16_t bit_offset = bits_per_value * id;
    uint16_t byte = bit_offset / 8;
    uint16_t bit_shift = bit_offset % 8;
    uint16_t mask = (BIT(bits_per_value) - 1) << bit_shift;
    uint16_t *base = (uint16_t *) ((uint8_t *) start + byte);
    return (*base & mask) >> bit_shift;
}

static inline void G_SetAmmoStat(uint16_t *start, uint8_t ammo_id, uint16_t count)
{
    set_compressed_integer(NUM_BITS_FOR_AMMO, start, ammo_id, count);
}

static inline uint16_t G_GetAmmoStat(uint16_t *start, uint8_t ammo_id)
{
    return get_compressed_integer(NUM_BITS_FOR_AMMO, start, ammo_id);
}

static inline void G_SetPowerupStat(uint16_t *start, uint8_t powerup_id, uint16_t count)
{
    set_compressed_integer(NUM_BITS_PER_POWERUP, start, powerup_id, count);
}

static inline uint16_t G_GetPowerupStat(uint16_t *start, uint8_t powerup_id)
{
    return get_compressed_integer(NUM_BITS_PER_POWERUP, start, powerup_id);
}

static void CL_Carousel_Close(void)
{
    cl.carousel.awaiting_close = false;
    cl.carousel.open = false;
}

// populate slot list with stuff we own.
// runs every frame and when we open the carousel.
static bool CL_Carousel_Populate(void)
{
    int i;

    cl.carousel.num_slots = 0;

    int owned = cgame->GetOwnedWeaponWheelWeapons(&cl.frame.ps);

    for (i = 0; i < cl.wheel_data.num_weapons; i++) {
        if (!(owned & BIT(i)))
            continue;

        cl.carousel.slots[cl.carousel.num_slots].data_id = i;
        cl.carousel.slots[cl.carousel.num_slots].has_ammo = cl.wheel_data.weapons[i].ammo_index == -1 || cgame->GetWeaponWheelAmmoCount(&cl.frame.ps, cl.wheel_data.weapons[i].ammo_index);
        cl.carousel.slots[cl.carousel.num_slots].item_index = cl.wheel_data.weapons[i].item_index;
        cl.carousel.slots[cl.carousel.num_slots].is_powerup = false;
        cl.carousel.num_slots++;
    }

    // todo: sort by sort_id

    // todo: cl.wheel.powerups

    if (!cl.carousel.num_slots)
        return false;

    // check that we still have the item being selected
    if (cl.carousel.selected == -1) {
        cl.carousel.selected = cl.carousel.slots[0].item_index;
    } else {
        for (i = 0; i < cl.carousel.num_slots; i++)
            if (cl.carousel.slots[i].item_index == cl.carousel.selected)
                break;
    }

    if (i == cl.carousel.num_slots) {
        // TODO: maybe something smarter?
        return false;
    }

    return true;
}

static void CL_Carousel_Open(void)
{
    cl.carousel.open = true;

    if (!cl.carousel.awaiting_close) {
        cl.carousel.selected = (cl.frame.ps.stats[STAT_ACTIVE_WEAPON] == -1) ? -1 : cl.wheel_data.weapons[cl.frame.ps.stats[STAT_ACTIVE_WEAPON]].item_index;
    }

    cl.carousel.awaiting_close = false;

    if (!CL_Carousel_Populate()) {
        CL_Carousel_Close();
    }
}

#define CAROUSEL_ICON_SIZE (24 + 2)

static void R_DrawPicShadow(int x, int y, qhandle_t pic, int shadow_offset)
{
    R_SetColor(U32_BLACK);
    R_DrawPic(x + shadow_offset, y + shadow_offset, pic);
    R_SetColor(U32_WHITE);
    R_DrawPic(x, y, pic);
}

static void R_DrawStretchPicShadow(int x, int y, int w, int h, qhandle_t pic, int shadow_offset)
{
    R_SetColor(U32_BLACK);
    R_DrawStretchPic(x + shadow_offset, y + shadow_offset, w, h, pic);
    R_SetColor(U32_WHITE);
    R_DrawStretchPic(x, y, w, h, pic);
}

void CL_Carousel_Draw(void)
{
    if (!cl.carousel.open)
        return;

    if (!CL_Carousel_Populate()) {
        CL_Carousel_Close();
        return;
    }

    int carousel_w = cl.carousel.num_slots * CAROUSEL_ICON_SIZE;
    int center_x = scr.hud_width / 2;
    int carousel_x = center_x - (carousel_w / 2);
    int carousel_y = (int) (scr.hud_height * wc_screen_frac_y->value);
    
    for (int i = 0; i < cl.carousel.num_slots; i++, carousel_x += CAROUSEL_ICON_SIZE) {
        bool selected = cl.carousel.selected == cl.carousel.slots[i].item_index;
        const cl_wheel_weapon_t *weap = &cl.wheel_data.weapons[cl.carousel.slots[i].data_id];
        const cl_wheel_icon_t *icons = &weap->icons;

        R_DrawPicShadow(carousel_x, carousel_y, selected ? icons->selected : icons->wheel, 2);
        
        if (selected) {
            R_DrawPic(carousel_x - 1, carousel_y - 1, scr.carousel_selected);
            
            char localized[CS_MAX_STRING_LENGTH];

            // TODO: cache localized item names in cl somewhere.
            // make sure they get reset of language is changed.
            Loc_Localize(cl.configstrings[cl.csr.items + cl.carousel.slots[i].item_index], false, NULL, 0, localized, sizeof(localized));

            SCR_DrawString(center_x, carousel_y - 16, UI_CENTER | UI_DROPSHADOW, localized);
        }

        if (weap->ammo_index >= 0) {
            int count = cgame->GetWeaponWheelAmmoCount(&cl.frame.ps, weap->ammo_index);
            uint32_t color = count <= weap->quantity_warn ? U32_RED : U32_WHITE;

            // TODO: scale ammo
            R_SetColor(color);
            SCR_DrawString(carousel_x + 12, carousel_y, UI_DROPSHADOW | UI_CENTER, va("%i", count));
            R_SetColor(U32_WHITE);
        }
    }
}

void CL_Carousel_ClearInput(void)
{
    if (cl.carousel.open && cl.carousel.awaiting_close) {
        cl.carousel.open = false;
        cl.carousel.close_time = cls.realtime + (cl.sv_frametime * 2);
    }
}

void CL_Carousel_Input(void)
{
    if (!cl.carousel.open) {
        if (cl.carousel.awaiting_close && cls.realtime >= cl.carousel.close_time)
            cl.carousel.awaiting_close = false;

        return;
    }

    if (!CL_Carousel_Populate()) {
        CL_Carousel_Close();
        return;
    }

    // always holster while open
    cl.cmd.buttons |= BUTTON_HOLSTER;

    if (cls.realtime >= cl.carousel.close_time || (cl.cmd.buttons & BUTTON_ATTACK)) {

        // already using this weapon
        if (cl.carousel.selected == cl.wheel_data.weapons[cl.frame.ps.stats[STAT_ACTIVE_WEAPON]].item_index) {
            CL_Carousel_Close();
            return;
        }

        // switch
        CL_ClientCommand(va("use_index_only %i\n", cl.carousel.selected));
        cl.carousel.awaiting_close = true;

        cl.weapon_lock_time = cl.time + wc_lock_time->integer;
    }
}

void CL_Wheel_WeapNext(void)
{
    if (!cl.carousel.open) {
        CL_Carousel_Open();
    } else if (!CL_Carousel_Populate()) {
        CL_Carousel_Close();
        return;
    }

    for (int i = 0; i < cl.carousel.num_slots; i++)
        if (cl.carousel.slots[i].item_index == cl.carousel.selected) {
            cl.carousel.selected = cl.carousel.slots[i == cl.carousel.num_slots - 1 ? 0 : (i + 1)].item_index;
            break;
        }

    cl.carousel.close_time = cls.realtime + wc_timeout->integer;
}

void CL_Wheel_WeapPrev(void)
{
    if (!cl.carousel.open) {
        CL_Carousel_Open();
    } else if (!CL_Carousel_Populate()) {
        CL_Carousel_Close();
        return;
    }

    for (int i = 0; i < cl.carousel.num_slots; i++)
        if (cl.carousel.slots[i].item_index == cl.carousel.selected) {
            cl.carousel.selected = cl.carousel.slots[i == 0 ? cl.carousel.num_slots - 1 : (i - 1)].item_index;
            break;
        }

    cl.carousel.close_time = cls.realtime + wc_timeout->integer;
}

void CL_Wheel_Open(bool powerup)
{
    cl.wheel.open = true;
    Vector2Clear(cl.wheel.position);
}

void CL_Wheel_Close(void)
{
    cl.wheel.open = false;
}

void CL_Wheel_Input(int x, int y)
{
    if (!cl.wheel.open)
        return;

    cl.wheel.position[0] += x;
    cl.wheel.position[1] += y;
    
    // clamp position
    float distance = Vector2Length(cl.wheel.position);

    float inner_size = scr.wheel_size * 0.64f;

    if (distance > inner_size / 2) {
        distance = 1.f / distance;
        Vector2Scale(cl.wheel.position, distance, cl.wheel.position);
        Vector2Scale(cl.wheel.position, inner_size / 2, cl.wheel.position);
    }
}

void CL_Wheel_Draw(void)
{
    if (!cl.wheel.open)
        return;
    
    int center_x = (r_config.width / 2) + (r_config.width / 4);
    int center_y = r_config.height / 2;

    R_SetScale(1);

    R_DrawPic(center_x - (scr.wheel_size / 2), center_y - (scr.wheel_size / 2), scr.wheel_circle);

    vec2_t dir = { 0, 0 };
    float len = Vector2Length(cl.wheel.position);

    if (len) {
        float invlen = 1.f / len;
        Vector2Scale(cl.wheel.position, invlen, dir);
    }

    bool can_select = false;

    if (len > 140) {
        can_select = true;
    }

    float slice_deg = ((M_PI * 2) / cl.wheel_data.num_weapons);
    float slice_sin = cosf(slice_deg / 2);

    for (int i = 0; i < cl.wheel_data.num_weapons; i++) {
        float ang = slice_deg * i;
        float x = sinf(ang);
        float y = -cosf(ang);

        vec2_t d = { x, y };
        vec2_t p;
        Vector2Scale(d, (scr.wheel_size / 2) * 0.525f, p);

        float dot = Dot2Product(dir, d);

        bool selected = can_select && dot > slice_sin;

        float scale = 1.0f + ((dot + 1.0f) / 2.f);

        if (selected)
            scale = 2.5f;

        int size = 12 * scale;

        R_DrawStretchPicShadow(center_x + p[0] - size, center_y + p[1] - size, size * 2, size * 2, selected ? cl.wheel_data.weapons[i].icons.selected : cl.wheel_data.weapons[i].icons.wheel, 4);
    }

    R_SetColor(MakeColor(255, 255, 255, 127));
    R_DrawPic(center_x + (int) cl.wheel.position[0] - (scr.wheel_button_size / 2), center_y + (int) cl.wheel.position[1] - (scr.wheel_button_size / 2), scr.wheel_button);
}

void CL_Wheel_Precache(void)
{
    scr.carousel_selected = R_RegisterPic("carousel/selected");
    scr.wheel_circle = R_RegisterPic("/gfx/weaponwheel.png");
    R_GetPicSize(&scr.wheel_size, &scr.wheel_size, scr.wheel_circle);
    scr.wheel_button = R_RegisterPic("/gfx/wheelbutton.png");
    R_GetPicSize(&scr.wheel_button_size, &scr.wheel_button_size, scr.wheel_button);
}

void CL_Wheel_Init(void)
{
    wc_screen_frac_y = Cvar_Get("wc_screen_frac_y", "0.72", 0);
    wc_timeout = Cvar_Get("wc_timeout", "400", 0);
    wc_lock_time = Cvar_Get("wc_lock_time", "300", 0);
}