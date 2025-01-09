/* am_main.c -- automap */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"
#include <dc/vector.h>

#define COLOR_RED 0xA40000FF
#define COLOR_GREEN 0x00C000FF
#define COLOR_BROWN 0x8A5C30ff
#define COLOR_YELLOW 0xCCCC00FF
#define COLOR_GREY 0x808080FF
#define COLOR_AQUA 0x3373B3FF

#define MAXSCALE (1500-256)
#define MINSCALE 200

fixed_t am_box[4];
int am_plycolor;
int am_plyblink;

#define LINEWIDTH 2.0f

extern pvr_dr_state_t dr_state;

extern boolean M_BoxIntersect(fixed_t a[static 4], fixed_t b[static 4]);

void AM_DrawSubsectors(player_t *player, fixed_t cx, fixed_t cy,
		       fixed_t bbox[static 4]);
void AM_DrawThings(fixed_t x, fixed_t y, angle_t angle, int color);
void AM_DrawLine(player_t *player, fixed_t bbox[static 4]);
void AM_DrawLineThings(fixed_t x, fixed_t y, angle_t angle, int color);

/*================================================================= */
/* Start up Automap */
/*================================================================= */
static pvr_poly_hdr_t __attribute__((aligned(32))) line_hdr;
static pvr_poly_cxt_t line_cxt;

static pvr_poly_hdr_t __attribute__((aligned(32))) thing_hdr;
static pvr_poly_cxt_t thing_cxt;

static pvr_vertex_t __attribute__((aligned(32))) thing_verts[3];
static pvr_vertex_t __attribute__((aligned(32))) line_verts[4];

int ever_started = 0;

void AM_Start(void) // 800004D8
{
	am_plycolor = 95;
	am_plyblink = 16;
}

/*
==================
=
= AM_Control
=
= Called by P_PlayerThink before any other player processing
=
= Button bits can be eaten by clearing them in ticbuttons[playernum]
==================
*/

#define MAXSENSITIVITY 10

void AM_Control(player_t *player)
{
	int buttons, oldbuttons;

	buttons_t *cbuttons;
	fixed_t block[8];
	angle_t angle;
	fixed_t fs, fc;
	fixed_t x, y, x1, y1, x2, y2;
	int scale, sensitivity;
	int i;

	if (gamepaused) {
		return;
	}

	cbuttons = BT_DATA[0];
	buttons = ticbuttons[0];
	oldbuttons = oldticbuttons[0];

	if (player->playerstate != PST_LIVE) {
		am_plycolor = 79;
		return;
	}

	if ((buttons & cbuttons->BT_MAP) && !(oldbuttons & cbuttons->BT_MAP)) {
		if (player->automapflags & AF_SUBSEC) {
			player->automapflags &= ~AF_SUBSEC;
			player->automapflags |= AF_LINES;
		} else if (player->automapflags & AF_LINES) {
			player->automapflags &= ~AF_LINES;
		} else {
			player->automapflags |= AF_SUBSEC;
		}

		player->automapx = player->mo->x;
		player->automapy = player->mo->y;
	}

	if (!(player->automapflags & (AF_LINES | AF_SUBSEC))) {
		return;
	}

	/* update player flash */
	am_plycolor = (unsigned int)(am_plycolor + am_plyblink);
	if (am_plycolor < 80 || (am_plycolor >= 255)) {
		am_plyblink = -am_plyblink;
	}

	if (!(buttons & cbuttons->BT_USE)) {
		player->automapflags &= ~AF_FOLLOW;
		return;
	}

	if (!(player->automapflags & AF_FOLLOW)) {
		player->automapflags |= AF_FOLLOW;
		player->automapx = player->mo->x;
		player->automapy = player->mo->y;

		M_ClearBox(am_box);

		block[2] = block[4] = (bmapwidth << 23) + bmaporgx;
		block[1] = block[3] = (bmapheight << 23) + bmaporgy;
		block[0] = block[6] = bmaporgx;
		block[5] = block[7] = bmaporgy;

		angle = (ANG90 - player->mo->angle) >> ANGLETOFINESHIFT;

		fs = finesine[angle];
		fc = finecosine[angle];

		for (i = 0; i < 8; i += 2) {
			x = (block[i] - player->automapx) >> FRACBITS;
			y = (block[i + 1] - player->automapy) >> FRACBITS;

			x1 = (x * fc);
			y1 = (y * fs);
			x2 = (x * fs);
			y2 = (y * fc);

			x = (x1 - y1) + player->automapx;
			y = (x2 + y2) + player->automapy;

			M_AddToBox(am_box, x, y);
		}
	}

	if (!(player->automapflags & AF_FOLLOW))
		return;

	scale = player->automapscale << 15;
	scale = (scale / 1500) << 8;

	/* Analyze analog stick movement (left / right) */
	sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;

	if (sensitivity >= MAXSENSITIVITY || sensitivity <= -MAXSENSITIVITY) {
		player->automapx += (sensitivity * scale) / 80;
	}

	/* Analyze analog stick movement (up / down) */
	sensitivity = (int)((buttons) << 24) >> 24;

	if (sensitivity >= MAXSENSITIVITY || sensitivity <= -MAXSENSITIVITY) {
		player->automapy += (sensitivity * scale) / 80;
	}

	/* X movement */
	if (player->automapx > am_box[BOXRIGHT]) {
		player->automapx = am_box[BOXRIGHT];
	} else if (player->automapx < am_box[BOXLEFT]) {
		player->automapx = am_box[BOXLEFT];
	}

	/* Y movement */
	if (player->automapy > am_box[BOXTOP]) {
		player->automapy = am_box[BOXTOP];
	} else if (player->automapy < am_box[BOXBOTTOM]) {
		player->automapy = am_box[BOXBOTTOM];
	}

	/* Zoom scale in */
	if (buttons & PAD_L_TRIG) {
		player->automapscale -= 32;

		if (player->automapscale < MINSCALE) {
			player->automapscale = MINSCALE;
		}
	}

	/* Zoom scale out */
	if (buttons & PAD_R_TRIG) {
		player->automapscale += 32;

		if (player->automapscale > MAXSCALE)
			player->automapscale = MAXSCALE;
	}

	ticbuttons[0] &= ~(cbuttons->BT_LEFT | cbuttons->BT_RIGHT |
			   cbuttons->BT_FORWARD | cbuttons->BT_BACK |
			   PAD_L_TRIG | PAD_R_TRIG | 0xffff);
}

