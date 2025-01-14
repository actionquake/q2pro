/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2008 Andrey Nazarov

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

#pragma once

#include "shared/list.h"
#include "common/error.h"
#include "system/hunk.h"
#include "format/bsp.h"

// maximum size of a PVS row, in bytes
#define VIS_MAX_BYTES   (MAX_MAP_CLUSTERS >> 3)

// take advantage of 64-bit systems
#define VIS_FAST_LONGS(bsp) \
    (((bsp)->visrowsize + sizeof(size_t) - 1) / sizeof(size_t))

#if USE_CLIENT

enum {
    FOOTSTEP_ID_DEFAULT,
    FOOTSTEP_ID_LADDER,
    FOOTSTEP_RESERVED_COUNT
};

#endif

typedef struct mtexinfo_s {  // used internally due to name len probs //ZOID
    csurface_t          c;
    char                name[MAX_TEXNAME];

#if USE_REF
    vec3_t              axis[2];
    vec2_t              offset;
    struct image_s      *image; // used for texturing
    struct mtexinfo_s   *next; // used for animation
    int                 numframes;
#endif
#if USE_CLIENT
    char                material[16];
    int                 step_id;
#endif
} mtexinfo_t;

typedef struct {
    vec3_t      point;
} mvertex_t;

typedef struct {
    uint32_t    v[2];
} medge_t;

typedef struct {
    uint32_t    edge: 31;
    uint32_t    vert:  1;
} msurfedge_t;

#define SURF_TRANS_MASK (SURF_TRANS33 | SURF_TRANS66)
#define SURF_COLOR_MASK (SURF_TRANS_MASK | SURF_WARP)

#define SURF_NOLM_MASK_REMASTER     (SURF_SKY | SURF_NODRAW)
#define SURF_NOLM_MASK_DEFAULT      (SURF_COLOR_MASK | SURF_FLOWING | SURF_NOLM_MASK_REMASTER)

#define DSURF_PLANEBACK     1

typedef struct mface_s {
    msurfedge_t     *firstsurfedge;
    cplane_t        *plane;

    byte            *lightmap;
    byte            styles[MAX_LIGHTMAPS];
    byte            numstyles;

    byte            hash;
    uint16_t        numsurfedges;

    mtexinfo_t      *texinfo;
    vec3_t          lm_axis[2];
    vec2_t          lm_offset;
    vec2_t          lm_scale;
    uint16_t        lm_width;
    uint16_t        lm_height;

    int             drawflags; // DSURF_PLANEBACK, etc
    int             statebits;
    int             firstvert;
    uint16_t        light_s, light_t;
    float           stylecache[MAX_LIGHTMAPS];

    unsigned        drawframe;
    unsigned        dlightframe;
    uint64_t        dlightbits;

    struct lightmap_s   *light_m;
    struct entity_s     *entity;
    struct mface_s      *next;
} mface_t;

typedef struct mnode_s {
    /* ======> */
    cplane_t            *plane;     // never NULL to differentiate from leafs
#if USE_REF
    vec3_t              mins;
    vec3_t              maxs;

    unsigned            visframe;
#endif
    struct mnode_s      *parent;
    /* <====== */

    struct mnode_s      *children[2];

#if USE_REF
    int                 numfaces;
    mface_t             *firstface;
#endif
} mnode_t;

typedef struct {
    cplane_t            *plane;
    mtexinfo_t          *texinfo;
} mbrushside_t;

typedef struct {
    int                 contents;
    int                 numsides;
    mbrushside_t        *firstbrushside;
    unsigned            checkcount;         // to avoid repeated testings
} mbrush_t;

typedef struct {
    /* ======> */
    cplane_t            *plane;     // always NULL to differentiate from nodes
#if USE_REF
    vec3_t              mins;
    vec3_t              maxs;

    unsigned            visframe;
#endif
    struct mnode_s      *parent;
    /* <====== */

    int             contents;
    int             cluster;
    int             area;
    int             numleafbrushes;
    mbrush_t        **firstleafbrush;
#if USE_REF
    mface_t         **firstleafface;
    int             numleaffaces;
#endif
} mleaf_t;

