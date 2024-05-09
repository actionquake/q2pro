/*
Copyright (C) 2003-2006 Andrey Nazarov

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

/*
 * gl_main.c
 *
 */

#include "gl.h"

glRefdef_t glr;
glStatic_t gl_static;
glConfig_t gl_config;
statCounters_t  c;

entity_t gl_world;

refcfg_t r_config;

int registration_sequence;

// regular variables
cvar_t *gl_partscale;
cvar_t *gl_partstyle;
cvar_t *gl_celshading;
cvar_t *gl_dotshading;
cvar_t *gl_shadows;
cvar_t *gl_modulate;
cvar_t *gl_modulate_world;
cvar_t *gl_coloredlightmaps;
cvar_t *gl_brightness;
cvar_t *gl_dynamic;
cvar_t *gl_dlight_falloff;
cvar_t *gl_modulate_entities;
cvar_t *gl_doublelight_entities;
cvar_t *gl_glowmap_intensity;
cvar_t *gl_fontshadow;
cvar_t *gl_shaders;
#if USE_MD5
cvar_t *gl_md5_load;
cvar_t *gl_md5_use;
#endif
cvar_t *gl_waterwarp;
cvar_t *gl_swapinterval;

// development variables
cvar_t *gl_znear;
cvar_t *gl_drawworld;
cvar_t *gl_drawentities;
cvar_t *gl_drawsky;
cvar_t *gl_showtris;
cvar_t* gl_showedges; //rekkie -- gl_showedges
cvar_t *gl_showorigins;
cvar_t *gl_showtearing;
#if USE_DEBUG
cvar_t *gl_showstats;
cvar_t *gl_showscrap;
cvar_t *gl_nobind;
cvar_t *gl_test;
#endif
cvar_t *gl_cull_nodes;
cvar_t *gl_cull_models;
cvar_t *gl_clear;
cvar_t *gl_finish;
cvar_t *gl_novis;
cvar_t *gl_lockpvs;
cvar_t *gl_lightmap;
cvar_t *gl_fullbright;
cvar_t *gl_vertexlight;
cvar_t *gl_lightgrid;
cvar_t *gl_polyblend;
cvar_t *gl_showerrors;
//rekkie -- Attach model to player -- s
cvar_t* gl_vert_diff;
//rekkie -- Attach model to player -- e

draw_arrows_t draw_arrows[MAX_DRAW_ARROWS];
draw_boxes_t draw_boxes[MAX_DRAW_BOXES];
draw_crosses_t draw_crosses[MAX_DRAW_CROSSES];
nav_t* nav_;

// ==============================================================================

static const vec_t quad_tc[8] = { 0, 1, 0, 0, 1, 1, 1, 0 };

static void GL_SetupFrustum(void)
{
    vec_t angle, sf, cf;
    vec3_t forward, left, up;
    cplane_t *p;
    int i;

    // right/left
    angle = DEG2RAD(glr.fd.fov_x / 2);
    sf = sin(angle);
    cf = cos(angle);

    VectorScale(glr.viewaxis[0], sf, forward);
    VectorScale(glr.viewaxis[1], cf, left);

    VectorAdd(forward, left, glr.frustumPlanes[0].normal);
    VectorSubtract(forward, left, glr.frustumPlanes[1].normal);

    // top/bottom
    angle = DEG2RAD(glr.fd.fov_y / 2);
    sf = sin(angle);
    cf = cos(angle);

    VectorScale(glr.viewaxis[0], sf, forward);
    VectorScale(glr.viewaxis[2], cf, up);

    VectorAdd(forward, up, glr.frustumPlanes[2].normal);
    VectorSubtract(forward, up, glr.frustumPlanes[3].normal);

    for (i = 0, p = glr.frustumPlanes; i < 4; i++, p++) {
        p->dist = DotProduct(glr.fd.vieworg, p->normal);
        p->type = PLANE_NON_AXIAL;
        SetPlaneSignbits(p);
    }
}

glCullResult_t GL_CullBox(const vec3_t bounds[2])
{
    int i, bits;
    glCullResult_t cull;

    if (!gl_cull_models->integer) {
        return CULL_IN;
    }

    cull = CULL_IN;
    for (i = 0; i < 4; i++) {
        bits = BoxOnPlaneSide(bounds[0], bounds[1], &glr.frustumPlanes[i]);
        if (bits == BOX_BEHIND) {
            return CULL_OUT;
        }
        if (bits != BOX_INFRONT) {
            cull = CULL_CLIP;
        }
    }

    return cull;
}

glCullResult_t GL_CullSphere(const vec3_t origin, float radius)
{
    float dist;
    cplane_t *p;
    int i;
    glCullResult_t cull;

    if (!gl_cull_models->integer) {
        return CULL_IN;
    }

    radius *= glr.entscale;
    cull = CULL_IN;
    for (i = 0, p = glr.frustumPlanes; i < 4; i++, p++) {
        dist = PlaneDiff(origin, p);
        if (dist < -radius) {
            return CULL_OUT;
        }
        if (dist <= radius) {
            cull = CULL_CLIP;
        }
    }

    return cull;
}

glCullResult_t GL_CullLocalBox(const vec3_t origin, const vec3_t bounds[2])
{
    vec3_t points[8];
    cplane_t *p;
    int i, j;
    vec_t dot;
    bool infront;
    glCullResult_t cull;

    if (!gl_cull_models->integer) {
        return CULL_IN;
    }

    for (i = 0; i < 8; i++) {
        VectorCopy(origin, points[i]);
        VectorMA(points[i], bounds[(i >> 0) & 1][0], glr.entaxis[0], points[i]);
        VectorMA(points[i], bounds[(i >> 1) & 1][1], glr.entaxis[1], points[i]);
        VectorMA(points[i], bounds[(i >> 2) & 1][2], glr.entaxis[2], points[i]);
    }

    cull = CULL_IN;
    for (i = 0, p = glr.frustumPlanes; i < 4; i++, p++) {
        infront = false;
        for (j = 0; j < 8; j++) {
            dot = DotProduct(points[j], p->normal);
            if (dot >= p->dist) {
                infront = true;
                if (cull == CULL_CLIP) {
                    break;
                }
            } else {
                cull = CULL_CLIP;
                if (infront) {
                    break;
                }
            }
        }
        if (!infront) {
            return CULL_OUT;
        }
    }

    return cull;
}

// shared between lightmap and scrap allocators
bool GL_AllocBlock(int width, int height, int *inuse,
                   int w, int h, int *s, int *t)
{
    int i, j, k, x, y, max_inuse, min_inuse;

    x = 0; y = height;
    min_inuse = height;
    for (i = 0; i < width - w; i++) {
        max_inuse = 0;
        for (j = 0; j < w; j++) {
            k = inuse[i + j];
            if (k >= min_inuse) {
                break;
            }
            if (max_inuse < k) {
                max_inuse = k;
            }
        }
        if (j == w) {
            x = i;
            y = min_inuse = max_inuse;
        }
    }

    if (y + h > height) {
        return false;
    }

    for (i = 0; i < w; i++) {
        inuse[x + i] = y + h;
    }

    *s = x;
    *t = y;
    return true;
}

// P = A * B
void GL_MultMatrix(GLfloat *restrict p, const GLfloat *restrict a, const GLfloat *restrict b)
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            p[i * 4 + j] =
                a[0 * 4 + j] * b[i * 4 + 0] +
                a[1 * 4 + j] * b[i * 4 + 1] +
                a[2 * 4 + j] * b[i * 4 + 2] +
                a[3 * 4 + j] * b[i * 4 + 3];
        }
    }
}

void GL_SetEntityAxis(void)
{
    entity_t *e = glr.ent;

    glr.entrotated = false;
    glr.entscale = 1;

    if (VectorEmpty(e->angles)) {
        VectorSet(glr.entaxis[0], 1, 0, 0);
        VectorSet(glr.entaxis[1], 0, 1, 0);
        VectorSet(glr.entaxis[2], 0, 0, 1);
    } else {
        AnglesToAxis(e->angles, glr.entaxis);
        glr.entrotated = true;
    }

    if (e->scale && e->scale != 1) {
        VectorScale(glr.entaxis[0], e->scale, glr.entaxis[0]);
        VectorScale(glr.entaxis[1], e->scale, glr.entaxis[1]);
        VectorScale(glr.entaxis[2], e->scale, glr.entaxis[2]);
        glr.entrotated = true;
        glr.entscale = e->scale;
    }
}

void GL_RotationMatrix(GLfloat *matrix)
{
    matrix[0] = glr.entaxis[0][0];
    matrix[4] = glr.entaxis[1][0];
    matrix[8] = glr.entaxis[2][0];
    matrix[12] = glr.ent->origin[0];

    matrix[1] = glr.entaxis[0][1];
    matrix[5] = glr.entaxis[1][1];
    matrix[9] = glr.entaxis[2][1];
    matrix[13] = glr.ent->origin[1];

    matrix[2] = glr.entaxis[0][2];
    matrix[6] = glr.entaxis[1][2];
    matrix[10] = glr.entaxis[2][2];
    matrix[14] = glr.ent->origin[2];

    matrix[3] = 0;
    matrix[7] = 0;
    matrix[11] = 0;
    matrix[15] = 1;
}

void GL_RotateForEntity(void)
{
    GLfloat matrix[16];

    GL_RotationMatrix(matrix);
    GL_MultMatrix(glr.entmatrix, glr.viewmatrix, matrix);
    GL_ForceMatrix(glr.entmatrix);
}

static void GL_DrawSpriteModel(const model_t *model)
{
    const entity_t *e = glr.ent;
    const mspriteframe_t *frame = &model->spriteframes[(unsigned)e->frame % model->numframes];
    const image_t *image = frame->image;
    const float alpha = (e->flags & RF_TRANSLUCENT) ? e->alpha : 1;
    int bits = GLS_DEPTHMASK_FALSE;
    vec3_t up, down, left, right;
    vec3_t points[4];

    if (alpha == 1) {
        if (image->flags & IF_TRANSPARENT) {
            if (image->flags & IF_PALETTED) {
                bits |= GLS_ALPHATEST_ENABLE;
            } else {
                bits |= GLS_BLEND_BLEND;
            }
        }
    } else {
        bits |= GLS_BLEND_BLEND;
    }

    GL_LoadMatrix(glr.viewmatrix);
    GL_BindTexture(0, image->texnum);
    GL_StateBits(bits);
    GL_ArrayBits(GLA_VERTEX | GLA_TC);
    GL_Color(1, 1, 1, alpha);

    VectorScale(glr.viewaxis[1], frame->origin_x, left);
    VectorScale(glr.viewaxis[1], frame->origin_x - frame->width, right);
    VectorScale(glr.viewaxis[2], -frame->origin_y, down);
    VectorScale(glr.viewaxis[2], frame->height - frame->origin_y, up);

    VectorAdd3(e->origin, down, left, points[0]);
    VectorAdd3(e->origin, up, left, points[1]);
    VectorAdd3(e->origin, down, right, points[2]);
    VectorAdd3(e->origin, up, right, points[3]);

    GL_TexCoordPointer(2, 0, quad_tc);
    GL_VertexPointer(3, 0, &points[0][0]);
    qglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void GL_DrawNullModel(void)
{
    static const uint32_t colors[6] = {
        U32_RED, U32_RED,
        U32_GREEN, U32_GREEN,
        U32_BLUE, U32_BLUE
    };
    const entity_t *e = glr.ent;
    vec3_t points[6];

    VectorCopy(e->origin, points[0]);
    VectorCopy(e->origin, points[2]);
    VectorCopy(e->origin, points[4]);

    VectorMA(e->origin, 16, glr.entaxis[0], points[1]);
    VectorMA(e->origin, 16, glr.entaxis[1], points[3]);
    VectorMA(e->origin, 16, glr.entaxis[2], points[5]);

    //rekkie -- allow gl_showtris to show nodes points as a cross configuration -- s
    //if (gl_showtris->integer && gl_showtris->integer < 0) // gl_showtris -1, etc.
    //{
        // Extend NullModel points in opposite direction
        VectorMA(e->origin, -16, glr.entaxis[0], points[0]);
        VectorMA(e->origin, -16, glr.entaxis[1], points[2]);
        VectorMA(e->origin, -16, glr.entaxis[2], points[4]);
    //}
    //rekkie -- allow gl_showtris to show nodes points as a cross configuration -- e

    GL_LoadMatrix(glr.viewmatrix);
    GL_BindTexture(0, TEXNUM_WHITE);
    GL_StateBits(GLS_DEFAULT);
    GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
    GL_ColorBytePointer(4, 0, (GLubyte *)colors);
    GL_VertexPointer(3, 0, &points[0][0]);
    //rekkie -- allow NullModels to be seen behind walls -- s
    //if (gl_showtris->integer && gl_showtris->integer < 0)
        GL_DepthRange(0, 0); // Set the far clipping plane to 0 (NullModels can now be seen behind walls)
    //rekkie -- allow NullModels to be seen behind walls -- e

    qglDrawArrays(GL_LINES, 0, 6);

    //rekkie -- allow NullModels to be seen behind walls -- s
    //if (gl_showtris->integer && gl_showtris->integer < 0)
        GL_DepthRange(0, 1); // Set the depth buffer back to normal (NullModels are now obscured)
    //rekkie -- allow NullModels to be seen behind walls -- e
}


#if 0
//rekkie -- gl_showedges -- s
#include "../server/server.h"

#define STEPSIZE 18
#define MAX_JUMP_HEIGHT 60					// Maximum height that a player can jump to reach a higher level surface, such as a box
#define MAX_FALL_HEIGHT 210				    // Maximum height that a player can fall from without damage
#define MAX_SAFE_CROUCH_FALL_HEIGHT 214	    // Maximum height that a player can crouch fall from without damage
#define MAX_CROUCH_FALL_HEIGHT 224			// Maximum height that a player can crouch fall from without damage
#define MAX_CROUCH_FALL_HEIGHT_UNSAFE 256	// Maximum height that a player can crouch fall + minor leg damage
#define MAX_STEEPNESS 0.71
#define MAX_FACE_VERTS 64
#define INVALID -1
#define VectorLength(v)     (sqrtf(DotProduct((v),(v))))

const qboolean IgnoreSmallSurfaces = false;

void CM_BoxTrace(trace_t* trace, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, mnode_t* headnode, int brushmask);
int CM_PointContents(vec3_t p, mnode_t* headnode);
trace_t q_gameabi SV_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, edict_t* passedict, int contentmask);

// Returns the distance from origin to target
float UTIL_Distance(vec3_t origin, vec3_t target)
{
    vec3_t dist;
    VectorSubtract(target, origin, dist);
    return VectorLength(dist);
}

/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
/*
float* tv(float x, float y, float z)
{
    static int index;
    static vec3_t vecs[8];
    float* v;

    // use an array so that multiple tempvectors won't collide
    // for a while
    v = vecs[index];
    index = (index + 1) & 7;

    v[0] = x;
    v[1] = y;
    v[2] = z;

    return v;
}
*/

void G_ProjectSource(vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
    result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
    result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
    result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}

float VectorDistance(vec3_t start, vec3_t end)
{
    vec3_t v = { 0 };
    VectorSubtract(end, start, v);
    return VectorLength(v);
}



// TODO: Add GL_ColorPolygon() to the draw chain of rendered faces, this will fix the issue of entities being drawn over highlighted faces,
//       such as the func_wall entities found in Actctiy3, the ladder faces and one of the walkways
//
// Blends an color + alpha over a texture (useful to highlight faces for visual debugging)
void GL_ColorPolygon(vec3_t verts, int num_points, surface_data_t* face_data, float red, float green, float blue, float alpha)
{
    int		i;
    vec3_t v[MAX_FACE_VERTS];
    const float HEIGHT = 0.03;

    if (num_points > MAX_FACE_VERTS)
    {
        Com_Printf("%s - num_points > MAX_FACE_VERTS\n", __func__);
        return;
    }

    vec3_t vec, tmp;
    MoveAwayFromNormal(face_data->drawflags, face_data->normal, vec, HEIGHT); // Move vec away from the surface normal

    //if (face_data->normal[0] == 0 && face_data->normal[1] == 0 && face_data->normal[2] == 0)
	{
		//Com_Printf("%s - face_data->normal == 0\n", __func__);
		//return;
	}
    
    // Copy and lift the verts up a tiny bit so they're not z-clipping the surface
    for (i = 0; i < num_points; i++)
    {
        //VectorCopy(verts + i * 3, v[i]);
        //v[i][2] += 0.03;

        VectorCopy(verts + i * 3, tmp);
        VectorAdd(tmp, vec, v[i]); // Move tmp away from the surface normal
    }

    // Draw solid shade
    GL_BindTexture(0, TEXNUM_WHITE);
    GL_StateBits(GLS_BLEND_BLEND | GLS_DEPTHMASK_FALSE);
    GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
    GL_Color(red, green, blue, alpha);

    glBegin(GL_POLYGON);
    for (i = 0; i < num_points; i++)
        glVertex3fv((float*)v + i * 3);
    glEnd();
}

float HalfPlaneSign(vec2_t p1, vec2_t p2, vec2_t p3)
{
    return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1]);
}