/*
==================
=
= AM_Drawer
=
= Draws the current frame to workingscreen
==================
*/
extern Matrix R_ViewportMatrix;
extern Matrix R_ProjectionMatrix;

static Matrix MapRotX;
static Matrix MapRotY;
static Matrix MapTrans;

float empty_table[129] = { 0 };

void AM_Drawer(void)
{
	player_t *p;
	mobj_t *mo;
	mobj_t *next;
	fixed_t xpos, ypos;
	fixed_t ox, oy;
	fixed_t c;
	fixed_t s;
	angle_t angle;
	int color;
	int scale;
	int artflag;
	char map_name[48];
	char killcount[20]; // [Immorpher] Automap kill count
	char itemcount[20]; // [Immorpher] Automap item count
	char secretcount[20]; // [Immorpher] Automap secret count
	fixed_t screen_box[4];
	fixed_t boxscale;

	if (!ever_started) {
		pvr_poly_cxt_col(&thing_cxt, PVR_LIST_OP_POLY);
		pvr_poly_compile(&thing_hdr, &thing_cxt);

		for (int vn = 0; vn < 3; vn++) {
			thing_verts[vn].flags = PVR_CMD_VERTEX;
		}
		thing_verts[2].flags = PVR_CMD_VERTEX_EOL;

		pvr_poly_cxt_col(&line_cxt, PVR_LIST_OP_POLY);
		pvr_poly_compile(&line_hdr, &line_cxt);

		for (int vn = 0; vn < 4; vn++) {
			line_verts[vn].flags = PVR_CMD_VERTEX;
		}
		line_verts[3].flags = PVR_CMD_VERTEX_EOL;

		ever_started = 1;
	}

	pvr_set_bg_color(0, 0, 0);
	pvr_fog_table_color(0.0f, 0.0f, 0.0f, 0.0f);
	pvr_fog_table_custom(empty_table);

	p = &players[0];

	scale = (p->automapscale << 16);
	xpos = p->mo->x;
	ypos = p->mo->y;

	if (p->onground) {
		xpos += (quakeviewx >> 7);
		ypos += quakeviewy;
	}

	if (p->automapflags & AF_FOLLOW) {
		angle = (p->mo->angle + ANG270) >> ANGLETOFINESHIFT;
		ox = (p->automapx - xpos) >> 16;
		oy = (p->automapy - ypos) >> 16;
		xpos += ((ox * finecosine[angle]) - (oy * finesine[angle]));
		ypos += ((ox * finesine[angle]) + (oy * finecosine[angle]));
	}

	angle = p->mo->angle >> ANGLETOFINESHIFT;

	s = finesine[angle];
	c = finecosine[angle];

	DoomRotateX(MapRotX, -1.0, 0.0); // -pi/2 rad
	DoomRotateY(MapRotY, (float)s * recip64k, (float)c * recip64k);
	DoomTranslate(MapTrans, -((float)xpos * recip64k),
		      -((float)scale * recip64k), (float)ypos * recip64k);

	mat_load(&R_ViewportMatrix);
	mat_apply(&R_ProjectionMatrix);
	mat_apply(&MapRotX);
	mat_apply(&MapRotY);
	mat_apply(&MapTrans);

	boxscale = scale / 160;

	// bbox check for sectors to reduce automap draw
	{
		fixed_t ts, tc;
		fixed_t cx, cy, tx, x, y;
		angle_t thingangle;

		thingangle = (ANG90 - p->mo->angle) >> ANGLETOFINESHIFT;
		ts = finesine[thingangle];
		tc = finecosine[thingangle];

		cx = FixedMul(320 << (FRACBITS - 1), boxscale);
		cy = FixedMul(240 << (FRACBITS - 1), boxscale);

		M_ClearBox(screen_box);

		for (int i = 0; i < 2; i++) {
			tx = i ? -cx : cx;
			x = ((s64)tx * (s64)tc + (s64)cy * (s64)ts) >> FRACBITS;
			y = ((s64)-tx * (s64)ts + (s64)cy * (s64)tc) >>
			    FRACBITS;
			M_AddToBox(screen_box, x, y);
			M_AddToBox(screen_box, -x, -y);
		}

		screen_box[BOXTOP] += ypos;
		screen_box[BOXBOTTOM] += ypos;
		screen_box[BOXLEFT] += xpos;
		screen_box[BOXRIGHT] += xpos;
	}

	if (p->automapflags & AF_LINES) {
		// lines are all the same, submit header once
		sq_fast_cpy(SQ_MASK_DEST(PVR_TA_INPUT), &line_hdr, 1);
		AM_DrawLine(p, screen_box);
	} else {
		AM_DrawSubsectors(p, xpos, ypos, screen_box);
	}

	/* SHOW ALL MAP THINGS (CHEAT) */
	if (p->cheats & CF_ALLMAP) {
		for (mo = mobjhead.next; mo != &mobjhead; mo = next) {
			fixed_t bbox[4];
			next = mo->next;

			if (mo == p->mo)
				continue; /* Ignore player */

			if (mo->flags & (MF_NOSECTOR | MF_RENDERLASER))
				continue;

			if (mo->flags & (MF_SHOOTABLE | MF_MISSILE))
				color = COLOR_RED;
			else
				color = COLOR_AQUA;

			bbox[BOXTOP] = mo->y + 0x2d413c; // sqrt(2) * 32;
			bbox[BOXBOTTOM] = mo->y - 0x2d413c;
			bbox[BOXRIGHT] = mo->x + 0x2d413c;
			bbox[BOXLEFT] = mo->x - 0x2d413c;

			if (!M_BoxIntersect(bbox, screen_box))
				continue;

			if (p->automapflags & AF_LINES) {
				AM_DrawLineThings(mo->x, mo->y, mo->angle,
						  color);
			} else {
				AM_DrawThings(mo->x, mo->y, mo->angle, color);
			}
		}
	}

	if (p->automapflags & AF_LINES) {
		/* SHOW PLAYERS */
		AM_DrawLineThings(p->mo->x, p->mo->y, p->mo->angle,
				  am_plycolor << 16 | 0xff);
	} else {
		/* SHOW PLAYERS */
		AM_DrawThings(p->mo->x, p->mo->y, p->mo->angle,
			      am_plycolor << 16 | 0xff);
	}

	if (menu_settings.enable_messages) {
		if (p->messagetic <= 0) {
			sprintf(map_name, "LEVEL %d: %s", gamemap,
				MapInfo[gamemap].name);
			ST_Message(2 + menu_settings.HUDmargin, menu_settings.HUDmargin, map_name,
				   196 | 0xffffff00,0);
		} else {
			ST_Message(2 + menu_settings.HUDmargin, menu_settings.HUDmargin, p->message,
				   196 | p->messagecolor,0);
		}
	}

	// [Immorpher] kill count
	if (menu_settings.MapStats) {
		sprintf(killcount, "KILLS: %d/%d", players[0].killcount,
			totalkills);
		ST_Message(2 + menu_settings.HUDmargin, 212 - menu_settings.HUDmargin, killcount,
			   196 | 0xffffff00,0);
		sprintf(itemcount, "ITEMS: %d/%d", players[0].itemcount,
			totalitems);
		ST_Message(2 + menu_settings.HUDmargin, 222 - menu_settings.HUDmargin, itemcount,
			   196 | 0xffffff00,0);
		sprintf(secretcount, "SECRETS: %d/%d", players[0].secretcount,
			totalsecret);
		ST_Message(2 + menu_settings.HUDmargin, 232 - menu_settings.HUDmargin, secretcount,
			   196 | 0xffffff00,0);
	}

	xpos = 297 - menu_settings.HUDmargin;
	artflag = 4;
	do {
		if ((players->artifacts & artflag) != 0) {
			if (artflag == 4) {
				BufferedDrawSprite(MT_ITEM_ARTIFACT3,
						   &states[S_559], 0,
						   0xffffff80, xpos,
						   266 - menu_settings.HUDmargin);
			} else if (artflag == 2) {
				BufferedDrawSprite(MT_ITEM_ARTIFACT2,
						   &states[S_551], 0,
						   0xffffff80, xpos,
						   266 - menu_settings.HUDmargin);
			} else if (artflag == 1) {
				BufferedDrawSprite(MT_ITEM_ARTIFACT1,
						   &states[S_543], 0,
						   0xffffff80, xpos,
						   266 - menu_settings.HUDmargin);
			}

			xpos -= 40;
		}
		artflag >>= 1;
	} while (artflag != 0);
}