typedef struct {
    unsigned    portalnum;
    unsigned    otherarea;
} mareaportal_t;

typedef struct {
    int             numareaportals;
    mareaportal_t   *firstareaportal;
    unsigned        floodvalid;
} marea_t;

typedef struct {
    vec3_t          mins, maxs;
    vec3_t          origin;        // for sounds or lights
    mnode_t         *headnode;

#if USE_REF
    float           radius;

    int             numfaces;
    mface_t         *firstface;

    unsigned        drawframe;
#endif
} mmodel_t;


//rekkie -- surface data -- s
// Surface data structure to store information about surfaces: their faces, normals, edge verts, etc.
#define MAX_FACE_VERTS 64
#define MAX_FACE_CONNECTIONS 1024 //(MAX_SAVED_VERTS / 2)

// Face connection types - denotes how faces are connected to each other, and how the bot should move between them
typedef enum face_connection_type_e {
    FACE_CONN_NONE,         // No connection
    FACE_CONN_DIRECT,       // Two faces directly touch by a shared edge
    FACE_CONN_INDIRECT,     // Two faces are indirectly connected due to gaps between the two faces (such as the urban jump from roof to roof)
    FACE_CONN_LEDGE,        // The current edge is a ledge with a drop that will cause fall damage or death
    FACE_CONN_WATER,        // The current edge is a ledge that drops to water below
    FACE_CONN_DROP,		    // From the current face drop to the face below
    FACE_CONN_STEP,         // Face is step height (up or down - bidirectional)
    FACE_CONN_JUMP          // Jump to face (up or down - bidirectional)
} face_connection_type_t;

// Face movement types - gives an indication as to the type of position this leads to
typedef enum face_move_type_e {
    FACE_MOVE_NONE,         // No type
    FACE_MOVE_SAFE,         // Safe move: this interconnects with flat ground
    FACE_MOVE_CAUTION,      // Safe move: this interconnects to a step/stairs, drop (only down - one way), or jump (up or down - bidirectional), drop into water below
    FACE_MOVE_STOP,         // Caution move: this ends at dangerous ledge (damage or death), or possibly a good sniping spot :)
    FACE_MOVE_GROUND,       // The next interconnect is the same height
    FACE_MOVE_UP,           // The next interconnect is above
    FACE_MOVE_DOWN          // The next interconnect is below
} face_move_type_t;

// Edge offset types - centered, offset along its length, or moved inward (i.e. away from a wall)
typedef enum edge_offset_type_e {
    EDGE_OFFSET_NONE,       // No type
    EDGE_OFFSET_CENTER,     // Offset position: center edge
    EDGE_OFFSET_LENGTH,     // Offset position: along its length (somewhere between the two vectors (v1 & v2) that constitute an edge)
    EDGE_OFFSET_INNER       // Offset position: moved inward (inside the face), possibly to move away from a wall or obstacle
} edge_offset_type_t;

// The type of face
typedef enum facetype_e {
    FACETYPE_NONE = 0,      // No type
    FACETYPE_IGNORED = 1,   // Ignored surface (could be a tiny face, or malformed face, etc)
    FACETYPE_WALK = 2,      // Walkable surface
    FACETYPE_TINYWALK = 4,  // Walkable tiny surface
    FACETYPE_WALL = 8,      // Wall surface
    FACETYPE_LADDER = 16,    // Ladder surface
    FACETYPE_SKY = 32,       // Sky surface
    FACETYPE_ROOF = 64,      // Roof surface
    FACETYPE_WATER = 128,     // Water surface
    FACETYPE_DAMAGE = 256     // Hurt, lava, acid, etc.
} facetype_t;

