/* r_main.c */
#include "doomdef.h"
#include "r_local.h"

#include <dc/matrix.h>

/*===========================================================================*/

fixed_t viewx, viewy, viewz;
angle_t viewangle;
fixed_t viewcos, viewsin;
player_t *viewplayer;

/* increment every time a check is made */
int validcount; 

/* */
/* sky mapping */
/* */
boolean rendersky;


byte __attribute__((aligned(32))) solidcols[SOLIDCOLSC];
/* List of valid ranges to scan through */
subsector_t *solidsubsectors[MAXSUBSECTORS]; 
/* Pointer to the first free entry */
subsector_t **endsubsector; 
int numdrawsubsectors;


vissprite_t vissprites[MAXVISSPRITES];
vissprite_t *visspritehead;
int numdrawvissprites;

int globallump;
int globalcm;

Matrix R_ProjectionMatrix;
Matrix R_ModelMatrix;

/* */
/* precalculated math */
/* */
fixed_t *finecosine = &finesine[FINEANGLES / 4];

int infraredFactor;
int FlashEnvColor;
fixed_t quakeviewx;
fixed_t quakeviewy;
mobj_t *cameratarget;
angle_t camviewpitch;

fixed_t scrollfrac;
sector_t *frontsector;

/*===========================================================================*/

// used for EnvFlash effects
pvr_poly_cxt_t flash_cxt;
pvr_poly_hdr_t __attribute__((aligned(32))) flash_hdr;

Matrix R_ViewportMatrix;

/*
==============
=
= R_Init
=
==============
*/

void R_Init(void)
{
	R_InitData();

	guFrustumF(R_ProjectionMatrix, -8.0f, 8.0f, -6.0f, 6.0f, 8.0f, 3808.0f,
		   1.0f);

	guMtxIdentF(R_ModelMatrix);

	Viewport(R_ViewportMatrix, 0, 0, 640, 480);

	pvr_poly_cxt_col(&flash_cxt, PVR_LIST_TR_POLY);
	flash_cxt.blend.src = PVR_BLEND_ONE;
	flash_cxt.blend.dst = PVR_BLEND_ONE;
	pvr_poly_compile(&flash_hdr, &flash_cxt);
}

/*
==============
=
= R_RenderView
=
==============
*/
static Matrix RotX;
static Matrix RotY;
static Matrix Tran;

static pvr_vertex_t __attribute__((aligned(32))) flash_verts[4];

float pi_sub_viewangle;

void R_RenderPlayerView(void)
{
	fixed_t pitch;
	int fogfactor;
	float fogmin, fogmax, fogposition;

	viewplayer = &players[0];

	if (cameratarget == players[0].mo) {
		viewz = players[0].viewz;
		pitch = players[0].recoilpitch >> ANGLETOFINESHIFT;
	} else {
		viewz = cameratarget->z;
		pitch = camviewpitch >> ANGLETOFINESHIFT;
	}

	viewx = cameratarget->x;
	viewy = cameratarget->y;
	viewz += quakeviewy;

	viewangle = cameratarget->angle + quakeviewx;
    pi_sub_viewangle = pi_i754 - doomangletoQ(viewangle);
	viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];
	viewsin = finesine[viewangle >> ANGLETOFINESHIFT];

	// Phase 1
	R_BSP();

	// Phase 2
	if (rendersky) {
		R_RenderSKY();
	}