void R_RenderPlane(leaf_t *leaf, int numverts, int zpos, int texture, int xpos,
		   int ypos, int color, int ceiling, int lightlevel, int alpha);

static boolean AM_DrawSubsector(player_t *player, int bspnum)
{
	subsector_t *sub;
	sector_t *sec;

	if (!(bspnum & NF_SUBSECTOR)) {
		return false;
	}

	sub = &subsectors[bspnum & (~NF_SUBSECTOR)];

	if (!sub->drawindex && !player->powers[pw_allmap] &&
	    !(player->cheats & CF_ALLMAP)) {
		return true;
	}

	sec = sub->sector;

	if ((sec->flags & MS_HIDESSECTOR) || (sec->floorpic == -1)) {
		return true;
	}

	global_render_state.dont_color = 1;
	global_render_state.dont_bump = 1;
	R_RenderPlane(&leafs[sub->leaf], sub->numverts, 0,
		      textures[sec->floorpic], 0, 0,
		      lights[sec->colors[1]].rgba, 0, 0,
		      255); // no dynamic light
	global_render_state.dont_bump = 0;
	global_render_state.dont_color = 0;
	return true;
}
/*
==================
=
= AM_DrawSubsectors
=
==================
*/
// Nova took advantage of the GBA Doom stack rendering to improve the automap rendering speed
#define MAX_BSP_DEPTH 128