// Nodes or locations within the face
typedef struct surface_node_s
{
    qboolean init;      // 
    int type;           // Face connection type (enum face_connection_type_t)
    int move;           // Face movement type (enum face_move_type_t)
    int facenum;        // The remote face number (if any) that is connected to this face
    float dropheight;   // If the edge is a ledge, this is its drop height, otherwise 0
    vec3_t start;       // The start position of the connection point within the face (such as the center of the polygon)
    vec3_t end;         // The end position of the connection point (can be outside of the face)
} surface_node_t;

// Ledge data
typedef struct ledge_data_s
{
    qboolean is_ledge; // Flag if the edge is a ledge
    qboolean is_wall; // Flag if edge is a wall
    vec3_t normal; // Surface normal that was hit
    vec3_t v1;      // Adjusted edge v1
    vec3_t v2;      // Adjusted edge v2
    vec3_t startpos; // Start location
    vec3_t endpos; // Hit location
    float height; // Height difference from ledge to endpos, if zero then no ledge

    // Debug
    byte hit_side;      // If trace hit: none=0, left=1, right=2
    vec3_t left_start;  // Start trace left
    vec3_t left_end;    // End trace left
    vec3_t right_start; // Start trace right
    vec3_t right_end;   // End trace right
} ledge_data_t;

typedef struct surface_data_s
{
    // ========================================================
    // Raw BSP Data
    // Extracted from the BSP struct - Do not modify!
    // ========================================================

    int drawflags; // Drawflags -- DSURF_PLANEBACK
    char texture[MAX_TEXNAME]; // Texture name
    vec3_t normal; // Surface normal
    int contents; // Contents of the surface
    #if USE_REF
    mface_t* surf; // Surface pointer to the BSP struct
    #endif
    // Vertex
    vec3_t first_vert; // First vert - used when calculating triangles (i.e. gl_showtris "1")
    int num_verts; // Number of verts (edges)
    vec3_t verts[MAX_FACE_VERTS]; // Verts (inc lateral)

    // Positions
    vec3_t edge_center[MAX_FACE_CONNECTIONS]; // The center of each edge
    vec3_t edge_center_stepsize[MAX_FACE_CONNECTIONS]; // The edge_center moved up by 18 (STEPSIZE) units

    // ========================================================
    // End of Raw BSP Data
    // ========================================================
    

    // ========================================================
    // Custom BSP Data - Safe to modify
    // ========================================================

    short int facenum;	// The set of surfaces we've chosen to work with, i.e. walls are generally excluded, etc
    facetype_t face_type; // The type of face, walkable, wall, ladder, water, damage...

    // Aligned verts are connected edges that are parallel (same direction) combined into a single edge to form a straight line
    // Essentially we're removing all the inner verts, leaving only the outer verts, and thus forming a straight line between them
    // We do this so we can calculate such things as finding the center of a polygon
    // 
    // Normal edges:
    // v1, v2, v3, v4 are vertices
    // edge1 is parallel to edge2
    // 
    // v1     edge1      v2 v3     edge2    v4                      
    // | ----------------- | ----------------|
    //
    // Combined edges:
    // v2 and v3 are removed and edge1 and edge2 are combined into a single edge
    // 
    // v1                edge1               v2                      
    // | ------------------------------------|
    //
    int num_aligned_verts; // Number of aligned verts 
    vec3_t aligned_verts[MAX_FACE_VERTS]; // Aligned verts

    // Modified Edges
    // This will either be the center of the edge, or failing that a position somewhere along the edge that fits the player hitbox (because sometimes the center edge cannot fit the player hitbox due to other brushes obstructing)
    vec3_t edge_valid_pos[MAX_FACE_CONNECTIONS]; // Valid player hitbox position along the edge
    vec3_t edge_valid_stepsize[MAX_FACE_CONNECTIONS]; // The edge_valid_pos moved up by 18 (STEPSIZE) units
    edge_offset_type_t edge_offset_type[MAX_FACE_CONNECTIONS]; // Flag the type of offset

    // Detected ledges
    ledge_data_t ledge[MAX_FACE_CONNECTIONS];

    // Positions
    vec3_t center_poly; // The center of the ladder's polygon face, located directly on the surface
    vec3_t center_poly_32_units; // The center of the ladder face, moved away from the surface normal by 32 units (full player width)
    vec3_t aligned_edge_center[MAX_FACE_CONNECTIONS]; // The center of each aligned edge
    vec3_t aligned_edge_center_stepsize[MAX_FACE_CONNECTIONS]; // The aligned_edge_center moved up by 18 (STEPSIZE) units

    float volume; // Surface volume

    // The min and max lengths from edge to edge within a face
    float min_length;
    float max_length;

    // Connections within the face, and nearest connected faces
    int snode_counter; // How many faces are connected
    surface_node_t snodes[MAX_FACE_CONNECTIONS]; // Surface nodes

} surface_data_t;
//extern surface_data_t* surface_data_faces;