//#define PVR_MIN_Z 0.0001f
//	pvr_set_zclip(PVR_MIN_Z);

	fogfactor = (1000 - FogNear);

	if (fogfactor <= 0)
		fogfactor = 1;

	fogposition = ((float)fogfactor / 1000.0f);

	if (fogposition <= 0.0f)
		fogposition = 0.00001f;

	fogmin = 5.0f / fogposition;
	fogmax = 30.0f / fogposition;
	pvr_fog_table_color(1.0f, (float)UNPACK_R(FogColor) / 255.0f,
			    (float)UNPACK_G(FogColor) / 255.0f,
			    (float)UNPACK_B(FogColor) / 255.0f);
	pvr_fog_table_linear(fogmin, fogmax);

	DoomRotateX(RotX, (float)finesine[pitch] * recip64k,
		    (float)finecosine[pitch] * recip64k);

	DoomRotateY(
		RotY, (float)finesine[viewangle >> ANGLETOFINESHIFT] * recip64k,
		(float)finecosine[viewangle >> ANGLETOFINESHIFT] * recip64k);

	DoomTranslate(Tran, -((float)viewx * recip64k),
		      -((float)viewz * recip64k), (float)viewy * recip64k);

	mat_load(&R_ViewportMatrix);
	mat_apply(&R_ProjectionMatrix);
	mat_apply(&RotX);
	mat_apply(&RotY);
	mat_apply(&Tran);

	// Phase 3
	R_RenderAll();

	if (cameratarget == viewplayer->mo) {
		R_RenderPSprites();
	}

	if ((uint32_t)(FlashEnvColor & 0xFFFFFF00)) {
		// draw a flat shaded untextured quad across the entire screen
		// with the color and half alpha
		// this is one of the more inaccurate things compared to N64
		uint32_t color = D64_PVR_REPACK_COLOR_ALPHA(FlashEnvColor, 127);

		pvr_vertex_t *vert = flash_verts;
		vert->flags = PVR_CMD_VERTEX;
		vert->x = 0.0f;
		vert->y = 480;
		vert->z = 5.0f;
		vert->argb = color;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 0.0f;
		vert->y = 0.0f;
		vert->z = 5.0f;
		vert->argb = color;
		vert++;

		vert->flags = PVR_CMD_VERTEX;
		vert->x = 640;
		vert->y = 480;
		vert->z = 5.0f;
		vert->argb = color;
		vert++;

		vert->flags = PVR_CMD_VERTEX_EOL;
		vert->x = 640;
		vert->y = 0.0f;
		vert->z = 5.0f;
		vert->argb = color;

		pvr_list_prim(PVR_LIST_TR_POLY, &flash_hdr,
			      sizeof(pvr_poly_hdr_t));
		pvr_list_prim(PVR_LIST_TR_POLY, &flash_verts, sizeof(flash_verts));
	}
}

/*===========================================================================*/

/*
===============================================================================
=
= R_PointOnSide
=
= Returns side 0 (front) or 1 (back)
===============================================================================
*/
int R_PointOnSide(int x, int y, node_t *node)
{
	fixed_t dx, dy;
	fixed_t left, right;

	if (!node->line.dx) {
		if (x <= node->line.x)
			return (node->line.dy > 0);
		return (node->line.dy < 0);
	}

	if (!node->line.dy) {
		if (y <= node->line.y)
			return (node->line.dx < 0);
		return (node->line.dx > 0);
	}

	dx = (x - node->line.x);
	dy = (y - node->line.y);

	left = (node->line.dy >> 16) * (dx >> 16);
	right = (dy >> 16) * (node->line.dx >> 16);

	if (right < left)
		return 0; /* front side */

	return 1; /* back side */
}

/*
==============
=
= R_PointInSubsector
=
==============
*/

struct subsector_s *R_PointInSubsector(fixed_t x, fixed_t y)
{
	node_t *node;
	int side, nodenum;

#if RANGECHECK
	if (__builtin_expect(!numnodes,0)) /* single subsector is a special case */ {
		return subsectors;
	}
#endif	

	nodenum = numnodes - 1;

	while (!(nodenum & NF_SUBSECTOR)) {
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}

	return &subsectors[nodenum & ~NF_SUBSECTOR];
}

/*
===============================================================================
=
= R_PointToAngle
=
===============================================================================
*/

static int SlopeDiv(unsigned num, unsigned den)
{
	unsigned ans;

	if (den < 512) {
		return SLOPERANGE;
	}

//	float fans = ((float)(num << 3) / (float)(den >> 8));
//	ans = (unsigned)fans;

	ans = (num << 3) / (den >> 8);

	return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
	int x;
	int y;

	x = x2 - x1;
	y = y2 - y1;

	if ((!x) && (!y)) {
		return 0;
	}

	if (x >= 0) { /* x >=0 */
		if (y >= 0) { /* y>= 0 */
			if (x > y)
				return tantoangle[SlopeDiv(y, x)]; /* octant 0 */
			else
				return ANG90 - 1 -
				       tantoangle[SlopeDiv(x, y)]; /* octant 1 */
		} else { /* y<0 */
			y = -y;
			if (x > y)
				return -tantoangle[SlopeDiv(y, x)]; /* octant 8 */
			else
				return ANG270 +
				       tantoangle[SlopeDiv(x, y)]; /* octant 7 */
		}
	} else { /* x<0 */
		x = -x;
		if (y >= 0) { /* y>= 0 */
			if (x > y)
				return ANG180 - 1 -
				       tantoangle[SlopeDiv(y, x)]; /* octant 3 */
			else
				return ANG90 +
				       tantoangle[SlopeDiv(x, y)]; /* octant 2 */
		} else { /* y<0 */
			y = -y;
			if (x > y)
				return ANG180 +
				       tantoangle[SlopeDiv(y, x)]; /* octant 4 */
			else
				return ANG270 - 1 -
				       tantoangle[SlopeDiv(x, y)]; /* octant 5 */
		}
	}
}