void AM_DrawSubsectors(player_t *player, fixed_t cx, fixed_t cy,
		       fixed_t bbox[static 4])
{
	int sp = 0;
	node_t *bsp;
	int side;
	fixed_t dx, dy;
	fixed_t left, right;
	int bspnum = numnodes - 1;
	int bspstack[MAX_BSP_DEPTH];

	globallump = -1;

	while (true) {
		while (!AM_DrawSubsector(player, bspnum)) {
			if (sp == MAX_BSP_DEPTH) {
				break;
			}

			bsp = &nodes[bspnum];
			dx = (cx - bsp->line.x);
			dy = (cy - bsp->line.y);

			left = (bsp->line.dy >> 16) * (dx >> 16);
			right = (dy >> 16) * (bsp->line.dx >> 16);

			if (right < left) {
				side = 0; /* front side */
			} else {
				side = 1; /* back side */
			}

			bspstack[sp++] = bspnum;
			bspstack[sp++] = side;

			bspnum = bsp->children[side];
		}

		if (sp == 0) {
			//back at root node and not visible. All done!
			return;
		}

		//Back sides.
		side = bspstack[--sp];
		bspnum = bspstack[--sp];
		bsp = &nodes[bspnum];

		// Possibly divide back space.
		//Walk back up the tree until we find
		//a node that has a visible backspace.
		while (!M_BoxIntersect(bbox, bsp->bbox[side ^ 1])) {
			if (sp == 0) {
				//back at root node and not visible. All done!
				return;
			}

			//Back side next.
			side = bspstack[--sp];
			bspnum = bspstack[--sp];

			bsp = &nodes[bspnum];
		}

		bspnum = bsp->children[side ^ 1];
	}
}