typedef struct nav_s
{
    surface_data_t* surface_data_faces;
    unsigned surface_data_checksum; // Checksum of all surface data faces based on current map checksum
    short int faces_total; // Current number of faces in the array
    short int ignored_faces_total; // Ignored faces: FACETYPE_IGNORED, FACETYPE_NONE
} nav_t;
extern nav_t* nav_;
void MoveAwayFromNormal(const int drawflags, const vec3_t normal, vec3_t out, const float distance);
//rekkie -- surface data -- e


//rekkie -- debug drawing -- s
#define DEBUG_DRAWING 1
#if DEBUG_DRAWING
// Drawing functions
typedef struct debug_draw_s
{
    // Turns drawing on/off - reduce overhead when off
    qboolean arrows_inuse;
    qboolean boxes_inuse;
    qboolean crosses_inuse;
    qboolean strings_inuse;

    void* DrawSelection; // DrawSelection function pointer
    void* DrawString; // DrawString function pointer
    void* DrawCross; // DrawCross function pointer
    void* DrawBox; // DrawCross function pointer
    void* DrawArrow; // DrawArrow function pointer
    int draw_arrow_num; // Current arrow being drawn: increments until MAX_DRAW_ARROWS, then resets to zero
} debug_draw_t;

void GL_DrawLine(vec3_t verts, const int num_points, const uint32_t* colors, const float line_width, qboolean occluded);

void GL_DrawStrings(void);
void GL_DrawString(vec3_t origin, const char* string, const uint32_t color, qboolean occluded);
typedef struct draw_string_s
{
    qboolean inuse;
    int time;
    vec3_t origin;
    char string[1024];
    uint32_t color;
    qboolean occluded;
} draw_string_t;
#define MAX_DRAW_STRINGS 4096
void DrawString(int number, vec3_t origin, const char* string, const uint32_t color, const int time, qboolean occluded);



void GL_DrawCross(vec3_t origin, qboolean occluded);
typedef struct draw_crosses_s
{
    qboolean inuse;
    int time;
    vec3_t origin;
    qboolean occluded;
} draw_crosses_t;
#define MAX_DRAW_CROSSES 4096
void DrawCross(int number, vec3_t origin, int time, qboolean occluded);

void GL_DrawBox(vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, qboolean occluded);
typedef struct draw_boxes_s
{
    qboolean inuse;
    int time;
    vec3_t origin;
    uint32_t color;
    vec3_t mins;
    vec3_t maxs;
    qboolean occluded;
} draw_boxes_t;
#define MAX_DRAW_BOXES 6144
extern draw_boxes_t draw_boxes[MAX_DRAW_BOXES];
void DrawBox(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded);

void GL_AddDrawArrow(vec3_t start, vec3_t end, uint32_t color, float line_width, qboolean occluded);
typedef struct draw_arrows_s
{
    qboolean inuse;
    int time;
    vec3_t start;
    vec3_t end;
    uint32_t color;
    float line_width;
    qboolean occluded;
} draw_arrows_t;
#define MAX_DRAW_ARROWS 8192
extern draw_arrows_t draw_arrows[MAX_DRAW_ARROWS];
void DrawArrow(int number, vec3_t start, vec3_t end, uint32_t color, float line_width, int time, qboolean occluded);