// Check if point is inside triangle. 
// PT = (x,y,z) point location. V1,V2,V3 = 3 points of a triangle (x,y,z)
qboolean PointInTriangle(vec3_t pt, vec3_t v1, vec3_t v2, vec3_t v3)
{
    float d1, d2, d3;
    qboolean has_neg, has_pos;

    d1 = HalfPlaneSign(pt, v1, v2);
    d2 = HalfPlaneSign(pt, v2, v3);
    d3 = HalfPlaneSign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

// A function that finds the center of a 3D convex polygon
void BOTLIB_UTIL_PolygonCenter2D(vec3_t* v, int num_verts, vec3_t center)
{
    vec3_t sum = { 0,0,0 };
    for (int i = 0; i < num_verts; i++)
    {
        VectorAdd(sum, v[i], sum);
    }
    VectorScale(sum, 1.0 / num_verts, center);
}

// Cycles through RGB colors (0.0 to 1.0)
// Speed: 0.001 works well
// Example use:
/*
    float cycle_red;
    float cycle_green;
    float cycle_blue;
    UTIL_CycleThroughColors(&cycle_red, &cycle_green, &cycle_blue, 0.001);
*/
void UTIL_CycleThroughColors(float* red, float* green, float* blue, float speed)
{
    static int phase = 0;

    float frequency = speed;
    float center = 0.5f;
    float width = 0.5f;
    float redValue = sin(frequency * phase + 0) * width + center;
    float greenValue = sin(frequency * phase + 2) * width + center;
    float blueValue = sin(frequency * phase + 4) * width + center;

    *red = redValue;
    *green = greenValue;
    *blue = blueValue;

    phase++;
    if (phase > 100000)
        phase = 0;
}

//===========================================================================
// returns the surface area of the given face
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
float AAS_FaceArea(surface_data_t* surface)
{
    //int i, edgenum, side;
    float total;
    //vec_t* v;
    vec3_t d1, d2, cross;
    //aas_edge_t* edge;

    //edgenum = aasworld.edgeindex[face->firstedge];
    //side = edgenum < 0;
    //edge = &aasworld.edges[abs(edgenum)];
    //v = aasworld.vertexes[edge->v[side]];

    total = 0;
    //for (int i = 1; i < face->numedges - 1; i++)
    for (int i = 2; i < surface->num_aligned_verts; i += 2) // edges
    {
        //edgenum = aasworld.edgeindex[face->firstedge + i];
        //side = edgenum < 0;
        //edge = &aasworld.edges[abs(edgenum)];
        VectorSubtract(surface->aligned_verts[i + 0], surface->first_vert, d1);
        VectorSubtract(surface->aligned_verts[i + 1], surface->first_vert, d2);
        CrossProduct(d1, d2, cross);
        total += 0.5 * VectorLength(cross);
    } //end for
    return total;
} //end of the function AAS_FaceArea
//===========================================================================
// a simple cross product
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
// void AAS_OrthogonalToVectors(vec3_t v1, vec3_t v2, vec3_t res)
#define AAS_OrthogonalToVectors(v1, v2, res) \
	(res)[0] = ((v1)[1] * (v2)[2]) - ((v1)[2] * (v2)[1]);\
	(res)[1] = ((v1)[2] * (v2)[0]) - ((v1)[0] * (v2)[2]);\
	(res)[2] = ((v1)[0] * (v2)[1]) - ((v1)[1] * (v2)[0]);
//===========================================================================
// Tests if the given point is within the face boundaries
//
// Parameter:				face	: face to test if the point is in it
//							pnormal	: normal of the plane to use for the face
//							point	: point to test if inside face boundaries
// Returns:					true if the point is within the face boundaries
// Changes Globals:		-
//===========================================================================
qboolean AAS_InsideFace(surface_data_t* surface, vec3_t point, float epsilon, qboolean line_trace)
{
    vec3_t v1, v2;
    vec3_t edgevec, pointvec, sepnormal;

    // Scan center point only
    if (line_trace)
    {
        for (int v = 0; v < surface->num_aligned_verts; v += 2) // edges
        {
            VectorCopy(surface->aligned_verts[v + 0], v1);
            VectorCopy(surface->aligned_verts[v + 1], v2);
            //edge vector
            VectorSubtract(v2, v1, edgevec);
            //vector from first edge point to point possible in face
            VectorSubtract(point, v1, pointvec);
            //get a vector pointing inside the face orthogonal to both the
            //edge vector and the normal vector of the plane the face is in
            //this vector defines a plane through the origin (first vertex of
            //edge) and through both the edge vector and the normal vector
            //of the plane
            AAS_OrthogonalToVectors(edgevec, surface->normal, sepnormal);
            //check on which side of the above plane the point is
            //this is done by checking the sign of the dot product of the
            //vector orthogonal vector from above and the vector from the
            //origin (first vertex of edge) to the point 
            //if the dotproduct is smaller than zero the point is outside the face
            if (DotProduct(pointvec, sepnormal) < -epsilon) return qfalse;
        }

        return true; // Found point
    }

    // Scan center + forward,back,left,right of center with an offset
    else
    {
        qboolean found;
        vec3_t pnt;
        const float offset = 16; // Offset from center point

        for (int p = 0; p < 5; p++)
        {
            found = true; // Reset

            VectorCopy(point, pnt); // Reset pnt

            
            if (p == 0) // p == 0 is the center point
                ;
            else if (p == 1)
                pnt[0] += offset; // +X
            else if (p == 2)
                pnt[0] -= offset; // -X
            else if (p == 3)
                pnt[1] += offset; // +Y
            else
                pnt[1] -= offset; // -Y

            for (int v = 0; v < surface->num_aligned_verts; v += 2) // edges
            {
                VectorCopy(surface->aligned_verts[v + 0], v1);
                VectorCopy(surface->aligned_verts[v + 1], v2);
                //edge vector
                VectorSubtract(v2, v1, edgevec);
                //vector from first edge point to point possible in face
                VectorSubtract(pnt, v1, pointvec);
                //get a vector pointing inside the face orthogonal to both the
                //edge vector and the normal vector of the plane the face is in
                //this vector defines a plane through the origin (first vertex of
                //edge) and through both the edge vector and the normal vector
                //of the plane
                AAS_OrthogonalToVectors(edgevec, surface->normal, sepnormal);
                //check on which side of the above plane the point is
                //this is done by checking the sign of the dot product of the
                //vector orthogonal vector from above and the vector from the
                //origin (first vertex of edge) to the point 
                //if the dotproduct is smaller than zero the point is outside the face
                if (DotProduct(pointvec, sepnormal) < -epsilon)
                {
                    found = false; // Flag as failure
                    break;
                }
            } //end for

            if (found) // If no failures found
                return true; // We found a point
        }

        return false; // Failed to find a point
    }

} //end of the function AAS_InsideFace

qboolean UTIL_InsideFace(surface_data_t* face_data, vec3_t point, float epsilon)
{
    vec3_t v1, v2;
    vec3_t edgevec, pointvec, sepnormal;

    for (int v = 0; v < face_data->num_aligned_verts; v += 2) // edges
    {
        VectorCopy(face_data->aligned_verts[v + 0], v1);
        VectorCopy(face_data->aligned_verts[v + 1], v2);
        //edge vector
        VectorSubtract(v2, v1, edgevec);
        //vector from first edge point to point possible in face
        VectorSubtract(point, v1, pointvec);
        //get a vector pointing inside the face orthogonal to both the
        //edge vector and the normal vector of the plane the face is in
        //this vector defines a plane through the origin (first vertex of
        //edge) and through both the edge vector and the normal vector
        //of the plane
        AAS_OrthogonalToVectors(edgevec, face_data->normal, sepnormal);
        //check on which side of the above plane the point is
        //this is done by checking the sign of the dot product of the
        //vector orthogonal vector from above and the vector from the
        //origin (first vertex of edge) to the point 
        //if the dotproduct is smaller than zero the point is outside the face
        if (DotProduct(pointvec, sepnormal) < -epsilon) return qfalse;
    }

    return true; // Found point
}

//===========================================================================
// Tests if the given point is within the face boundaries
//
// Parameter:				face	: face to test if the point is in it
//							pnormal	: normal of the plane to use for the face
//							point	: point to test if inside face boundaries
// Returns:					true if the point is within the face boundaries
// Changes Globals:		-
//===========================================================================
qboolean AAS_InsideFaceBoxScan(surface_data_t* surface, vec3_t point, float epsilon, qboolean line_trace)
{
    // Try line trace first
    if (UTIL_InsideFace(surface, point, epsilon))
        return true;

    // Try to scan in a grid pattern
    vec3_t pnt = { 0, 0, 0 }; // Point to test
    float offset = 0; // Offset from center point
    const float grid_size = 16; // Offset maximum from center point
    while (offset < grid_size) // Scan until grid_size is reached
    {
        offset += 4; // Increase offset - check every nth steps

        for (int p = 0; p < 8; p++) // Scan: forward, back, left, right + diagonals
        {
            VectorCopy(point, pnt); // Reset pnt to point

            // Forward, back, left, right scans
            if (p == 0)
                pnt[0] += offset; // +X
            else if (p == 1)
                pnt[0] -= offset; // -X
            else if (p == 2)
                pnt[1] += offset; // +Y
            else if (p == 3)
                pnt[1] -= offset; // -Y
            // Diagonal scans
            else if (p == 4)
			{
				pnt[0] += offset; // +X
				pnt[1] += offset; // +Y
			}
            else if (p == 5)
			{
                pnt[0] += offset; // +X
                pnt[1] -= offset; // -Y
			}
            else if (p == 6)
            {
                pnt[0] -= offset; // -X
                pnt[1] += offset; // +Y
			}
            else if (p == 7)
			{
                pnt[0] -= offset; // -X
                pnt[1] -= offset; // -Y
			}

            // This is expensive, but rarely used. More often than not this occurs on a slope
            // when a line trace from the center of the player to the ground does not hit the ground;
            // Instances like this occur if the player collision box is on the edge of a slope.
            // Therefore, we need to find the Z height at each offset adjustment.
            if (line_trace)
            {
                vec3_t start, end;
                VectorCopy(pnt, start);
                VectorCopy(pnt, end);
                start[2] += 16; // Start 16 units above the point
                end[2] -= 106; // End (90 + 16) units below the point
                trace_t tr = SV_Trace(start, NULL, NULL, end, NULL, MASK_SOLID); // Picked MASK_SOLID over MASK_PLAYERSOLID so player clip on ladders is ignored
				if (tr.fraction == 1.0)
					continue; // Skip this point if it doesn't hit anything
                else
                    pnt[2] = tr.endpos[2]; // Set the Z height to the trace endpos
            }

            if (UTIL_InsideFace(surface, pnt, epsilon)) // Try with the new offset
                return true; // Success: found a face!
        }
    }
    return false; // Fail: could not find a face
}

// Moves a vector away from a face based on the normal of the surface and distance
// drawflags = DSURF_PLANEBACK if the surface is facing away from the camera
// normal = The normal of the surface
// out = The vector to move
// distance = The distance to move the vector
void MoveAwayFromNormal(const int drawflags, const vec3_t normal, vec3_t out, const float distance)
{
    VectorClear(out);

    vec3_t norm;
    VectorCopy(normal, norm);
    if (drawflags & DSURF_PLANEBACK)
    {
        //VectorInverse(norm);
    }

    // Floors
    if (norm[2] == 1)
        out[2] = distance;
    // Slopes
    else if (norm[0] >= -MAX_STEEPNESS && norm[0] <= MAX_STEEPNESS && norm[1] >= -MAX_STEEPNESS && norm[1] <= MAX_STEEPNESS && norm[2] >= MAX_STEEPNESS)
    {
        out[2] = (distance * norm[2]);

        if (norm[0] != 0)
            out[0] = (distance * norm[0]);
        if (norm[1] != 0)
            out[1] = (distance * norm[1]);
    }
    // Walls and steep slopes
    else
    {
        out[2] = (distance * norm[2]);

        if (norm[0] != 0)
            out[0] = (distance * norm[0]);
        if (norm[1] != 0)
            out[1] = (distance * norm[1]);
    }

    /*
    // Floors
    if (normal[2] == 1)
        out[2] += distance;
    // Slopes
    else if (normal[0] >= -MAX_STEEPNESS && normal[0] <= MAX_STEEPNESS && normal[1] >= -MAX_STEEPNESS && normal[1] <= MAX_STEEPNESS && normal[2] >= MAX_STEEPNESS)
    {
        out[2] += (distance * normal[2]);

        if (drawflags & DSURF_PLANEBACK)
        {
            if (normal[0] != 0)
                out[0] -= (distance * normal[0]);
            if (normal[1] != 0)
                out[1] -= (distance * normal[1]);
        }
        else
        {
            if (normal[0] != 0)
                out[0] += (distance * normal[0]);
            if (normal[1] != 0)
                out[1] += (distance * normal[1]);
        }
    }
    // Walls and steep slopes
    else
    {
        out[2] += (distance * normal[2]);

        if (drawflags & DSURF_PLANEBACK)
        {
            if (normal[0] != 0)
                out[0] -= (distance * normal[0]);
            if (normal[1] != 0)
                out[1] -= (distance * normal[1]);
        }
        else
        {
            if (normal[0] != 0)
                out[0] += (distance * normal[0]);
            if (normal[1] != 0)
                out[1] += (distance * normal[1]);
        }
    }
    */
}

// Project trace from the camera view (center screen) to the world
// parameters: distance (absolute max: 8192)
trace_t UTIL_CameraTrace(float distance)
{
    //const float distance = 64; // Max ray cast distance (absolute max: 8192)
    //const float min_max = 0.01; // Min/max value for the mins/maxs of the trace
    //vec3_t mins = { -min_max,-min_max,-min_max };
    //vec3_t maxs = { min_max,min_max,min_max };
    vec3_t start, end;
    vec3_t forward, right;
    vec3_t view_offset = { 0 };
    AngleVectors(glr.fd.viewangles, forward, right, NULL);
    //VectorSet(view_offset, 0, 0, 0);
    //view_offset[1] = 0;
    G_ProjectSource(glr.fd.vieworg, view_offset, forward, right, start);
    VectorMA(start, distance, forward, end);
    return SV_Trace(start, NULL, NULL, end, NULL, MASK_SOLID); // Picked MASK_SOLID over MASK_PLAYERSOLID so player clip on ladders is ignored
    //return SV_Trace(start, mins, maxs, end, NULL, MASK_SOLID); // Picked MASK_SOLID over MASK_PLAYERSOLID so player clip on ladders is ignored
}
// Project trace from the player's feet (from camera origin) to the world below
trace_t UTIL_FeetLineTrace(void)
{
    vec3_t start, end;
    vec3_t forward, right;
    vec3_t view_offset = { 0, 0, 0 };
    AngleVectors(glr.fd.viewangles, forward, right, NULL);
    G_ProjectSource(glr.fd.vieworg, view_offset, forward, right, start);
    VectorCopy(start, end);
    end[2] -= 90;
    return SV_Trace(start, NULL, NULL, end, NULL, MASK_SOLID); // Picked MASK_SOLID over MASK_PLAYERSOLID so player clip on ladders is ignored
}
// Project trace from the player's feet (from camera origin) to the world below
trace_t UTIL_FeetBoxTrace(void)
{
    const float min_max = 16; // Min/max value for the mins/maxs of the trace
    vec3_t mins = { -min_max,-min_max, 0 };
    vec3_t maxs = { min_max,min_max, 1 };
    //vec3_t mins = { -min_max,-min_max,-min_max };
    //vec3_t maxs = { min_max,min_max,min_max };
    vec3_t start, end;
    vec3_t forward, right;
    vec3_t view_offset = { 0, 0, 0 };
    AngleVectors(glr.fd.viewangles, forward, right, NULL);
    G_ProjectSource(glr.fd.vieworg, view_offset, forward, right, start);
    VectorCopy(start, end);
    end[2] -= 90;
    return SV_Trace(start, mins, maxs, end, NULL, MASK_SOLID); // Picked MASK_SOLID over MASK_PLAYERSOLID so player clip on ladders is ignored
}
// Project trace of a player's box size (from camera origin)
trace_t UTIL_PlayerBoxTraceStanding(qboolean crouching)
{
    vec3_t mins = { -16, -16, 24 };
    vec3_t maxs = { 16, 16, 32 };
    vec3_t start, end;
    vec3_t forward, right;
    vec3_t view_offset = { 0, 0, 0 };
    AngleVectors(glr.fd.viewangles, forward, right, NULL);
    G_ProjectSource(glr.fd.vieworg, view_offset, forward, right, start);
    VectorCopy(start, end);
    if (crouching) maxs[2] = 8;
    return SV_Trace(start, mins, maxs, end, NULL, MASK_SOLID); // Picked MASK_SOLID over MASK_PLAYERSOLID so player clip on ladders is ignored
}

// Checks if two face normals are similar, with a leeway
qboolean UTIL_FaceNormalsMatch(const vec3_t normal_1, const vec3_t normal_2)
{
    const float leeway = 0.03;
    vec3_t normal, n1, n2;
    VectorCopy(normal_1, n1);
    VectorCopy(normal_2, n2);

    // Normalize range between 0 and 1
    if (n1[0] == -1)
        n1[0] = 1;
    if (n1[1] == -1)
        n1[1] = 1;
    if (n1[2] == -1)
        n1[2] = 1;
    // Normalize range between 0 and 1
    if (n2[0] == -1)
        n2[0] = 1;
    if (n2[1] == -1)
        n2[1] = 1;
	if (n2[2] == -1)
        n2[2] = 1;

    // Get the difference between the two normals
    normal[0] = fabs(n1[0] - n2[0]);
    normal[1] = fabs(n1[1] - n2[1]);
    normal[2] = fabs(n1[2] - n2[2]);

    // Check if the difference is within the leeway
    if (normal[0] > leeway || normal[1] > leeway || normal[2] > leeway)
        return false;
    
    return true;
}



// Compares two normals to see if they're roughly equal (within leeway percent, 0.03 is 3%)
qboolean UTIL_CompareNormal(vec3_t n1, vec3_t n2, float leeway)
{
    vec3_t norm, norm_1, norm_2;
    VectorCopy(n1, norm_1);
    VectorCopy(n2, norm_2);

    norm[0] = fabs(norm_1[0] - norm_2[0]);
    norm[1] = fabs(norm_1[1] - norm_2[1]);
    norm[2] = fabs(norm_1[2] - norm_2[2]);

    if (norm[0] > leeway || norm[1] > leeway || norm[2] > leeway)
        return false; // Normal does not match
    else
        return true; // Normal within range
}

// Compares two vect_3 verts, checking if they're on the same x or y or z plane, allowing for a leeway, returning the result as true/false. The function definition: qboolean UTIL_CompareVert(vec3_t v1, vec3_t v2, vec3_t normal, float leeway)
qboolean UTIL_CompareVertPlanes(vec3_t v1, vec3_t v2, vec3_t normal, float leeway)
{
    vec3_t pNormal;
    VectorSubtract(v2, v1, pNormal);
    VectorNormalize(pNormal);

    if (fabs(DotProduct(pNormal, normal)) > leeway)
        return false;
    else
        return true;
}

qboolean UTIL_CompareVertPlanesSlope(vec3_t verts, int num_verts, vec3_t v2, vec3_t normal, float leeway)
{
    vec3_t pNormal = { 0 };
    for (int i = 0; i < num_verts; i++)
    {
        VectorSubtract(v2, verts+i, pNormal);
        VectorNormalize(pNormal);
        if (fabs(DotProduct(pNormal, normal)) <= leeway)
            return true;
    }
    return false;
}

// Checks if a point is inside/touching a face, this is done by splitting up a face into triangles and checking if the point is inside any of the triangles
//static qboolean UTIL_PointInsideFace(vec3_t pos, vec3_t* verts, int total_verts, vec3_t normal, int drawflags)
static qboolean UTIL_PointInsideFace(vec3_t pos, mface_t* surf, int face, int contents)
{
    int e = 0, v = 0; // Edge and vert index
    msurfedge_t* surfedge; // Surface edge
    int total_verts = 0;
    vec3_t verts[MAX_FACE_VERTS]; // All verts
    for (surfedge = surf->firstsurfedge; e < surf->numsurfedges; surfedge++)
    {
        VectorCopy(surfedge->edge->v[0]->point, verts[v]);
        v++;
        VectorCopy(surfedge->edge->v[1]->point, verts[v]);
        v++;
        total_verts += 2;
        e++;
    }
    vec3_t normal;
    VectorCopy(surf->plane->normal, normal);
    int drawflags = surf->drawflags;

    if (total_verts <= 2) // If there are less than 3 verts, then it's not a face
        return false;

    //Com_Printf("%s f:%d normal[%f %f %f]\n", __func__, face, normal[0], normal[1], normal[2]);

    const float adjust_dist = 0.01;
    vec3_t center = { 0 }; // Center point of a triangle
    vec3_t pt0 = { 0 }; // Point 1 of a triangle
    vec3_t pt1 = { 0 }; // Point 2 of a triangle
    vec3_t pt2 = { 0 }; // Point 3 of a triangle



    // Get the starting point of all the triangles
    if (VectorCompare(verts[0], verts[total_verts - 1]) || VectorCompare(verts[0], verts[total_verts - 2]))
    {
        VectorCopy(verts[0], pt0); // Starting point - Vertex 1
    }
    else
    {
        VectorCopy(verts[1], pt0); // Starting point - Vertex 2
    }

    
    // Testing starting vector direction & angle
    qboolean follows_starting_vec = true; // If the point follows the starting vector direction
    vec3_t dir = { 0 }; // Dir vector
    vec3_t angle = { 0 }; // Angle vector
    // Get the direction of the edge to test against
    vec3_t dir_forward = { 0 };
    vec3_t dir_backward = { 0 };
    // Get vector angle of dir_forward
    vec3_t angle_forward = { 0 };
    vec3_t angle_backward = { 0 };
    // Init the forward and backward angles
    VectorSubtract(verts[1], verts[0], dir_forward); // Forward vector
    VectorSubtract(verts[0], verts[1], dir_backward); // Backward vector
    vectoangles2(dir_forward, angle_forward); // Forward angle
    vectoangles2(dir_backward, angle_backward); // Backward angle


    // Get the centoid of a face triangle
    qboolean inside = false;
    for (int vert_count = 0; vert_count < total_verts; vert_count += 2)
    {
        //if (vert_count + 2 == total_verts) // ignore the last edge
        //    continue;

        //if (normal[2] == 1 || (normal[0] >= -MAX_STEEPNESS && normal[0] <= MAX_STEEPNESS && normal[1] >= -MAX_STEEPNESS && normal[1] <= MAX_STEEPNESS && normal[2] >= MAX_STEEPNESS))
        //    ;
        //else
        //    continue; // ignore walls and steep slopes

        // An edge is comprised of two verts, it needs a third vert to make a triangle. 
        // pt0 (the third vert) will be using either the very first vertex verts[0] or verts[1]
        // In this example verts[1] becomes the third vert for [pt2] and [pt3], 
        //     together they form a triangle as per the command gl_showtris "1"
        //
        //		
        // verts[3] -->	[pt3] -------------- [pt0]	<-- verts[0]
        //                    |\           |
        //                    |  \         |
        //                    |    \       |
        //                    |      \     |
        //                    |        \   |
        //                    |          \ |
        // verts[2] --> [pt2] -------------- [pt1]  <-- verts[1]	<=== pick this for pt0
        //
        /*
        if (vert_count == 2)
        {
            // Point 1 == 3
            if (verts[0][0] == verts[vert_count][0] && verts[0][1] == verts[vert_count][1] && verts[0][2] == verts[vert_count][2])
                VectorCopy(verts[1], pt0);

            // Point 1 == 4
            else if (verts[0][0] == verts[vert_count + 1][0] && verts[0][1] == verts[vert_count + 1][1] && verts[0][2] == verts[vert_count + 1][2])
                VectorCopy(verts[1], pt0);

            // Point 2 == 3
            else if (verts[1][0] == verts[vert_count][0] && verts[1][1] == verts[vert_count][1] && verts[1][2] == verts[vert_count][2])
                VectorCopy(verts[0], pt0);

            // Point 2 == 4
            else
                VectorCopy(verts[0], pt0);
        }
        */
        /*
        // Ignore if three verts form a line instead of a triangle
        // Floors, Slopes
        if (normal[2] == 1 || (normal[0] >= -MAX_STEEPNESS && normal[0] <= MAX_STEEPNESS && normal[1] >= -MAX_STEEPNESS && normal[1] <= MAX_STEEPNESS && normal[2] >= MAX_STEEPNESS))
        {
            // [pt0] ------ [pt2] ------ [pt3] ----- []   [line != triangle]
            // [pt1] ------ [pt2] ------ [pt3] ----- []   [line != triangle]
            //
            // If three verts on the X axis are the same, ignore because they form a line and not a valid triangle
            if (pt0[0] == verts[vert_count][0] && pt0[0] == verts[vert_count + 1][0])
                continue;

            // If three verts on the Y axis are the same, ignore because they form a line and not a valid triangle
            if (pt0[1] == verts[vert_count][1] && pt0[1] == verts[vert_count + 1][1])
                continue;
        }
        else // Walls
        {
            // If three verts on the Z axis are the same
            if (pt0[2] == verts[vert_count][2] && pt0[2] == verts[vert_count + 1][2])
                continue;
        }
        */

        // If the next edge after the starting edge is heading in the same direction, skip it
        // We check this by comparing the angle of the next edge to the angle of the starting edge
        // This is done until the next edge is heading in a different direction, whereby we make follows_starting_vec false
        // Get the direction of the current edge
        VectorSubtract(verts[vert_count + 1], verts[vert_count], dir); // Current edge vector
        vectoangles2(dir, angle); // Current edge angle
        // Check if the angle is the same as angle_forward or angle_backward
        if (follows_starting_vec && (VectorCompare(angle, angle_forward) || VectorCompare(angle, angle_backward)))
        {
            // The current edge is in the same direction as the previous edge, skip it
            continue;
        }
        else
        {
            follows_starting_vec = false;
        }


        // Copy the next two verts of the current edge
        VectorCopy(verts[vert_count], pt1);
        VectorCopy(verts[vert_count + 1], pt2);

        // Calculate the centroid of a triangle ((A+B+C)/3)
        float x, y, z;
        x = (pt0[0] + pt1[0] + pt2[0]) / 3;
        y = (pt0[1] + pt1[1] + pt2[1]) / 3;
        z = (pt0[2] + pt1[2] + pt2[2]) / 3;

        // Copy the centoid
        center[0] = x;
        center[1] = y;
        center[2] = z;

        if (0) // Look for surface normals that point down, we wish to ignore these triangle faces
        {
            vec3_t normal_end = { 0 };
            vec3_t center_vec = { 0 };
            const float offset = 32;
            MoveAwayFromNormal(drawflags, normal, center_vec, offset);
            VectorAdd(center, center_vec, normal_end); // Move away from surface
            if (normal_end[2] < center[2]) // Normal points down
                return false; // Ignore surface
        }

        if (0) // DEBUG: draw triangles
        {
            vec3_t points[6];

            VectorCopy(pt0, points[0]);
            VectorCopy(pt1, points[1]);

            VectorCopy(pt1, points[2]);
			VectorCopy(pt2, points[3]);

            VectorCopy(pt2, points[4]);
            VectorCopy(pt0, points[5]);

            static const uint32_t color_white[6] = { U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN };
            GL_LoadMatrix(glr.viewmatrix);
            GL_BindTexture(0, TEXNUM_WHITE);
            GL_StateBits(GLS_DEFAULT);
            GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
            GL_VertexPointer(3, 0, &points[0][0]);
            glLineWidth(1.0);
            GL_ColorBytePointer(4, 0, (GLubyte*)color_white);
            if (gl_showedges && gl_showedges->integer < 0) // -1
                GL_DepthRange(0, 1); // Set the far clipping plane to 0 (edges are now obscured)
            else
                GL_DepthRange(0, 0); // Set the far clipping plane to 0 (edges can now be seen behind walls)
            qglDrawArrays(GL_LINES, 0, 6); // GL_LINES is the type of primitive to render, 0 is the starting index, 2 is the number of indices to render
            GL_DepthRange(0, 1); // Set the depth buffer back to normal (edges are now obscured)
            glLineWidth(1.0);
        }

        if (0) // DEBUG: draws triangle centoids
        {
            vec3_t points[2];
            vec3_t center_vec = { 0 };
            
            const float offset = 32; // Player model width
            MoveAwayFromNormal(drawflags, normal, center_vec, offset);
            
            VectorCopy(center, points[0]);
            VectorAdd(center, center_vec, points[1]); // Move away from surface

            static const uint32_t color_white[2] = { U32_RED, U32_RED };
            GL_DrawLine(points[0], 2, color_white, 1, false);
        }

        vec3_t pt;
        vec3_t v1; v1[0] = pt0[0]; v1[1] = pt0[1]; v1[2] = pt0[2];
        vec3_t v2; v2[0] = pt1[0]; v2[1] = pt1[1]; v2[2] = pt1[2];
        vec3_t v3; v3[0] = pt2[0]; v3[1] = pt2[1]; v3[2] = pt2[2];



        









        // Center point
        // Check to ensure the centoid (x,y) coordinates are truly inside of the triangle
        // vec3_t pt; pt[0] = center[0]; pt[1] = center[1]; pt[2] = center[2];
        //if (PointInTriangle(pt, v1, v2, v3)) // Is the point inside the triangle?
        //{
        //    is_point_inside = true;
        //}

        // Check if origin is a ladder
        qboolean is_ladder = false;
        if (1)
        {
            const float min_max = 0.01; // Min/max value for the mins/maxs of the trace
            vec3_t mins = { -min_max,-min_max,-min_max };
            vec3_t maxs = { min_max,min_max,min_max };
            vec3_t start = { 0 };
            vec3_t pos_vec = { 0 };
            const float offset = 32;
            MoveAwayFromNormal(drawflags, normal, pos_vec, offset);
            VectorAdd(pos, pos_vec, start); // Move away from surface
            trace_t tr = SV_Trace(start, mins, maxs, pos, NULL, MASK_PLAYERSOLID); // From origin outside to inside
            if (tr.contents & CONTENTS_LADDER)
                is_ladder = true;
        }
        if (contents & CONTENTS_LADDER)
            is_ladder = true;





        //is_ladder = true; // Debug






        // Target point
        pt[0] = pos[0]; pt[1] = pos[1]; pt[2] = pos[2];
        if (is_ladder || PointInTriangle(pt, v1, v2, v3)) // Is the point inside the triangle?
        {
            float distance = 1; // within 2 units from the surface
            vec3_t org = { 0,0,0 };
            vec3_t vec = { 0,0,0 };
            VectorCopy(pos, org); // We only want the height

            // Floors
            if (normal[2] == 1 && normal[0] == 0 && normal[1] == 0)
            {
                org[0] = 0; // Remove X component
                org[1] = 0; // Remove Y component
                center[0] = 0; // Remove X component
                center[1] = 0; // Remove Y component

                VectorSubtract(center, org, vec);
                if (VectorLength(vec) <= distance)
                    return true;
                    //inside = true;
            }
            // Slopes
            //else if (normal[0] >= -MAX_STEEPNESS && normal[0] <= MAX_STEEPNESS && normal[1] >= -MAX_STEEPNESS && normal[1] <= MAX_STEEPNESS && normal[2] >= MAX_STEEPNESS)
            else
            {
                // Look for surface normals that point down, we wish to ignore these faces
                //vec3_t normal_end = { 0 };
                //vec3_t center_vec = { 0 };
                //const float offset = 32;
                //MoveAwayFromNormal(drawflags, normal, center_vec, offset);
                //VectorAdd(center, center_vec, normal_end); // Move away from surface
                //if (normal_end[2] < center[2]) // Normal points down
				//	return false; // Ignore surface

                // Flatten the center triangle by averaging the height between all 3 points, and save the difference to a float
                //float avg_height = (pt0[2] + pt1[2] + pt2[2]) / 3;
                //float diff = avg_height - center[2];

                /*
                // Get the min height of the triangle
                float min_height = pt0[2];
                if (pt1[2] < min_height)
                    min_height = pt1[2];
                if (pt2[2] < min_height)
                    min_height = pt2[2];

                // Get the max height of the triangle
                float max_height = pt0[2];
                if (pt1[2] > max_height)
                    max_height = pt1[2];
                if (pt2[2] > max_height)
                    max_height = pt2[2];
                */

                /*
                // Flatten the center by averaging the height between all vertex points, and save the difference
                float avg_height = 0;
                float diff = 0;
                for (int v = 0; v < total_verts; v++)
                {
                    avg_height += verts[v][2];
                }
                avg_height /= total_verts;
                diff = avg_height - center[2];
                */

                // Min and max vertex height
                float min_x = 999999;
                float max_x = -999999;
                float min_y = 999999;
                float max_y = -999999;
                float min_z = 999999;
                float max_z = -999999;
                for (int v = 0; v < total_verts; v++)
                {
                    if (verts[v][0] < min_x)
                        min_x = verts[v][0];
                    if (verts[v][0] > max_x)
                        max_x = verts[v][0];
                    if (verts[v][1] < min_y)
                        min_y = verts[v][1];
                    if (verts[v][1] > max_y)
                        max_y = verts[v][1];
                    if (verts[v][2] < min_z)
                        min_z = verts[v][2];
                    if (verts[v][2] > max_z)
                        max_z = verts[v][2];
                }

                // Add leeway to the min/max
                if (0)
                {
                    const float leeway = 0.3125;
                    if (min_x > 0) min_x -= leeway;
                    else min_x += -(leeway);
                    max_x += leeway;

                    if (min_y > 0) min_y -= leeway;
                    else min_y += -(leeway);
                    max_y += leeway;

                    if (min_z > 0) min_z -= leeway;
                    else min_z += -(leeway);
                    max_z += leeway;
                }




                // height diff between center and origin in absolute
                //float abs_diff = fabs(org[2] - center[2]);
                //float abs_diff = fabs(org[2] - avg_height);

/*
face:59 org[256.031250 -79.877510 -718.222961] -x[256.000000] +x[256.000000] -y[-112.000000] +y[-33.000000] -z[-800.000000] +z[-608.000000]
face:59 org[256.031250 -79.877510 -718.222961] -x[256.000000] +x[256.000000] -y[-112.000000] +y[-33.000000] -z[-800.000000] +z[-608.000000]
face:59 org[256.031250 -79.877510 -718.222961] -x[256.000000] +x[256.000000] -y[-112.000000] +y[-33.000000] -z[-800.000000] +z[-608.000000]

face:1 org[-285.238037 -0.031258 -706.308044] -x[-287.899994] +x[-128.100006] -y[0.100000] +y[-0.100000] -z[-800.000000] +z[-704.000000]
*/

                //if (org[2] >= min_z && org[2] <= max_z)
                if (org[0] >= min_x && org[0] <= max_x && org[1] >= min_y && org[1] <= max_y && org[2] >= min_z && org[2] <= max_z)
				{
                    //Com_Printf("--- %s face:%d org[%f %f %f] -x[%f] +x[%f] -y[%f] +y[%f] -z[%f] +z[%f]\n", __func__, face, org[0], org[1], org[2], min_x, max_x, min_y, max_y, min_z, max_z);
					//Com_Printf("--- %s diff:%f avg_height:%f abs_diff:%f center[%f] org[%f] mm[%f %f]\n", __func__, diff, avg_height, abs_diff, center[2], org[2], min_height, max_height);
					return true;
				}

                /*
                if (face == 1)
                {
                    Com_Printf("%s face:%d org[%f %f %f] -x[%f] +x[%f] -y[%f] +y[%f] -z[%f] +z[%f]\n", __func__, face, org[0], org[1], org[2], min_x, max_x, min_y, max_y, min_z, max_z);
                    return true;
                }
                else
                    return false;
                */

                //Com_Printf("%s face:%d org[%f %f %f] -x[%f] +x[%f] -y[%f] +y[%f] -z[%f] +z[%f]\n", __func__, face, org[0], org[1], org[2], min_x, max_x, min_y, max_y, min_z, max_z);

                //if (abs_diff < 32)
                //    Com_Printf("%s diff:%f avg_height:%f abs_diff:%f center[%f] org[%f] mm[%f %f]\n", __func__, diff, avg_height, abs_diff, center[2], org[2], min_z, max_z);

                //center[2] = avg_height; // Flatten the center

                // Add the difference to the origin
                //org[2] += diff;
            }

            /*
            if (0) // DEBUG: draws triangle centoids
            {
                vec3_t points[2];
                vec3_t center_vec = { 0 };

                const float offset = 32; // Player model width
                MoveAwayFromNormal(drawflags, normal, center_vec, offset);

                VectorCopy(center, points[0]);
                VectorAdd(center, center_vec, points[1]); // Move away from surface

                static const uint32_t color_white[2] = { U32_RED, U32_RED };
                GL_LoadMatrix(glr.viewmatrix);
                GL_BindTexture(0, TEXNUM_WHITE);
                GL_StateBits(GLS_DEFAULT);
                GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
                GL_VertexPointer(3, 0, &points[0][0]);
                glLineWidth(2.5);
                GL_ColorBytePointer(4, 0, (GLubyte*)color_white);
                if (gl_showedges && gl_showedges->integer < 0) // -1
                    GL_DepthRange(0, 1); // Set the far clipping plane to 0 (edges are now obscured)
                else
                    GL_DepthRange(0, 0); // Set the far clipping plane to 0 (edges can now be seen behind walls)
                qglDrawArrays(GL_LINES, 0, 2); // GL_LINES is the type of primitive to render, 0 is the starting index, 2 is the number of indices to render
                GL_DepthRange(0, 1); // Set the depth buffer back to normal (edges are now obscured)
                glLineWidth(1.0);
            }

            org[0] = 0; // Remove X component
            org[1] = 0; // Remove Y component
            center[0] = 0; // Remove X component
            center[1] = 0; // Remove Y component

            VectorSubtract(center, org, vec);
            if (VectorLength(vec) <= distance)
                inside = true;
            */


            /*
            // Ensure the PointInTriangle is roughly within the same z height as origin
            vec3_t org = { 0,0,0 };
            vec3_t vec = { 0,0,0 };
            VectorCopy(pos, org); // We only want the height
            org[0] = 0; // Remove X component
            org[1] = 0; // Remove Y component
            center[0] = 0; // Remove X component
            center[1] = 0; // Remove Y component
            VectorSubtract(center, org, vec);
            float distance = VectorLength(vec);
            if (distance < 2) // Height
                inside = true;
            */
        }




        // Check if floor
        if (normal[0] == 0 && normal[1] == 0 && normal[2] == 1)
        {
            //Com_Printf("--- %s\n", __func__);
        }
        // Slopes
        else if (normal[0] >= -MAX_STEEPNESS && normal[0] <= MAX_STEEPNESS && normal[1] >= -MAX_STEEPNESS && normal[1] <= MAX_STEEPNESS && normal[2] >= MAX_STEEPNESS)
        {
            //Com_Printf("--- %s\n", __func__);
        }
        else // Wall
        {
            // Min and max vertex height
            float min_x = 999999;
            float max_x = -999999;
            float min_y = 999999;
            float max_y = -999999;
            float min_z = 999999;
            float max_z = -999999;
            for (int v = 0; v < total_verts; v++)
            {
                if (verts[v][0] < min_x)
                    min_x = verts[v][0];
                if (verts[v][0] > max_x)
                    max_x = verts[v][0];
                if (verts[v][1] < min_y)
                    min_y = verts[v][1];
                if (verts[v][1] > max_y)
                    max_y = verts[v][1];
                if (verts[v][2] < min_z)
                    min_z = verts[v][2];
                if (verts[v][2] > max_z)
                    max_z = verts[v][2];
            }

            // Add leeway to the min/max
            if (1)
            {
                //const float leeway = 0.3125;
                const float leeway = 0.03125;
                if (min_x > 0) min_x -= leeway;
                else min_x += -(leeway);
                max_x += leeway;

                if (min_y > 0) min_y -= leeway;
                else min_y += -(leeway);
                max_y += leeway;

                if (min_z > 0) min_z -= leeway;
                else min_z += -(leeway);
                max_z += leeway;
            }

            if (pos[0] >= min_x && pos[0] <= max_x && pos[1] >= min_y && pos[1] <= max_y && pos[2] >= min_z && pos[2] <= max_z)
            {
                //Com_Printf("--- %s face:%d org[%f %f %f] -x[%f] +x[%f] -y[%f] +y[%f] -z[%f] +z[%f]\n", __func__, face, pos[0], pos[1], pos[2], min_x, max_x, min_y, max_y, min_z, max_z);
                //Com_Printf("--- %s diff:%f avg_height:%f abs_diff:%f center[%f] org[%f] mm[%f %f]\n", __func__, diff, avg_height, abs_diff, center[2], pos[2], min_height, max_height);
                return true;
            }
        }


    }

    return inside;
}





// Functionally this works similar to the #define VectorCompare(), with the exception of an epsilon
// Compare two vectors and return true if they are the same within a certain epsilon
qboolean UTIL_VectorCompare(vec3_t v1, vec3_t v2, const float epsilon)
{
    for (int i = 0; i < 3; i++)
    {
        if (fabs(v1[i] - v2[i]) > epsilon)
        {
            return false;
        }
    }
    return true;
}

// Checks the direction of v1 and v2 against v3 and v4
// Also add a qboolean to allow checking for the opposite direction
// Include an epsilon to allow for floating point errors
qboolean UTIL_VectorDirectionCompare(vec3_t v1, vec3_t v2, vec3_t v3, vec3_t v4, qboolean opposite, const float epsilon)
{
    vec3_t dir_forward[2], dir_backward[2]; // Forward and backward vectors
    vec3_t angle_forward[2], angle_backward[2]; // Forward and backward angles

    VectorSubtract(v2, v1, dir_forward[0]); // Forward vector
    VectorSubtract(v4, v3, dir_forward[1]); // Forward vector

    vectoangles2(dir_forward[0], angle_forward[0]); // Forward angle
    vectoangles2(dir_forward[1], angle_forward[1]); // Forward angle

    if (opposite)
    {
        VectorSubtract(v1, v2, dir_backward[0]); // Backward vector
        VectorSubtract(v3, v4, dir_backward[1]); // Backward vector

        vectoangles2(dir_backward[0], angle_backward[0]); // Backward angle
        vectoangles2(dir_backward[1], angle_backward[1]); // Backward angle

        if (UTIL_VectorCompare(angle_forward[0], angle_forward[1], epsilon) || UTIL_VectorCompare(angle_backward[0], angle_backward[1], epsilon)
            || UTIL_VectorCompare(angle_forward[0], angle_backward[1], epsilon) || UTIL_VectorCompare(angle_backward[0], angle_forward[1], epsilon))
        {
            return true; // Return true if either forward or backward angles are the same
        }
    }
    else
    {
        // Vector compare the forward angles
        if (UTIL_VectorCompare(angle_forward[0], angle_forward[1], epsilon))
        {
            return true; // Return true if forward angles are the same
        }
    }

	return false; // Default to false
}

// Gets the raw vertex data from a surface and returns the result to face_data
void InitVertexData(mface_t* surf, surface_data_t* face_data)
{
    int e = 0, v = 0; // Edge and vert index
    msurfedge_t* surfedge; // Surface edge
    face_data->num_verts = 0; // Init num verts

    for (surfedge = surf->firstsurfedge; e < surf->numsurfedges; surfedge++)
    {
        VectorCopy(surfedge->edge->v[0]->point, face_data->verts[v]);
        v++;
        VectorCopy(surfedge->edge->v[1]->point, face_data->verts[v]);
        v++;
        face_data->num_verts += 2;
        e++;
    }
}

// Repair any malformed vertex data
void FixMalformedVertexData(surface_data_t* face_data)
{
    /*
    * MAP: Deepcanyon
    GL_DrawEdges face:988 verts:10 norm[1.000000 0.000000 0.000000] tr.norm[1.000000 0.000000 0.000000] vol:7202.355957
    --
    GL_DrawEdges face:988 edge:0 [16.000000 1641.672607 912.000000] -- [16.000000 1641.170410 911.749451]
    GL_DrawEdges face:988 edge:1 [16.000000 1641.170410 911.749451] -- [16.000000 1472.000000 827.353455]
    GL_DrawEdges face:988 edge:2 [16.000000 1472.000000 912.000000] -- [16.000000 1472.000000 827.353455]
    GL_DrawEdges face:988 edge:3 [16.000000 1641.170410 911.749451] -- [16.000000 1472.000000 912.000000]
    GL_DrawEdges face:988 edge:4 [16.000000 1641.672607 912.000000] -- [16.000000 1641.170410 911.749451]
    */
    // Fix a bug in the vertex data where the first and second vertex are the same as the last and second last vertex
    if ((UTIL_VectorCompare(face_data->verts[0], face_data->verts[face_data->num_verts - 2], 0.01) || UTIL_VectorCompare(face_data->verts[0], face_data->verts[face_data->num_verts - 1], 0.01))
        && (UTIL_VectorCompare(face_data->verts[1], face_data->verts[face_data->num_verts - 1], 0.01) || UTIL_VectorCompare(face_data->verts[1], face_data->verts[0], 0.01)))
    {
        // Move all the verts down by two
        for (int i = 0; i < face_data->num_verts; i++)
        {
            VectorCopy(face_data->verts[i + 2], face_data->verts[i]);
        }

        // Remove the first two and last two verts
        face_data->num_verts -= 4;

        Com_Printf("%s BUGFIX: removed first and last two verts of face: %d\n", __func__, face_data->facenum);
    }

    // Look for an uneven amount of verts
    if (face_data->num_verts % 2 != 0)
	{
		// Remove the last vert
		face_data->num_verts -= 1;

		Com_Printf("%s BUGFIX: removed last vert of face: %d\n", __func__, face_data->facenum);
	}
}

// Using the raw vertex data output the result of aligning parallel edges so they form a straight line
// 1) Nearest edges that are parallel are aligned into a single edge, 
// 2) Remove the inner vertex points leaving only the outer vertex points
// 3) The result is a straight line with a start and end point
void AlignVertexPoints(surface_data_t* face_data)
{
    vec3_t v1 = { 0 }; // Current vert 1 (current edge)
    vec3_t v2 = { 0 }; // Current vert 2 (current edge)
    vec3_t p1 = { 0 }; // Previous vert 1 (prev edge)
    vec3_t p2 = { 0 }; // Previous vert 2 (prev edge)
    vec3_t dir = { 0 }; // Dir vector
    vec3_t angle = { 0 }; // Angle vector

    // Get the direction of the edge to test against
    vec3_t dir_forward = { 0 };
    vec3_t dir_backward = { 0 };

    // Get vector angle of dir_forward
    vec3_t angle_forward = { 0 };
    vec3_t angle_backward = { 0 };

    if (face_data->num_verts >= 6) // Min 3 edges (triangle), 6 verts
    {
        // Get the first vert
        if (VectorCompare(face_data->verts[0], face_data->verts[face_data->num_verts - 2]) || VectorCompare(face_data->verts[0], face_data->verts[face_data->num_verts - 1]))
        {
            VectorCopy(face_data->verts[0], face_data->first_vert);
        }
        else if (VectorCompare(face_data->verts[1], face_data->verts[face_data->num_verts - 2]) || VectorCompare(face_data->verts[1], face_data->verts[face_data->num_verts - 1]))
        {
            VectorCopy(face_data->verts[1], face_data->first_vert);
        }

        // Save the first vert
        VectorCopy(face_data->first_vert, face_data->aligned_verts[0]);
        face_data->num_aligned_verts = 1; // Init the first aligned vert

        // Init the forward and backward angles
        VectorCopy(face_data->verts[0], v1); // Current vert v1
        VectorCopy(face_data->verts[1], v2); // Current vert v2
        VectorSubtract(v2, v1, dir_forward); // Forward vector
        VectorSubtract(v1, v2, dir_backward); // Backward vector
        vectoangles2(dir_forward, angle_forward); // Forward angle
        vectoangles2(dir_backward, angle_backward); // Backward angle

        // Align the raw verts
        for (int v = 2; v < face_data->num_verts; v += 2) // Edges
        {
            // Copy the current verts
            VectorCopy(face_data->verts[v + 0], v1); // Current vert v1
            VectorCopy(face_data->verts[v + 1], v2); // Current vert v2

            // Copy the previous verts
            VectorCopy(face_data->verts[v - 2], p1); // Previous vert p1
            VectorCopy(face_data->verts[v - 1], p2); // Previous vert p2

            // Get the direction of the current verts
            VectorSubtract(v2, v1, dir); // Current edge vector
            vectoangles2(dir, angle); // Current edge angle

            // Check if the angle is the same as angle_forward or angle_backward
            //if (UTIL_VectorCompare(angle, angle_forward, angle_tolerance) || UTIL_VectorCompare(angle, angle_backward, angle_tolerance))
            if (VectorCompare(angle, angle_forward) || VectorCompare(angle, angle_backward))
            {
                if (v == face_data->num_verts - 2) // Last edge
                {
                    VectorCopy(face_data->first_vert, face_data->aligned_verts[face_data->num_aligned_verts]);
                    face_data->num_aligned_verts++;
                    break; // Finished
                }
                // The current edge is in the same direction as the previous edge, skip it
                continue;
            }
            else
            {
                if (VectorCompare(v1, p1) || VectorCompare(v1, p2))
                {
                    VectorCopy(v1, face_data->aligned_verts[face_data->num_aligned_verts]);
                    face_data->num_aligned_verts++;
                    VectorCopy(v1, face_data->aligned_verts[face_data->num_aligned_verts]);
                    face_data->num_aligned_verts++;
                }
                else if (VectorCompare(v2, p1) || VectorCompare(v2, p2))
                {
                    VectorCopy(v2, face_data->aligned_verts[face_data->num_aligned_verts]);
                    face_data->num_aligned_verts++;
                    VectorCopy(v2, face_data->aligned_verts[face_data->num_aligned_verts]);
                    face_data->num_aligned_verts++;
                }

                if (v == face_data->num_verts - 2)
                {
                    VectorCopy(face_data->first_vert, face_data->aligned_verts[face_data->num_aligned_verts]);
                    face_data->num_aligned_verts++;
                    break; // Finished
                }

                // Set the new forward and backward angles
                VectorCopy(face_data->verts[v + 0], v1); // Current vert v1
                VectorCopy(face_data->verts[v + 1], v2); // Current vert v2
                VectorSubtract(v2, v1, dir_forward); // Forward vector
                VectorSubtract(v1, v2, dir_backward); // Backward vector
                vectoangles2(dir_forward, angle_forward); // Forward angle
                vectoangles2(dir_backward, angle_backward); // Backward angle
            }
        }
    }
}

// Checks if face is liquid: water/lava/slime
qboolean CheckFaceLiquid(surface_data_t* face_data)
{
    const float min_max = 0.1; // Min/max value for the mins/maxs of the trace
    vec3_t mins = { -min_max,-min_max,-min_max };
    vec3_t maxs = { min_max,min_max,min_max };
    trace_t tr = SV_Trace(face_data->center_poly, mins, maxs, face_data->center_poly, NULL, MASK_WATER);
    if (tr.startsolid == false)
        return false;
    else
        return true;
}

// Checks if face is solid
qboolean CheckFaceSolid(surface_data_t* face_data)
{
    const float min_max = 0.1; // Min/max value for the mins/maxs of the trace
    vec3_t mins = { -min_max,-min_max,-min_max };
    vec3_t maxs = { min_max,min_max,min_max };
    trace_t tr = SV_Trace(face_data->center_poly, mins, maxs, face_data->center_poly, NULL, MASK_PLAYERSOLID);
    if (tr.startsolid == false)
        return false;
    else
        return true;
}

// Checks if player fits above face
qboolean CheckSkyAboveFace(surface_data_t* face_data)
{
    vec3_t start = { face_data->center_poly[0], face_data->center_poly[1], face_data->center_poly[2] + 0.1 };
    vec3_t end = { face_data->center_poly[0], face_data->center_poly[1], face_data->center_poly[2] + 56 };
    trace_t tr = SV_Trace(start, NULL, NULL, end, NULL, MASK_PLAYERSOLID);
    //if (tr.startsolid || tr.fraction < 1)
    {

        //vec3_t start_higher = { face_data->center_poly[0], face_data->center_poly[1], face_data->center_poly[2] + 16 };
        //tr = SV_Trace(start_higher, NULL, NULL, end, NULL, MASK_PLAYERSOLID);
        //if (tr.startsolid)
        //    Com_Printf("%s eeeep!!!\n", __func__);
    }
    if (tr.surface->flags & SURF_SKY)
        return false;
    else
        return true;
}

void GetFaceContents(surface_data_t* face_data)
{
    face_data->contents = 0; // Surface contents
    trace_t tr_center; // Trace to the center of the polygon
    const float min_max = 0.01; // Min/max value for the mins/maxs of the trace
    vec3_t mins = { -min_max,-min_max,-min_max };
    vec3_t maxs = { min_max,min_max,min_max };
    qboolean foundLadder = true; // Normalize ladder to be true, until found otherwise

    // Check if our starting point is not inside a solid
    tr_center = SV_Trace(face_data->center_poly_32_units, mins, maxs, face_data->center_poly_32_units, NULL, MASK_PLAYERSOLID); // From polygon centoid outside
    if (tr_center.startsolid == false)
    {
        // Test from starting point to center of polygon
        tr_center = SV_Trace(face_data->center_poly_32_units, mins, maxs, face_data->center_poly, NULL, MASK_PLAYERSOLID); // From polygon centoid outside to inside MASK_UNLIMITED

        face_data->contents = tr_center.contents;

        // Get distances - If the difference is greater than MAX_STEEPNESS, then the trace is not accurate enough
        float target_dist = VectorDistance(face_data->center_poly_32_units, face_data->center_poly); // Distance between starting point and center of polygon
        float trace_dist = VectorDistance(face_data->center_poly_32_units, tr_center.endpos); // Distance between starting point and end of a trace
        float diff_dist = fabs(target_dist - trace_dist); // Get the difference between the two distances in absolute value

        // Test surface is ladder and (the difference is less than MAX_STEEPNESS OR if the trace hit a playerclip)
        // NOTE: Some ladders have a playerclip around them, so the trace will hit the playerclip and not the intended ladder surface,
        //       but because the playerclip is CONTENTS_LADDER, this will be our ladder surface
        if ((tr_center.contents & CONTENTS_LADDER) && (diff_dist <= MAX_STEEPNESS || (tr_center.contents & CONTENTS_PLAYERCLIP)))
        {
        }
        else
            foundLadder = false; // Not a ladder

        // Look for liquids: water, lava, slime
        tr_center = SV_Trace(face_data->center_poly_32_units, mins, maxs, face_data->center_poly, NULL, MASK_WATER);
        if (tr_center.contents & CONTENTS_WATER)
			face_data->contents |= CONTENTS_WATER;
        else if (tr_center.contents & CONTENTS_LAVA)
            face_data->contents |= CONTENTS_LAVA;
        else if (tr_center.contents & CONTENTS_SLIME)
            face_data->contents |= CONTENTS_SLIME;


        // Look for trigger edicts (trigger_hurt)
        //tr_center = SV_Trace(face_data->center_poly_32_units, mins, maxs, face_data->center_poly, NULL, MASK_ALL);
        //if (tr_center.ent && tr_center.ent->s.number > 0)
        //    Com_Printf("%s: ENT num[%d]\n", __func__, tr_center.ent->s.number);
        //if (tr_center.ent->solid == SOLID_TRIGGER)
        //    Com_Printf("%s: ENT solid[%d] num[%d]\n", __func__, tr_center.ent->solid, tr_center.ent->s.number);
    }

    // Remove ladder contents from floors
    if (face_data->normal[2] == 1 && (face_data->contents & CONTENTS_LADDER))
    {
        //contents &= ~CONTENTS_LADDER;
    }

    // Remove ladders from invalid faces (surfaces that does not have its contents flagged as a CONTENTS_LADDER)
    if (foundLadder == false)
        face_data->contents &= ~CONTENTS_LADDER;
}

void InverseNormals(surface_data_t* face_data)
{
    VectorCopy(face_data->surf->plane->normal, face_data->normal);
    if (face_data->drawflags & DSURF_PLANEBACK)
    {
        VectorInverse(face_data->normal);
    }
}

facetype_t SetSurfaceType(surface_data_t *face_data)
{
    unsigned int surface_type = 0;

    //#define MASK_SURF_IGNORE (SURF_LIGHT|SURF_SLICK|SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING|SURF_NODRAW)
    //if (face_data->drawflags & (SURF_LIGHT | SURF_SLICK | SURF_WARP | SURF_TRANS33 | SURF_TRANS66 | SURF_FLOWING | SURF_NODRAW))
    if (face_data->drawflags & SURF_NODRAW)
    {
        //Com_Printf("%s: face[%d] drawflags:[0x%x]\n", __func__, face_data->facenum, face_data->drawflags);
        surface_type |= FACETYPE_IGNORED;
    }

    // Sky surfaces
    if (face_data->drawflags & SURF_SKY)
    {
        surface_type |= FACETYPE_SKY;
    }
    // Roof surfaces
    //if (face_data->surf->plane->normal[2] == 1) // Floor
    //{
        //if (face_data->surf->drawflags & DSURF_PLANEBACK) // Roof
        //    surface_type |= FACETYPE_ROOF;
    //}
    // Floors - perfectly flat
    if (face_data->normal[0] == 0 && face_data->normal[1] == 0 && face_data->normal[2] == 1)
    {
        if (face_data->surf->drawflags & DSURF_PLANEBACK) // Roof
            surface_type |= FACETYPE_ROOF;
        else // Floor
            surface_type |= FACETYPE_WALK;
    }
    // Any climbable sloped surface
    else if (face_data->normal[0] >= -MAX_STEEPNESS && face_data->normal[0] <= MAX_STEEPNESS && face_data->normal[1] >= -MAX_STEEPNESS && face_data->normal[1] <= MAX_STEEPNESS && face_data->normal[2] >= (MAX_STEEPNESS - 0.10)) // && face_data->normal[2] >= MAX_STEEPNESS)
        surface_type |= FACETYPE_WALK; // Allow variable steep surfaces, except overly steep ones
    // Anything too steep to climb (except ladders) becomes a wall -- Wall with normal[2] == 0 means the wall is perfectly upright
    else
        surface_type |= FACETYPE_WALL; // Ignore walls
    // Angled slope with normal facing down
    //if (face_data->normal[2] < 0) return true;

    if (face_data->contents & CONTENTS_LADDER)
        surface_type |= FACETYPE_LADDER;

    if (face_data->contents & CONTENTS_WATER)
        surface_type |= FACETYPE_WATER;

    if ( (face_data->contents & CONTENTS_LAVA) || (face_data->contents & CONTENTS_SLIME))
        surface_type |= FACETYPE_DAMAGE;

    if (IgnoreSmallSurfaces)
    {
        // Ignore small volumes
        if (face_data->volume < 8) // 128
            surface_type |= FACETYPE_IGNORED;

        // Ignore small surfaces
        if (face_data->max_length < 16)
            surface_type |= FACETYPE_IGNORED;
        if (face_data->min_length < 4)
            surface_type |= FACETYPE_IGNORED;
    }
    else if (surface_type & FACETYPE_WALK)
    {
        // Flag small volumes
        if (face_data->volume < 8) // 128
            surface_type |= FACETYPE_TINYWALK;

        // Ignore small surfaces
        if (face_data->max_length < 16)
            surface_type |= FACETYPE_TINYWALK;
        if (face_data->min_length < 4)
            surface_type |= FACETYPE_TINYWALK;
    }

    return surface_type;
}

// Gets the min / max length of the face
void UTIL_GetFaceLengthMinMax(surface_data_t* face_data)
{
    int i, j = 0;
    float length = 0;

    // Init min/max
    face_data->max_length = 0.0;
    face_data->min_length = 9999.0;

    if (face_data->facenum == 593 || face_data->facenum == 594)
    {
		int a = 0;
    }

    // Poly center -> edge
    // For each aligned edge
    for (i = 0; i < face_data->num_aligned_verts / 2; i++)
	{
        // Get the distance from poly center to edge
        length = VectorDistance(face_data->center_poly, face_data->aligned_edge_center[i]);

		// If the length is greater than the current greatest length, set it as the greatest length
		if (length > face_data->max_length)
			face_data->max_length = length;

        // If the length is less than the current smallest length, set it as the smallest length
		if (length < face_data->min_length)
			face_data->min_length = length;
	}

    // Edge -> edge
    // For each aligned edge
    for (i = 0; i < face_data->num_aligned_verts / 2; i++)
    {
        for (j = 0; j < face_data->num_aligned_verts / 2; j++)
        {
            if (i == j) continue; // Skip self

            // Get the distance from edge to edge
            length = VectorDistance(face_data->aligned_edge_center[i], face_data->aligned_edge_center[j]);

            // If the length is greater than the current greatest length, set it as the greatest length
            if (length > face_data->max_length)
                face_data->max_length = length;

            // If the length is less than the current smallest length, set it as the smallest length
            if (length < face_data->min_length)
                face_data->min_length = length;
        }
    }
}


void UTIL_GetCenterOfVertEdges(surface_data_t* face_data)
{
    // For each vert edge
    for (int i = 0; i < face_data->num_verts; i += 2)
    {
		// Get the center of the edge
		VectorAdd(face_data->verts[i], face_data->verts[i + 1], face_data->edge_center[i / 2]);
		VectorScale(face_data->edge_center[i / 2], 0.5, face_data->edge_center[i / 2]);

        // Center edge moved up by STEPSIZE
        vec3_t center_vec = { 0 }; // Center point vector
        MoveAwayFromNormal(face_data->drawflags, face_data->normal, center_vec, STEPSIZE); // Get the vector
        VectorAdd(face_data->edge_center[i / 2], center_vec, face_data->edge_center_stepsize[i / 2]); // Move away from the surface normal by STEPSIZE units
    }
}

void UTIL_GetCenterOfAlignedEdges(surface_data_t* face_data)
{
    // For each aligned edge
    for(int i = 0; i < face_data->num_aligned_verts; i += 2)
    {
        // Get the center of the edge
        VectorAdd(face_data->aligned_verts[i], face_data->aligned_verts[i + 1], face_data->aligned_edge_center[i / 2]);
        VectorScale(face_data->aligned_edge_center[i / 2], 0.5, face_data->aligned_edge_center[i / 2]);

        // Center edge moved up by STEPSIZE
        vec3_t center_vec = { 0 }; // Center point vector
        MoveAwayFromNormal(face_data->drawflags, face_data->normal, center_vec, STEPSIZE); // Get the vector
        VectorAdd(face_data->aligned_edge_center[i / 2], center_vec, face_data->aligned_edge_center_stepsize[i / 2]); // Move away from the surface normal by STEPSIZE units
    }
}

// Similar to VectorCompare, except with a height tolerance
qboolean UTIL_VectorCompareHeight(vec3_t v1, vec3_t v2, float height_difference)
{
	if (v1[0] == v2[0] && v1[1] == v2[1] && v1[2] >= (v2[2] - height_difference) && v1[2] <= (v2[2] + height_difference))
		return true;
	return false;
}
// Compare two edges to see if they are the same
qboolean UTIL_EdgeCompare(vec3_t edge1_v1, vec3_t edge1_v2, vec3_t edge2_v1, vec3_t edge2_v2) //, vec3_t center_edge)
{
	if ( VectorCompare(edge1_v1, edge2_v1) || VectorCompare(edge1_v1, edge2_v2) ) // Edge 1 (vertex 1) == Edge 2 (vertex 1 or 2)
    {
		if ( VectorCompare(edge1_v2, edge2_v1) || VectorCompare(edge1_v2, edge2_v2) ) // Edge 1 (vertex 2) == Edge 2 (vertex 1 or 2)
		{
            return true;
		}
	}
    return false;
}
// Compares two edges to see if they share the same edge
// v1 & v2 is edge 1
// v3 & v4 is edge 2
qboolean UTIL_EdgeCompare2(vec3_t v1, vec3_t v2, vec3_t v3, vec3_t v4)
{
    if (VectorCompare(v1, v3) && VectorCompare(v2, v4)) return true;
    if (VectorCompare(v1, v4) && VectorCompare(v2, v3)) return true;
    return false;
}
//#define EdgeCompare(v1, v2, v3, v4) ( ( VectorCompare(v1, v3) && VectorCompare(v2, v4) ) || ( VectorCompare(v1, v4) && VectorCompare(v2, v3) ) )
//VectorCompare(v1, v3)         ((v1)[0]==(v3)[0]&&(v1)[1]==(v3)[1]&&(v1)[2]==(v3)[2])
//VectorCompare(v2, v4)         ((v2)[0]==(v4)[0]&&(v2)[1]==(v4)[1]&&(v2)[2]==(v4)[2])
//VectorCompare(v1, v4)         ((v1)[0]==(v4)[0]&&(v1)[1]==(v4)[1]&&(v1)[2]==(v4)[2])
//VectorCompare(v2, v3)         ((v2)[0]==(v3)[0]&&(v2)[1]==(v3)[1]&&(v2)[2]==(v3)[2])
#define EdgeCompare(v1, v2, v3, v4) ( ( ((v1)[0]==(v3)[0]&&(v1)[1]==(v3)[1]&&(v1)[2]==(v3)[2]) && ((v2)[0]==(v4)[0]&&(v2)[1]==(v4)[1]&&(v2)[2]==(v4)[2]) ) || ( ((v1)[0]==(v4)[0]&&(v1)[1]==(v4)[1]&&(v1)[2]==(v4)[2]) && ((v2)[0]==(v3)[0]&&(v2)[1]==(v3)[1]&&(v2)[2]==(v3)[2]) ) )

// Runs a player sized box test from the position of the feet to the position of the head
qboolean UTIL_PlayerBoxTestCouched(vec3_t pos)
{
    // Height test lifted from STEPSIZE 18 to 28 to accommodate for box test edges and slopes
    vec3_t start = { pos[0], pos[1], pos[2] + STEPSIZE };
    const float min_height = 10;

    vec3_t min = { -15.99, -15.99, 0 };
    vec3_t max = { 15.99, 15.99, min_height };

    trace_t tr = SV_Trace(start, min, max, start, NULL, MASK_PLAYERSOLID);
    if (tr.startsolid || tr.allsolid)
        return false;
    if (tr.fraction == 1.0)
        return true;
    else
        return false;
}
qboolean UTIL_PlayerBoxTestStanding(vec3_t pos, vec3_t normal)
{
    // Adjust the start position to be away from the surface normal
    vec3_t start = { pos[0], pos[1], pos[2] };
    vec3_t center_vec;
    const float normal_height = 8; // Move distance
    MoveAwayFromNormal(0, normal, center_vec, normal_height); // Get the vector
    VectorAdd(start, center_vec, start); // Move away from the surface normal

    //const float min_height = 20; // crouched min/max (-24 + 4) = 28.  (28 - normal_height = 20)
    const float max_height = 48; // standing min/max (-24 + 32) = 56    (56 - normal_height = 48)

    vec3_t min = { -15.99, -15.99, 0 };
    vec3_t max = { 15.99, 15.99, max_height };

    trace_t tr = SV_Trace(start, min, max, start, NULL, MASK_PLAYERSOLID);
    if (tr.startsolid || tr.allsolid)
        return false;
    if (tr.fraction == 1.0)
        return true;
    else
        return false;
}

qboolean UTIL_LineOfSight(vec3_t start, vec3_t end, vec3_t normal)
{
    // Calculate distance
    if (VectorDistance(start, end) > 256) // Ignore long distances
		return false;

    // Caculate the height difference
    //if (fabs(start[2] - end[2]) > 60) //max jump height //STEPSIZE
    //    return false;

    // Box test the start position
    //if (normal[0] == 0 && normal[1] == 0 && normal[2] == 1) // Flat floors
    //    if (UTIL_PlayerBoxTestCouched(start) == false) // Test if the start position fits
	//	    return false;

    // LOS + PLAYER WIDTH + CROUCH HEIGHT test start to end
    vec3_t min = { -15.99, -15.99, 28 };
    //if (normal[0] == 0 && normal[1] == 0 && normal[2] == 1) // Flat floors
        min[2] = 17.99;
    vec3_t max = { 15.99, 15.99, 28 };
    trace_t tr = SV_Trace(start, min, max, end, NULL, MASK_PLAYERSOLID);
    
    if (tr.startsolid || tr.allsolid)
		return false;

    if (tr.fraction == 1.0)
    {
        // Test if the end position fits
        return UTIL_PlayerBoxTestCouched(tr.endpos);
        //return true;
    }

    return false;
}

// Accepts two vectors and a percent, returns a point between the two vectors based on the percent
// Percent is from 0.0 to 1.0
qboolean BOTLIB_UTIL_PositionBetweenTwoPoints(vec3_t v1, vec3_t v2, float percent, vec3_t out)
{
    if (percent < 0 || percent > 1)
        return false;

    // Calculate the distance between the two vectors
    float distance = VectorDistance(v1, v2);

    // Ensure we have a valid distance
    if (distance > 32)
    {
        // Get the position between v1 and v2 based on the percent, putting the result in out
        VectorAdd(v1, v2, out);
        VectorScale(out, percent, out);

        return true;
    }

    return false;
}

// Rotate the center of an edge around a center point, then extend the center point out by a distance
// 
//            (end) <--- desired result
//              |   <--- distance
//              |
//              |   
//              |\
//              | \ <-- rotated edge (rotation here is 90 degrees left)
//              |  \
//    (v1) -----+----- (v2)  ( 0 degrees --> )
//           (center)
//
// v1 and v2 are the start and end points of an edge
// float rotate the degrees of rotation
// float distance is the distance from the center point
// vec3_t end is the rotated end point
void BOTLIB_UTIL_ROTATE_CENTER(vec3_t v1, vec3_t v2, float rotate, float distance, vec3_t out, vec3_t normal)
{
    vec3_t center, angle, forward; // , right, offset;

    // print warning if distance is > 3000
    if (distance > 3000)
    {
        Com_Printf("%s WARNING: distance > 3000. Cannot rotate accurately.\n", __func__);
    }

    // Find the middle between two points of an edge
    LerpVector(v1, v2, 0.50, center);

    // Get direction vector
    VectorSubtract(v2, v1, forward);

    // Normalize the direction vector
    VectorNormalize(forward);

    // Get the angle of the direction vector
    vectoangles2(forward, angle);

    // Rotation
    angle[YAW] += rotate; // Rotation angle

    // Rotate the direction vector
    AngleVectors(angle, forward, NULL, NULL);

    // Get the rotated end point and extend it out based on distance
    VectorMA(center, distance, forward, out);

    // Calculate the new normal of the edge
    if (normal != NULL)
    {
        CrossProduct(center, out, normal);
        VectorNormalize(normal);
    }
}
// v1 -> v2 is the edge (use to get direction
// point is what we want to move against the direction of the edge (90 degrees rotatation is perpendicular to the edge)
// rotation of the point, with the direction of the edge at 0 degrees
// distance to move away from the edge
// out is the result
void BOTLIB_UTIL_ROTATE_EDGE_POINT(vec3_t v1, vec3_t v2, vec3_t point, float rotate, float distance, vec3_t out)
{
    vec3_t angle, forward; // , right, offset;

    // print warning if distance is > 3000
    if (distance > 3000)
    {
        Com_Printf("%s WARNING: distance > 3000. Cannot rotate accurately.\n", __func__);
    }

    // Get direction vector
    VectorSubtract(v2, v1, forward);

    // Normalize the direction vector
    VectorNormalize(forward);

    // Get the angle of the direction vector
    vectoangles2(forward, angle);

    // Rotation
    angle[YAW] += rotate; // Rotation angle

    // Rotate the direction vector
    AngleVectors(angle, forward, NULL, NULL);

    // Get the rotated end point and extend it out based on distance
    VectorMA(point, distance, forward, out);
}

// Returns true for any climbable sloped surface
qboolean UTIL_IsClimableSlope(vec3_t normal)
{
    // Floor
    if (normal[0] == 0 && normal[1] == 0 && normal[2] == 1)
        return false;
    // Climable slope
    if (normal[0] >= -MAX_STEEPNESS && normal[0] <= MAX_STEEPNESS && normal[1] >= -MAX_STEEPNESS && normal[1] <= MAX_STEEPNESS && normal[2] >= (MAX_STEEPNESS - 0.10)) // && tr_right.plane.normal[2] >= MAX_STEEPNESS)
        return true;
    else
        return false;
}
qboolean UTIL_IsFlatFloor(vec3_t normal)
{
    if (normal[0] == 0 && normal[1] == 0 && normal[2] == 1)
		return true;
	else
		return false;
}

// Finds if the edge is a ledge
// Returns the ledge data struct: normal, endpos, ledge height
ledge_data_t UTIL_FindLedge(nav_t* nav, surface_data_t* face_data, vec3_t v1, vec3_t v2)
{
    ledge_data_t ledge;
    memset(&ledge, 0, sizeof(ledge_data_t));

    int facenum;
    vec3_t start_left, start_right;
    //vec3_t normal = { 0, 0, 1 }; // perfectly flat

    if (0)
    {
        // Debug
        LerpVector(v1, v2, 0.50, start_left);
        LerpVector(v2, v1, 0.50, start_right);
        start_left[2] += 8;
        start_right[2] += 8;
        vec3_t end_left = { start_left[0], start_left[1], -4096 };
        vec3_t end_right = { start_right[0], start_right[1], -4096 };
        VectorCopy(start_left, ledge.left_start);
        VectorCopy(start_right, ledge.right_start);
        VectorCopy(end_left, ledge.left_end);
        VectorCopy(end_right, ledge.right_end);
        return ledge;
    }

    // Ignore all non-walkable faces
    if ( (face_data->face_type & FACETYPE_WALK) == 0 && (face_data->face_type & FACETYPE_TINYWALK) == 0)
        return ledge;

    // Ignore liquid faces
    //if ((face_data->contents & CONTENTS_WATER) || (face_data->contents & CONTENTS_LAVA) || (face_data->contents & CONTENTS_SLIME))
	//	return ledge;

    for (int f = 0; f < nav->faces_total; f++)
    {
        for (int i = 0; i < nav->surface_data_faces[f].num_verts; i += 2)
        {
            // Compare the two edges to see if they share the same edge
            if (UTIL_EdgeCompare(v1, v2, nav->surface_data_faces[f].verts[i], nav->surface_data_faces[f].verts[i + 1]))
            {
                // Ignore all walkable faces
                if ((nav->surface_data_faces[f].face_type & FACETYPE_WALK) == 0 && (nav->surface_data_faces[f].face_type & FACETYPE_TINYWALK) == 0)
                {
                    // Test if 'ledge' is a non-solid face
                    if (1)
                    {
                        vec3_t mins = { -1, -1, -0 };
                        vec3_t maxs = { 1, 1, 1 };
                        trace_t tr = SV_Trace(v1, mins, maxs, v2, NULL, MASK_ALL); // MASK_PLAYERSOLID | MASK_WATER);
                        if (tr.startsolid == false && tr.fraction == 1.0)
                            return ledge; // Not a ledge

                        // Ignore liquid faces
                        //if ((face_data->contents & CONTENTS_WATER) || (face_data->contents & CONTENTS_LAVA) || (face_data->contents & CONTENTS_SLIME))
                        //	return ledge;
                    }

                    // Find a face which is not our own face

                    /*
                    // Left
                    BOTLIB_UTIL_ROTATE_CENTER(v1, v2, 90, 4.01, start_left, NULL);    // Turn 90 degrees left and move x units
                    start_left[2] += 8.1;
                    vec3_t end_left = { start_left[0], start_left[1], -4096 };
                    trace_t tr_left = SV_Trace(start_left, NULL, NULL, end_left, NULL, MASK_PLAYERSOLID | MASK_WATER);
                    short facenum_left = UTIL_FindFace(nav, tr_left.endpos, tr_left.plane.normal); // Get facenum of the face that was hit
                    if (facenum_left == f) // same face
                    {
                        VectorCopy(start_left, end_left);
                        end_left[2] += 32;
                        BOTLIB_UTIL_ROTATE_CENTER(v1, v2, 90, 4.01, start_left, NULL);    // Turn 90 degrees left and move x units
                        BOTLIB_BSP_AddAreaConnections(nav, f, f, start_left, end_left, FACE_CONN_DIRECT, FACE_MOVE_GROUND, 0);
                    }
                    */

                    // Right
                    BOTLIB_UTIL_ROTATE_CENTER(v2, v1, 90, 4.01, start_right, NULL);   // Turn 90 degrees right and move x units
                    start_right[2] += 8.1;
                    vec3_t end_right = { start_right[0], start_right[1], -4096 };
                    trace_t tr_right = SV_Trace(start_right, NULL, NULL, end_right, NULL, MASK_PLAYERSOLID | MASK_WATER);
                    short facenum_right = UTIL_FindFace(nav, tr_right.endpos, tr_right.plane.normal); // Get facenum of the face that was hit

                    /*
                    // Left
                    if (facenum_left != INVALID && facenum_left != face_data->facenum) // If valid face (i.e. any surface that was included in the surface_data_t array)
                    {
                        ledge.is_ledge = true;
                        VectorCopy(tr_left.plane.normal, ledge.normal);
                        VectorCopy(start_left, ledge.startpos);
                        VectorCopy(tr_left.endpos, ledge.endpos);
                        ledge.height = abs(tr_left.endpos[2] - start_left[2]);
                        //return ledge;

                        // Debug
                        ledge.hit_side = 1; // none = 0, left = 1, right = 2
                    }
                    */

                    // Right
                    //if (facenum_right != INVALID && facenum_right != face_data->facenum) // If valid face (i.e. any surface that was included in the surface_data_t array)
                    {


                        ledge.is_ledge = true;
                        VectorCopy(tr_right.plane.normal, ledge.normal);
                        VectorCopy(start_right, ledge.startpos);
                        VectorCopy(tr_right.endpos, ledge.endpos);
                        VectorCopy(v1, ledge.v1);
                        VectorCopy(v2, ledge.v2);
                        ledge.height = abs(tr_right.endpos[2] - start_right[2]);
                        //return ledge;




                        // Test if ledge is a wall
                        vec3_t v1_wall, v2_wall;
                        const float MOVE_AWAY = 0.5;
                        vec3_t forward;
                        vec3_t center_start, center_end;
                        LerpVector(v1, v2, 0.50, center_start);
                        VectorCopy(center_start, center_end);
                        center_start[2] += 0.25;
                        center_end[2] += 32;
                        vec3_t mins = { -0.1, -0.1, 0 };
                        vec3_t maxs = { 0.1, 0.1, 32 };
                        //trace_t tr_point = SV_Trace(center_start, NULL, NULL, center_start, NULL, MASK_PLAYERSOLID | MASK_WATER);
                        //trace_t tr_line = SV_Trace(center_start, NULL, NULL, center_end, NULL, MASK_PLAYERSOLID | MASK_WATER);
                        trace_t tr_box = SV_Trace(center_start, mins, maxs, center_start, NULL, MASK_SOLID);
                        //if (tr_point.startsolid || tr_line.fraction < 1.0)
                        if (tr_box.startsolid || tr_box.fraction < 1.0)
                        {
                            ledge.is_ledge = false;
                            ledge.is_wall = true;

                            if (0) // Adjust wall edge to be away from the wall
                            {
                                // Move away from surface normal
                                //nav->surface_data_faces[f].normal;

                                // Try heading 90 degrees right
                                BOTLIB_UTIL_ROTATE_EDGE_POINT(v2, v1, v1, 90, MOVE_AWAY, v1_wall);   // Move v1 away from the wall
                                BOTLIB_UTIL_ROTATE_EDGE_POINT(v2, v1, v2, 90, MOVE_AWAY, v2_wall);   // Move v2 away from the wall

                                // Move v1_wall and v2_wall slightly above the ground
                                v1_wall[2] += 1;
                                v2_wall[2] += 1;

                                // Test new edge to see if it's free from the wall
                                LerpVector(v1_wall, v2_wall, 0.50, center_start);
                                //center_start[2] += 1;
                                trace_t tr_point = SV_Trace(center_start, NULL, NULL, center_start, NULL, MASK_SOLID);
                                if (tr_point.startsolid) // Still inside wall, switch sides
                                {
                                    // Try heading 90 degrees left
                                    BOTLIB_UTIL_ROTATE_EDGE_POINT(v1, v2, v1, 90, MOVE_AWAY, v1_wall);   // Move v1 away from the wall
                                    BOTLIB_UTIL_ROTATE_EDGE_POINT(v1, v2, v2, 90, MOVE_AWAY, v2_wall);   // Move v2 away from the wall

                                    // Move v1_wall and v2_wall slightly above the ground
                                    v1_wall[2] += 1;
                                    v2_wall[2] += 1;
                                }

                                // Test if v1_wall or v2_wall are inside a solid
                                VectorSubtract(v2_wall, v1_wall, forward);
                                VectorNormalize(forward);
                                tr_point = SV_Trace(v1_wall, NULL, NULL, v1_wall, NULL, MASK_PLAYERSOLID | MASK_WATER);
                                if (tr_point.startsolid)
                                {
                                    VectorMA(v1_wall, MOVE_AWAY, forward, v1_wall); // Bring v1_wall closer to v2_wall by MOVE_AWAY units
                                }
                                else
                                {
                                    VectorMA(v1_wall, -MOVE_AWAY, forward, v1_wall); // Take v1_wall further from v2_wall by MOVE_AWAY units
                                }

                                tr_point = SV_Trace(v2_wall, NULL, NULL, v2_wall, NULL, MASK_PLAYERSOLID | MASK_WATER);
                                if (tr_point.startsolid)
                                {
                                    VectorMA(v2_wall, -MOVE_AWAY, forward, v2_wall); // Bring v2_wall closer to v1_wall by MOVE_AWAY units
                                }
                                else
                                {
                                    VectorMA(v2_wall, MOVE_AWAY, forward, v2_wall); // Take v2_wall further from v1_wall by MOVE_AWAY units
                                }





                                /*
                                // Move v1_wall and v2_wall x units apart from each other
                                // Get direction vector
                                vec3_t forward;
                                VectorSubtract(v2_wall, v1_wall, forward);
                                VectorNormalize(forward);
                                VectorMA(v2_wall, 0.25, forward, v2_wall);
                                VectorMA(v1_wall, -0.25, forward, v1_wall);
                                */

                                // Move v1_wall and v2_wall slightly above the ground
                                //v1_wall[2] += 4;
                                //v2_wall[2] += 4;
                                //trace_t tr_v1 = SV_Trace(v1_wall, NULL, NULL, v1, NULL, MASK_PLAYERSOLID | MASK_WATER);
                                //v1_wall[2] = tr_v1.endpos[2];
                                //trace_t tr_v2 = SV_Trace(v2_wall, NULL, NULL, v2, NULL, MASK_PLAYERSOLID | MASK_WATER);
                                //v2_wall[2] = tr_v2.endpos[2];

                                //v1_wall[2] -= 0.9;
                                //v2_wall[2] -= 0.9;

                                // Copy the final result
                                VectorCopy(v1_wall, ledge.v1);
                                VectorCopy(v2_wall, ledge.v2);
                            }
                        }



                        // Debug
                        ledge.hit_side = 2; // none = 0, left = 1, right = 2
                    }

                    /*
                    // Debug
                    //ledge.hit_side = 2; // none = 0, left = 1, right = 2
                    VectorCopy(start_left, ledge.left_start);
                    VectorCopy(tr_left.endpos, ledge.left_end);
                    VectorCopy(start_right, ledge.right_start);
                    VectorCopy(tr_right.endpos, ledge.right_end);
                    //
                    if (0)
                    {
                        LerpVector(v1, v2, 0.50, start_left);
                        LerpVector(v2, v1, 0.50, start_right);
                        start_left[2] += 8;
                        start_right[2] += 8;
                        VectorCopy(start_left, ledge.left_start);
                        VectorCopy(start_right, ledge.right_start);
                        VectorCopy(end_left, ledge.left_end);
                        VectorCopy(end_right, ledge.right_end);
                    }
                    */

                    //Com_Printf("%s could not find edge at face %d\n", __func__, f);
                    //ledge.is_ledge = true;
                    return ledge;
                }
            }
        }
    }








    return ledge;







    const float DROP_DIST = 1.0; // 1.0
    const float MOVE_DIST = 1.0; // 16.0
    vec3_t mins = { -15.99, -15.99, 0 };
    vec3_t maxs = { 15.99, 15.99, 1 };

    // Trace sideways to find if two faces are connected
    {
        BOTLIB_UTIL_ROTATE_CENTER(v1, v2, 90, 0.1, start_left, NULL);	// Turn 90 degrees left and move 0.1 units
        BOTLIB_UTIL_ROTATE_CENTER(v2, v1, 90, 0.1, start_right, NULL);  // Turn 90 degrees right and move 0.1 units
        vec3_t end_left = { start_left[0], start_left[1], -4096 };
        vec3_t end_right = { start_right[0], start_right[1], -4096 };
        trace_t tr_left = SV_Trace(start_left, NULL, NULL, end_left, NULL, MASK_PLAYERSOLID);
        trace_t tr_right = SV_Trace(start_right, NULL, NULL, end_right, NULL, MASK_PLAYERSOLID);
        if (tr_left.startsolid && tr_right.startsolid) // Not a ledge
            return ledge;  // is_ledge == false;
    }

    //if (UTIL_IsClimableSlope(face_data->normal))
    //    return ledge;

    // Do another trace, hitting the face on the left and right of the edge, find a face which is not our own face
    BOTLIB_UTIL_ROTATE_CENTER(v1, v2, 90, 0.25, start_left, NULL);    // Turn 90 degrees left and move 0.25 units
    BOTLIB_UTIL_ROTATE_CENTER(v2, v1, 90, 0.25, start_right, NULL);   // Turn 90 degrees right and move 0.25 units
    start_left[2] += 0.1;
    start_right[2] += 0.1;
    vec3_t end_left = { start_left[0], start_left[1], -4096 };
    vec3_t end_right = { start_right[0], start_right[1], -4096 };
    trace_t tr_left = SV_Trace(start_left, NULL, NULL, end_left, NULL, MASK_PLAYERSOLID);
    trace_t tr_right = SV_Trace(start_right, NULL, NULL, end_right, NULL, MASK_PLAYERSOLID);


    

    // If we went left and hit the same face as our own, then trace right
    facenum = UTIL_FindFace(nav, tr_left.endpos, tr_left.plane.normal); // Get facenum of the face that was hit
    if (facenum != INVALID) // If valid face (i.e. any surface that was included in the surface_data_t array)
    {
        //if (UTIL_IsClimableSlope(tr_left.plane.normal)) return ledge;

        if (facenum == face_data->facenum)
        {
            BOTLIB_UTIL_ROTATE_CENTER(v2, v1, 90, MOVE_DIST, start_right, NULL); // Turn 90 degrees right and move MOVE_DIST units
            end_right[0] = start_right[0];
            end_right[1] = start_right[1];
            end_right[2] = -4096;
            //tr_right = SV_Trace(start_right, mins, maxs, end_right, NULL, MASK_PLAYERSOLID);
            tr_right = SV_Trace(start_right, NULL, NULL, end_right, NULL, MASK_PLAYERSOLID);
            if (tr_right.startsolid)
                return ledge;

            facenum = UTIL_FindFace(nav, tr_right.endpos, tr_right.plane.normal); // Get facenum of the face that was hit
            if (facenum != INVALID) // If valid face (i.e. any surface that was included in the surface_data_t array)
            {
                if (facenum == face_data->facenum)
                    Com_Printf("%s facenum == face_data->facenum %d\n", __func__, facenum);

                for (int i = 0; i < nav->surface_data_faces[facenum].num_verts; i += 2)
                {
                    // Compare the two edges to see if they share the same edge
                    if (UTIL_EdgeCompare(v1, v2, nav->surface_data_faces[facenum].verts[i], nav->surface_data_faces[facenum].verts[i + 1]))
                    {
                        //if (nav->surface_data_faces[facenum].face_type == FACETYPE_WALK || nav->surface_data_faces[facenum].face_type == FACETYPE_TINYWALK)
                        if ((face_data->face_type & FACETYPE_WALK) || (face_data->face_type & FACETYPE_TINYWALK))
                            return ledge;
                    }
                }
            }

            if (abs(tr_right.endpos[2] - start_right[2]) > DROP_DIST)
            {
                ledge.is_ledge = true;
                VectorCopy(tr_right.plane.normal, ledge.normal);
                VectorCopy(start_right, ledge.startpos);
                VectorCopy(tr_right.endpos, ledge.endpos);
                ledge.height = abs(tr_right.endpos[2] - start_right[2]);
                return ledge;
            }
        }

        // Facenum was different
        //if (0)
        //else if (UTIL_IsClimableSlope(tr_left.plane.normal))
        //else if (UTIL_IsClimableSlope(face_data->normal))
        else
        {
            BOTLIB_UTIL_ROTATE_CENTER(v2, v1, 90, 1, start_left, NULL); // Turn 90 degrees right and move MOVE_DIST units
            start_left[2] += 4;
            end_left[0] = start_left[0];
            end_left[1] = start_left[1];
            end_left[2] = -4096;
            //tr_left = SV_Trace(start_left, mins, maxs, end_left, NULL, MASK_PLAYERSOLID);
            tr_left = SV_Trace(start_left, NULL, NULL, end_left, NULL, MASK_PLAYERSOLID);
            if (tr_left.startsolid)
                return ledge;

            for (int i = 0; i < nav->surface_data_faces[facenum].num_verts; i += 2)
            {
                // Compare the two edges to see if they share the same edge
                if (UTIL_EdgeCompare(v1, v2, nav->surface_data_faces[facenum].verts[i], nav->surface_data_faces[facenum].verts[i + 1]))
                {
                    if (nav->surface_data_faces[facenum].face_type == FACETYPE_WALK || nav->surface_data_faces[facenum].face_type == FACETYPE_TINYWALK)
                        return ledge;
                }
            }

            //if (nav->surface_data_faces[facenum].face_type != FACETYPE_WALK && nav->surface_data_faces[facenum].face_type != FACETYPE_TINYWALK)
            if (abs(tr_left.endpos[2] - start_left[2]) > 4) // DROP_DIST
            {
                ledge.is_ledge = true;
                VectorCopy(tr_left.plane.normal, ledge.normal);
                VectorCopy(start_left, ledge.startpos);
                VectorCopy(tr_left.endpos, ledge.endpos);
                ledge.height = abs(tr_left.endpos[2] - start_left[2]);
                return ledge;
            }
        }
    }

    // If we went right and hit the same face as our own, then trace left
    facenum = UTIL_FindFace(nav, tr_right.endpos, tr_right.plane.normal); // Get facenum of the face that was hit
    if (facenum != INVALID) // If valid face (i.e. any surface that was included in the surface_data_t array)
    {
        //if (UTIL_IsClimableSlope(tr_right.plane.normal)) return ledge;

        // Facenum was the same as our own, so trace left
        if (facenum == face_data->facenum)
        {
            BOTLIB_UTIL_ROTATE_CENTER(v1, v2, 90, MOVE_DIST, start_left, NULL);	// Turn 90 degrees left and move MOVE_DIST units
            end_left[0] = start_left[0];
            end_left[1] = start_left[1];
            end_left[2] = -4096;
            //tr_left = SV_Trace(start_left, mins, maxs, end_left, NULL, MASK_PLAYERSOLID);
            tr_left = SV_Trace(start_left, NULL, NULL, end_left, NULL, MASK_PLAYERSOLID);
            if (tr_left.startsolid)
                return ledge;

            facenum = UTIL_FindFace(nav, tr_left.endpos, tr_left.plane.normal); // Get facenum of the face that was hit
            if (facenum != INVALID) // If valid face (i.e. any surface that was included in the surface_data_t array)
            {
                if (facenum == face_data->facenum)
                    Com_Printf("%s facenum == face_data->facenum %d\n", __func__, facenum);

                for (int i = 0; i < nav->surface_data_faces[facenum].num_verts; i += 2)
                {
                    // Compare the two edges to see if they share the same edge
                    if (UTIL_EdgeCompare(v1, v2, nav->surface_data_faces[facenum].verts[i], nav->surface_data_faces[facenum].verts[i + 1]))
                    {
                        if (nav->surface_data_faces[facenum].face_type == FACETYPE_WALK || nav->surface_data_faces[facenum].face_type == FACETYPE_TINYWALK)
                            return ledge;
                    }
                }
            }

            if (abs(tr_left.endpos[2] - start_left[2]) > DROP_DIST)
            {
                ledge.is_ledge = true;
                VectorCopy(tr_left.plane.normal, ledge.normal);
                VectorCopy(start_left, ledge.startpos);
                VectorCopy(tr_left.endpos, ledge.endpos);
                ledge.height = abs(tr_left.endpos[2] - start_left[2]);
                return ledge;
            }
        }

        // Facenum was different
        //if (0)
        //else if (UTIL_IsClimableSlope(tr_right.plane.normal))
        else
        {
            BOTLIB_UTIL_ROTATE_CENTER(v2, v1, 90, 1, start_right, NULL); // Turn 90 degrees right and move MOVE_DIST units
            start_right[2] += 4;
            end_right[0] = start_right[0];
            end_right[1] = start_right[1];
            end_right[2] = -4096;
            //tr_right = SV_Trace(start_right, mins, maxs, end_right, NULL, MASK_PLAYERSOLID);
            tr_right = SV_Trace(start_right, NULL, NULL, end_right, NULL, MASK_PLAYERSOLID);
            if (tr_right.startsolid)
                return ledge;

            for (int i = 0; i < nav->surface_data_faces[facenum].num_verts; i += 2)
            {
                // Compare the two edges to see if they share the same edge
                if (UTIL_EdgeCompare(v1, v2, nav->surface_data_faces[facenum].verts[i], nav->surface_data_faces[facenum].verts[i + 1]))
                {
                    if (nav->surface_data_faces[facenum].face_type == FACETYPE_WALK || nav->surface_data_faces[facenum].face_type == FACETYPE_TINYWALK)
                        return ledge;
                }
            }

            //if (nav->surface_data_faces[facenum].face_type != FACETYPE_WALK && nav->surface_data_faces[facenum].face_type != FACETYPE_TINYWALK)
            if (abs(tr_right.endpos[2] - start_right[2]) > 4) // DROP_DIST
            {
                ledge.is_ledge = true;
                VectorCopy(tr_right.plane.normal, ledge.normal);
                VectorCopy(start_right, ledge.startpos);
                VectorCopy(tr_right.endpos, ledge.endpos);
                ledge.height = abs(tr_right.endpos[2] - start_right[2]);
                return ledge;
            }
        }
    }





    if (0)
    {
        const float left_drop = abs(tr_left.endpos[2] - start_left[2]);
        const float right_drop = abs(tr_right.endpos[2] - start_right[2]);

        if (left_drop > right_drop)
        {
            ledge.is_ledge = true;
            VectorCopy(tr_left.plane.normal, ledge.normal);
            VectorCopy(start_left, ledge.startpos);
            VectorCopy(tr_left.endpos, ledge.endpos);
            ledge.height = abs(tr_left.endpos[2] - start_left[2]);
            return ledge;
        }
        else
        {
            ledge.is_ledge = true;
            VectorCopy(tr_right.plane.normal, ledge.normal);
            VectorCopy(start_right, ledge.startpos);
            VectorCopy(tr_right.endpos, ledge.endpos);
            ledge.height = abs(tr_right.endpos[2] - start_right[2]);
            return ledge;
        }
    }




    return ledge;














    if (tr_left.startsolid && tr_right.startsolid) // Not a ledge
    {
        return ledge;  // this will be: ledge.is_ledge = false;
    }
    //else if (abs(tr_left.endpos[2] - start_left[2]) <= 0.01 || abs(tr_right.endpos[2] - start_right[2]) <= 0.01)
    //{
    //    return ledge;  // this will be: ledge.is_ledge = false;
    //}    

    if (tr_left.startsolid == 0 && tr_right.startsolid) // Ledge is left side
    {
        //if (tr_left.plane.normal[0] < -MAX_STEEPNESS && tr_left.plane.normal[0] > MAX_STEEPNESS && tr_left.plane.normal[1] < -MAX_STEEPNESS && tr_left.plane.normal[1] > MAX_STEEPNESS && tr_left.plane.normal[2] < (MAX_STEEPNESS - 0.10))
        //if (abs(tr_left.endpos[2] - start_left[2]) > 0.01 || (tr_left.plane.normal[0] < -MAX_STEEPNESS && tr_left.plane.normal[0] > MAX_STEEPNESS && tr_left.plane.normal[1] < -MAX_STEEPNESS && tr_left.plane.normal[1] > MAX_STEEPNESS && tr_left.plane.normal[2] < (MAX_STEEPNESS - 0.10)))
        //if (abs(tr_left.endpos[2] - start_left[2]) > 0.01)
        {
            if (1)
            {
                BOTLIB_UTIL_ROTATE_CENTER(v1, v2, 90, MOVE_DIST, start_left, NULL);	// Turn 90 degrees left and move MOVE_DIST units
                end_left[0] = start_left[0];
                end_left[1] = start_left[1];
                end_left[2] = -4096;
                //tr_left = SV_Trace(start_left, mins, maxs, end_left, NULL, MASK_PLAYERSOLID);
                tr_left = SV_Trace(start_left, NULL, NULL, end_left, NULL, MASK_PLAYERSOLID);

                //if (abs(tr_left.endpos[2] - start_left[2]) < 4) // Ignore tiny ledges
                //    return ledge;
            }

            // Check if face that was hit shares an edge with the current face
            int facenum = UTIL_FindFace(nav, tr_left.endpos, tr_left.plane.normal); // Get facenum of the face that was hit
            if (facenum != INVALID) // If valid face (i.e. any surface that was included in the surface_data_t array)
            {
                //if (facenum == face_data->facenum)
                //    Com_Printf("%s facenum == face_data->facenum %d\n", __func__, facenum);

                for (int i = 0; i < nav->surface_data_faces[facenum].num_verts; i += 2)
                {
                    // Compare the two edges to see if they share the same edge
                    if (UTIL_EdgeCompare(v1, v2, nav->surface_data_faces[facenum].verts[i], nav->surface_data_faces[facenum].verts[i + 1]))
                    {
                        //if (tr_left.plane.normal[0] < -MAX_STEEPNESS && tr_left.plane.normal[0] > MAX_STEEPNESS && tr_left.plane.normal[1] < -MAX_STEEPNESS && tr_left.plane.normal[1] > MAX_STEEPNESS && tr_left.plane.normal[2] < (MAX_STEEPNESS - 0.10))
                        // Any climbable sloped surface
                        //if (tr_left.plane.normal[0] >= -MAX_STEEPNESS && tr_left.plane.normal[0] <= MAX_STEEPNESS && tr_left.plane.normal[1] >= -MAX_STEEPNESS && tr_left.plane.normal[1] <= MAX_STEEPNESS && tr_left.plane.normal[2] >= (MAX_STEEPNESS - 0.10)) // && tr_right.plane.normal[2] >= MAX_STEEPNESS)
                        return ledge;
                    }
                }
            }

            //if (tr_left.startsolid == 0)
            if (abs(tr_left.endpos[2] - start_left[2]) > DROP_DIST)
            {
                ledge.is_ledge = true;
                VectorCopy(tr_left.plane.normal, ledge.normal);
                VectorCopy(start_left, ledge.startpos);
                VectorCopy(tr_left.endpos, ledge.endpos);
                ledge.height = abs(tr_left.endpos[2] - start_left[2]);
            }
        }
    }
    else if (tr_left.startsolid && tr_right.startsolid == 0) // Ledge is right side
    {
        //if (tr_right.plane.normal[0] < -MAX_STEEPNESS && tr_right.plane.normal[0] > MAX_STEEPNESS && tr_right.plane.normal[1] < -MAX_STEEPNESS && tr_right.plane.normal[1] > MAX_STEEPNESS && tr_right.plane.normal[2] < (MAX_STEEPNESS - 0.10))
        //if (abs(tr_right.endpos[2] - start_right[2]) > 0.01 || (tr_right.plane.normal[0] < -MAX_STEEPNESS && tr_right.plane.normal[0] > MAX_STEEPNESS && tr_right.plane.normal[1] < -MAX_STEEPNESS && tr_right.plane.normal[1] > MAX_STEEPNESS && tr_right.plane.normal[2] < (MAX_STEEPNESS - 0.10)))
        //if (abs(tr_right.endpos[2] - start_right[2]) > 0.01)
        {
            if (1)
            {
                BOTLIB_UTIL_ROTATE_CENTER(v2, v1, 90, MOVE_DIST, start_right, NULL); // Turn 90 degrees right and move MOVE_DIST units
                end_right[0] = start_right[0];
                end_right[1] = start_right[1];
                end_right[2] = -4096;
                //tr_right = SV_Trace(start_right, mins, maxs, end_right, NULL, MASK_PLAYERSOLID);
                tr_right = SV_Trace(start_right, NULL, NULL, end_right, NULL, MASK_PLAYERSOLID);

                //if (abs(tr_right.endpos[2] - start_right[2]) < 4) // Ignore tiny ledges
                //    return ledge;
            }

            // Check if face that was hit shares an edge with the current face
            int facenum = UTIL_FindFace(nav, tr_right.endpos, tr_right.plane.normal); // Get facenum of the face that was hit
            if (facenum != INVALID) // If valid face (i.e. any surface that was included in the surface_data_t array)
            {
                //if (facenum == face_data->facenum)
                //    Com_Printf("%s facenum == face_data->facenum %d\n", __func__, facenum);

                for (int i = 0; i < nav->surface_data_faces[facenum].num_verts; i += 2)
                {
                    // Compare the two edges to see if they share the same edge
                    if (UTIL_EdgeCompare(v1, v2, nav->surface_data_faces[facenum].verts[i], nav->surface_data_faces[facenum].verts[i + 1]))
                    {
                        //if (tr_right.plane.normal[0] < -MAX_STEEPNESS && tr_right.plane.normal[0] > MAX_STEEPNESS && tr_right.plane.normal[1] < -MAX_STEEPNESS && tr_right.plane.normal[1] > MAX_STEEPNESS && tr_right.plane.normal[2] < (MAX_STEEPNESS - 0.10))
                        //if (tr_right.plane.normal[0] >= -MAX_STEEPNESS && tr_right.plane.normal[0] <= MAX_STEEPNESS && tr_right.plane.normal[1] >= -MAX_STEEPNESS && tr_right.plane.normal[1] <= MAX_STEEPNESS && tr_right.plane.normal[2] >= (MAX_STEEPNESS - 0.10)) // && tr_right.plane.normal[2] >= MAX_STEEPNESS)
                        return ledge;
                    }
                }
            }

            //if (tr_right.startsolid == 0)
            if (abs(tr_right.endpos[2] - start_right[2]) > DROP_DIST)
            {
                ledge.is_ledge = true;
                VectorCopy(tr_right.plane.normal, ledge.normal);
                VectorCopy(start_right, ledge.startpos);
                VectorCopy(tr_right.endpos, ledge.endpos);
                ledge.height = abs(tr_right.endpos[2] - start_right[2]);
            }
        }
    }

    /*
    // Ledge is both left and right, this might occur on very thin ledges less than 0.01 width
    // Perhaps we should ignore these ledges? Even if very rare...
    else
    {
        const float left_drop = abs(tr_left.endpos[2] - start_left[2]);
        const float right_drop = abs(tr_right.endpos[2] - start_right[2]);

        if (left_drop > right_drop)
        {
            ledge.is_ledge = true;
            VectorCopy(tr_left.plane.normal, ledge.normal);
            VectorCopy(tr_left.endpos, ledge.endpos);
            ledge.height = left_drop;
        }
        else
        {
            ledge.is_ledge = true;
            VectorCopy(tr_right.plane.normal, ledge.normal);
            VectorCopy(tr_right.endpos, ledge.endpos);
            ledge.height = right_drop;
        }
    }
    */

    return ledge;
}

// Using the given position and normal, find the nearest face
// Returns the face number, or INVALID (-1) if none found
// INVALID surfaces are is surface that wasn't included in the surface_data_t array, or the position is outside the map
int UTIL_FindFace(nav_t* nav, vec3_t pos, vec3_t normal)
{
    for (int f = 0; f < nav->faces_total; f++)
    {
        // Skip far away faces
        if (VectorDistance(nav->surface_data_faces[f].center_poly, pos) > 512) // Distance from face center to pos
            continue;

        // Skip if not same normal
        if (UTIL_CompareNormal(nav->surface_data_faces[f].normal, normal, 0.05) == false) // 0.05
            continue;

        // Skip if not same plane
        if (UTIL_CompareVertPlanes(nav->surface_data_faces[f].verts[0], pos, normal, 0.01) == false) // 0.01
            continue;

        // Inside face
        if (UTIL_InsideFace(&nav->surface_data_faces[f], pos, 0.01)) // Point inside face
            return f;

        //if (AAS_InsideFaceBoxScan(&nav->surface_data_faces[f], pos, 0.01, true)) // Boxscan point inside face
        //    return f;
    }
    return INVALID;
}

// Takes an edge (v1 to v2) and finds an position that is perpendicular to the edge and is inside the face
// If the edge is in a corner, the function will try to move the position further inside the face
// Returns true if a valid position was found, and out is set to the valid inner position
qboolean UTIL_FindValidInnerPosition(surface_data_t* face_data, int facenum, vec3_t normal, float distance, vec3_t v1, vec3_t v2, vec3_t out)
{
    vec3_t left, right;
    vec3_t offset, center;
    
    BOTLIB_UTIL_ROTATE_CENTER(v1, v2, 90, distance, left, NULL);	// Turn 90 degrees left and move 'distance' units
    BOTLIB_UTIL_ROTATE_CENTER(v1, v2, -90, distance, right, NULL); // Turn -90 degrees right and move 'distance' units

    //if (UTIL_FindFace(nav, left, normal) == facenum) // Check we're still inside the same face
    if (UTIL_InsideFace(face_data, left, 0.01)) // Point inside face
    {
        // See if the new position is valid, if not then readjust and try new position
        // Reasons this might fail: edge might be in a corner, or the face is too small, etc
        if (UTIL_PlayerBoxTestCouched(left) == false)
		{
            // Print facenum
            //Com_Printf("%s  facenum: %d\n", __func__, facenum);

            // Same as the initial left rotation, except double the distance.
            BOTLIB_UTIL_ROTATE_CENTER(v1, v2, 90, distance * 2, left, NULL);	// Turn 90 degrees left and move 'distance * 2' units

            // Find the middle
            LerpVector(v1, v2, 0.50, center);

            // Try left
            BOTLIB_UTIL_ROTATE_CENTER(center, left, 90, distance, offset, NULL);	// Turn 90 degrees left and move 'distance' units
            //if (UTIL_FindFace(nav, offset, normal) == facenum) // Check we're still inside the same face
            if (UTIL_InsideFace(face_data, offset, 0.01)) // Point inside face
            {
                if (UTIL_PlayerBoxTestCouched(offset))
                {
                    VectorCopy(offset, out);
                    return true;
                }
            }

            // Try right
            BOTLIB_UTIL_ROTATE_CENTER(center, left, -90, distance, offset, NULL);	// Turn -90 degrees right and move 'distance' units
            //if (UTIL_FindFace(nav, offset, normal) == facenum) // Check we're still inside the same face
            if (UTIL_InsideFace(face_data, offset, 0.01)) // Point inside face
            {
                if (UTIL_PlayerBoxTestCouched(offset))
                {
                    VectorCopy(offset, out);
                    return true;
                }
            }

            // Failed to find a valid position
            return false;
		}
        else
        {
            VectorCopy(left, out);
            return true;
        }
    }
    //else if (UTIL_FindFace(nav, right, normal) == facenum) // Check we're still inside the same face
    else if (UTIL_InsideFace(face_data, right, 0.01)) // Point inside face
    {
        // See if the new position is valid, if not then readjust and try new position
        // Reasons this might fail: edge might be in a corner, or the face is too small, etc
        if (UTIL_PlayerBoxTestCouched(right) == false)
        {
            // Same as the initial right rotation, except double the distance.
            BOTLIB_UTIL_ROTATE_CENTER(v1, v2, -90, distance * 2, right, NULL);	// Turn 90 degrees right and move 'distance * 2' units

            // Find the middle
            LerpVector(v1, v2, 0.50, center);

            // Try left
            BOTLIB_UTIL_ROTATE_CENTER(center, right, 90, distance, offset, NULL);	// Turn 90 degrees left and move 'distance' units
            //if (UTIL_FindFace(nav, offset, normal) == facenum) // Check we're still inside the same face
            if (UTIL_InsideFace(face_data, offset, 0.01)) // Point inside face
            {
                if (UTIL_PlayerBoxTestCouched(offset))
                {
                    VectorCopy(offset, out);
                    return true;
                }
            }

            // Try right
            BOTLIB_UTIL_ROTATE_CENTER(center, right, -90, distance, offset, NULL);	// Turn -90 degrees right and move 'distance' units
            //if (UTIL_FindFace(nav, offset, normal) == facenum) // Check we're still inside the same face
            if (UTIL_InsideFace(face_data, offset, 0.01)) // Point inside face
            {
                if (UTIL_PlayerBoxTestCouched(offset))
                {
                    VectorCopy(offset, out);
                    return true;
                }
            }

            // Failed to find a valid position
            return false;
        }
        else
        {
            VectorCopy(right, out);
            return true;
        }
    }
    else
    {
        return false;
    }
}

qboolean surface_data_added = false; // True if surface data has been added to the map
qboolean surface_faces_connected = false; // True if the surface faces have been connected
const int SURFACE_MEM_CHUNK_SIZE = 8192; // How many surface faces to allocate at a time
qboolean Alloc_Surface_Memory(nav_t* nav)
{
    // ------------------------------
    // Alloc memory for surface faces
    // ------------------------------
    surface_data_t* prev_surface_data_faces = NULL; // Used to free memory if realloc fails

    // Malloc first surface added
    if (nav->surface_data_faces == NULL)
    {
        // This malloc is purposely freed in CM_FreeMap() -- this should occur every map change
        nav->surface_data_faces = (surface_data_t*)Z_TagMallocz(sizeof(surface_data_t) * SURFACE_MEM_CHUNK_SIZE, TAG_CMODEL);
        //nav->surface_data_faces = (surface_data_t*)malloc(sizeof(surface_data_t) * SURFACE_MEM_CHUNK_SIZE);
        //if (nav->surface_data_faces != NULL)
        //    memset(nav->surface_data_faces, 0, sizeof(surface_data_t) * SURFACE_MEM_CHUNK_SIZE);
    }
    // Realloc all other surfaces added
    else if (nav->faces_total > 0 && nav->faces_total >= SURFACE_MEM_CHUNK_SIZE)
    {
        Com_Printf("%s failed to malloc surface_data_faces. Increase SURFACE_MEM_CHUNK_SIZE > %d\n", __func__, SURFACE_MEM_CHUNK_SIZE);

        //prev_surface_data_faces = nav->surface_data_faces; // Keep a copy
        //nav->surface_data_faces = (surface_data_t*)realloc(nav->surface_data_faces, sizeof(surface_data_t) * (nav->faces_total + 1));
    }

    // Deal with malloc/realloc failure
    if (nav->surface_data_faces == NULL)
    {
        Com_Printf("%s failed to malloc prev_surface_data_faces. Out of memory!\n", __func__);
        if (prev_surface_data_faces)
        {
            free(prev_surface_data_faces); // Free using the copy, because nodes is null
            nav->surface_data_faces = NULL;
            prev_surface_data_faces = NULL;
        }
        nav->faces_total--;
        return false; // Failed to allocate memory
    }

    return true; // Success
}
qboolean Save_Surface_Data(nav_t* nav, surface_data_t* face_data)
{
    int i;

    if (face_data == NULL)
        return false;

    // Add the data
    if (nav->surface_data_faces)
    {
        int num = nav->faces_total; // Current total surfaces

        // ========================================================
        // Save raw BSP Data
        // ========================================================
        nav->surface_data_faces[num].surf = face_data->surf; // Surface pointer
        for (int c = 0; c < MAX_TEXNAME; c++) // Copy the texture name
        {
            nav->surface_data_faces[num].texture[c] = face_data->texture[c]; // Surface texture name
        }
        VectorCopy(face_data->normal, nav->surface_data_faces[num].normal); // Surface normal
        nav->surface_data_faces[num].contents = face_data->contents; // Surface contents
        nav->surface_data_faces[num].drawflags = face_data->drawflags; // Surface drawflags

        VectorCopy(face_data->first_vert, nav->surface_data_faces[num].first_vert); // First vert - used when calculating triangles (i.e. gl_showtris "1")

        // Raw verts
        nav->surface_data_faces[num].num_verts = face_data->num_verts;
        for (i = 0; i < face_data->num_verts; i++)
        {
            VectorCopy(face_data->verts[i], nav->surface_data_faces[num].verts[i]); // Verts
        }

        // Raw Edges
        for (i = 0; i < face_data->num_verts; i += 2)
        {
            VectorCopy(face_data->edge_center[i / 2], nav->surface_data_faces[num].edge_center[i / 2]); // Center edge
            VectorCopy(face_data->edge_center_stepsize[i / 2], nav->surface_data_faces[num].edge_center_stepsize[i / 2]); // Center[2] edge + 32
        }

        // ========================================================
        // Save Custom BSP Data - Safe to modify
        // ========================================================
        nav->surface_data_faces[num].facenum = nav->faces_total; // Assigned surface number

        nav->surface_data_faces[num].face_type = face_data->face_type; // Surface type: none, ignore, walkable, wall, ladder, water, damage...

        // Aligned verts
        nav->surface_data_faces[num].num_aligned_verts = face_data->num_aligned_verts;
        for (i = 0; i < face_data->num_aligned_verts; i++)
        {
            VectorCopy(face_data->aligned_verts[i], nav->surface_data_faces[num].aligned_verts[i]); // Aligned verts
        }

        // Aligned edges
        for (i = 0; i < face_data->num_aligned_verts; i += 2)
        {
            VectorCopy(face_data->aligned_edge_center[i / 2], nav->surface_data_faces[num].aligned_edge_center[i / 2]); // Center edge
            VectorCopy(face_data->aligned_edge_center_stepsize[i / 2], nav->surface_data_faces[num].aligned_edge_center_stepsize[i / 2]); // Center[2] edge + 18
        }

        // Offset edges
        for (i = 0; i < face_data->num_verts; i += 2)
        {
            nav->surface_data_faces[num].edge_offset_type[i / 2] = face_data->edge_offset_type[i / 2]; // Offset type
            VectorCopy(face_data->edge_valid_pos[i / 2], nav->surface_data_faces[num].edge_valid_pos[i / 2]); // Offset edge
            VectorCopy(face_data->edge_valid_stepsize[i / 2], nav->surface_data_faces[num].edge_valid_stepsize[i / 2]); // Offset edge_valid_pos[2] + 18
        }

        // Ledges
        for (i = 0; i < face_data->num_verts; i += 2)
        {
            nav->surface_data_faces[num].ledge[i / 2].is_ledge = face_data->ledge[i / 2].is_ledge; // Is ledge
            nav->surface_data_faces[num].ledge[i / 2].height = face_data->ledge[i / 2].height; // Ledge height
            VectorCopy(face_data->ledge[i / 2].startpos, nav->surface_data_faces[num].ledge[i / 2].startpos); // Start of ledge
            VectorCopy(face_data->ledge[i / 2].endpos, nav->surface_data_faces[num].ledge[i / 2].endpos); // End of ledge
            VectorCopy(face_data->ledge[i / 2].normal, nav->surface_data_faces[num].ledge[i / 2].normal); // Normal

            // Debug
            nav->surface_data_faces[num].ledge[i / 2].hit_side = face_data->ledge[i / 2].hit_side; // Hit side: none, left, right
            VectorCopy(face_data->ledge[i / 2].left_start, nav->surface_data_faces[num].ledge[i / 2].left_start); // Trace left start
            VectorCopy(face_data->ledge[i / 2].left_end, nav->surface_data_faces[num].ledge[i / 2].left_end); // Trace left end
            VectorCopy(face_data->ledge[i / 2].right_start, nav->surface_data_faces[num].ledge[i / 2].right_start); // Trace right start
			VectorCopy(face_data->ledge[i / 2].right_end, nav->surface_data_faces[num].ledge[i / 2].right_end); // Trace right end
        }

        // Polygon Center
        VectorCopy(face_data->center_poly, nav->surface_data_faces[num].center_poly); // Center
        VectorCopy(face_data->center_poly_32_units, nav->surface_data_faces[num].center_poly_32_units); // Center[2] + 32

        nav->surface_data_faces[num].volume = face_data->volume;

        // Min/max surface lengths (from poly center to edge)
        nav->surface_data_faces[num].min_length = face_data->min_length;
        nav->surface_data_faces[num].max_length = face_data->max_length;

        nav->faces_total++; // Increment face counter

        if (face_data->face_type & FACETYPE_NONE)
            nav->ignored_faces_total++; // Increment ignored face counter
        if ( (face_data->face_type & FACETYPE_IGNORED) || (face_data->face_type & FACETYPE_WALL))
            nav->ignored_faces_total++; // Increment ignored face counter

        return true; // Successfully added surface data
    }

    return false; // Failure to added surface data
}

// This will attempt to locate the most valid position on the edge, and store it in edge_valid_pos
// The first pick will be the center of the edge, 
// and if that fails, it will try a position closest to the center to maximize the chance of LoS with other edges
void UTIL_FindValidEdgePositions(surface_data_t* face_data)
{
    const int MIN_EDGE_WIDTH = 8; // Minimum edge width to consider
    const float EDGE_OFFSET_STEP = 0.25; // How far to offset the edge each time

    float edge_length = 0; // Length of the edge
    float edge_pos = 0; // Position along the edge
    vec3_t curr_pos = { 0 }; // Current position along the edge
    int total_positions = 0; // Total valid positions found along the edge
    #define MAX_EDGE_POSITIONS 512 // Maximum number of valid positions to find along the edge
    vec3_t positions[MAX_EDGE_POSITIONS]; // All the valid positions found along the edge
    vec3_t vec = { 0 }; // Vector used for calculating a direction away from the edge

    // Test player hitbox against the edge to see if it fits 
    // (some edges are against a wall, so move it inside the face a bit)
    for (int e = 0; e < face_data->num_verts; e += 2)
    {
        face_data->edge_offset_type[e / 2] = EDGE_OFFSET_NONE; // Set null offset flag

        // Skip very small edges
        if (VectorDistance(face_data->verts[e], face_data->verts[e + 1]) < MIN_EDGE_WIDTH)
        {
            VectorCopy(face_data->edge_center[e / 2], face_data->edge_valid_pos[e / 2]); // Copy center edge
            face_data->edge_offset_type[e / 2] = EDGE_OFFSET_CENTER;

            MoveAwayFromNormal(face_data->drawflags, face_data->normal, vec, STEPSIZE); // Get the vector
            VectorAdd(face_data->edge_valid_pos[e / 2], vec, face_data->edge_valid_stepsize[e / 2]); // Move away from the surface normal by STEPSIZE

            continue;
        }

        total_positions = 0; // Reset number of positions

        // Test if center edge fits player hitbox
        // 
        // Does it for on the center edge?
        if (UTIL_PlayerBoxTestCouched(face_data->edge_center[e / 2]))
        {
            VectorCopy(face_data->edge_center[e / 2], face_data->edge_valid_pos[e / 2]); // Copy center edge
            face_data->edge_offset_type[e / 2] = EDGE_OFFSET_CENTER;
            
        }
        // Does it fit somewhere else along the edge?
        else // It didn't fit, so lets try different positions along the edge to see if it will fit. Keeping the most centered position
        {
            edge_length = VectorDistance(face_data->verts[e], face_data->verts[e + 1]); // Get the length of the edge

            //if (face_data->facenum == 2450)
            //    int a = 0;

            // While loop to find the most centered position that fits the player hitbox along the edge
            edge_pos = 1; // Reset edge position
            while (edge_pos < edge_length)
			{
                if (total_positions + 1 > MAX_EDGE_POSITIONS)
                {
                    Com_Printf("%s total_positions > MAX_EDGE_POSITIONS overflow error. Increase MAX_EDGE_POSITIONS\n", __func__);
                    break;
                }
				// LerpVector (vert -> vert, position along edge, output)
                LerpVector(face_data->verts[e], face_data->verts[e + 1], edge_pos / edge_length, curr_pos); // Get position along the edge
				if (UTIL_PlayerBoxTestCouched(curr_pos)) // Yes, it fits
				{
                    VectorCopy(curr_pos, positions[total_positions]);
                    total_positions++;
					//break; // Stop searching
				}
                edge_pos += EDGE_OFFSET_STEP; // Increment edge position by EDGE_OFFSET_STEP
			}
        }

        if (total_positions)
        {
            // Find the position closest to the center of the edge to maximize the chance of LoS with other edges
            int closest = 0;
            float closest_dist = 99999;
            float dist = 0;
            for (int i = 0; i < total_positions; i++)
			{
                dist = VectorDistance(face_data->edge_center[e / 2], positions[i]);
				if (dist < closest_dist)
				{
                    closest_dist = dist;
                    closest = i;
				}
			}
            VectorCopy(positions[closest], face_data->edge_valid_pos[e / 2]); // Copy modified edge pos
            face_data->edge_offset_type[e / 2] = EDGE_OFFSET_LENGTH;
        }
        else // All else failed, so just use the center edge
        {
            VectorCopy(face_data->edge_center[e / 2], face_data->edge_valid_pos[e / 2]); // Copy center edge pos
            face_data->edge_offset_type[e / 2] = EDGE_OFFSET_CENTER;
        }
        
        MoveAwayFromNormal(face_data->drawflags, face_data->normal, vec, STEPSIZE); // Get the vector
        VectorAdd(face_data->edge_valid_pos[e / 2], vec, face_data->edge_valid_stepsize[e / 2]); // Move away from the surface normal by STEPSIZE
    }
}

// If the edge is too close to an obstacle, move it away from the obstacle
// Test if the new position is valid (fits the player hitbox)
void UTIL_FindValidInnerEdgePositions(surface_data_t* face_data)
{
	const int MIN_EDGE_WIDTH = 8; // Minimum edge width to consider
    vec3_t edge2_inside; // The same edge but moved further inside the face

    // Test player hitbox against the edge to see if it fits 
    // (some edges are against a wall, so move it inside the face a bit)
    for (int e = 0; e < face_data->num_verts; e += 2)
    {
        // Skip very small edges
        if (VectorDistance(face_data->verts[e], face_data->verts[e + 1]) < MIN_EDGE_WIDTH)
            continue;

        // Test if the pos fits
        if (UTIL_PlayerBoxTestCouched(face_data->edge_valid_pos[e / 2]))
            continue;

        if (UTIL_FindValidInnerPosition(face_data, face_data->facenum, face_data->normal, 16, face_data->verts[e], face_data->verts[e + 1], edge2_inside))
        {
            VectorCopy(edge2_inside, face_data->edge_valid_pos[e / 2]); // Success: update edge with the new position
        }

    }
}

void UTIL_FindLedges(nav_t *nav, surface_data_t* face_data)
{
    for (int e = 0; e < face_data->num_verts; e += 2)
    {
        face_data->ledge[e/2] = UTIL_FindLedge(nav, face_data, face_data->verts[e], face_data->verts[e + 1]);
    }
}

// Gets the surface data
qboolean GetSurfaceData(nav_t* nav, mface_t* surf, int face)
{
    //surface_data_t face_data; // Surface data
    vec3_t center_vec = { 0 }; // Center point vector

    if (nav->faces_total + 1 > SURFACE_MEM_CHUNK_SIZE)
	{
		Com_Printf("%s SURFACE_MEM_CHUNK_SIZE overflow error. Increase SURFACE_MEM_CHUNK_SIZE\n", __func__);
		return false;
	}

    Alloc_Surface_Memory(nav); // Alloc memory for surface face
    int num = nav->faces_total;
    surface_data_t* face_data = nav->surface_data_faces + num;
    face_data->facenum = num; // WARNING: facenum might not a be trustworthy number yet: some faces haven't been added to Save_Surface_Data(), or some might get rejected

    face_data->surf = surf; // Copy surface reference
    face_data->facenum = face; // Init face num

    face_data->drawflags = surf->drawflags; // Init draw flags
    for (int c = 0; c < MAX_TEXNAME; c++) // Copy the texture name
    {
        face_data->texture[c] = surf->texinfo->name[c];
    }
    //if (strlen(face_data->texture) == 0) // Check if the texture name is empty
    //{
        //Com_Printf("%s: WARNING: texture name is empty. Len:%d [%s]\n", __func__, strlen(face_data->texture), face_data->texture);
        //return false;
    //}
    
    if (Q_strcasestr(surf->texinfo->name, "trigger") != NULL) // Look for "trigger" textures, and ignore them
    {
        //Com_Printf("%s: removing face with texture [%s]\n", __func__, surf->texinfo->name);
        return false;
    }
    //Com_Printf("%s: texture [%s]\n", __func__, surf->texinfo->name);


    InverseNormals(face_data); // Inverse the surface normal if needed
    InitVertexData(surf, face_data); // Init the vertex data
    FixMalformedVertexData(face_data); // Fix any malformed vertex data
    AlignVertexPoints(face_data); // Align verts
    BOTLIB_UTIL_PolygonCenter2D(face_data->aligned_verts, face_data->num_aligned_verts, face_data->center_poly); // Get the center of the face polygon

    // 32 - Player model width
    MoveAwayFromNormal(face_data->drawflags, face_data->normal, center_vec, 32); // Get the vector
    VectorAdd(face_data->center_poly, center_vec, face_data->center_poly_32_units); // Move away from the surface normal by 32 units (full player width)

    // Check for liquids, solids, and space
    if (CheckFaceLiquid(face_data) == false && CheckFaceSolid(face_data) == false) return false;
    if (CheckSkyAboveFace(face_data) == false) return false;

    // Edges
    UTIL_GetCenterOfVertEdges(face_data); // Get the center each VERT edge
    UTIL_GetCenterOfAlignedEdges(face_data); // Get the center each ALIGNED edge
    UTIL_FindValidEdgePositions(face_data); // Find a valid edge position
    UTIL_FindValidInnerEdgePositions(face_data); // Find a valid inner edge position

    face_data->volume = AAS_FaceArea(face_data); // Surface area
    UTIL_GetFaceLengthMinMax(face_data); // Get the min/max length of the face
    GetFaceContents(face_data); // Get the face contents

    face_data->face_type = SetSurfaceType(face_data);

    // Rejecting faces here might mean that a valid face_data->facenum might not be detected when searching the faces array
    if (face_data->face_type & (FACETYPE_IGNORED | FACETYPE_SKY))
        return false; // Ignore face

    // Save all the data relating this surface
    Save_Surface_Data(nav, face_data);

    return true;
}

// Cache the surface data & save it
static void BOTLIB_SaveSurfaceData(nav_t* nav, bsp_t *bsp)
{
    int face = 0;
    short int processed = 0;
    mface_t* surf;

    // Clear out data (either from a previous map or init a new instance)
    if (bsp->checksum != nav->surface_data_checksum)
    {
        surface_data_added = false;
        surface_faces_connected = false; // Reset nearest neighbor faces
    }
    // Add the surface data
    if (surface_data_added == false)
    {
        for (face = 0, surf = bsp->faces; face < bsp->numfaces; face++, surf++)
        {
            processed++;

            // Ignore sky surfaces
            //if (surf->drawflags & SURF_SKY)
            //    continue;
            
            // Ignore roof surfaces
            //if (surf->plane->normal[2] == 1) // Floor
            //{
                //if (surf->drawflags & DSURF_PLANEBACK) // Roof
                //    continue;
            //}

            if (GetSurfaceData(nav, surf, face) == false)
                continue;  
        }
        surface_data_added = true;
        nav->surface_data_checksum = bsp->checksum;
        Com_Printf("%s [map surfaces %d] [%d processed] [%d ignored] [%d usable]\n", __func__, processed, nav->faces_total, nav->ignored_faces_total, nav->faces_total - nav->ignored_faces_total);
    }
}

// Add a connection between two faces
// f1 = Face 1, f2 = Face 2
// e1 = Edge 1, e2 = Edge 2
// end_pos = The position where the touch was made
// Return: 1 = connected, 0 = error
int BOTLIB_BSP_AddAreaConnections(nav_t* nav, int f1, int f2, vec3_t start_pos, vec3_t end_pos, face_connection_type_t ftype, face_move_type_t mtype, float drop_height)
{
    const int fcc = nav->surface_data_faces[f1].snode_counter; // Face Connection Counter (code readability)

    if (nav->surface_data_faces[f1].snode_counter >= MAX_FACE_CONNECTIONS)
    {
        Com_Printf("%s - MAX_FACE_CONNECTIONS: %d overflow\n", __func__, MAX_FACE_CONNECTIONS);
        return 1; // Error
    }

    //if (ftype == FACE_CONN_NONE) return 0;

    // Check to make we've not already found this face
    for (int c = 0; c < fcc; c++)
    {
        // Same face
        if (nav->surface_data_faces[f1].snodes[c].facenum == nav->surface_data_faces[f2].facenum)
        {
            // Same start & end points
            if (VectorCompare(start_pos, nav->surface_data_faces[f1].snodes[c].start) && VectorCompare(end_pos, nav->surface_data_faces[f1].snodes[c].end))
            {
                //Com_Printf("%s alreaded added connection from face[%d to %d] s[%f %f %f] e[%f %f %f]\n", __func__, f1, f2, start_pos[0], start_pos[1], start_pos[2], end_pos[0], end_pos[1], end_pos[2]);
                return 0; // Success (already added)
            }
        }
    }

    

    // Add it to the list
    nav->surface_data_faces[f1].snodes[fcc].type = ftype; // Connection type
    nav->surface_data_faces[f1].snodes[fcc].move = mtype; // Movement type
    nav->surface_data_faces[f1].snodes[fcc].facenum = nav->surface_data_faces[f2].facenum; // The face we touched
    nav->surface_data_faces[f1].snodes[fcc].dropheight = drop_height; // Drop height
    VectorCopy(start_pos, nav->surface_data_faces[f1].snodes[fcc].start); // 
    VectorCopy(end_pos, nav->surface_data_faces[f1].snodes[fcc].end); // 
    //VectorCopy(nav->surface_data_faces[f2].edge_center[e2 / 2], nav->surface_data_faces[f1].snodes[fcc].end); // The edge center
    nav->surface_data_faces[f1].snode_counter++; // Increment how many faces we touched

    return 0; // Success
}

// Searches all faces to find the nearest connected edges - faces that share the same edge
void FindNearestFacesByConnectedEdges(nav_t* nav)
{
    const int MIN_EDGE_WIDTH = 8; // Minimum edge width to consider
    int facenum = 0; // Face number
    vec3_t start_pos; // The start position
    vec3_t end_pos; // The end position
    int area_connections = 0; // Total connections found
    face_connection_type_t face_conn_type = FACE_CONN_NONE;
    face_move_type_t move_type = FACE_MOVE_NONE;
    vec3_t edge1, edge2; // Edges

    // Test and add local face edges
    if (1)
    for (int f1 = 0; f1 < nav->faces_total; f1++) // Face 1
    {
        for (int e1 = 0; e1 < nav->surface_data_faces[f1].num_verts; e1 += 2) // Edge 1 (face1)
        {
            if (IgnoreSmallSurfaces)
            {
                // Skip very small edges
                if (VectorDistance(nav->surface_data_faces[f1].verts[e1], nav->surface_data_faces[f1].verts[e1 + 1]) < MIN_EDGE_WIDTH)
                    continue;
            }

            // For each edge, test the start and end positions
            for (int e2 = 0; e2 < nav->surface_data_faces[f1].num_verts; e2 += 2)
            {
                if (e1 == e2) continue; // Skip self edge

                if (IgnoreSmallSurfaces)
                {
                    // Skip very small edges
                    if (VectorDistance(nav->surface_data_faces[f1].verts[e2], nav->surface_data_faces[f1].verts[e2 + 1]) < MIN_EDGE_WIDTH)
                        continue;
                }

                // Skip edges going in the same direction
                //if (UTIL_VectorDirectionCompare(nav->surface_data_faces[f1].verts[e1], nav->surface_data_faces[f1].verts[e1 + 1], nav->surface_data_faces[f1].verts[e3], nav->surface_data_faces[f1].verts[e3 + 1], true, 0.1) == true)
                //    continue;

                VectorCopy(nav->surface_data_faces[f1].edge_valid_pos[e1 / 2], edge1);
                VectorCopy(nav->surface_data_faces[f1].edge_valid_pos[e2 / 2], edge2);

                //VectorCopy(nav->surface_data_faces[f1].verts[e1 / 2], edge1);
                //VectorCopy(nav->surface_data_faces[f1].verts[e2 / 2], edge2);
                
                // Skip if not LoS
                if (UTIL_LineOfSight(edge1, edge2, nav->surface_data_faces[f1].normal) == false)
                {
                    //vec3_t empty = { 0, 0, 0 };
                    //BOTLIB_BSP_AddAreaConnections(nav, INVALID, INVALID, empty, empty, 0, 0, 0);
                    continue;
                }

                // Starting position away from the ledge (pointing towards a ledge)
                face_conn_type = FACE_CONN_NONE;
                ledge_data_t *ledge = &nav->surface_data_faces[f1].ledge[e2 / 2]; // Get reference to ledge data
                facenum = INVALID; // Reset facenum
                if (ledge->height) // Only run UTIL_FindFace() if we have a ledge height
                    facenum = UTIL_FindFace(nav, ledge->endpos, ledge->normal); // Find the face we're touching

                //if (ledge->height && facenum == INVALID)
                    //Com_Printf("%s - invalid facenum at face:%d edge:%d -> edge:%d\n", __func__, f1, e1/2, e2/2);

                // Drops into water
                if (facenum != INVALID && nav->surface_data_faces[facenum].contents & CONTENTS_WATER)
				{
					face_conn_type = FACE_CONN_WATER;
                    move_type = FACE_MOVE_DOWN;
				}
                // Flat surface (no drop - faces are on the same plane)
                else if (ledge->height == 0)
                {
                    face_conn_type = FACE_CONN_DIRECT;
                    move_type = FACE_MOVE_GROUND;
                }
                // Stepsize
                else if (ledge->height <= STEPSIZE)
                {
                    face_conn_type = FACE_CONN_STEP;
                    move_type = FACE_MOVE_DOWN;
                }
                // Drop down height #1 (can jump back up)
                else if (ledge->height <= MAX_JUMP_HEIGHT)
                {
                    face_conn_type = FACE_CONN_JUMP;
                    move_type = FACE_MOVE_DOWN;
                }
                // Drop down height #2 (safe drop, cannot jump back up)
                else if (ledge->height <= MAX_CROUCH_FALL_HEIGHT)
                {
                    face_conn_type = FACE_CONN_DROP;
                    move_type = FACE_MOVE_DOWN;
                }
                // Falling height (injury or fall death)
                else if (ledge->height > MAX_CROUCH_FALL_HEIGHT)
                {
                    face_conn_type = FACE_CONN_LEDGE;
                    move_type = FACE_MOVE_STOP;
                }
                else
                {
                    //vec3_t empty = { 0, 0, 0 };
                    //BOTLIB_BSP_AddAreaConnections(nav, INVALID, INVALID, empty, empty, 0, 0, 0);
                    Com_Printf("%s WARNING: could not connect face %d to edge %d\n", __func__, f1, e2);
                    continue;
                }

                // Check if edge is shared with another face
                int face2 = f1; // Face2 defaults to face1
                for (int f2 = 0; f2 < nav->faces_total; f2++) // Face 1
                {
                    if (nav->surface_data_faces[f2].face_type == FACETYPE_WALL) continue; // Skip walls
                    if (f1 == f2) continue; // skip self
                    for (int e3 = 0; e3 < nav->surface_data_faces[f2].num_verts; e3 += 2) // Edge 2 (face2)
                    {
                        if (IgnoreSmallSurfaces) // Skip very small edges
                        {
                            if (VectorDistance(nav->surface_data_faces[f2].verts[e3], nav->surface_data_faces[f2].verts[e3 + 1]) < MIN_EDGE_WIDTH)
                                continue;
                        }

                        // Skip unconnected edges
                        if (UTIL_EdgeCompare(nav->surface_data_faces[f1].verts[e1], nav->surface_data_faces[f1].verts[e1 + 1], nav->surface_data_faces[f2].verts[e3], nav->surface_data_faces[f2].verts[e3 + 1]) == false)
                            continue;

                        face2 = f2; // Update face2
                        break;
                    }
                    if (face2 != f1) break; // Break if face was changed
                }

                // Add connection from EDGE TO EDGE within the same face
                if (BOTLIB_BSP_AddAreaConnections(nav, f1, face2, edge1, edge2, face_conn_type, move_type, ledge->height) == 0)
                    area_connections++;

                // Add connection from LEDGE to the ENDPOS of face below
                if (1)
                {
                    if (face_conn_type != FACE_CONN_DIRECT && face_conn_type != FACE_CONN_LEDGE)
                    {
                        if (facenum == INVALID)
                            continue;

                        //if (ledge.endpos[2] > edge2[2])
                            //  continue;

                        // Add connection from LEDGE to the ENDPOS of face below
                        if (BOTLIB_BSP_AddAreaConnections(nav, f1, facenum, edge2, ledge->endpos, face_conn_type, FACE_MOVE_DOWN, ledge->height) == 0)
                            //if (BOTLIB_BSP_AddAreaConnections(nav, f1, facenum, ledge.startpos, ledge.endpos, face_conn_type, ledge.height) == 0)
                            area_connections++;

                        // Add reverse connection from ENDPOS of face below to the LEDGE above
                        if (1 && face_conn_type == FACE_CONN_STEP || face_conn_type == FACE_CONN_JUMP)
                        {
                            if (BOTLIB_BSP_AddAreaConnections(nav, facenum, f1, ledge->endpos, edge2, face_conn_type, FACE_MOVE_UP, ledge->height) == 0)
                                //if (BOTLIB_BSP_AddAreaConnections(nav, facenum, f1, ledge.endpos, ledge.startpos, face_conn_type, ledge.height) == 0)
                                area_connections++;

                            /*
                            // Add EDGE connections to the ENDPOS of face below
                            for (int e3 = 0; e3 < nav->surface_data_faces[f1].num_verts; e3 += 2)
                            {
                                // Test if the start pos fits
                                if (UTIL_PlayerBoxTestCouched(nav->surface_data_faces[f1].edge_center[e3 / 2]) == false)
                                    continue;

                                if (BOTLIB_BSP_AddAreaConnections(nav, facenum, f1, nav->surface_data_faces[f1].edge_center[e3 / 2], ledge->endpos, face_conn_type, ledge->height) == 0)
                                    area_connections++;
                            }
                            */
                        }
                    }
                }
            }
        }
    }



    return;



    // Test and add connected faces by an edge connection
    for (int f1 = 0; f1 < nav->faces_total; f1++) // Face 1
    {
        if (nav->surface_data_faces[f1].face_type == FACETYPE_WALL) // Skip walls
            continue;

        for (int f2 = 0; f2 < nav->faces_total; f2++) // Face 2
        {
            if (f1 == f2) continue; // skip self

            if (nav->surface_data_faces[f2].face_type == FACETYPE_WALL) // Skip walls
                continue;

            // Ignore faces far away
            //if (VectorDistance(nav->surface_data_faces[f1].center_poly, nav->surface_data_faces[f2].center_poly) > 256)
            //	continue;

            //qboolean connected = false;
            for (int e1 = 0; e1 < nav->surface_data_faces[f1].num_verts; e1 += 2) // Edge 1 (face1)
            {
                if (IgnoreSmallSurfaces) // Skip very small edges
                {
                    if (VectorDistance(nav->surface_data_faces[f2].verts[e1], nav->surface_data_faces[f2].verts[e1 + 1]) < MIN_EDGE_WIDTH)
                        continue;
                }

                for (int e2 = 0; e2 < nav->surface_data_faces[f2].num_verts; e2 += 2) // Edge 2 (face2)
                {
                    if (IgnoreSmallSurfaces) // Skip very small edges
                    {
                        if (VectorDistance(nav->surface_data_faces[f2].verts[e2], nav->surface_data_faces[f2].verts[e2 + 1]) < MIN_EDGE_WIDTH)
                            continue;
                    }

                    // Skip unconnected edges
                    if (UTIL_EdgeCompare(nav->surface_data_faces[f1].verts[e1], nav->surface_data_faces[f1].verts[e1 + 1], nav->surface_data_faces[f2].verts[e2], nav->surface_data_faces[f2].verts[e2 + 1]) == false)
                        continue;

                    if (0)
                    {
                        for (int c = 0; c < nav->surface_data_faces[f2].snode_counter; c++)
                        {
                            //VectorCompare( nav->surface_data_faces[f1].verts[e1], nav->surface_data_faces[f2].snodes[c].start)
                        }
                    }

                    if (0) // Add connection from EDGE to EDGE of nearby face
                    {
                        //for (int e3 = 0; e3 < nav->surface_data_faces[f1].num_verts; e3)
                        //for (int c1 = 0; c1 < nav->surface_data_faces[f1].snode_counter; c1++)
                        {
                            for (int c = 0; c < nav->surface_data_faces[f2].snode_counter; c++)
                            {
                                VectorCopy(nav->surface_data_faces[f1].verts[e1], start_pos);
                                //VectorCopy(nav->surface_data_faces[f1].snodes[c1].start, start_pos);
                                VectorCopy(nav->surface_data_faces[f2].snodes[c].start, end_pos);
                                if (BOTLIB_BSP_AddAreaConnections(nav, f1, f2, start_pos, end_pos, FACE_CONN_DIRECT, FACE_MOVE_SAFE, 0) == 0)
                                    area_connections++;

                                VectorCopy(nav->surface_data_faces[f1].verts[e1 + 1], start_pos);
                                if (BOTLIB_BSP_AddAreaConnections(nav, f1, f2, start_pos, end_pos, FACE_CONN_DIRECT, FACE_MOVE_SAFE, 0) == 0)
                                    area_connections++;
                            }
                        }
                    }

                    // Connected face edges
                    if (0)
                    {
                        //VectorCopy(nav->surface_data_faces[f1].edge_center[e1 / 2], start_pos);
                        //VectorCopy(nav->surface_data_faces[f2].edge_center[e2 / 2], end_pos);
                        //if (BOTLIB_BSP_AddAreaConnections(nav, f1, f2, start_pos, end_pos, FACE_CONN_DIRECT, FACE_MOVE_SAFE, 0) == 0)
                        //    area_connections++;

                        //connected = true; // We connected

                        for (int c = 0; c < nav->surface_data_faces[f2].snode_counter; c++)
                        {
                            //if (nav->surface_data_faces[f2].snodes[c].type == FACETYPE_WALK)
                            {
                                //if (nav->surface_data_faces[f2].snodes[c].start)
                                {
                                    // Add connection from EDGE TO EDGE within the same face
                                    VectorCopy(nav->surface_data_faces[f1].edge_center[e1 / 2], start_pos);
                                    VectorCopy(nav->surface_data_faces[f2].snodes[c].start, end_pos);
                                    if (BOTLIB_BSP_AddAreaConnections(nav, f1, f2, start_pos, end_pos, FACE_CONN_DIRECT, FACE_MOVE_SAFE, 0) == 0)
                                        area_connections++;

                                    //break;
                                }
                            }
                        }
                    }

                    //break; // Finished connecting
                }
                //if (connected) break; // Finished connecting, search another face
            }
        }
    }





    return;




    // Test and add connected face edges
    for (int f1 = 0; f1 < nav->faces_total; f1++) // Face 1
    {
        //if (IgnoreSurface(&nav->surface_data_faces[f1])) // Ignore surface
        //    continue;

        if (nav->surface_data_faces[f1].face_type == FACETYPE_WALL)
			continue;

        for (int f2 = 0; f2 < nav->faces_total; f2++) // Face 2
        {
            if (f1 == f2) continue; // skip self

            if (nav->surface_data_faces[f2].face_type == FACETYPE_WALL)
                continue;

            //if (IgnoreSurface(&nav->surface_data_faces[f2])) // Ignore surface
            //    continue;

            // Ignore faces far away
            //if (VectorDistance(nav->surface_data_faces[f1].center_poly, nav->surface_data_faces[f2].center_poly) > 256)
			//	continue;

            for (int e1 = 0; e1 < nav->surface_data_faces[f1].num_verts; e1 += 2) // Edge 1 (face1)
            {
                if (IgnoreSmallSurfaces) // Skip very small edges
                {
                    //if (VectorDistance(nav->surface_data_faces[f2].verts[e1], nav->surface_data_faces[f2].verts[e1 + 1]) < MIN_EDGE_WIDTH)
                    //    continue;
                }

                for (int e2 = 0; e2 < nav->surface_data_faces[f2].num_verts; e2 += 2) // Edge 2 (face2)
                {
                    if (IgnoreSmallSurfaces) // Skip very small edges
                    {
                        //if (VectorDistance(nav->surface_data_faces[f2].verts[e2], nav->surface_data_faces[f2].verts[e2 + 1]) < MIN_EDGE_WIDTH)
                        //    continue;
                    }

                    // Do edges connect?
                    //if (UTIL_EdgeCompare(nav->surface_data_faces[f1].verts[e1], nav->surface_data_faces[f1].verts[e1 + 1], nav->surface_data_faces[f2].verts[e2], nav->surface_data_faces[f2].verts[e2 + 1], nav->surface_data_faces[f2].edge_center[e2 / 2]) == false)
                    if (UTIL_EdgeCompare(nav->surface_data_faces[f1].verts[e1], nav->surface_data_faces[f1].verts[e1 + 1], nav->surface_data_faces[f2].verts[e2], nav->surface_data_faces[f2].verts[e2 + 1]) == false)
                    {
                        //if (UTIL_FindLedge == false)
							continue;
                    }

                    // For each edge, test the start and end positions
                    for (int e3 = 0; e3 < nav->surface_data_faces[f1].num_verts; e3 += 2)
                    {
                        if (e1 == e3) continue; // Skip same edge

                        // Skip edges going in the same direction
                        if (UTIL_VectorDirectionCompare(nav->surface_data_faces[f1].verts[e1], nav->surface_data_faces[f1].verts[e1 + 1], nav->surface_data_faces[f1].verts[e3], nav->surface_data_faces[f1].verts[e3 + 1], true, 0.1) == true)
                            continue;

                        // Test if the start pos fits
                        if (UTIL_PlayerBoxTestCouched(nav->surface_data_faces[f1].edge_center[e3 / 2]) == false)
                            continue;

                        // Test LoS
                        if (UTIL_LineOfSight(nav->surface_data_faces[f1].edge_center[e1 / 2], nav->surface_data_faces[f1].edge_center[e3 / 2], nav->surface_data_faces[f1].normal))
                        {
                            VectorCopy(nav->surface_data_faces[f1].edge_center[e3 / 2], start_pos);
                            VectorCopy(nav->surface_data_faces[f1].edge_center[e1 / 2], end_pos);

                            if (BOTLIB_BSP_AddAreaConnections(nav, f1, f2, start_pos, end_pos, FACE_CONN_DIRECT, FACE_MOVE_SAFE, 0) == 0)
                            {
                                area_connections++;
                                //goto NEXT_FACE; // Success (either added or already added)
                            }
                            //else
                            //    return; // Error
                        }
                    }

                    /*
                    // For each vertex, test the start and end positions
                    for (int e3 = 0; e3 < bsp->surface_data_faces[f1].num_verts; e3 += 2)
                    {
                        if (e1 == e2) continue; // Skip same edge

                        // Test if the start pos fits
                        if (UTIL_PlayerBoxTestCouched(bsp->surface_data_faces[f1].edge_center[e3 / 2]) == false)
                            continue;

                        // Test LoS
                        if (UTIL_LineOfSight(bsp->surface_data_faces[f2].edge_center[e2 / 2], bsp->surface_data_faces[f1].edge_center[e3 / 2], bsp->surface_data_faces[f1].normal))
                        {
                            VectorCopy(bsp->surface_data_faces[f1].edge_center[e3 / 2], start_pos);
                            VectorCopy(bsp->surface_data_faces[f1].edge_center[e1 / 2], end_pos);

                            if (BOTLIB_BSP_AddAreaConnections(bsp, f1, f2, start_pos, end_pos, FACE_CONN_DIRECT) == 0)
                            {
                                area_connections++;
                                //goto NEXT_FACE; // Success (either added or already added)
                            }
                            else
                                return; // Error
                        }
                    }
                    */

                } // End Edge 2
            } // End Edge 1
          

        } // End Face 2
    } //End Face 1

    Com_Printf("%s - Found %d area connections\n", __func__, area_connections);
}

// Init face connections
void InitFaceConnections(nav_t* nav)
{
    if (surface_faces_connected)
        return;
    surface_faces_connected = true; // Surface faces are now connected

    if (nav->surface_data_faces == NULL)
    {
        Com_Printf("%s - surface data is NULL\n", __func__);
        return;
    }

    // Init the nearest face edges
    for (int f = 0; f < nav->faces_total; f++) // Face
    {
        nav->surface_data_faces[f].snode_counter = 0; // Init the nearest counter

        for (int fc = 0; fc < MAX_FACE_CONNECTIONS; fc++)
        {
            nav->surface_data_faces[f].snodes[fc].type = FACE_CONN_NONE; // (face_connection_type_t)
            nav->surface_data_faces[f].snodes[fc].facenum = INVALID;
            VectorClear(nav->surface_data_faces[f].snodes[fc].end);
        }
    }

    // Find ledges
    for (int f = 0; f < nav->faces_total; f++) // Face
    {
        UTIL_FindLedges(nav, &nav->surface_data_faces[f]); // Find ledges
    }

    FindNearestFacesByConnectedEdges(nav); // Find the nearest connected face edges
}



// Recursively checks for connected polygons
void ShowConnectedPolygons_r(nav_t* nav, int face, int *facelist, int *facelist_count)
{
    qboolean found = false;
    int new_face = INVALID;

    if ((*facelist_count) + 1 >= MAX_FACE_CONNECTIONS)
    {
        Com_Printf("%s - MAX_FACE_CONNECTIONS reached\n", __func__);
        return;
    }

	// Check if face has any connections
	for (int c = 0; c < nav->surface_data_faces[face].snode_counter; c++)
	{
        if ((*facelist_count) + 1 >= MAX_FACE_CONNECTIONS)
        {
            Com_Printf("%s - MAX_FACE_CONNECTIONS reached\n", __func__);
            return;
        }

		if (nav->surface_data_faces[face].snodes[c].type == FACE_CONN_NONE)
			continue;

        new_face = nav->surface_data_faces[face].snodes[c].facenum; // get face number

        if (new_face == face) // Skip self face
			continue;

		// Check if face is already in the list
        found = false;
		for (int f = 0; f < *facelist_count; f++)
		{
            if (new_face == facelist[f])
            {
                found = true;
                break;
            }
		}
		if (found) continue;

		// Add face to list
		facelist[*facelist_count] = new_face;
		(*facelist_count)++;

		// Recursively check for other connected faces
		ShowConnectedPolygons_r(nav, new_face, facelist, facelist_count);
	}

	return;
}
void ShowConnectedPolygons(nav_t* nav, int face)
{
    int facelist[MAX_FACE_CONNECTIONS];
    int facelist_count = 0;
    memset( facelist, 0, sizeof(facelist) );

    facelist[0] = face; // Add the initial face to array
    facelist_count++;

    ShowConnectedPolygons_r(nav, face, facelist, &facelist_count); // From the initial face, recursively find all the connected faces

    // Draw the faces
    for (int f = 0; f < facelist_count; f++)
	{
        face = facelist[f]; // Get the next face in the array

        // Draw all polygon faces
        //GL_ColorPolygon(nav->surface_data_faces[face].aligned_verts[0], nav->surface_data_faces[face].num_aligned_verts, &nav->surface_data_faces[face], 0, 0, 1, 0.4);

        // Draw all arrows / lines
        if (1)
        {
            for (int c = 0; c < nav->surface_data_faces[face].snode_counter; c++)
            {
                //GL_DrawArrow(nav->surface_data_faces[face].snodes[nf].start, nav->surface_data_faces[face].snodes[nf].end, U32_CYAN, 10);

                const uint32_t red[24] = { U32_RED, U32_RED };
                vec3_t points[2];
                VectorCopy(nav->surface_data_faces[face].snodes[c].start, points[0]);
                VectorCopy(nav->surface_data_faces[face].snodes[c].end, points[1]);
                GL_DrawLine(points[0], 2, red, 10, false);
            }
        }
	}

    /*
    for (int nf = 0; nf < nav->surface_data_faces[face].snode_counter; nf++)
    {
        int nearest_face = nav->surface_data_faces[face].snodes[nf].facenum; // get face number
        if (nearest_face == INVALID) continue; // skip invalid
        //if (nearest_face == face) continue; // skip self
        GL_ColorPolygon(nav->surface_data_faces[nearest_face].aligned_verts[0], nav->surface_data_faces[nearest_face].num_aligned_verts, &nav->surface_data_faces[nearest_face], 0, 0, 1, 0.4);
    }
    */
}

// Draws the current face the player it looking at
//nav_t* nav_ = NULL;
static void BOTLIB_DrawFaces(void)
{
    int f;
    const uint32_t color_magenta[24] = { U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA, U32_MAGENTA };
    const uint32_t color_cyan[24] = { U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN, U32_CYAN };
    const uint32_t color_green[24] = { U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN };
    #define U32_LIGHT_BLUE    MakeColor(0, 128, 255, 255)
    const uint32_t color_light_blue[24] = { U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE, U32_LIGHT_BLUE };
    const uint32_t color_red[24] = { U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED, U32_RED };
    const uint32_t color_yellow[24] = { U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW, U32_YELLOW };
    #define U32_ORANGE  MakeColor(255, 165, 0, 255)
    const uint32_t color_orange[24] = { U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE,
                                    U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE,
                                    U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE,
                                    U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE, U32_ORANGE };
    const uint32_t all_colors[24] = { U32_RED, U32_RED, U32_GREEN, U32_GREEN, U32_BLUE, U32_BLUE, U32_YELLOW, U32_YELLOW, U32_CYAN, U32_CYAN, U32_MAGENTA, U32_MAGENTA, U32_RED, U32_RED, U32_GREEN, U32_GREEN, U32_BLUE, U32_BLUE, U32_YELLOW, U32_YELLOW, U32_CYAN, U32_CYAN, U32_MAGENTA, U32_MAGENTA };

    //if (!gl_static.world.cache) // Client side
    if (!sv.cm.cache) // Server side
    {
        Com_Error(ERR_DROP, "%s: no map loaded", __func__);
        return;
    }
    //bsp_t* bsp = gl_static.world.cache; // Client side
    bsp_t* bsp = sv.cm.cache; // Server side

    // Malloc nav
    if (sv.cm.nav == NULL)
    {
        sv.cm.nav = (nav_t*) Z_TagMallocz(sizeof(nav_t), TAG_CMODEL); // This malloc is purposely freed in CM_FreeMap() -- this should occur every map change
        if (sv.cm.nav == NULL)
        {
            Com_Error(ERR_DROP, "%s: malloc failed", __func__);
            return;
        }
        //memset(sv.cm.nav, 0, sizeof(nav_t));
    }
    nav_t* nav = sv.cm.nav;


    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////

    if (0)        return;       // Disable nav

    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////


    // Save the surface data
    BOTLIB_SaveSurfaceData(nav, bsp);

    if (nav->surface_data_faces == NULL)
	{
		Com_Printf("%s: nav->surface_data_faces == NULL\n", __func__);
		return;
	}

    //surface_data_t* sdf = &nav->surface_data_faces[255];
    //nav->surface_data_faces[255].facenum;

    InitFaceConnections(nav); // Init the faces


    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////

    if (1)        return;       // Disable realtime nav view

    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////

    // Feet trace
    float vert_plane_epsilon = 0.01;
    qboolean line_trace = false;
    trace_t tr;
    tr = UTIL_FeetBoxTrace(); // Box trace from player feet to the world below
    //if (tr.fraction == 1.0 && tr.plane.normal[2] != 1) // Found nothing, try a box trace
    if (tr.plane.normal[2] != 1) // Not on flat ground
    {
        line_trace = true;
        //tr = UTIL_FeetLineTrace(); // Line trace from player feet to the world below
    }

    // Camera trace
    trace_t tr_cam = UTIL_CameraTrace(64);  // 64 - Trace from the camera to the world
    qboolean ladder = (tr_cam.contents & CONTENTS_LADDER);
    //if (tr_cam.contents & CONTENTS_LADDER) Com_Printf("%s: tr_cam.contents & CONTENTS_LADDER\n", __func__);

    if (0)
    {
        // Cycles through RGB colors (0.0 to 1.0)
        float cycle_red;
        float cycle_green;
        float cycle_blue;
        UTIL_CycleThroughColors(&cycle_red, &cycle_green, &cycle_blue, 0.001);
    }

    for (f = 0; f < nav->faces_total; f++)
    {
        // Skip checking these faces
        //if (nav->surface_data_faces[f].face_type != FACETYPE_WALK && nav->surface_data_faces[f].face_type != FACETYPE_TINYWALK && nav->surface_data_faces[f].face_type != FACETYPE_LADDER) // || nav->surface_data_faces[f].face_type == FACETYPE_WATER)
        if ((nav->surface_data_faces[f].face_type & FACETYPE_WALK) || (nav->surface_data_faces[f].face_type & FACETYPE_TINYWALK) || (nav->surface_data_faces[f].face_type & FACETYPE_LADDER) || (nav->surface_data_faces[f].face_type & FACETYPE_WATER))
            ;
        else
            continue;

        //if (nav->surface_data_faces[f].face_type & FACETYPE_ROOF)
		//	continue;

        if (1) //if (ladder == false)
        {
            if (1) // Set to false to show all faces
            {
                // Ignore faces that are too far away
                if (VectorDistance(nav->surface_data_faces[f].center_poly, tr.endpos) > 512) // Distance from the face center to the player
                    continue;
                //GL_ColorPolygon(nav->surface_data_faces[f].aligned_verts[0], nav->surface_data_faces[f].num_aligned_verts, 0, 255, 0, 0.25); // debug to show distance

                if (UTIL_CompareNormal(nav->surface_data_faces[f].normal, tr.plane.normal, 0.05) == false) // Same normals
                    continue;

                if (line_trace) vert_plane_epsilon = 0.31; // Slopes
                else vert_plane_epsilon = 0.01; // Flat ground

                // Slope planes
                if (line_trace && UTIL_CompareVertPlanesSlope(nav->surface_data_faces[f].aligned_verts[0], nav->surface_data_faces[f].num_aligned_verts, tr.endpos, tr.plane.normal, vert_plane_epsilon) == false)
                    continue; // Not on the same plane
                // Flat planes
                else if (UTIL_CompareVertPlanes(nav->surface_data_faces[f].verts[0], tr.endpos, tr.plane.normal, vert_plane_epsilon) == false)
                    continue; // Not on the same plane

                //if (AAS_InsideFace(&nav->surface_data_faces[f], tr.endpos, 0.01, line_trace) == false) // Point inside face
                //    continue;

                if (AAS_InsideFaceBoxScan(&nav->surface_data_faces[f], tr.endpos, 0.01, line_trace) == false) // Box inside face
                    continue;

                ShowConnectedPolygons(nav, f); // Show connected polygons
            }


            //trace_t tr_test = UTIL_FeetBoxTrace(); // Box trace from player feet to the world below
            //Com_Printf("%s line_trace_height:%f box_trace_height:%f\n", __func__, tr.endpos[2], tr_test.endpos[2]);

    
            if (0) // Show colored face
            {
                GL_ColorPolygon(nav->surface_data_faces[f].aligned_verts[0], nav->surface_data_faces[f].num_aligned_verts, &nav->surface_data_faces[f], 0, 0.0, 1, 0.4);
                //nav->surface_data_faces[f].surf->texnum[1] = TEXNUM_WHITE;
            }

            if (0) // DEBUG: Show edges
            {
                if (0 && nav->surface_data_faces[f].contents & CONTENTS_LADDER) // Draw ladder edges
                {
                    // Get the distance from the camera to poly center
                    vec3_t vec;
                    VectorSubtract(nav->surface_data_faces[f].center_poly, glr.fd.vieworg, vec);
                    float distance = VectorLength(vec);
                    float line_width;
                    // Change line width to be thinner the further away the distance is
                    if (distance < 1024) line_width = 12;
                    else if (distance < 2048) line_width = 4;
                    else line_width = 2;

                    //Com_Printf("%s [LADDER] face:%d verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_aligned_verts, volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                    GL_DrawLine(nav->surface_data_faces[f].verts[0], nav->surface_data_faces[f].num_verts, color_cyan, line_width, false);
                }
                else
                {
                    //Com_Printf("%s face:%d verts:%d vol:%f frac:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_verts, volume, tr.fraction, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                    //for (int i = 0; i < surface_data_faces[f].num_verts; i += 2)
                    {
                        //int edge = i / 2; // edge num
                        //Com_Printf("%s face:%d edge:%d [%f %f %f] -- [%f %f %f]\n", __func__, f, edge, nav->surface_data_faces[f].verts[i][0], nav->surface_data_faces[f].verts[i][1], nav->surface_data_faces[f].verts[i][2], nav->surface_data_faces[f].verts[i + 1][0], nav->surface_data_faces[f].verts[i + 1][1], nav->surface_data_faces[f].verts[i + 1][2]);
                    }


                    if (0) // Draw ledge lines
                    {
                        int num_ledge_verts = 0;
                        vec3_t ledge_verts[MAX_FACE_VERTS];
                        for (int i = 0; i < nav->surface_data_faces[f].num_verts; i += 2)
                        {
                            if (nav->surface_data_faces[f].ledge[i / 2].is_ledge) // Copy only ledge verts
                            {
                                //VectorCopy(nav->surface_data_faces[f].verts[i], ledge_verts[num_ledge_verts]);
                                //VectorCopy(nav->surface_data_faces[f].verts[i + 1], ledge_verts[num_ledge_verts + 1]);
                                VectorCopy(nav->surface_data_faces[f].ledge[i / 2].v1, ledge_verts[num_ledge_verts]);
                                VectorCopy(nav->surface_data_faces[f].ledge[i / 2].v2, ledge_verts[num_ledge_verts + 1]);
                                num_ledge_verts += 2;
                            }
                        }

                        GL_DrawLine(ledge_verts[0], num_ledge_verts, color_orange, 4.0, false);
                    }
                    if (0) // Draw wall lines
                    {
                        int num_ledge_verts = 0;
                        vec3_t ledge_verts[MAX_FACE_VERTS];
                        for (int i = 0; i < nav->surface_data_faces[f].num_verts; i += 2)
                        {
                            if (nav->surface_data_faces[f].ledge[i / 2].is_wall) // Copy only wall verts
                            {
                                VectorCopy(nav->surface_data_faces[f].ledge[i / 2].v1, ledge_verts[num_ledge_verts]);
                                VectorCopy(nav->surface_data_faces[f].ledge[i / 2].v2, ledge_verts[num_ledge_verts + 1]);
                                num_ledge_verts += 2;
                            }
                        }

                        GL_DrawLine(ledge_verts[0], num_ledge_verts, color_red, 4.0, false);
                    }


                    // Draw the edges of face player is standing on
                    //GL_DrawLine((const vec3_t*)&nav->surface_data_faces[f].verts, nav->surface_data_faces[f].num_verts, all_colors, 3.0);

                    //Draw Arrows
                    if (1)
                    {
                        // Find the closest starting point
                        float closest = 999999;
                        int nearest_start_num = INVALID;
                        //vec3_t nearest_start_vec = { 0 };
                        for (int nf = 0; nf < nav->surface_data_faces[f].snode_counter; nf++)
                        {
                            int nearest_face = nav->surface_data_faces[f].snodes[nf].facenum;
                            if (nearest_face != -1)
                            {
                                //volume = AAS_FaceArea(&nav->surface_data_faces[nearest_face]); // Surface area
                                //Com_Printf("%s nearest face:%d verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, nearest_face, nav->surface_data_faces[nearest_face].num_verts, volume, nav->surface_data_faces[nearest_face].normal[0], nav->surface_data_faces[nearest_face].normal[1], nav->surface_data_faces[nearest_face].normal[2], nav->surface_data_faces[nearest_face].texture);

                                // Draw the connected faces to the face player is standing on (except the face player is standing on)
                                if (0)
                                {
                                    if (nearest_face != f)
                                        GL_DrawLine(nav->surface_data_faces[nearest_face].verts[0], nav->surface_data_faces[nearest_face].num_verts, all_colors, 3.0, false);
                                }

                                // Draw arrows to connected faces
                                //GL_DrawArrow(nav->surface_data_faces[f].snodes[nf].start, nav->surface_data_faces[f].snodes[nf].end);

                                // Get the distance from the player to the nearest connected face
                                float nearest_end_point = VectorDistance(nav->surface_data_faces[f].snodes[nf].start, tr.endpos);
                                if (nearest_end_point < closest)
                                {
                                    closest = nearest_end_point;
                                    //VectorCopy(nav->surface_data_faces[f].snodes[nf].start, nearest_start_vec);
                                    nearest_start_num = nf;
                                }
                            }
                        }

                        // Draw nearest connection within face
                        for (int nf = 0; nf < nav->surface_data_faces[f].snode_counter; nf++)
                        {
                            // Ledge arrows down to connected faces (have a different starting point)
                            if (nav->surface_data_faces[f].snodes[nf].dropheight > 0)
                            {
                                // If the x,y coords are the same (with an EPSILON of 0.1) then it's a ledge
                                // We use EPSILON here because a trace endpos is not 100% accurately located below and we don't want to draw a ledge arrow if it's not a ledge
                                if (fabs(nav->surface_data_faces[f].snodes[nf].start[0] - nav->surface_data_faces[f].snodes[nf].end[0]) < 0.1
                                    && fabs(nav->surface_data_faces[f].snodes[nf].start[1] - nav->surface_data_faces[f].snodes[nf].end[1]) < 0.1)
                                    //if (nav->surface_data_faces[f].snodes[nf].type == FACE_CONN_DROP 
                                    //    || nav->surface_data_faces[f].snodes[nf].type == FACE_CONN_STEP
                                    //    || nav->surface_data_faces[f].snodes[nf].type == FACE_CONN_JUMP)
                                {
                                    // Only draw drop arrows if the drop height is less than the max crouch fall height
                                    if (nav->surface_data_faces[f].snodes[nf].dropheight < MAX_CROUCH_FALL_HEIGHT)
                                    {
                                        //GL_DrawArrow(nav->surface_data_faces[f].snodes[nf].start, nav->surface_data_faces[f].snodes[nf].end, U32_YELLOW, 10, false); // Caution
                                        continue;
                                    }
                                }
                            }

                            if (nav->surface_data_faces[f].snodes[nf].type == FACE_CONN_DROP
                                || nav->surface_data_faces[f].snodes[nf].type == FACE_CONN_STEP
                                || nav->surface_data_faces[f].snodes[nf].type == FACE_CONN_JUMP)
                                continue;

                            // Using the closest starting point, ensure it is valid AND face_connection_start[nf] is the same location as the closest starting point
                            if (nearest_start_num == INVALID)
                                continue;
                            if (VectorCompare(nav->surface_data_faces[f].snodes[nf].start, nav->surface_data_faces[f].snodes[nearest_start_num].start) == false)
                                continue;

                            /*
                            // Draw arrows to connected faces
                            if (nav->surface_data_faces[f].snodes[nf].dropheight == 0)
                                GL_DrawArrow(nav->surface_data_faces[f].snodes[nf].start, nav->surface_data_faces[f].snodes[nf].end, U32_CYAN, 10, false); // Go
                            else if (nav->surface_data_faces[f].snodes[nf].dropheight < MAX_CROUCH_FALL_HEIGHT)
                                GL_DrawArrow(nav->surface_data_faces[f].snodes[nf].start, nav->surface_data_faces[f].snodes[nf].end, U32_YELLOW, 10, false); // Caution
                            else
                                GL_DrawArrow(nav->surface_data_faces[f].snodes[nf].start, nav->surface_data_faces[f].snodes[nf].end, U32_RED, 10, false); // Stop
                            */

                            //if (nav->surface_data_faces[f].snodes[nf].dropheight)
                            //    Com_Printf("%s [%d] dropheight: %f\n", __func__, nf, nav->surface_data_faces[f].snodes[nf].dropheight);
                        }

                        /*
                        // Draw nearest connection from connected face
                        if (0)
                        for (int nf = 0; nf < nav->surface_data_faces[f].snode_counter; nf++)
                        {
                            int nearest_face = nav->surface_data_faces[f].face_connection_facenum[nf];
                            if (nearest_face != -1)
                            {
                                for (int ncf = 0; ncf < nav->surface_data_faces[nearest_face].snode_counter; ncf++)
                                {
                                    if (VectorCompare(nav->surface_data_faces[nearest_face].face_connection_start[ncf], nearest_start_vec))
                                    {
                                        // Draw arrows to connected faces
                                        GL_DrawArrow(nearest_start_vec, nav->surface_data_faces[nearest_face].face_connection_end[ncf], U32_WHITE, 10);
                                    }
                                }
                            }
                        }
                        */
                    }
                                
                }
            }
            if (0) // DEBUG: Show vert edges
            {
                if (nav->surface_data_faces[f].contents & CONTENTS_LADDER)
                {
                    //Com_Printf("%s [LADDER] face:%d nl_verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_aligned_verts, nav->surface_data_faces[f].volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                    GL_DrawLine(nav->surface_data_faces[f].verts[0], nav->surface_data_faces[f].num_verts, all_colors, 7.0, false);
                }
                else
                {
                    //Com_Printf("%s face:%d nl_verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_verts, nav->surface_data_faces[f].volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                    for (int i = 0; i < nav->surface_data_faces[f].num_verts; i += 2)
                    {
                        //int edge = i / 2; // edge num
                        //Com_Printf("%s face:%d edge:%d [%f %f %f] -- [%f %f %f]\n", __func__, f, edge, nav->surface_data_faces[f].aligned_verts[i][0], nav->surface_data_faces[f].aligned_verts[i][1], nav->surface_data_faces[f].aligned_verts[i][2], nav->surface_data_faces[f].aligned_verts[i + 1][0], nav->surface_data_faces[f].aligned_verts[i + 1][1], nav->surface_data_faces[f].aligned_verts[i + 1][2]);
                    }
                    GL_DrawLine(nav->surface_data_faces[f].verts[0], nav->surface_data_faces[f].num_verts, all_colors, 3.0, false);
                }
            }
            if (0) // DEBUG: Show aligned edges
            {
                if (nav->surface_data_faces[f].contents & CONTENTS_LADDER)
                {
                    Com_Printf("%s [LADDER] face:%d nl_verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_aligned_verts, nav->surface_data_faces[f].volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                    GL_DrawLine(nav->surface_data_faces[f].aligned_verts[0], nav->surface_data_faces[f].num_aligned_verts, all_colors, 7.0, false);
                }
                else
                {
                    Com_Printf("%s face:%d nl_verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_aligned_verts, nav->surface_data_faces[f].volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                    for (int i = 0; i < nav->surface_data_faces[f].num_aligned_verts; i += 2)
                    {
                        //int edge = i / 2; // edge num
                        //Com_Printf("%s face:%d edge:%d [%f %f %f] -- [%f %f %f]\n", __func__, f, edge, nav->surface_data_faces[f].aligned_verts[i][0], nav->surface_data_faces[f].aligned_verts[i][1], nav->surface_data_faces[f].aligned_verts[i][2], nav->surface_data_faces[f].aligned_verts[i + 1][0], nav->surface_data_faces[f].aligned_verts[i + 1][1], nav->surface_data_faces[f].aligned_verts[i + 1][2]);
                    }
                    GL_DrawLine(nav->surface_data_faces[f].aligned_verts[0], nav->surface_data_faces[f].num_aligned_verts, all_colors, 3.0, false);
                }
            }
            if (0) // DEBUG: Show face normals (center_poly -> center_poly_32_units)
            {
                vec3_t points[2];
                VectorCopy(nav->surface_data_faces[f].center_poly, points[0]);
                VectorCopy(nav->surface_data_faces[f].center_poly_32_units, points[1]);
                GL_DrawLine(points[0], 2, color_red, 3.0, false);
                // Print the min and max length of the face
                Com_Printf("%s face:%d min_length:%f max_length:%f volume:%f\n", __func__, f, nav->surface_data_faces[f].min_length, nav->surface_data_faces[f].max_length, nav->surface_data_faces[f].volume);
            }
            if (0) // DEBUG: Show vert center edges
			{
				vec3_t points[2];
                for (int i = 0; i < nav->surface_data_faces[f].num_verts / 2; i++)
				{
					VectorCopy(nav->surface_data_faces[f].edge_center[i], points[0]);
					VectorCopy(nav->surface_data_faces[f].edge_center_stepsize[i], points[1]);
					GL_DrawLine(points[0], 2, color_red, 3.0, false);
				}
			}
            if (0) // DEBUG: Show aligned center edges
            {
                vec3_t points[2];
                for (int i = 0; i < nav->surface_data_faces[f].num_aligned_verts / 2; i++)
                {
                    VectorCopy(nav->surface_data_faces[f].aligned_edge_center[i], points[0]);
                    VectorCopy(nav->surface_data_faces[f].aligned_edge_center_stepsize[i], points[1]);
                    GL_DrawLine(points[0], 2, color_red, 3.0, false);
                }
            }
            if (0) // DEBUG: Show valid edge positions
            {
                vec3_t points[2];
                for (int i = 0; i < nav->surface_data_faces[f].num_verts / 2; i++)
                {
                    VectorCopy(nav->surface_data_faces[f].edge_valid_pos[i], points[0]);
                    //VectorCopy(nav->surface_data_faces[f].edge_valid_stepsize[i], points[1]);
                    VectorCopy(nav->surface_data_faces[f].edge_center_stepsize[i], points[1]);
                    if (nav->surface_data_faces[f].edge_offset_type[i] == EDGE_OFFSET_LENGTH)
						GL_DrawLine(points[0], 2, color_magenta, 3.0, false); // Edge was offset
                    //else
                    //    GL_DrawLine(points[0], 2, color_red, 3.0, false); // Edge is centered
                }
            }
            if (0) // DEBUG: Show ledge start -> end
            {
                vec3_t points[2];
                for (int i = 0; i < nav->surface_data_faces[f].num_verts; i += 2)
                {
                    if (0)
                    {
                        if (nav->surface_data_faces[f].ledge->is_ledge)
                        {
                            VectorCopy(nav->surface_data_faces[f].ledge[i / 2].startpos, points[0]);
                            VectorCopy(nav->surface_data_faces[f].ledge[i / 2].endpos, points[1]);
                            GL_DrawLine(points[0], 2, color_red, 3.0, false); // Edge was offset
                        }
                    }

                    if (0)
                    {
                        if (nav->surface_data_faces[f].ledge[i / 2].hit_side == 1) // none = 0, left = 1, right = 2)
                        {
                            VectorCopy(nav->surface_data_faces[f].ledge[i / 2].left_start, points[0]);
                            VectorCopy(nav->surface_data_faces[f].ledge[i / 2].left_end, points[1]);
                            GL_DrawLine(points[0], 2, color_red, 3.0, false); // Edge was offset
                        }

                        if (nav->surface_data_faces[f].ledge[i / 2].hit_side == 2) // none = 0, left = 1, right = 2)
                        {
                            VectorCopy(nav->surface_data_faces[f].ledge[i / 2].right_start, points[0]);
                            VectorCopy(nav->surface_data_faces[f].ledge[i / 2].right_end, points[1]);
                            GL_DrawLine(points[0], 2, color_green, 3.0, false); // Edge was offset
                        }
                    }
                }
            }


        } // End feet trace


        // Camera trace
        if (0 && ladder) // Found ladder
        if (UTIL_CompareNormal(nav->surface_data_faces[f].normal, tr_cam.plane.normal, 0.05)) // Same normals
        {
            if (UTIL_CompareVertPlanes(nav->surface_data_faces[f].verts[0], tr_cam.endpos, tr_cam.plane.normal, 0.01)) // Same planes
            {
                if (AAS_InsideFace(&nav->surface_data_faces[f], tr_cam.endpos, 0.01, tr.fraction)) // Point inside face
                {
                    //float volume = AAS_FaceArea(&nav->surface_data_faces[f]); // Surface area
                    //if (volume)
                    {
                        if (0) // DEBUG: Show edges
                        {
                            if (nav->surface_data_faces[f].contents & CONTENTS_LADDER)
                            {
                                Com_Printf("%s [LADDER] face:%d verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_aligned_verts, nav->surface_data_faces[f].volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                                GL_DrawLine(nav->surface_data_faces[f].verts[0], nav->surface_data_faces[f].num_verts, all_colors, 3.0, false);
                            }
                            else
                            {
                                Com_Printf("%s face:%d verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_verts, nav->surface_data_faces[f].volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                                //for (int i = 0; i < nav->surface_data_faces[f].num_verts; i += 2)
                                {
                                    //int edge = i / 2; // edge num
                                    //Com_Printf("%s face:%d edge:%d [%f %f %f] -- [%f %f %f]\n", __func__, f, edge, nav->surface_data_faces[f].verts[i][0], nav->surface_data_faces[f].verts[i][1], nav->surface_data_faces[f].verts[i][2], nav->surface_data_faces[f].verts[i + 1][0], nav->surface_data_faces[f].verts[i + 1][1], nav->surface_data_faces[f].verts[i + 1][2]);
                                }
                                GL_DrawLine(nav->surface_data_faces[f].verts[0], nav->surface_data_faces[f].num_verts, all_colors, 3.0, false);
                            }
                        }
                        if (1) // DEBUG: Show aligned edges
                        {
                            if (nav->surface_data_faces[f].contents & CONTENTS_LADDER)
                            {
                                Com_Printf("%s [LADDER] face:%d nl_verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_aligned_verts, nav->surface_data_faces[f].volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                                GL_DrawLine(nav->surface_data_faces[f].aligned_verts[0], nav->surface_data_faces[f].num_aligned_verts, all_colors, 7.0, false);
                            }
                            else
                            {
                                Com_Printf("%s face:%d nl_verts:%d volume:%f normal[%f %f %f] texture:%s\n", __func__, f, nav->surface_data_faces[f].num_aligned_verts, nav->surface_data_faces[f].volume, nav->surface_data_faces[f].normal[0], nav->surface_data_faces[f].normal[1], nav->surface_data_faces[f].normal[2], nav->surface_data_faces[f].texture);
                                for (int i = 0; i < nav->surface_data_faces[f].num_aligned_verts; i += 2)
                                {
                                    //int edge = i / 2; // edge num
                                    //Com_Printf("%s face:%d edge:%d [%f %f %f] -- [%f %f %f]\n", __func__, f, edge, nav->surface_data_faces[f].aligned_verts[i][0], nav->surface_data_faces[f].aligned_verts[i][1], nav->surface_data_faces[f].aligned_verts[i][2], nav->surface_data_faces[f].aligned_verts[i + 1][0], nav->surface_data_faces[f].aligned_verts[i + 1][1], nav->surface_data_faces[f].aligned_verts[i + 1][2]);
                                }
                                GL_DrawLine(nav->surface_data_faces[f].aligned_verts[0], nav->surface_data_faces[f].num_aligned_verts, all_colors, 3.0, false);
                            }
                        }
                        if (1) // DEBUG: Show face normals (center_poly -> center_poly_32_units)
                        {
                            vec3_t points[2];
                            VectorCopy(nav->surface_data_faces[f].center_poly, points[0]);
                            VectorCopy(nav->surface_data_faces[f].center_poly_32_units, points[1]);
                            GL_DrawLine(points[0], 2, color_red, 3.0, false);
                        }
                    }
                }
            }
        } // End feet trace
    }
}

static void GL_DrawEdges(void)
{
    BOTLIB_DrawFaces();
}
#endif
//rekkie -- gl_showedges -- e

//rekkie -- debug drawing -- s
#if DEBUG_DRAWING
#include "../server/server.h"
// Draws a line (useful to highlight edges, normals, etc for visual debugging)
void GL_DrawLine(vec3_t verts, const int num_points, const uint32_t* colors, const float line_width, qboolean occluded)
{
    // Copy and lift the verts up a tiny bit so they're not z-clipping the surface
    const float HEIGHT = 0.03;
    vec3_t v[MAX_FACE_VERTS];
    for (int i = 0; i < num_points; i++)
    {
        VectorCopy(verts + i * 3, v[i]);
        v[i][2] += HEIGHT;
    }

    /*
    //static const uint32_t color_white[6] = { U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN, U32_GREEN };
    if (colors == NULL) // If no color was sent, pick a generic color for all lines
    {
        uint32_t color[64];
        for (int i = 0; i < num_points * 2; i++)
        {
            color[i] = U32_WHITE; // Make all the line colors the same
        }
        GL_ColorBytePointer(4, 0, (GLubyte*)color); // Set the color of the line
    }
    else
    {
        GL_ColorBytePointer(4, 0, (GLubyte*)colors); // Set the color of the line
    }
    */

    //GL_LoadMatrix(glr.viewmatrix);
    GL_BindTexture(0, TEXNUM_WHITE);
    GL_StateBits(GLS_DEFAULT);
    //GL_StateBits(GLS_BLEND_BLEND | GLS_DEPTHMASK_FALSE);
    GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
    //GL_VertexPointer(3, 0, &verts[0][0]);
    GL_VertexPointer(3, 0, &v[0][0]);
    glLineWidth(line_width); // Set the line width
    GL_ColorBytePointer(4, 0, (GLubyte*)colors); // Set the color of the line
    if (occluded)
        GL_DepthRange(0, 1); // Set the far clipping plane to 1 (obscured behind walls)
    else
        GL_DepthRange(0, 0); // Set the far clipping plane to 0 (seen behind walls)
    qglDrawArrays(GL_LINES, 0, num_points); // GL_LINES is the type of primitive to render, 0 is the starting index, 2 is the number of indices to render
    GL_DepthRange(0, 1); // Set the depth buffer back to normal (edges are now obscured)
    glLineWidth(1.0); // Reset the line width to default
}

#if 0
image_t* r_charset = NULL;
void RenderText(const char* text, float x, float y, float z, float size) 
{
    if (!r_charset) {
        qhandle_t tmp;
        tmp = R_RegisterFont("conchars");
        if (!tmp) return;
        r_charset = IMG_ForHandle(tmp);
        r_charset->texnum;
    }

    if (0)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    GL_LoadMatrix(glr.viewmatrix);
    //
    glTranslatef(x, y, z);
    glScalef(size, -size, size);
    //glEnable(GL_TEXTURE_2D);
    GL_BindTexture(0, r_charset->texnum); //fontTexture);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    float texcoord_value = 0.0625f;
    for (const char* c = text; *c; c++) {
        float cx = (float)(*c & 15) / 16.0f;
        float cy = (float)(*c >> 4) / 16.0f;
        glTexCoord2f(cx, 1 - cy - texcoord_value);
        glVertex3f(-0.5f, -0.5f, 0.0f);
        glTexCoord2f(cx + texcoord_value, 1 - cy - texcoord_value);
        glVertex3f(0.5f, -0.5f, 0.0f);
        glTexCoord2f(cx + texcoord_value, 1 - cy);
        glVertex3f(0.5f, 0.5f, 0.0f);
        glTexCoord2f(cx, 1 - cy);
        glVertex3f(-0.5f, 0.5f, 0.0f);
        glTranslatef(0.5f, 0.0f, 0.0f);
    }
    glEnd();

    //glDisable(GL_TEXTURE_2D);
    
    if (0)
    {
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }

    /*
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); 
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); 
    glLoadIdentity();

    glTranslatef(x, y, z);
    glScalef(size, -size, size);
    glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, r_charset->texnum); //fontTexture);
    GL_BindTexture(GL_TEXTURE_2D, r_charset->texnum); //fontTexture);
    //GL_BindTexture(0, TEXNUM_WHITE);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    float texcoord_value = 0.0625f;
    for (const char* c = text; *c; c++) {
        float cx = (float)(*c & 15) / 16.0f;
        float cy = (float)(*c >> 4) / 16.0f;
        glTexCoord2f(cx, 1 - cy - texcoord_value);
        glVertex3f(-0.5f, -0.5f, 0.0f);
        glTexCoord2f(cx + texcoord_value, 1 - cy - texcoord_value);
        glVertex3f(0.5f, -0.5f, 0.0f);
        glTexCoord2f(cx + texcoord_value, 1 - cy);
        glVertex3f(0.5f, 0.5f, 0.0f);
        glTexCoord2f(cx, 1 - cy);
        glVertex3f(-0.5f, 0.5f, 0.0f);
        glTranslatef(0.5f, 0.0f, 0.0f);
    }
    glEnd();
    
    glDisable(GL_TEXTURE_2D);


    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    */
}
#endif
#if 0
// This function uses the player's view angles to calculate the position and orientation of the text, 
// and then calls the glDrawString function to render the actual text.
// The function also adjusts the scale and transparency of the text based on the distance from the player's viewpoint.
void DrawStringAtOrigin(char* text, float x, float y, float z, float size) {
    // Calculate the position of the text relative to the player's view
    float position[3] = { x, y, z };
    float distance = sqrt(x * x + y * y + z * z);
    float scale = 1.0f / distance * size;
    float alpha = 1.0f;

    // Set up the OpenGL states for rendering the text as 2D billboard
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(position[0], position[1], position[2]);
    glRotatef(-r_refdef.viewangles[1], 0.0f, 1.0f, 0.0f);
    glRotatef(-r_refdef.viewangles[0], 1.0f, 0.0f, 0.0f);
    glScalef(scale, scale, scale);

    // Call the glDrawString function with the desired inputs
    glDrawString(text, 0.0f, 0.0f, 0.0f, alpha);

    // Reset the OpenGL states
    glPopMatrix();
}
#endif




// Show a cross configuration, useful to show nodes (instead of using ent + model)
void GL_DrawCross(vec3_t origin, qboolean occluded)
{
    static const uint32_t colors[6] = {
        U32_RED, U32_RED,
        U32_GREEN, U32_GREEN,
        U32_BLUE, U32_BLUE
    };
    vec3_t points[6];

    // Center
    VectorCopy(origin, points[0]); // X center
    VectorCopy(origin, points[2]); // Y center
    VectorCopy(origin, points[4]); // Z center

    // Positive axis
    VectorMA(origin, 16, glr.entaxis[0], points[1]); // X+
    VectorMA(origin, 16, glr.entaxis[1], points[3]); // Y+
    VectorMA(origin, 16, glr.entaxis[2], points[5]); // Z+

    // Negative axis (extend points in the opposite direction)
    VectorMA(origin, -16, glr.entaxis[0], points[0]); // X-
    VectorMA(origin, -16, glr.entaxis[1], points[2]); // Y-
    VectorMA(origin, -16, glr.entaxis[2], points[4]); // Z-

    GL_LoadMatrix(glr.viewmatrix);
    GL_BindTexture(0, TEXNUM_WHITE);
    GL_StateBits(GLS_DEFAULT);
    GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
    GL_ColorBytePointer(4, 0, (GLubyte*)colors);
    GL_VertexPointer(3, 0, &points[0][0]);
    glLineWidth(1); // Set the line width

    if (occluded)
        GL_DepthRange(0, 1); // Set the far clipping plane to 1 (obscured behind walls)
    else
        GL_DepthRange(0, 0); // Set the far clipping plane to 0 (seen behind walls)
    qglDrawArrays(GL_LINES, 0, 6);
    GL_DepthRange(0, 1); // Set the depth buffer back to normal (obscured behind walls)
}

void GL_DrawString(vec3_t origin, const char* string, const uint32_t color, qboolean occluded)
{
    int x, y;
    color_t color_base;
    color_base.u32 = 0xFFFFFFFF;
    float r_viewmatrix[16];

    //build view and projection matricies
    float modelview[16];
    float proj[16];
        
    Matrix4x4_CM_ModelViewMatrix(modelview, glr.fd.viewangles, glr.fd.vieworg);
    Matrix4x4_CM_Projection2(proj, glr.fd.fov_x, glr.fd.fov_y, 4);

    //build the vp matrix
    Matrix4_Multiply(proj, modelview, r_viewmatrix);


    float v[4], tempv[4], out[4];

    // get position
    v[0] = origin[0];
    v[1] = origin[1];
    v[2] = origin[2];
    v[3] = 1;

    Matrix4x4_CM_Transform4(r_viewmatrix, v, tempv);

    if (tempv[3] < 0) // the element is behind us
        return;

    tempv[0] /= tempv[3];
    tempv[1] /= tempv[3];
    tempv[2] /= tempv[3];

    out[0] = (1 + tempv[0]) / 2;
    out[1] = 1 - (1 + tempv[1]) / 2;
    out[2] = tempv[2];

    int hud_width = (r_config.width / 2);
    int hud_height = (r_config.height / 2);
    int hud_x = 0;
    int hud_y = 0;

    x = hud_x + out[0] * hud_width;
    y = hud_y + out[1] * hud_height;

    vec3_t org;
    VectorSubtract(origin, glr.fd.vieworg, org);
    float distance = VectorLength(org);
    float mult = 300 / distance; // 300
    Q_clip(mult, 0.25, 5);

    int sizex = 8 * mult;
    int sizey = 8 * mult;
    x -= (sizex / 2);
    y -= (sizey / 2);

    int uiflags = 0;

    // Center string on the X axis
    //int length = strlen(string);
    //x -= (length * CHAR_WIDTH * 0.5);

    if (1) // Fade out text as it gets further away
    {
        float alpha = (384 - distance) / 2;
        if (distance > 384) alpha = 0;

        color_base.u8[0] = 255;
        color_base.u8[1] = 255;
        color_base.u8[2] = 255;
        color_base.u8[3] = alpha;
        R_SetColor(color_base.u32);
    }

    int font_pic = R_RegisterFont("conchars");
    R_DrawString(x, y, uiflags, MAX_STRING_CHARS, string, font_pic);
    R_ClearColor();
    R_SetAlpha(1);
}

int drawstring_total = 0; // Draw string total
int drawstring_count = 0; // Draw string counter
// GL_DrawString() is called to render the text
draw_string_t draw_strings[MAX_DRAW_STRINGS];
void DrawString(int number, vec3_t origin, const char* string, const uint32_t color, const int time, qboolean occluded)
{
    if (number + 1 > MAX_DRAW_STRINGS) return;

    if (draw_strings[number].inuse == false)
    {
        draw_strings[number].inuse = true;
        draw_strings[number].time = time;
        VectorCopy(origin, draw_strings[number].origin);
        draw_strings[number].color = color;
        strncpy(draw_strings[number].string, string, sizeof(string));
        draw_strings[number].occluded = occluded;
    }
}
void GL_DrawStrings(void) // This is called in client/screen.c
{
    qboolean strings_in_use = false; // Strings in use
    if (sv.cm.draw && sv.cm.draw->strings_inuse == true) // Only run when there's at least one or more in use
    {
        for (int i = 0; i < MAX_DRAW_STRINGS; i++)
        {
            if (draw_strings[i].time < 100) // Give the gamelib some time to renew the draw request
            {
                draw_strings[i].inuse = false; // Unlock
            }

            if (draw_strings[i].time > 0)
            {
                strings_in_use = true;
                draw_strings[i].time--;
                GL_DrawString(draw_strings[i].origin, draw_strings[i].string, draw_strings[i].color, draw_strings[i].occluded);
            }
        }
    }

    if (sv.cm.draw && sv.cm.draw->strings_inuse && strings_in_use == false) // Check if it's possible to reduce debug drawing calls when not in use
        sv.cm.draw->strings_inuse = false;
}

int drawbox_total = 0; // Box point total
int drawbox_count = 0; // Box point counter
vec3_t *box_points;
uint32_t* box_colors;
static qboolean ALLOC_BoxPoints(int num_boxes)
{
    void* if_null_1 = NULL;
    void* if_null_2 = NULL;
    if (drawbox_total == 0)
    {
        drawbox_total = (num_boxes * 24);
		box_points = (vec3_t*)malloc(drawbox_total * sizeof(vec3_t));
        box_colors = (uint32_t*)malloc(drawbox_total * sizeof(uint32_t));
        
	}
	else if (num_boxes != drawbox_total)
	{
        if (box_points[0] == NULL || box_colors[0] == NULL)
            return false;

        if_null_1 = box_points;
        if_null_2 = box_colors;
        drawbox_total = (num_boxes * 24);
		box_points = (vec3_t*)realloc(box_points, drawbox_total * sizeof(vec3_t));
		box_colors = (uint32_t*)realloc(box_colors, drawbox_total * sizeof(uint32_t));
	}

    if (box_points == NULL)
    {
        Com_Printf("%s: Failed to allocate memory for box_points\n", __func__);
        if (if_null_1)
        {
            free(if_null_1); // Free previous memory
            if_null_1 = NULL; // Nullify dangling pointer
        }
		return false;
	}
    if (box_colors == NULL)
    {
        Com_Printf("%s: Failed to allocate memory for box_colors\n", __func__);
        if (if_null_2)
        {
            free(if_null_2); // Free previous memory
            if_null_2 = NULL; // Nullify dangling pointer
        }
        return false;
    }



    return true;
}
static void FREE_BoxPoint(void)
{
	if (box_points != NULL)
	{
		free(box_points);
		box_points = NULL;
	}
    if (box_colors != NULL)
    {
        free(box_colors);
        box_colors = NULL;
    }
    drawbox_total = 0;
    drawbox_count = 0;
}

// Single draw call: render min/max box
void GL_DrawBox(vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, qboolean occluded)
{
    const vec3_t axis[3] = { {1,0,0}, {0,1,0}, {0,0,1} };

    uint32_t colors[24];
    for (int i = 0; i < 24; i++)
        colors[i] = color;

    vec3_t points[24];

    // Top face
    vec3_t top;
    VectorMA(origin, maxs[2], axis[2], top);


    // Top
    // Start at the top right corner
    VectorMA(top, mins[1], axis[1], points[0]); //
    VectorMA(points[0], maxs[0], axis[0], points[0]); //
    VectorMA(points[0], (mins[0] * 2), axis[0], points[1]); //

    VectorCopy(points[1], points[2]); //
    VectorMA(points[2], (maxs[1] * 2), axis[1], points[3]); //

    VectorCopy(points[3], points[4]); //
    VectorMA(points[4], (maxs[0] * 2), axis[0], points[5]); //

    VectorCopy(points[5], points[6]); //
    VectorMA(points[6], (mins[1] * 2), axis[1], points[7]); //


    // Bottom face
    vec3_t bottom;
    VectorMA(origin, mins[2], axis[2], bottom); // Down from origin

    // Bottom
    VectorMA(bottom, mins[1], axis[1], points[8]); //
    VectorMA(points[8], maxs[0], axis[0], points[8]); //
    VectorMA(points[8], (mins[0] * 2), axis[0], points[9]); //

    VectorCopy(points[9], points[10]); //
    VectorMA(points[10], (maxs[1] * 2), axis[1], points[11]); //

    VectorCopy(points[11], points[12]); //
    VectorMA(points[12], (maxs[0] * 2), axis[0], points[13]); //

    VectorCopy(points[13], points[14]); //
    VectorMA(points[14], (mins[1] * 2), axis[1], points[15]); //


    // Sides
    VectorCopy(points[0], points[16]); //
    VectorCopy(points[8], points[17]); //

    VectorCopy(points[2], points[18]); //
    VectorCopy(points[10], points[19]); //

    VectorCopy(points[4], points[20]); //
    VectorCopy(points[12], points[21]); //

    VectorCopy(points[6], points[22]); //
    VectorCopy(points[14], points[23]); //


    GL_LoadMatrix(glr.viewmatrix);
    GL_BindTexture(0, TEXNUM_WHITE);
    GL_StateBits(GLS_DEFAULT);
    GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
    GL_ColorBytePointer(4, 0, (GLubyte*)colors);
    GL_VertexPointer(3, 0, &points[0][0]);
    glLineWidth(2); // Set the line width

    if (occluded)
        GL_DepthRange(0, 1); // Set the far clipping plane to 1 (obscured behind walls)
    else
        GL_DepthRange(0, 0); // Set the far clipping plane to 0 (seen behind walls)
    qglDrawArrays(GL_LINES, 0, 24);
    GL_DepthRange(0, 1); // Set the depth buffer back to normal (obscured behind walls)
}

// Batch draw call: render min/max box
static void GL_AddDrawBox(vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, qboolean occluded)
{
    const vec3_t axis[3] = { {1,0,0}, {0,1,0}, {0,0,1} };
    
    if (drawbox_total && drawbox_count < drawbox_total)
    {
        int nbp = drawbox_count; // Box point
        drawbox_count += 24;

        for (int i = 0; i < 24; i++)
            box_colors[nbp + i] = color;

        // Top face
        vec3_t top;
        VectorMA(origin, maxs[2], axis[2], top);

        // Top
        // Start at the top right corner
        VectorMA(top, mins[1], axis[1], box_points[nbp]); //
        VectorMA(box_points[nbp], maxs[0], axis[0], box_points[nbp]); //
        VectorMA(box_points[nbp], (mins[0] * 2), axis[0], box_points[nbp + 1]); //

        VectorCopy(box_points[nbp + 1], box_points[nbp + 2]); //
        VectorMA(box_points[nbp + 2], (maxs[1] * 2), axis[1], box_points[nbp + 3]); //

        VectorCopy(box_points[nbp + 3], box_points[nbp + 4]); //
        VectorMA(box_points[nbp + 4], (maxs[0] * 2), axis[0], box_points[nbp + 5]); //

        VectorCopy(box_points[nbp + 5], box_points[nbp + 6]); //
        VectorMA(box_points[nbp + 6], (mins[1] * 2), axis[1], box_points[nbp + 7]); //


        // Bottom face
        vec3_t bottom;
        VectorMA(origin, mins[2], axis[2], bottom); // Down from origin

        // Bottom
        VectorMA(bottom, mins[1], axis[1], box_points[nbp + 8]); //
        VectorMA(box_points[nbp + 8], maxs[0], axis[0], box_points[nbp + 8]); //
        VectorMA(box_points[nbp + 8], (mins[0] * 2), axis[0], box_points[nbp + 9]); //

        VectorCopy(box_points[nbp + 9], box_points[nbp + 10]); //
        VectorMA(box_points[nbp + 10], (maxs[1] * 2), axis[1], box_points[nbp + 11]); //

        VectorCopy(box_points[nbp + 11], box_points[nbp + 12]); //
        VectorMA(box_points[nbp + 12], (maxs[0] * 2), axis[0], box_points[nbp + 13]); //

        VectorCopy(box_points[nbp + 13], box_points[nbp + 14]); //
        VectorMA(box_points[nbp + 14], (mins[1] * 2), axis[1], box_points[nbp + 15]); //


        // Sides
        VectorCopy(box_points[nbp + 0], box_points[nbp + 16]); //
        VectorCopy(box_points[nbp + 8], box_points[nbp + 17]); //

        VectorCopy(box_points[nbp + 2], box_points[nbp + 18]); //
        VectorCopy(box_points[nbp + 10], box_points[nbp + 19]); //

        VectorCopy(box_points[nbp + 4], box_points[nbp + 20]); //
        VectorCopy(box_points[nbp + 12], box_points[nbp + 21]); //

        VectorCopy(box_points[nbp + 6], box_points[nbp + 22]); //
        VectorCopy(box_points[nbp + 14], box_points[nbp + 23]); //

    }
}
static void GL_BatchDrawBoxes(int num_boxes, qboolean occluded)
{
    if (drawbox_total)
    {
        GL_LoadMatrix(glr.viewmatrix);
        GL_BindTexture(0, TEXNUM_WHITE);
        GL_StateBits(GLS_DEFAULT);
        GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
        GL_ColorBytePointer(4, 0, (GLubyte*)box_colors);
        GL_VertexPointer(3, 0, &box_points[0][0]);
        glLineWidth(2); // Set the line width

        if (occluded)
            GL_DepthRange(0, 1); // Set the far clipping plane to 1 (obscured behind walls)
        else
            GL_DepthRange(0, 0); // Set the far clipping plane to 0 (seen behind walls)

        qglDrawArrays(GL_LINES, 0, drawbox_total);

        GL_DepthRange(0, 1); // Set the depth buffer back to normal (obscured behind walls)
    }
}

// Single draw call: render a flat 2D square
void GL_DrawSelectionSquare(vec3_t start, vec3_t end, float min, float max, uint32_t color, float line_width, qboolean occluded)
{
    // I just need the starting point and an end point. X and Y are offset from the starting point.
    // Start 0,0   End 100,100
    // 
    // Areas: 
    // * Go to area selection mode in the nav editor
    // * Only show nodes (no links) and a number to indicate their area num. By default all nodes are area 0.
    // * Select desired nodes
    // * Use a console command to assign a number nav_areanum <num>

    //start[0] = 100;
    //start[1] = 100;
    //start[2] = 0;
    //end[0] = 100;
    //vec3_t st;
    //vec3_t en;
    

    uint32_t colors[12];
    for (int i = 0; i < 12; i++)
        colors[i] = color;

    vec3_t points[8];
    float x_diff = (end[0] - start[0]);
    float y_diff = (end[1] - start[1]);

    // Draw the lower selection box
    {
        start[2] = min;
        end[2] = min;

        // Draw line from start to end on the X axis
        VectorCopy(start, points[0]); // Start
        VectorCopy(start, points[1]); // End (right 100)
        points[1][0] += x_diff;
        GL_DrawLine(points[0], 2, colors, line_width, occluded);

        // Draw line from start to end on the Y axis
        VectorCopy(start, points[2]); // Start
        VectorCopy(start, points[3]); // End (up 100)
        points[3][1] += y_diff;
        GL_DrawLine(points[2], 2, colors, line_width, occluded);

        // Draw line from end X to end X,Y
        VectorCopy(points[1], points[4]); // Start (right 100)
        VectorCopy(points[4], points[5]); // End (up 100)
        points[4][1] += y_diff;
        GL_DrawLine(points[4], 2, colors, line_width, occluded);

        // Draw line from end Y to end X,Y
        VectorCopy(points[3], points[6]); // Start (up 100)
        VectorCopy(points[1], points[7]); // End (right 100)
        points[7][1] += y_diff;
        GL_DrawLine(points[6], 2, colors, line_width, occluded);
    }

    // Draw the sides of the selection box
    {
        // Draw line from point 0 going up
        //VectorCopy(start, points[0]); // Start
        VectorCopy(start, points[1]); // Up
        points[1][2] = max;
        GL_DrawLine(points[0], 2, colors, line_width, occluded);

        // Draw line from point 2 going up
        VectorCopy(points[3], points[2]); // Start
        //VectorCopy(start, points[3]); // End (up 100)
        //points[3][1] += y_diff;
        points[3][2] = max;
        GL_DrawLine(points[2], 2, colors, line_width, occluded);

        // Draw line from point 5 going up
        //VectorCopy(points[4], points[4]); // Start (right 100)
        VectorCopy(points[4], points[5]); // End (up 100)
        //points[4][1] += y_diff;
        points[5][2] = max;
        GL_DrawLine(points[4], 2, colors, line_width, occluded);

        // Draw line from point 7 going up
        VectorCopy(start, points[6]);
        points[6][0] += x_diff;
        //points[6][1] += y_diff;
        points[6][2] = min;
        VectorCopy(points[6], points[7]);
        points[7][2] = max;
        GL_DrawLine(points[6], 2, colors, line_width, occluded);
    }

    // Draw the upper selection box
    {
        start[2] = max;
        end[2] = max;

        // Draw line from start to end on the X axis
        VectorCopy(start, points[0]); // Start
        VectorCopy(start, points[1]); // End (right 100)
        points[1][0] += x_diff;
        GL_DrawLine(points[0], 2, colors, line_width, occluded);

        // Draw line from start to end on the Y axis
        VectorCopy(start, points[2]); // Start
        VectorCopy(start, points[3]); // End (up 100)
        points[3][1] += y_diff;
        GL_DrawLine(points[2], 2, colors, line_width, occluded);

        // Draw line from end X to end X,Y
        VectorCopy(points[1], points[4]); // Start (right 100)
        VectorCopy(points[4], points[5]); // End (up 100)
        points[4][1] += y_diff;
        GL_DrawLine(points[4], 2, colors, line_width, occluded);

        // Draw line from end Y to end X,Y
        VectorCopy(points[3], points[6]); // Start (up 100)
        VectorCopy(points[1], points[7]); // End (right 100)
        points[7][1] += y_diff;
        GL_DrawLine(points[6], 2, colors, line_width, occluded);
    }

}



int drawarrow_total = 0; // Draw arrows total
int drawarrow_count = 0; // Draw arrows counter
vec3_t* arrow_points = NULL;
uint32_t* arrow_colors = NULL;
static qboolean ALLOC_Arrows(int num_arrows)
{
    void* if_null_1 = NULL;
    void* if_null_2 = NULL;

    if (drawarrow_total == 0)
    {
        drawarrow_total = (num_arrows * 8); // 8 == 4 lines (edges) or 8 verts
        arrow_points = (vec3_t*)malloc(drawarrow_total * sizeof(vec3_t));
        arrow_colors = (uint32_t*)malloc(drawarrow_total * sizeof(uint32_t));

    }
    //else if (num_arrows != drawarrow_total)
    else if ((num_arrows * 8) != drawarrow_total)
    {
        if (arrow_points == NULL || arrow_colors == NULL)
            return false;

        if_null_1 = arrow_points;
        if_null_2 = arrow_colors;
        drawarrow_total = (num_arrows * 8);
        arrow_points = (vec3_t*)realloc(arrow_points, drawarrow_total * sizeof(vec3_t));
        arrow_colors = (uint32_t*)realloc(arrow_colors, drawarrow_total * sizeof(uint32_t));
    }

    if (arrow_points == NULL)
    {
        Com_Printf("%s: Failed to allocate memory for arrow_points\n", __func__);
        if (if_null_1)
        {
            free(if_null_1); // Free previous memory
            if_null_1 = NULL; // Nullify dangling pointer
        }
        return false;
    }
    if (arrow_colors == NULL)
    {
        Com_Printf("%s: Failed to allocate memory for arrow_colors\n", __func__);
        if (if_null_2)
        {
            free(if_null_2); // Free previous memory
            if_null_2 = NULL; // Nullify dangling pointer
        }
        return false;
    }

    return true;
}
static void FREE_Arrows(void)
{
    if (arrow_points != NULL)
    {
        free(arrow_points);
        arrow_points = NULL;
    }
    if (arrow_colors != NULL)
    {
        free(arrow_colors);
        arrow_colors = NULL;
    }
    drawarrow_total = 0;
    drawarrow_count = 0;
}

//===========================================================================
// 
//===========================================================================
static void GL_DrawArrow(vec3_t start, vec3_t end, uint32_t color, float line_width, qboolean occluded)
{
    const uint32_t colors[2] = { color, color };
    vec3_t points[2];
    vec3_t dir, cross, p1, p2, up = { 0, 0, 1 };
    float dot;

    // Limit the width for short distance runs
    //if (line_width > 4 && VectorDistance(start, end) <= 32)
    vec3_t v = { 0 };
    VectorSubtract(end, start, v);
    if (line_width > 4 && VectorLength(v) <= 32)
        line_width = 4;

    VectorSubtract(end, start, dir);
    VectorNormalize(dir);
    dot = DotProduct(dir, up);
    if (dot > 0.99 || dot < -0.99) VectorSet(cross, 1, 0, 0);
    else CrossProduct(dir, up, cross);

    // Draw the arrow line
    VectorCopy(start, points[0]);
    VectorCopy(end, points[1]);
    GL_DrawLine(points[0], 2, colors, line_width, occluded);

    // Arrow at the start
    {
        vec3_t start2 = { start[0], start[1], start[2] };
        VectorMA(start, 16, dir, start2); // Move the start forward a bit

        VectorMA(start2, -8, dir, p1);
        VectorCopy(p1, p2);
        VectorMA(p1, 8, cross, p1);
        VectorMA(p2, -8, cross, p2);

        // Draw the arrow head p1 - right
        VectorCopy(p1, points[0]);
        VectorCopy(start2, points[1]);
        GL_DrawLine(points[0], 2, colors, line_width * 3, occluded);

        // Draw the arrow head p2 - left
        VectorCopy(p2, points[0]);
        VectorCopy(start2, points[1]);
        GL_DrawLine(points[0], 2, colors, line_width * 3, occluded);

        // Draw the arrow head p3 - bottom
        VectorCopy(p1, points[0]);
        VectorCopy(p2, points[1]);
        GL_DrawLine(points[0], 2, colors, line_width * 3, occluded);
    }
}
void GL_AddDrawArrow(vec3_t start, vec3_t end, uint32_t color, float line_width, qboolean occluded)
{
    if (drawarrow_total && drawarrow_count < drawarrow_total)
    {
        int arc = drawarrow_count; // Arrow counter
        drawarrow_count += 8;

        for (int i = 0; i < 8; i++)
            arrow_colors[arc + i] = color;

        vec3_t dir, cross, p1, p2, up = { 0, 0, 1 };
        float dot;

        // Limit the width for short distance runs
        //if (line_width > 4 && VectorDistance(start, end) <= 32)
        vec3_t v = { 0 };
        VectorSubtract(end, start, v);
        if (line_width > 4 && VectorLength(v) <= 32)
            line_width = 4;

        VectorSubtract(end, start, dir);
        VectorNormalize(dir);
        dot = DotProduct(dir, up);
        if (dot > 0.99 || dot < -0.99) VectorSet(cross, 1, 0, 0);
        else CrossProduct(dir, up, cross);

        // Draw the arrow line
        VectorCopy(start, arrow_points[arc]);
        VectorCopy(end, arrow_points[arc + 1]);
        //GL_DrawLine(arrow_points[0], 2, arrow_colors, line_width, occluded);

        // Arrow at the start
        {
            vec3_t start2 = { start[0], start[1], start[2] };
            VectorMA(start, 16, dir, start2); // Move the start forward a bit

            VectorMA(start2, -8, dir, p1);
            VectorCopy(p1, p2);
            VectorMA(p1, 8, cross, p1);
            VectorMA(p2, -8, cross, p2);

            // Draw the arrow head p1 - right
            VectorCopy(p1, arrow_points[arc + 2]);
            VectorCopy(start2, arrow_points[arc + 3]);
            //GL_DrawLine(arrow_points[0], 2, arrow_colors, line_width * 3, occluded);

            // Draw the arrow head p2 - left
            VectorCopy(p2, arrow_points[arc + 4]);
            VectorCopy(start2, arrow_points[arc + 5]);
            //GL_DrawLine(arrow_points[0], 2, arrow_colors, line_width * 3, occluded);

            // Draw the arrow head p3 - bottom
            VectorCopy(p1, arrow_points[arc + 6]);
            VectorCopy(p2, arrow_points[arc + 7]);
            //GL_DrawLine(arrow_points[0], 2, arrow_colors, line_width * 3, occluded);
        }
    }
} //end of the function GL_DrawArrow
static void GL_BatchDrawArrows(qboolean occluded)
{
    if (drawarrow_total)
    {
        GL_LoadMatrix(glr.viewmatrix);
        GL_BindTexture(0, TEXNUM_WHITE);
        GL_StateBits(GLS_DEFAULT);
        GL_ArrayBits(GLA_VERTEX | GLA_COLOR);
        GL_VertexPointer(3, 0, &arrow_points[0][0]);
        GL_ColorBytePointer(4, 0, (GLubyte*)arrow_colors); // Set the color of the line
        //glLineWidth(line_width); // Set the line width

        if (occluded)
            GL_DepthRange(0, 1); // Set the far clipping plane to 1 (obscured behind walls)
        else
            GL_DepthRange(0, 0); // Set the far clipping plane to 0 (seen behind walls)

        qglDrawArrays(GL_LINES, 0, drawarrow_total); // GL_LINES is the type of primitive to render, 0 is the starting index, 2 is the number of indices to render
        
        GL_DepthRange(0, 1); // Set the depth buffer back to normal (edges are now obscured)
        glLineWidth(1.0); // Reset the line width to default
    }
}

int drawcross_total = 0; // Draw cross total
int drawcross_count = 0; // Draw cross counter
// GL_DrawCross() is called to render the array

void DrawCross(int number, vec3_t origin, int time, qboolean occluded)
{
    if (number + 1 > MAX_DRAW_CROSSES) return;

    if (draw_crosses[number].time < 100) // Give the gamelib some time to renew the draw request
    {
        draw_crosses[number].inuse = false; // Unlock
    }

    // Only update the cross if it's not in use or the origin has changed
    if (draw_crosses[number].inuse == false || VectorCompare(origin, draw_crosses[number].origin) == false)
    {
        draw_crosses[number].inuse = true;
        draw_crosses[number].time = time;
        VectorCopy(origin, draw_crosses[number].origin);
        draw_crosses[number].occluded = occluded;
    }
}
static void GL_DrawCrosses(void)
{
    qboolean crosses_in_use = false; // Crosses in use
    if (sv.cm.draw && sv.cm.draw->boxes_inuse == true) // Only run when there's at least one or more in use
    {
        for (int i = 0; i < MAX_DRAW_CROSSES; i++)
        {
            if (draw_crosses[i].time > 0)
            {
                crosses_in_use = true;
                draw_crosses[i].time--;
                GL_DrawCross(draw_crosses[i].origin, draw_crosses[i].occluded);
            }
        }
    }
    if (sv.cm.draw && sv.cm.draw->crosses_inuse && crosses_in_use == false) // Check if it's possible to reduce debug drawing calls when not in use
        sv.cm.draw->crosses_inuse = false;
}

// GL_DrawBoxes() is called to render the array
void DrawBox(int number, vec3_t origin, uint32_t color, vec3_t mins, vec3_t maxs, int time, qboolean occluded)
{
    if (number + 1 > MAX_DRAW_BOXES) return;

    if (draw_boxes[number].time < 100) // Give the gamelib some time to renew the draw request
    {
        draw_boxes[number].inuse = false; // Unlock
    }

    // Only update the box if it's not in use or the origin has changed
    if (draw_boxes[number].inuse == false || VectorCompare(origin, draw_boxes[number].origin) == false)
    {
        draw_boxes[number].inuse = true;
        draw_boxes[number].time = time;
        draw_boxes[number].color = color;
        VectorCopy(mins, draw_boxes[number].mins);
        VectorCopy(maxs, draw_boxes[number].maxs);
        VectorCopy(origin, draw_boxes[number].origin);
        draw_boxes[number].occluded = occluded;
    }
}
static void GL_DrawBoxes(void)
{
    // Get number of boxes to render
    qboolean boxes_in_use = false; // Boxes in use
    int num_boxes = 0; // Batches boxes
    if (sv.cm.draw && sv.cm.draw->boxes_inuse == true) // Only run when there's at least one or more in use
    {
        for (int i = 0; i < MAX_DRAW_BOXES; i++)
        {
            if (draw_boxes[i].time)
            {
                boxes_in_use = true;
                draw_boxes[i].time--;

                if (draw_boxes[i].occluded) // Batched boxes
                    num_boxes++;
                else // Just do a single draw call for non-occluded boxes
                    GL_DrawBox(draw_boxes[i].origin, draw_boxes[i].color, draw_boxes[i].mins, draw_boxes[i].maxs, draw_boxes[i].occluded);
            }
        }
    }

    if (num_boxes) // Batch draw the boxes
    {
        ALLOC_BoxPoints(num_boxes); // Memory allocation

        // Add them to the batch
        for (int i = 0; i < MAX_DRAW_BOXES; i++)
        {
            if (draw_boxes[i].time)
            {
                //draw_boxes[i].time--;
                GL_AddDrawBox(draw_boxes[i].origin, draw_boxes[i].color, draw_boxes[i].mins, draw_boxes[i].maxs, draw_boxes[i].occluded);
            }
        }
        drawbox_count = 0; // Reset the box point counter

        // Batch render them all in one pass
        GL_BatchDrawBoxes(num_boxes, true);
    }
    else if (drawbox_total)
        FREE_BoxPoint(); // Free memory

    if (sv.cm.draw && sv.cm.draw->boxes_inuse && boxes_in_use == false) // Check if it's possible to reduce debug drawing calls when not in use
        sv.cm.draw->boxes_inuse = false;
}

// Adds the arrow to an array of arrows to draw.
// GL_DrawArrows() is called to render the array
// occluded: true - don't draw if occluded by a wall
void DrawArrow(int number, vec3_t start, vec3_t end, uint32_t color, float line_width, int time, qboolean occluded)
{
    if (number + 1 > MAX_DRAW_ARROWS) return;

    if (draw_arrows[number].time < 100) // Give the gamelib some time to renew the draw request
    {
        draw_arrows[number].inuse = false; // Unlock
    }

    if (draw_arrows[number].inuse == false)
    {
        draw_arrows[number].inuse = true;
        draw_arrows[number].time = time;
        draw_arrows[number].color = color;
        draw_arrows[number].line_width = line_width;
        VectorCopy(start, draw_arrows[number].start);
        VectorCopy(end, draw_arrows[number].end);
        draw_arrows[number].occluded = occluded;
    }
}
static void GL_DrawArrows(void)
{
    // Get number of arrows to render
    qboolean arrows_in_use = false; // Arrows in use
    int num_arrows = 0; // Batched arrows
    if (sv.cm.draw && sv.cm.draw->arrows_inuse == true) // Only run when there's at least one or more in use
    {
        for (int i = 0; i < MAX_DRAW_ARROWS; i++)
        {
            if (draw_arrows[i].time)
            {
                arrows_in_use = true;
                draw_arrows[i].time--;

                if (draw_arrows[i].occluded && draw_arrows[i].line_width == 1.0f) // Batched arrows
                    num_arrows++;
                else  // Just do a single draw call for non-occluded arrows
                    GL_DrawArrow(draw_arrows[i].start, draw_arrows[i].end, draw_arrows[i].color, draw_arrows[i].line_width, draw_arrows[i].occluded);
            }
        }
    }

    if (num_arrows) // Batch draw the arrows
    {
        ALLOC_Arrows(num_arrows); // Memory allocation

        // Add them to the batch
        for (int i = 0; i < MAX_DRAW_ARROWS; i++)
        {
            if (draw_arrows[i].time)
            {
                GL_AddDrawArrow(draw_arrows[i].start, draw_arrows[i].end, draw_arrows[i].color, draw_arrows[i].line_width, draw_arrows[i].occluded);
            }
        }
        drawarrow_count = 0; // Reset the counter

        // Batch render them all in one pass
        GL_BatchDrawArrows(true);
    }
    else if (drawarrow_total || num_arrows == 0)
        FREE_Arrows(); // Free memory

    if (sv.cm.draw && sv.cm.draw->arrows_inuse && arrows_in_use == false) // Check if it's possible to reduce debug drawing calls when not in use
        sv.cm.draw->arrows_inuse = false;
}

//GL_DrawSelectionSquare
draw_selection_t draw_selection;
void DrawSelection(vec3_t start, vec3_t end, float min, float max, uint32_t color, float line_width, int time, qboolean occluded)
{
    if (draw_selection.time < 100) // Give the gamelib some time to renew the draw request
    {
        draw_selection.inuse = false; // Unlock
    }

    if (draw_selection.inuse == false)
    {
        draw_selection.inuse = true;
        draw_selection.time = time;
        draw_selection.color = color;
        draw_selection.line_width = line_width;
        VectorCopy(start, draw_selection.start);
        VectorCopy(end, draw_selection.end);
        draw_selection.min = min;
        draw_selection.max = max;
        draw_selection.occluded = occluded;
    }
}
static void GL_DrawSelection(void)
{
    if (draw_selection.time)
    {
        //Com_Printf("%s start [%f %f %f] -- end [%f %f %f]\n", __func__, draw_selection.start[0], draw_selection.start[1], draw_selection.start[2], draw_selection.end[0], draw_selection.end[1], draw_selection.end[2]);
        draw_selection.time--;
        GL_DrawSelectionSquare(draw_selection.start, draw_selection.end, draw_selection.min, draw_selection.max, draw_selection.color, draw_selection.line_width, draw_selection.occluded);
    }
}

static void GL_InitDebugDraw(void)
{
    // Malloc nav
    if (sv.cm.draw == NULL)
    {
        sv.cm.draw = (debug_draw_t*)Z_TagMallocz(sizeof(debug_draw_t), TAG_CMODEL); // This malloc is automatically freed in CM_FreeMap() upon every map change
        if (sv.cm.draw == NULL)
        {
            Com_Error(ERR_DROP, "%s: malloc failed", __func__);
            return;
        }

                                                   // Nothing to memset
        sv.cm.draw->DrawSelection = DrawSelection; // Init the draw selection function

        memset(draw_strings, 0, sizeof(draw_strings)); // Clear the draw strings array
        sv.cm.draw->DrawString = DrawString; // Init the draw string function

        memset(draw_crosses, 0, sizeof(draw_crosses)); // Clear the draw arrows array
        sv.cm.draw->DrawCross = DrawCross; // Init the draw arrow function

        memset(draw_boxes, 0, sizeof(draw_boxes)); // Clear the draw arrows array
        sv.cm.draw->DrawBox = DrawBox; // Init the draw arrow function

        memset(draw_arrows, 0, sizeof(draw_arrows)); // Clear the draw arrows array
        sv.cm.draw->DrawArrow = DrawArrow; // Init the draw arrow function
    }
}
#endif
//rekkie -- debug drawing -- e

static void make_flare_quad(const entity_t *e, float scale, vec3_t points[4])
{
    vec3_t up, down, left, right;

    scale *= e->scale;

    VectorScale(glr.viewaxis[1], scale, left);
    VectorScale(glr.viewaxis[1], -scale, right);
    VectorScale(glr.viewaxis[2], -scale, down);
    VectorScale(glr.viewaxis[2], scale, up);

    VectorAdd3(e->origin, down, left, points[0]);
    VectorAdd3(e->origin, up, left, points[1]);
    VectorAdd3(e->origin, down, right, points[2]);
    VectorAdd3(e->origin, up, right, points[3]);
}

static void GL_OccludeFlares(void)
{
    vec3_t points[4];
    entity_t *e;
    glquery_t *q;
    int i;

    if (!glr.num_flares)
        return;
    if (!gl_static.queries)
        return;

    GL_LoadMatrix(glr.viewmatrix);
    GL_StateBits(GLS_DEPTHMASK_FALSE);
    GL_ArrayBits(GLA_VERTEX);
    qglColorMask(0, 0, 0, 0);
    GL_BindTexture(0, TEXNUM_WHITE);
    GL_VertexPointer(3, 0, &points[0][0]);

    for (i = 0, e = glr.fd.entities; i < glr.fd.num_entities; i++, e++) {
        if (!(e->flags & RF_FLARE))
            continue;

        q = HashMap_Lookup(glquery_t, gl_static.queries, &e->skinnum);
        if (q && q->pending)
            continue;

        if (!q) {
            glquery_t new = { 0 };
            qglGenQueries(1, &new.query);
            HashMap_Insert(gl_static.queries, &e->skinnum, &new);
            q = HashMap_GetValue(glquery_t, gl_static.queries, HashMap_Size(gl_static.queries) - 1);
        }

        make_flare_quad(e, 2.5f, points);

        qglBeginQuery(gl_static.samples_passed, q->query);
        qglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        qglEndQuery(gl_static.samples_passed);

        q->pending = true;
    }

    qglColorMask(1, 1, 1, 1);
}

static void GL_DrawFlare(const entity_t *e)
{
    vec3_t points[4];
    GLuint result;
    glquery_t *q;

    if (!gl_static.queries)
        return;

    q = HashMap_Lookup(glquery_t, gl_static.queries, &e->skinnum);
    if (!q) {
        glr.num_flares++;
        return;
    }

    if (q->pending) {
        qglGetQueryObjectuiv(q->query, GL_QUERY_RESULT_AVAILABLE, &result);
        if (result) {
            qglGetQueryObjectuiv(q->query, GL_QUERY_RESULT, &result);
            q->visible = result;
            q->pending = false;
        }
    }

    if (!q->pending)
        glr.num_flares++;

    if (!q->visible)
        return;

    GL_LoadMatrix(glr.viewmatrix);
    GL_BindTexture(0, IMG_ForHandle(e->skin)->texnum);
    GL_StateBits(GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_FALSE | GLS_BLEND_ADD);
    GL_ArrayBits(GLA_VERTEX | GLA_TC);
    GL_Color(e->rgba.u8[0] / 255.0f,
             e->rgba.u8[1] / 255.0f,
             e->rgba.u8[2] / 255.0f,
             e->alpha * 0.5f);

    make_flare_quad(e, 25.0f, points);

    GL_TexCoordPointer(2, 0, quad_tc);
    GL_VertexPointer(3, 0, &points[0][0]);
    qglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void GL_DrawEntities(int musthave, int canthave)
{
    entity_t *ent, *last;
    model_t *model;

    if (!gl_drawentities->integer) {
        return;
    }

    last = glr.fd.entities + glr.fd.num_entities;
    for (ent = glr.fd.entities; ent != last; ent++) {
        if (ent->flags & RF_BEAM) {
            // beams are drawn elsewhere in single batch
            glr.num_beams++;
            continue;
        }
        if ((ent->flags & musthave) != musthave || (ent->flags & canthave)) {
            continue;
        }
        if (ent->flags & RF_FLARE) {
            GL_DrawFlare(ent);
            continue;
        }

        glr.ent = ent;

        // convert angles to axis
        GL_SetEntityAxis();

        // inline BSP model
        if (ent->model & BIT(31)) {
            bsp_t *bsp = gl_static.world.cache;
            int index = ~ent->model;

            if (glr.fd.rdflags & RDF_NOWORLDMODEL) {
                Com_Error(ERR_DROP, "%s: inline model without world",
                          __func__);
            }

            if (index < 1 || index >= bsp->nummodels) {
                Com_Error(ERR_DROP, "%s: inline model %d out of range",
                          __func__, index);
            }

            GL_DrawBspModel(&bsp->models[index]);
            continue;
        }

        model = MOD_ForHandle(ent->model);
        if (!model) {
            GL_DrawNullModel();
            continue;
        }

        switch (model->type) {
        case MOD_ALIAS:
            GL_DrawAliasModel(model);
            break;
        case MOD_SPRITE:
            GL_DrawSpriteModel(model);
            break;
        case MOD_EMPTY:
            break;
        default:
            Q_assert(!"bad model type");
        }

        if (gl_showorigins->integer) {
            GL_DrawNullModel();
        }
    }
}

static void GL_DrawTearing(void)
{
    static int i;

    // alternate colors to make tearing obvious
    i++;
    if (i & 1) {
        qglClearColor(1, 1, 1, 1);
    } else {
        qglClearColor(1, 0, 0, 0);
    }

    qglClear(GL_COLOR_BUFFER_BIT);
    qglClearColor(0, 0, 0, 1);
}

static const char *GL_ErrorString(GLenum err)
{
    const char *str;

    switch (err) {
#define E(x) case GL_##x: str = "GL_"#x; break;
        E(NO_ERROR)
        E(INVALID_ENUM)
        E(INVALID_VALUE)
        E(INVALID_OPERATION)
        E(STACK_OVERFLOW)
        E(STACK_UNDERFLOW)
        E(OUT_OF_MEMORY)
    default: str = "UNKNOWN ERROR";
#undef E
    }

    return str;
}

void GL_ClearErrors(void)
{
    GLenum err;

    while ((err = qglGetError()) != GL_NO_ERROR)
        ;
}

bool GL_ShowErrors(const char *func)
{
    GLenum err = qglGetError();

    if (err == GL_NO_ERROR) {
        return false;
    }

    do {
        if (gl_showerrors->integer) {
            Com_EPrintf("%s: %s\n", func, GL_ErrorString(err));
        }
    } while ((err = qglGetError()) != GL_NO_ERROR);

    return true;
}

static void GL_WaterWarp(void)
{
    GL_ForceTexture(0, gl_static.warp_texture);
    GL_StateBits(GLS_DEPTHTEST_DISABLE | GLS_DEPTHMASK_FALSE |
                 GLS_CULL_DISABLE | GLS_TEXTURE_REPLACE | GLS_WARP_ENABLE);
    GL_ArrayBits(GLA_VERTEX | GLA_TC);

    vec_t points[8] = {
        glr.fd.x,                glr.fd.y,
        glr.fd.x,                glr.fd.y + glr.fd.height,
        glr.fd.x + glr.fd.width, glr.fd.y,
        glr.fd.x + glr.fd.width, glr.fd.y + glr.fd.height,
    };

    GL_TexCoordPointer(2, 0, quad_tc);
    GL_VertexPointer(2, 0, points);
    qglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void R_RenderFrame(refdef_t *fd)
{
    GL_Flush2D();

    Q_assert(gl_static.world.cache || (fd->rdflags & RDF_NOWORLDMODEL));

    glr.drawframe++;
    glr.rand_seed = fd->time * 20;

    glr.fd = *fd;
    glr.num_beams = 0;
    glr.num_flares = 0;

    if (gl_dynamic->integer != 1 || gl_vertexlight->integer) {
        glr.fd.num_dlights = 0;
    }

    if (lm.dirty) {
        GL_RebuildLighting();
        lm.dirty = false;
    }

    bool waterwarp = (glr.fd.rdflags & RDF_UNDERWATER) && gl_static.use_shaders && gl_waterwarp->integer;

    if (waterwarp) {
        if (glr.fd.width != glr.framebuffer_width || glr.fd.height != glr.framebuffer_height) {
            glr.framebuffer_ok = GL_InitWarpTexture();
            glr.framebuffer_width = glr.fd.width;
            glr.framebuffer_height = glr.fd.height;
        }
        waterwarp = glr.framebuffer_ok;
    }

    if (waterwarp) {
        qglBindFramebuffer(GL_FRAMEBUFFER, gl_static.warp_framebuffer);
    }

    GL_Setup3D(waterwarp);

    if (gl_cull_nodes->integer) {
        GL_SetupFrustum();
    }

    if (!(glr.fd.rdflags & RDF_NOWORLDMODEL) && gl_drawworld->integer) {
        GL_DrawWorld();
    }

    //rekkie -- gl_showedges -- s
#if 0
    if (gl_showedges->integer) {
        GL_DrawEdges();
    }
#endif
    //rekkie -- gl_showedges -- e

    //rekkie -- debug drawing -- s
#if DEBUG_DRAWING
    //vec3_t start = { 0, 0, 0 };
    //vec3_t end = { 100, 100, 0 };
    //uint32_t color = MakeColor(0, 255, 0, 255); // Green
    //GL_DrawSelectionSquare(start, end, color, 1.0, false);
    GL_DrawSelection();
    //GL_DrawCrosses(); // Draw crosses
    GL_DrawArrows(); // Draw arrows
    GL_DrawBoxes(); // Draw crosses
    //GL_DrawStrings() is called in client/screen.c
#endif
    //rekkie -- debug drawing -- e

    GL_DrawEntities(0, RF_TRANSLUCENT);

    GL_DrawBeams();

    GL_DrawParticles();

    GL_DrawEntities(RF_TRANSLUCENT, RF_WEAPONMODEL);

    GL_OccludeFlares();

    if (!(glr.fd.rdflags & RDF_NOWORLDMODEL)) {
        GL_DrawAlphaFaces();
    }

    GL_DrawEntities(RF_TRANSLUCENT | RF_WEAPONMODEL, 0);

    if (waterwarp) {
        qglBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // go back into 2D mode
    GL_Setup2D();

    if (waterwarp) {
        GL_WaterWarp();
    }

    if (gl_polyblend->integer && glr.fd.blend[3] != 0) {
        GL_Blend();
    }

#if USE_DEBUG
    if (gl_lightmap->integer > 1) {
        Draw_Lightmaps();
    }
#endif

    GL_ShowErrors(__func__);
}

void R_BeginFrame(void)
{
    memset(&c, 0, sizeof(c));

    if (gl_finish->integer) {
        qglFinish();
    }

    GL_Setup2D();

    if (gl_clear->integer) {
        qglClear(GL_COLOR_BUFFER_BIT);
    }

    GL_ShowErrors(__func__);
}

void R_EndFrame(void)
{
#if USE_DEBUG
    if (gl_showstats->integer) {
        GL_Flush2D();
        Draw_Stats();
    }
    if (gl_showscrap->integer) {
        Draw_Scrap();
    }
#endif
    GL_Flush2D();

    if (gl_showtearing->integer) {
        GL_DrawTearing();
    }

    GL_ShowErrors(__func__);

    vid.swap_buffers();
}

// ==============================================================================

static void GL_Strings_f(void)
{
    GLint integer = 0;
    GLfloat value = 0;

    Com_Printf("GL_VENDOR: %s\n", qglGetString(GL_VENDOR));
    Com_Printf("GL_RENDERER: %s\n", qglGetString(GL_RENDERER));
    Com_Printf("GL_VERSION: %s\n", qglGetString(GL_VERSION));

    if (gl_config.ver_sl) {
        Com_Printf("GL_SHADING_LANGUAGE_VERSION: %s\n", qglGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    if (Cmd_Argc() > 1) {
        Com_Printf("GL_EXTENSIONS: ");
        if (qglGetStringi) {
            qglGetIntegerv(GL_NUM_EXTENSIONS, &integer);
            for (int i = 0; i < integer; i++)
                Com_Printf("%s ", qglGetStringi(GL_EXTENSIONS, i));
        } else {
            const char *s = (const char *)qglGetString(GL_EXTENSIONS);
            if (s) {
                while (*s) {
                    Com_Printf("%s", s);
                    s += min(strlen(s), MAXPRINTMSG - 1);
                }
            }
        }
        Com_Printf("\n");
    }

    qglGetIntegerv(GL_MAX_TEXTURE_SIZE, &integer);
    Com_Printf("GL_MAX_TEXTURE_SIZE: %d\n", integer);

    if (qglClientActiveTexture) {
        qglGetIntegerv(GL_MAX_TEXTURE_UNITS, &integer);
        Com_Printf("GL_MAX_TEXTURE_UNITS: %d\n", integer);
    }

    if (gl_config.caps & QGL_CAP_TEXTURE_ANISOTROPY) {
        qglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value);
        Com_Printf("GL_MAX_TEXTURE_MAX_ANISOTROPY: %.f\n", value);
    }

    Com_Printf("GL_PFD: color(%d-bit) Z(%d-bit) stencil(%d-bit)\n",
               gl_config.colorbits, gl_config.depthbits, gl_config.stencilbits);
}

static size_t GL_ViewCluster_m(char *buffer, size_t size)
{
    return Q_scnprintf(buffer, size, "%d", glr.viewcluster1);
}

static void gl_lightmap_changed(cvar_t *self)
{
    lm.scale = Cvar_ClampValue(gl_coloredlightmaps, 0, 1);
    lm.comp = !(gl_config.caps & QGL_CAP_TEXTURE_BITS) ? GL_RGBA : lm.scale ? GL_RGB : GL_LUMINANCE;
    lm.add = 255 * Cvar_ClampValue(gl_brightness, -1, 1);
    lm.modulate = Cvar_ClampValue(gl_modulate, 0, 1e6f);
    lm.modulate *= Cvar_ClampValue(gl_modulate_world, 0, 1e6f);
    if (gl_static.use_shaders && (self == gl_brightness || self == gl_modulate || self == gl_modulate_world) && !gl_vertexlight->integer)
        return;
    lm.dirty = true; // rebuild all lightmaps next frame
}

static void gl_modulate_entities_changed(cvar_t *self)
{
    gl_static.entity_modulate = Cvar_ClampValue(gl_modulate, 0, 1e6f);
    gl_static.entity_modulate *= Cvar_ClampValue(gl_modulate_entities, 0, 1e6f);
}

static void gl_modulate_changed(cvar_t *self)
{
    gl_lightmap_changed(self);
    gl_modulate_entities_changed(self);
}

// ugly hack to reset sky
static void gl_drawsky_changed(cvar_t *self)
{
    if (gl_static.world.cache)
        CL_SetSky();
}

static void gl_novis_changed(cvar_t *self)
{
    glr.viewcluster1 = glr.viewcluster2 = -2;
}

static void gl_swapinterval_changed(cvar_t *self)
{
    if (vid.swap_interval)
        vid.swap_interval(self->integer);
}

static void GL_Register(void)
{
    // regular variables
    gl_partscale = Cvar_Get("gl_partscale", "2", 0);
    gl_partstyle = Cvar_Get("gl_partstyle", "0", 0);
    gl_celshading = Cvar_Get("gl_celshading", "0", 0);
    gl_dotshading = Cvar_Get("gl_dotshading", "1", 0);
    gl_shadows = Cvar_Get("gl_shadows", "0", CVAR_ARCHIVE);
    gl_modulate = Cvar_Get("gl_modulate", "1", CVAR_ARCHIVE);
    gl_modulate->changed = gl_modulate_changed;
    gl_modulate_world = Cvar_Get("gl_modulate_world", "1", 0);
    gl_modulate_world->changed = gl_lightmap_changed;
    gl_coloredlightmaps = Cvar_Get("gl_coloredlightmaps", "1", 0);
    gl_coloredlightmaps->changed = gl_lightmap_changed;
    gl_brightness = Cvar_Get("gl_brightness", "0", 0);
    gl_brightness->changed = gl_lightmap_changed;
    gl_dynamic = Cvar_Get("gl_dynamic", "1", 0);
    gl_dynamic->changed = gl_lightmap_changed;
    gl_dlight_falloff = Cvar_Get("gl_dlight_falloff", "1", 0);
    gl_modulate_entities = Cvar_Get("gl_modulate_entities", "1", 0);
    gl_modulate_entities->changed = gl_modulate_entities_changed;
    gl_doublelight_entities = Cvar_Get("gl_doublelight_entities", "1", 0);
    gl_glowmap_intensity = Cvar_Get("gl_glowmap_intensity", "0.75", 0);
    gl_fontshadow = Cvar_Get("gl_fontshadow", "0", 0);
    gl_shaders = Cvar_Get("gl_shaders", (gl_config.caps & QGL_CAP_SHADER) ? "1" : "0", CVAR_REFRESH);
#if USE_MD5
    gl_md5_load = Cvar_Get("gl_md5_load", "1", CVAR_FILES);
    gl_md5_use = Cvar_Get("gl_md5_use", "1", 0);
#endif
    gl_waterwarp = Cvar_Get("gl_waterwarp", "0", 0);
    gl_swapinterval = Cvar_Get("gl_swapinterval", "1", CVAR_ARCHIVE);
    gl_swapinterval->changed = gl_swapinterval_changed;

    // development variables
    gl_znear = Cvar_Get("gl_znear", "2", CVAR_CHEAT);
    gl_drawworld = Cvar_Get("gl_drawworld", "1", CVAR_CHEAT);
    gl_drawentities = Cvar_Get("gl_drawentities", "1", CVAR_CHEAT);
    gl_drawsky = Cvar_Get("gl_drawsky", "1", 0);
    gl_drawsky->changed = gl_drawsky_changed;
    gl_showtris = Cvar_Get("gl_showtris", "0", CVAR_CHEAT);
    gl_showedges = Cvar_Get("gl_showedges", "0", CVAR_CHEAT); //rekkie -- gl_showedges
    gl_showorigins = Cvar_Get("gl_showorigins", "0", CVAR_CHEAT);
    gl_showtearing = Cvar_Get("gl_showtearing", "0", CVAR_CHEAT);
#if USE_DEBUG
    gl_showstats = Cvar_Get("gl_showstats", "0", 0);
    gl_showscrap = Cvar_Get("gl_showscrap", "0", 0);
    gl_nobind = Cvar_Get("gl_nobind", "0", CVAR_CHEAT);
    gl_test = Cvar_Get("gl_test", "0", 0);
#endif
    gl_cull_nodes = Cvar_Get("gl_cull_nodes", "1", 0);
    gl_cull_models = Cvar_Get("gl_cull_models", "1", 0);
    gl_clear = Cvar_Get("gl_clear", "0", 0);
    gl_finish = Cvar_Get("gl_finish", "0", 0);
    gl_novis = Cvar_Get("gl_novis", "0", 0);
    gl_novis->changed = gl_novis_changed;
    gl_lockpvs = Cvar_Get("gl_lockpvs", "0", CVAR_CHEAT);
    gl_lightmap = Cvar_Get("gl_lightmap", "0", CVAR_CHEAT);
    gl_fullbright = Cvar_Get("r_fullbright", "0", CVAR_CHEAT);
    gl_fullbright->changed = gl_lightmap_changed;
    gl_vertexlight = Cvar_Get("gl_vertexlight", "0", 0);
    gl_vertexlight->changed = gl_lightmap_changed;
    gl_lightgrid = Cvar_Get("gl_lightgrid", "1", 0);
    gl_polyblend = Cvar_Get("gl_polyblend", "1", 0);
    gl_showerrors = Cvar_Get("gl_showerrors", "1", 0);

    //rekkie -- Attach model to player -- s
    gl_vert_diff = Cvar_Get("gl_vert_diff", "0", 0);
    //rekkie -- Attach model to player -- e

    gl_lightmap_changed(NULL);
    gl_modulate_entities_changed(NULL);
    gl_swapinterval_changed(gl_swapinterval);

    Cmd_AddCommand("strings", GL_Strings_f);
    Cmd_AddMacro("gl_viewcluster", GL_ViewCluster_m);
}

static void GL_Unregister(void)
{
    Cmd_RemoveCommand("strings");
}

static void APIENTRY myDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity,
                                 GLsizei length, const GLchar *message, const void *userParam)
{
    int level = PRINT_DEVELOPER;

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:   level = PRINT_ERROR;   break;
    case GL_DEBUG_SEVERITY_MEDIUM: level = PRINT_WARNING; break;
    case GL_DEBUG_SEVERITY_LOW:    level = PRINT_ALL;     break;
    }

    Com_LPrintf(level, "%s\n", message);
}

static void GL_SetupConfig(void)
{
    GLint integer = 0;

    gl_config.colorbits = 0;
    qglGetIntegerv(GL_RED_BITS, &integer);
    gl_config.colorbits += integer;
    qglGetIntegerv(GL_GREEN_BITS, &integer);
    gl_config.colorbits += integer;
    qglGetIntegerv(GL_BLUE_BITS, &integer);
    gl_config.colorbits += integer;

    qglGetIntegerv(GL_DEPTH_BITS, &integer);
    gl_config.depthbits = integer;

    qglGetIntegerv(GL_STENCIL_BITS, &integer);
    gl_config.stencilbits = integer;

    if (qglDebugMessageCallback && qglIsEnabled(GL_DEBUG_OUTPUT)) {
        Com_Printf("Enabling GL debug output.\n");
        qglEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        qglDebugMessageCallback(myDebugProc, NULL);
    }
}

static void GL_InitTables(void)
{
    vec_t lat, lng;
    const vec_t *v;
    int i;

    for (i = 0; i < NUMVERTEXNORMALS; i++) {
        v = bytedirs[i];
        lat = acos(v[2]);
        lng = atan2(v[1], v[0]);
        gl_static.latlngtab[i][0] = (int)(lat * (float)(255 / (2 * M_PI))) & 255;
        gl_static.latlngtab[i][1] = (int)(lng * (float)(255 / (2 * M_PI))) & 255;
    }

    for (i = 0; i < 256; i++) {
        gl_static.sintab[i] = sin(i * (2 * M_PI / 255));
    }
}

static void GL_PostInit(void)
{
    registration_sequence = 1;

    GL_ClearState();
    GL_InitImages();
    MOD_Init();
}

static void GL_InitQueries(void)
{
    if (!qglBeginQuery)
        return;

    gl_static.samples_passed = GL_SAMPLES_PASSED;
    if (gl_config.ver_gl >= QGL_VER(3, 3) || gl_config.ver_es >= QGL_VER(3, 0))
        gl_static.samples_passed = GL_ANY_SAMPLES_PASSED;

    gl_static.queries = HashMap_Create(int, glquery_t, HashInt32, NULL);
}

static void GL_ShutdownQueries(void)
{
    if (!gl_static.queries)
        return;

    uint32_t map_size = HashMap_Size(gl_static.queries);
    for (int i = 0; i < map_size; i++) {
        glquery_t *q = HashMap_GetValue(glquery_t, gl_static.queries, i);
        qglDeleteQueries(1, &q->query);
    }

    HashMap_Destroy(gl_static.queries);
    gl_static.queries = NULL;
}

static void GL_ClearQueries(void)
{
    if (!gl_static.queries)
        return;

    uint32_t map_size = HashMap_Size(gl_static.queries);
    for (int i = 0; i < map_size; i++) {
        glquery_t *q = HashMap_GetValue(glquery_t, gl_static.queries, i);
        q->pending = q->visible = false;
    }
}

// ==============================================================================

/*
===============
R_Init
===============
*/
bool R_Init(bool total)
{
    Com_DPrintf("GL_Init( %i )\n", total);

    if (!total) {
        GL_PostInit();
        return true;
    }

    Com_Printf("------- R_Init -------\n");
    Com_Printf("Using video driver: %s\n", vid.name);

    // initialize OS-specific parts of OpenGL
    // create the window and set up the context
    if (!vid.init()) {
        return false;
    }

    // initialize our QGL dynamic bindings
    if (!QGL_Init()) {
        goto fail;
    }

    // get various limits from OpenGL
    GL_SetupConfig();

    // register our variables
    GL_Register();

    GL_InitState();

    GL_InitQueries();

    GL_InitTables();

    GL_PostInit();

    GL_ShowErrors(__func__);

    Com_Printf("----------------------\n");

    return true;

fail:
    memset(&gl_static, 0, sizeof(gl_static));
    memset(&gl_config, 0, sizeof(gl_config));
    QGL_Shutdown();
    vid.shutdown();
    return false;
}

/*
===============
R_Shutdown
===============
*/
void R_Shutdown(bool total)
{
    Com_DPrintf("GL_Shutdown( %i )\n", total);

    GL_FreeWorld();
    GL_ShutdownImages();
    MOD_Shutdown();

    if (!total) {
        return;
    }

    GL_ShutdownQueries();

    GL_ShutdownState();

    // shutdown our QGL subsystem
    QGL_Shutdown();

    // shut down OS specific OpenGL stuff like contexts, etc.
    vid.shutdown();

    GL_Unregister();

    memset(&gl_static, 0, sizeof(gl_static));
    memset(&gl_config, 0, sizeof(gl_config));
}

/*
===============
R_GetGLConfig
===============
*/
r_opengl_config_t *R_GetGLConfig(void)
{
    static r_opengl_config_t cfg;

    cfg.colorbits    = Cvar_ClampInteger(Cvar_Get("gl_colorbits",    "0", CVAR_REFRESH), 0, 32);
    cfg.depthbits    = Cvar_ClampInteger(Cvar_Get("gl_depthbits",    "0", CVAR_REFRESH), 0, 32);
    cfg.stencilbits  = Cvar_ClampInteger(Cvar_Get("gl_stencilbits",  "8", CVAR_REFRESH), 0,  8);
    cfg.multisamples = Cvar_ClampInteger(Cvar_Get("gl_multisamples", "0", CVAR_REFRESH), 0, 32);

    if (cfg.colorbits == 0)
        cfg.colorbits = 24;

    if (cfg.depthbits == 0)
        cfg.depthbits = cfg.colorbits > 16 ? 24 : 16;

    if (cfg.depthbits < 24)
        cfg.stencilbits = 0;

    if (cfg.multisamples < 2)
        cfg.multisamples = 0;

    cfg.debug = Cvar_Get("gl_debug", "0", CVAR_REFRESH)->integer;
    return &cfg;
}

/*
===============
R_BeginRegistration
===============
*/
void R_BeginRegistration(const char *name)
{
    char fullname[MAX_QPATH];

    gl_static.registering = true;
    registration_sequence++;

    memset(&glr, 0, sizeof(glr));
    glr.viewcluster1 = glr.viewcluster2 = -2;

    if (name) {
        Q_concat(fullname, sizeof(fullname), "maps/", name, ".bsp");
        GL_LoadWorld(fullname);
    }

    //rekkie -- surface data -- s
#if DEBUG_DRAWING
    GL_InitDebugDraw(); // Init debug drawing functions
#endif
    //rekkie -- surface data -- e
    
    GL_ClearQueries();
}

/*
===============
R_EndRegistration
===============
*/
void R_EndRegistration(void)
{
    IMG_FreeUnused();
    MOD_FreeUnused();
    Scrap_Upload();
    gl_static.registering = false;
}

/*
===============
R_ModeChanged
===============
*/
void R_ModeChanged(int width, int height, int flags)
{
    r_config.width = width;
    r_config.height = height;
    r_config.flags = flags;
}