/*
==================
=
= AM_DrawLine
=
==================
*/

void draw_pvr_line_hdr(vector_t *v1, vector_t *v2, int color) {
	if (ever_started) {
		sq_fast_cpy(SQ_MASK_DEST(PVR_TA_INPUT), &line_hdr, 1);
		draw_pvr_line(v1, v2, color);
	}
}


void draw_pvr_line(vector_t *v1, vector_t *v2, int color)
{
	vector_t *ov1;
	vector_t *ov2;
	pvr_vertex_t *vert;
	float hlw_invmag;
	float dx,dy;
	float nx,ny;

	if (v1->x <= v2->x) {
		ov1 = v1;
		ov2 = v2;
	} else {
		ov1 = v2;
		ov2 = v1;
	}

	// https://devcry.heiho.net/html/2017/20170820-opengl-line-drawing.html
	dx = ov2->x - ov1->x;
	dy = ov2->y - ov1->y;
	hlw_invmag = //frsqrt
	(1.0f / sqrtf((dx * dx) + (dy * dy))) * (LINEWIDTH * 0.5f);
	nx = -dy * hlw_invmag;
	ny = dx * hlw_invmag;

	vert = pvr_dr_target(dr_state);
	vert->flags = PVR_CMD_VERTEX;
	vert->x = ov1->x + nx;
	vert->y = ov1->y + ny;
	vert->z = ov1->z;
	vert->argb = color;
	pvr_dr_commit(vert);

	vert = pvr_dr_target(dr_state);
	vert->flags = PVR_CMD_VERTEX;
	vert->x = ov1->x - nx;
	vert->y = ov1->y - ny;
	vert->z = ov2->z;
	vert->argb = color;
	pvr_dr_commit(vert);

	vert = pvr_dr_target(dr_state);
	vert->flags = PVR_CMD_VERTEX;
	vert->x = ov2->x + nx;
	vert->y = ov2->y + ny;
	vert->z = ov1->z;
	vert->argb = color;
	pvr_dr_commit(vert);

	vert = pvr_dr_target(dr_state);
	vert->flags = PVR_CMD_VERTEX_EOL;
	vert->x = ov2->x - nx;
	vert->y = ov2->y - ny;
	vert->z = ov2->z;
	vert->argb = color;
	pvr_dr_commit(vert);
}

void AM_DrawLineThings(fixed_t x, fixed_t y, angle_t angle, int color)
{
	vector_t v1,v2,v3;
	angle_t ang;

	int repacked_color = D64_PVR_REPACK_COLOR(color);

	float thing_height = 0.0f;

	if ((am_plycolor << 16 | 0xff) == color) {
		thing_height = 5.3f;
	} else if (COLOR_RED == color) {
		thing_height = 5.1f;
	} else if (COLOR_AQUA) {
		thing_height = 4.9f;
	}

	ang = angle >> ANGLETOFINESHIFT;

	v1.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	v1.y = 0.0f;
	v1.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);
	transform_vector(&v1);
	perspdiv_vector(&v1);
	v1.z += thing_height;

	ang = (angle + 0xA0000000) >> ANGLETOFINESHIFT;

	v2.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	v2.y = 0.0f;
	v2.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);
	transform_vector(&v2);
	perspdiv_vector(&v2);
	v2.z += thing_height;

	ang = (angle + 0x60000000) >> ANGLETOFINESHIFT;

	v3.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	v3.y = 0.0f;
	v3.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);
	transform_vector(&v3);
	perspdiv_vector(&v3);
	v3.z += thing_height;

	draw_pvr_line(&v1, &v2,	repacked_color);
	draw_pvr_line(&v2, &v3,	repacked_color);
	draw_pvr_line(&v3, &v1,	repacked_color);
}