// OpenGL selection square
void GL_DrawSelectionSquare(vec3_t start, vec3_t end, float min, float max, uint32_t color, float line_width, qboolean occluded);
typedef struct draw_selection_s
{
    qboolean inuse;
    int time;
    vec3_t start;
    vec3_t end;
    float min;
    float max;
    uint32_t color;
    float line_width;
    qboolean occluded;
} draw_selection_t;
extern draw_selection_t draw_selection;
void DrawSelection(vec3_t start, vec3_t end, float min, float max, uint32_t color, float line_width, int time, qboolean occluded);

#endif
//rekkie -- debug drawing -- e

//#if USE_REF

typedef struct {
    uint32_t point[3];
    uint32_t children[8];
} lightgrid_node_t;

typedef struct {
    byte style;
    byte rgb[3];
} lightgrid_sample_t;

typedef struct {
    uint32_t mins[3];
    uint32_t size[3];
    uint32_t numsamples;
    uint32_t firstsample;
} lightgrid_leaf_t;

typedef struct {
    vec3_t scale;
    vec3_t mins;
    uint32_t size[3];
    uint32_t numstyles;
    uint32_t numnodes;
    uint32_t numleafs;
    uint32_t numsamples;
    uint32_t rootnode;
    lightgrid_node_t *nodes;
    lightgrid_leaf_t *leafs;
    lightgrid_sample_t *samples;
} lightgrid_t;

//#endif

typedef struct {
    list_t      entry;
    int         refcount;

    unsigned    checksum;

    memhunk_t   hunk;

    int             numbrushsides;
    mbrushside_t    *brushsides;

    int             numtexinfo;
    mtexinfo_t      *texinfo;

    int             numplanes;
    cplane_t        *planes;

    int             numnodes;
    mnode_t         *nodes;

    int             numleafs;
    mleaf_t         *leafs;

    int             numleafbrushes;
    mbrush_t        **leafbrushes;

    int             nummodels;
    mmodel_t        *models;

    int             numbrushes;
    mbrush_t        *brushes;

    int             numvisibility;
    int             visrowsize;
    dvis_t          *vis;

    int             numentitychars;
    char            *entitystring;

    int             numareas;
    marea_t         *areas;

    int             numportals;     // largest portal number used plus one
    int             numareaportals; // size of the array below
    mareaportal_t   *areaportals;

    int             numfaces;
    mface_t         *faces;

    int             numleaffaces;
    mface_t         **leaffaces;

    int             numlightmapbytes;
    byte            *lightmap;

    int             numvertices;
    mvertex_t       *vertices;

    int             numedges;
    medge_t         *edges;

    int             numsurfedges;
    msurfedge_t     *surfedges;

//#if USE_REF
    lightgrid_t     lightgrid;

    qboolean        lm_decoupled;
//#endif
    qboolean        extended;

    char            name[1];
} bsp_t;

int BSP_Load(const char *name, bsp_t **bsp_p);
void BSP_Free(bsp_t *bsp);
const char *BSP_ErrorString(int err);

#if USE_CLIENT
int BSP_LoadMaterials(bsp_t *bsp);
#endif

#if USE_REF
typedef struct {
    mface_t     *surf;
    cplane_t    plane;
    float       s, t;
    float       fraction;
} lightpoint_t;

void BSP_LightPoint(lightpoint_t *point, const vec3_t start, const vec3_t end, const mnode_t *headnode, int nolm_mask);
void BSP_TransformedLightPoint(lightpoint_t *point, const vec3_t start, const vec3_t end,
                               const mnode_t *headnode, int nolm_mask, const vec3_t origin, const vec3_t angles);

const lightgrid_sample_t *BSP_LookupLightgrid(const lightgrid_t *grid, const uint32_t point[3]);
#endif

byte *BSP_ClusterVis(const bsp_t *bsp, byte *mask, int cluster, int vis);
const mleaf_t *BSP_PointLeaf(const mnode_t *node, const vec3_t p);
const mmodel_t *BSP_InlineModel(const bsp_t *bsp, const char *name);

void BSP_Init(void);