void AM_DrawLine(player_t *player, fixed_t bbox[static 4])
{
	vector_t v1, v2;
	line_t *l;
	int i, color;

	l = lines;
	for (i = 0; i < numlines; i++, l++) {
		if (l->flags & ML_DONTDRAW) {
			continue;
		}

		if (!M_BoxIntersect(bbox, l->bbox)) {
			continue;
		}

		if (((l->flags & ML_MAPPED) || player->powers[pw_allmap]) ||
		    (player->cheats & CF_ALLMAP)) {
			/* Figure out color */
			color = COLOR_BROWN;

			if ((player->powers[pw_allmap] ||
			     (player->cheats & CF_ALLMAP)) &&
			    !(l->flags & ML_MAPPED)) {
				color = COLOR_GREY;
			} else if (l->flags & ML_SECRET) {
				color = COLOR_RED;
			} else if (l->special &&
				   !(l->flags & ML_HIDEAUTOMAPTRIGGER)) {
				color = COLOR_YELLOW;
			} else if (!(l->flags &
				     ML_TWOSIDED)) { /* ONE-SIDED LINE */
				color = COLOR_RED;
			}

			v1.x = (float)(l->v1->x >> 16);
			v1.y = 0;
			v1.z = -((float)(l->v1->y >> 16));
			transform_vector(&v1);
			perspdiv_vector(&v1);

			v2.x = (float)(l->v2->x >> 16);
			v2.y = 0;
			v2.z = -((float)(l->v2->y >> 16));
			transform_vector(&v2);
			perspdiv_vector(&v2);

			draw_pvr_line(&v1, &v2,
				      D64_PVR_REPACK_COLOR(color));
		}
	}
}

/*
==================
=
= AM_DrawThings
=
==================
*/

void AM_DrawThings(fixed_t x, fixed_t y, angle_t angle, int color)
{
	vector_t v1,v2,v3;
	pvr_vertex_t *vert = thing_verts;
	float thing_height = 0.0f;
	int repacked_color = D64_PVR_REPACK_COLOR(color);
	angle_t ang;

	if ((am_plycolor << 16 | 0xff) == color) {
		thing_height = 5.3f;
	} else if (COLOR_RED == color) {
		thing_height = 5.1f;
	} else if (COLOR_AQUA) {
		thing_height = 4.9f;
	}

	for (int i = 0; i < 3; i++) {
		thing_verts[i].argb = repacked_color;
	}

	ang = angle >> ANGLETOFINESHIFT;

	v1.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	v1.y = 0.0f;
	v1.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);
	transform_vector(&v1);
	perspdiv_vector(&v1);
	vert->x = v1.x;
	vert->y = v1.y;
	vert->z = v1.z + thing_height;
	vert++;

	ang = (angle + 0xA0000000) >> ANGLETOFINESHIFT;

	v2.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	v2.y = 0.0f;
	v2.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);
	transform_vector(&v2);
	perspdiv_vector(&v2);
	vert->x = v2.x;
	vert->y = v2.y;
	vert->z = v2.z + thing_height;
	vert++;

	ang = (angle + 0x60000000) >> ANGLETOFINESHIFT;

	v3.x = (float)(((finecosine[ang] << 5) + x) >> FRACBITS);
	v3.y = 0.0f;
	v3.z = (float)(-((finesine[ang] << 5) + y) >> FRACBITS);
	transform_vector(&v3);
	perspdiv_vector(&v3);
	vert->x = v3.x;
	vert->y = v3.y;
	vert->z = v3.z + thing_height;

	sq_fast_cpy(SQ_MASK_DEST(PVR_TA_INPUT), &thing_hdr, 1);
	sq_fast_cpy(SQ_MASK_DEST(PVR_TA_INPUT), thing_verts, 3);
}
