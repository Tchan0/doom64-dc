/* DoomDef.h */

#include <kos.h>
#include <stdint.h>
#include <strings.h>
#include <math.h>

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t

#define s64 int64_t
#define s32 int32_t
#define s16 int16_t
#define s8 int8_t

typedef int fixed_t;

#include "i_main.h"

typedef struct doom64_settings_s {
	int HUDopacity;
	int SfxVolume;
	int MusVolume;
	int brightness;
	int enable_messages;
	int M_SENSITIVITY;
	int MotionBob;
	int Rumble;
	int VideoFilter;
	int Autorun;
	int runintroduction;
	int StoryText;
	int MapStats;
	int HUDmargin;
	int ColoredHUD;
	int Quality;
	int FpsUncap;
} doom64_settings_t;

extern doom64_settings_t __attribute__((aligned(32))) menu_settings;

extern void M_ResetSettings(doom64_settings_t *s);
extern int I_ReadPakSettings(void);
extern int I_SavePakSettings(void);
#define halfover1024 0.00048828125f
#define recip16 0.0625f
#define recip60 0.01666666753590106964111328125f
#define recip64 0.015625f
#define recip128 0.0078125f
#define recip255 0.0039215688593685626983642578125f
#define recip256 0.00390625f
#define recip512 0.001953125f
#define recip1k 0.0009765625f
#define recip64k 0.0000152587890625f
#define recip64kx64 2.4220300099206349206349206349206e-7f

#define quarterpi_i754 0.785398185253143310546875f
#define halfpi_i754 1.57079637050628662109375f
#define pi_i754 3.1415927410125732421875f
#define twopi_i754 6.283185482025146484375f

#define D64_TARGB	PVR_TXRFMT_ARGB1555 | PVR_TXRFMT_TWIDDLED
#define D64_TPAL(n)	PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_8BPP_PAL((n)) | PVR_TXRFMT_TWIDDLED

#define NUM_DYNLIGHT 16

//https://stackoverflow.com/a/3693557
#define quickDistCheck(dx,dy,lr) (((dx) + (dy)) <= ((lr)<<1))

extern const char *fnpre;

typedef struct {
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	float radius;
	float distance;
} projectile_light_t;

#define STORAGE_PREFIX "/cd"
#define MAX_CACHED_SPRITES 256

#define FUNLEVEL(map)	(((map) == 25 || (map) == 26 || (map) == 27 || (map) == 33 || (map) == 40))

#define TR_VERTBUF_SIZE (1536 * 1024)
extern uint8_t __attribute__((aligned(32))) tr_buf[TR_VERTBUF_SIZE];

extern int context_change;

extern unsigned char lightcurve[256];
extern unsigned char lightmax[256];
#define get_color_argb1555(rrr, ggg, bbb, aaa)						\
	((uint16_t)(((aaa & 1) << 15) | (((rrr >> 3) & 0x1f) << 10) |	\
		    (((ggg >> 3) & 0x1f) << 5) | ((bbb >> 3) & 0x1f)))

// rrggbbaa
// color>>8 == xxrrggbb & 0x00ffffff == 00rrggbb
// | col

#define LOSTLEVEL 34
#define KNEEDEEP 41

#define UNPACK_R(color) ((color >> 24) & 0xff)
#define UNPACK_G(color) ((color >> 16) & 0xff)
#define UNPACK_B(color) ((color >> 8) & 0xff)
#define UNPACK_A(color) (color & 0xff)

#define D64_PVR_REPACK_COLOR(color)	\
	(((color >> 8) & 0x00ffffff) | (color << 24))
#define D64_PVR_REPACK_COLOR_ALPHA(color, a)	\
	(((color >> 8) & 0x00ffffff) | (a << 24))
#define D64_PVR_PACK_COLOR(a, r, g, b) ((a << 24) | (r << 16) | (g << 8) | b)

#define SCREEN_WD 320

#define RES_RATIO 2.0f

#define RECIP_FINEANGLES 0.0001220703125f

#define doomangletoQ(x)  ((float)x * 0.00000000023283064365386962890625f)
//((float)((x) >> ANGLETOFINESHIFT) * RECIP_FINEANGLES)
//((float)((x) >> ANGLETOFINESHIFT) / (float)FINEANGLES))

// next power of 2 greater than / equal to v
static inline uint32_t np2(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

#define PFS_ERR_NOPACK 1
#define PFS_ERR_ID_FATAL 2

extern s32 Pak_Memory;
extern u8 *Pak_Data;

short SwapShort(short dat);

typedef struct subsector_s subsector_t;

typedef struct {
	// 0
	uint32_t global_lit;
	// 4
	subsector_t *global_sub;
	// 8
	// 0 low 1 med 2 ultra
	uint8_t quality;
	// 9
	// 0 30 1 uncapped
	uint8_t fps_uncap;
	// 10
	uint8_t has_bump;
	// 11
	// 1 floor 2 ceiling 0 other
	uint8_t in_floor;
	// 12
	uint8_t in_things;
	// 13
	uint8_t context_change;
	// 14
	uint8_t floor_split_override;
	// 15
	uint8_t dont_bump;
	// 16
	uint8_t dont_color;
} render_state_t;
extern render_state_t __attribute__((aligned(32))) global_render_state;

typedef struct {
	pvr_vertex_t *v; // 0
	float w; // 4
	unsigned lit; // 8
	float pad1; // 12
	float r; // 16
	float g; // 20
	float b; // 24
	float pad2; // 28
} d64ListVert_t;

typedef struct {
	unsigned n_verts;
	pvr_poly_hdr_t *hdr;
	d64ListVert_t __attribute__((aligned(32))) dVerts[5];
} d64Poly_t;

void draw_pvr_line(vector_t *v1, vector_t *v2, int color);

#define transform_d64ListVert(d64v) mat_trans_single3_nodivw((d64v)->v->x, (d64v)->v->y, (d64v)->v->z, (d64v)->w)
// only works for positive x
#define frapprox_inverse(x) (1.0f / sqrtf((x)*(x)))

//frsqrt((x) * (x))

// legacy renderer functions, used by laser and wireframe automap

static inline void transform_vector(vector_t *d64v)
{
	/* no divide, for trivial rejection and near-z clipping */
	mat_trans_single3_nodivw(d64v->x, d64v->y, d64v->z, d64v->w);
}

static inline void perspdiv_vector(vector_t *v)
{
	float invw = frapprox_inverse(v->w);
	v->x *= invw;
	v->y *= invw;
	v->z = invw;
}

typedef union rumble_fields {
  uint32_t raw;
  struct {
    /* Special Effects and motor select. The normal purupuru packs will
only have one motor. Selecting MOTOR2 for these is probably not
a good idea. The PULSE setting here supposably creates a sharp
pulse effect, when ORed with the special field. */

    /** \brief  Yet another pulse effect.
        This supposedly creates a sharp pulse effect.
    */
    uint32_t special_pulse : 1;
    uint32_t : 3; // unused

    /** \brief  Select motor #1.

        Most jump packs only have one motor, but on things that do have more
       than one motor (like PS1->Dreamcast controller adapters that support
       rumble), this selects the first motor.
    */
    uint32_t special_motor1 : 1;
    uint32_t : 2; // unused

    /** \brief  Select motor #2.

        Most jump packs only have one motor, but on things that do have more
       than one motor (like PS1->Dreamcast controller adapters that support
       rumble), this selects the second motor.
    */
    uint32_t special_motor2 : 1;

    /** \brief  Ignore this command.

        Valid value 15 (0xF).

        Most jump packs will ignore commands with this set in effect1,
       apparently.
    */
    uint32_t fx1_powersave : 4;

    /** \brief  Upper nibble of effect1.

        This value works with the lower nibble of the effect2 field to
        increase the intensity of the rumble effect.
        Valid values are 0-7.

        \see    rumble_fields_t.fx2_lintensity
    */
    uint32_t fx1_intensity : 3;

    /** \brief  Give a pulse effect to the rumble.

        This probably should be used with rumble_fields_t.fx1_pulse as well.

        \see    rumble_fields_t.fx2_pulse
    */
    uint32_t fx1_pulse : 1;

    /** \brief  Lower-nibble of effect2.

        This value works with the upper nibble of the effect1
        field to increase the intensity of the rumble effect.
        Valid values are 0-7.

        \see    rumble_fields_t.fx1_intensity
    */
    uint32_t fx2_lintensity : 3;

    /** \brief  Give a pulse effect to the rumble.

        This probably should be used with rumble_fields_t.fx1_pulse as well.

        \see    rumble_fields_t.fx1_intensity
    */
    uint32_t fx2_pulse : 1;

    /** \brief  Upper-nibble of effect2.

        This apparently lowers the rumble's intensity somewhat.
        Valid values are 0-7.
    */
    uint32_t fx2_uintensity : 3;

    /* OR these in with your effect2 value if you feel so inclined.
       if you or the PULSE effect in here, you probably should also
       do so with the effect1 one below. */

    /** \brief  Give a decay effect to the rumble on some packs. */
    uint32_t fx2_decay : 1;

    /** \brief  The duration of the effect. No idea on units... */
    uint32_t duration : 8;
  };
} rumble_fields_t;

/*-----------*/
/* SYSTEM IO */
/*-----------*/

/*============================================================================= */

/* Fixes and Version Update Here*/

// NEWS Updates
// Nightmare Mode Originally Activated in the project [GEC] Master Edition.

// FIXES

// Fixes for the 'linedef deletion' bug. From PsyDoom
#define FIX_LINEDEFS_DELETION 1

/*============================================================================= */
#define backcheck o_d122b67458594bfc8c1b920e63847f5b
/* all external data is defined here */
#include "doomdata.h"

/* header generated by multigen utility */
#include "doominfo.h"

#define MAXCHAR ((char)0x7f)
#define MAXSHORT ((short)0x7fff)
#define MAXINT ((int)0x7fffffff) /* max pos 32-bit int */
#define MAXLONG ((long)0x7fffffff)

#define MINCHAR ((char)0x80)
#define MINSHORT ((short)0x8000)
#define MININT ((int)0x80000000) /* max negative 32-bit integer */
#define MINLONG ((long)0x80000000)

#if 0
#ifndef NULL
#define NULL 0
#endif
#endif

int D_vsprintf(char *string, const char *format, int *argptr);

void ST_Init(void);

/* c_convert.c  */
int LightGetHSV(int r, int g, int b);
int LightGetRGB(int h, int s, int v);

/* p* */
void P_RefreshBrightness(void);
void P_RefreshVideo(void);

typedef float __attribute__((aligned(32))) Matrix[4][4];

static inline void guMtxIdentF(Matrix mf)
{
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (i == j)
				mf[i][j] = 1.0;
			else
				mf[i][j] = 0.0;
		}
	}
}

static inline void guFrustumF(Matrix mf, float l, float r, float b, float t,
			      float n, float f, float scale)
{
	int i, j;
	guMtxIdentF(mf);
	mf[0][0] = 2 * n / (r - l);
	mf[1][1] = 2 * n / (t - b);
	mf[2][0] = (r + l) / (r - l);
	mf[2][1] = (t + b) / (t - b);
	mf[2][2] = -(f + n) / (f - n);
	mf[2][3] = -1;
	mf[3][2] = -2 * f * n / (f - n);
	mf[3][3] = 0;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			mf[i][j] *= scale;
		}
	}
}

static inline void Viewport(Matrix mf, int x, int y, int width, int height) {
    mf[0][0] = (float)width / 2.0f;
    mf[1][1] = -(float)height / 2.0f;
    mf[2][2] = 1.0f;
    mf[3][3] = 1.0f;

    mf[3][0] = (float)x + ((float)width / 2.0f);
    mf[3][1] = 480.0f - ((float)y + ((float)height / 2.0f));
}

static inline void DoomTranslate(Matrix mf, float x, float y, float z)
{
	guMtxIdentF(mf);
	mf[3][0] = x;
	mf[3][1] = y;
	mf[3][2] = z;
}

static inline void DoomRotateX(Matrix mf, float in_sin, float in_cos)
{
	guMtxIdentF(mf);
	mf[1][1] = in_cos;
	mf[1][2] = -in_sin;
	mf[2][1] = in_sin;
	mf[2][2] = in_cos;
}

static inline void DoomRotateY(Matrix mf, float in_sin, float in_cos)
{
	guMtxIdentF(mf);
	mf[0][0] = in_sin;
	mf[0][2] = -in_cos;
	mf[2][0] = in_cos;
	mf[2][2] = in_sin;
}

static inline void DoomRotateZ(Matrix mf, float in_sin, float in_cos)
{
	guMtxIdentF(mf);
	mf[0][0] = in_cos;
	mf[0][1] = -in_sin;
	mf[1][0] = in_sin;
	mf[1][1] = in_cos;
}

#define backres o_ad675382a0ccc360672c24686a0f93ee

/*
===============================================================================

						GLOBAL TYPES

===============================================================================
*/

#define MAXPLAYERS 1 /* D64 has one player */
#define TICRATE 30 /* number of tics / second */

#define FRACBITS 16
#define FRACUNIT (1 << FRACBITS)

#define ANG45 0x20000000
#define ANG90 0x40000000
#define ANG180 0x80000000
#define ANG270 0xc0000000
#define ANG5 0x38e0000 // (ANG90/18)
#define ANG1 0xb60000 // (ANG45/45)
typedef unsigned angle_t;

#define FINEANGLES 8192
#define FINEMASK (FINEANGLES - 1)
#define ANGLETOFINESHIFT 19 /* 0x100000000 to 0x2000 */

#define TRUEANGLES(x) ((float)x * 0.00000008381903171539306640625f)
//(((x) >> ANGLETOFINESHIFT) * 0.0439453125f)
// 								* 360.0f / FINEANGLES

int D_abs(int v);

extern fixed_t __attribute__((aligned(64))) finesine[5 * FINEANGLES / 4];
extern fixed_t *finecosine;

extern const angle_t tantoangle[2049];

extern char fnbuf[256];

typedef enum { sk_baby, sk_easy, sk_medium, sk_hard, sk_nightmare } skill_t;

typedef enum {
	ga_nothing,
	ga_died,
	ga_completed,
	ga_secretexit, // no used
	ga_warped,
	ga_exitdemo,
	//News
	//ga_recorddemo,// no used
	ga_timeout,
	ga_restart,
	ga_exit
} gameaction_t;

//#define LASTLEVEL 34
//#define TOTALMAPS 33

#define ABS_LASTLEVEL 34
#define ABS_TOTALMAPS 33

#define LOST_LASTLEVEL 41
#define LOST_TOTALMAPS 40

#define KNEE_LASTLEVEL 50
#define KNEE_TOTALMAPS 49


/* */
/* library replacements */
/* */
#include <string.h>
//void D_memset (void *dest, int val, int count);
//void D_memcpy (void *dest, void *src, int count);
//void D_strncpy (char *dest, char *src, int maxcount);
//int D_strncasecmp (char *s1, char *s2, int len);
#define D_memset memset
#define D_memcpy memcpy
#define D_strncpy strncpy
#define D_strncasecmp strncasecmp
//void D_memmove(void *dest, void *src);
#define D_memmove strcpy
void D_strupr(char *s);
//int D_strlen(char *s);
#define D_strlen strlen
/*
===============================================================================

							MAPOBJ DATA

===============================================================================
*/

struct mobj_s;

/* think_t is a function pointer to a routine to handle an actor */
typedef void (*think_t)();

/* a latecall is a function that needs to be called after p_base is done */
typedef void (*latecall_t)(struct mobj_s *mo);

typedef struct thinker_s {
	struct thinker_s *prev, *next;
	think_t function;
} thinker_t;

struct player_s;

typedef struct mobj_s {
	/* info for drawing */
	fixed_t x, y, z;

	struct subsector_s *subsector;

	int flags;
	struct player_s *player; /* only valid if type == MT_PLAYER */

	struct mobj_s *prev, *next;
	struct mobj_s *snext, *sprev; /* links in sector (if needed) */
	struct mobj_s *bnext, *bprev; /* links in blocks (if needed) */

	struct mobj_s *target; /* thing being chased/attacked (or NULL) */
	struct mobj_s *tracer; /* Thing being chased/attacked for tracers. */

	angle_t angle;
	int sprite; /* used to find patch_t and flip value */
	int frame; /* might be ord with FF_FULLBRIGHT */
	fixed_t floorz, ceilingz; /* closest together of contacted secs */
	fixed_t radius, height; /* for movement checking */
	fixed_t momx, momy, momz; /* momentums */

	mobjtype_t type;
	mobjinfo_t *info; /* &mobjinfo[mobj->type] */
	int tics; /* state tic counter	 */
	state_t *state;

	int health;
	int movedir; /* 0-7 */
	int movecount; /* when 0, select a new dir */

	/* also the originator for missiles */
	int reactiontime; /* if non 0, don't attack yet */
	/* used by player to freeze a bit after */
	/* teleporting */
	int threshold; /* if >0, the target will be chased */
	/* no matter what (even if shot) */

	int alpha; /* [D64] alpha value */

	void *extradata; /* for latecall functions */

	latecall_t latecall; /* set in p_base if more work needed */

	int tid; /* [D64] tid value */

	int sfx_chn;
} mobj_t;

/* each sector has a degenmobj_t in it's center for sound origin purposes */
struct subsector_s;
typedef struct {
	fixed_t x, y, z;
	struct subsector_s *subsec; // Psx Doom / Doom 64 New
} degenmobj_t;

typedef struct laserdata_s {
	fixed_t x1, y1, z1;
	fixed_t x2, y2, z2;
	fixed_t slopex, slopey, slopez;
	fixed_t distmax, dist;
	mobj_t *marker;
	struct laserdata_s *next;
} laserdata_t;

typedef struct laser_s {
	thinker_t thinker;
	laserdata_t *laserdata;
	mobj_t *marker;
} laser_t;

/* */
/* frame flags */
/* */
#define FF_FULLBRIGHT 0x8000 /* flag in thing->frame */
#define FF_FRAMEMASK 0x7fff

/* */
/* mobj flags */
/* */
#define MF_SPECIAL 1 /* call P_SpecialThing when touched */
#define MF_SOLID 2
#define MF_SHOOTABLE 4
#define MF_NOSECTOR 8 /* don't use the sector links */
/* (invisible but touchable)  */
#define MF_NOBLOCKMAP 16 /* don't use the blocklinks  */
/* (inert but displayable) */
#define MF_AMBUSH 32
#define MF_JUSTHIT 64 /* try to attack right back */
#define MF_JUSTATTACKED 128 /* take at least one step before attacking */
#define MF_SPAWNCEILING 256 /* hang from ceiling instead of floor */
//#define	MF_NOGRAVITY	512			/* don't apply gravity every tic */
#define MF_GRAVITY 512 /* apply gravity every tic */

/* movement flags */
#define MF_DROPOFF 0x400 /* allow jumps from high places */
#define MF_PICKUP 0x800 /* for players to pick up items */
#define MF_NOCLIP 0x1000 /* player cheat */
#define MF_TRIGDEATH 0x2000 /* [d64] trigger line special on death */
#define MF_FLOAT 0x4000 /* allow moves to any height, no gravity */
#define MF_TELEPORT 0x8000 /* don't cross lines or look at heights */
#define MF_MISSILE 0x10000 /* don't hit same species, explode on block */

#define MF_DROPPED 0x20000 /* dropped by a demon, not level spawned */
#define MF_TRIGTOUCH 0x40000 /* [d64] trigger line special on touch/pickup */
#define MF_NOBLOOD 0x80000 /* don't bleed when shot (use puff) */
#define MF_CORPSE 0x100000 /* don't stop moving halfway off a step */
#define MF_INFLOAT 0x200000 /* floating to a height for a move, don't */
/* auto float to target's height */
#define MF_COUNTKILL 0x400000 /* count towards intermission kill total */
#define MF_COUNTITEM 0x800000 /* count towards intermission item total */

#define MF_SKULLFLY 0x1000000 /* skull in flight */
#define MF_NOTDMATCH 0x2000000 /* don't spawn in death match (key cards) */

#define MF_SEETARGET 0x4000000 /* is target visible? */

/* Doom 64 New Flags */
#define MF_COUNTSECRET \
	0x8000000 /* [d64] Count as secret when picked up (for intermissions) */
#define MF_RENDERLASER 0x10000000 /* [d64] Exclusive to MT_LASERMARKER only */
#define MF_SHADOW 0x40000000 /* temporary player invisibility powerup. */
#define MF_NOINFIGHTING 0x80000000 /* [d64] Do not switch targets */

//(val << 0 < 0) 0x80000000
//(val << 1 < 0) 0x40000000
//(val << 2 < 0) 0x20000000
//(val << 3 < 0) 0x10000000
//(val << 4 < 0) 0x8000000
//(val << 5 < 0) 0x4000000
//(val << 6 < 0) 0x2000000
//(val << 7 < 0) 0x1000000
//(val << 8 < 0) 0x800000
//(val << 9 < 0) 0x400000
//(val << a < 0) 0x200000
//(val << b < 0) 0x100000
//(val << c < 0) 0x80000
//(val << d < 0) 0x40000
//(val << e < 0) 0x20000
//(val << f < 0) 0x10000

/* Exclusive Psx Doom Flags */
//#define	MF_BLENDMASK1	0x10000000
//#define	MF_BLENDMASK2	0x20000000
//#define	MF_BLENDMASK3	0x40000000
//#define	MF_ALL_BLEND_MASKS  (MF_BLENDMASK1 | MF_BLENDMASK2 | MF_BLENDMASK3)

/*============================================================================= */
typedef enum {
	PST_LIVE, /* playing */
	PST_DEAD, /* dead on the ground */
	PST_REBORN /* ready to restart */
} playerstate_t;

/* psprites are scaled shapes directly on the view screen */
/* coordinates are given for a 320*200 view screen */
typedef enum {
	ps_weapon,
	ps_flash,
	ps_flashalpha, // New Doom64
	NUMPSPRITES
} psprnum_t;

typedef struct {
	state_t *state; /* a NULL state means not active */
	int tics;
	int alpha;
	fixed_t sx, sy;
} pspdef_t;

typedef enum {
	it_bluecard,
	it_yellowcard,
	it_redcard,
	it_blueskull,
	it_yellowskull,
	it_redskull,
	NUMCARDS
} card_t;

typedef enum {
	wp_chainsaw,
	wp_fist,
	wp_pistol,
	wp_shotgun,
	wp_supershotgun, // [psx]
	wp_chaingun,
	wp_missile,
	wp_plasma,
	wp_bfg,
	wp_laser, // [d64]
	NUMWEAPONS,
	wp_nochange
} weapontype_t;

typedef enum {
	am_clip, /* pistol / chaingun */
	am_shell, /* shotgun */
	am_cell, /* BFG / plasma / #$&%*/
	am_misl, /* missile launcher */
	NUMAMMO,
	am_noammo /* chainsaw / fist */
} ammotype_t;

typedef enum {
	ART_FAST = 1,
	ART_DOUBLE = 2,
	ART_TRIPLE = 3,
} artifacts_t;

typedef struct {
	ammotype_t ammo;
	int upstate;
	int downstate;
	int readystate;
	int atkstate;
	int flashstate;
} weaponinfo_t;

extern weaponinfo_t weaponinfo[NUMWEAPONS]; // 8005AD80

typedef enum {
	pw_invulnerability,
	pw_strength,
	pw_invisibility,
	pw_ironfeet,
	pw_allmap,
	pw_infrared,
	NUMPOWERS
} powertype_t;

#define INVULNTICS (30 * 30)
#define INVISTICS (60 * 30)
#define INFRATICS (120 * 30)
#define IRONTICS (60 * 30)
#define STRTICS (3 * 30)

#define MSGTICS (5 * 30)

/*
================
=
= player_t
=
================
*/

typedef struct player_s {
	mobj_t *mo;
	playerstate_t playerstate;

	fixed_t forwardmove, sidemove; /* built from ticbuttons */
	/*angle_t*/int angleturn; /* built from ticbuttons */

	fixed_t viewz; /* focal origin above r.z */
	fixed_t viewheight; /* base height above floor for viewz */
	fixed_t deltaviewheight; /* squat speed */
	fixed_t bob; /* bounded/scaled total momentum */
	fixed_t recoilpitch; /* [D64] new*/

	int health; /* only used between levels, mo->health */
	/* is used during levels	 */
	int armorpoints, armortype; /* armor type is 0-2 */

	int powers[NUMPOWERS]; /* invinc and invis are tic counters	 */
	float f_powers[NUMPOWERS];
	boolean cards[NUMCARDS];
	int artifacts; /* [d64]*/
	boolean backpack;
	int frags; /* kills of other player */
	weapontype_t readyweapon;
	weapontype_t pendingweapon; /* wp_nochange if not changing */
	boolean weaponowned[NUMWEAPONS];
	int ammo[NUMAMMO];
	int maxammo[NUMAMMO];
	int attackdown, usedown; /* true if button down last tic */
	int cheats; /* bit flags */

	int refire; /* refired shots are less accurate */

	int killcount, itemcount, secretcount; /* for intermission */
	char *message; /* hint messages */
	char *message1; // [Immorpher] additional message levels
	char *message2; // [Immorpher] additional message levels
	char *message3; // [Immorpher] additional message levels
	int messagetic; /* messages tic countdown*/
	int messagetic1; // [Immorpher] message tic buffer
	int messagetic2; // [Immorpher] message tic buffer
	int messagetic3; // [Immorpher] message tic buffer
	unsigned int messagecolor; // [Immorpher] message color
	unsigned int messagecolor1; // [Immorpher] message color 1
	unsigned int messagecolor2; // [Immorpher] message color 2
	unsigned int messagecolor3; // [Immorpher] message color 3
	int damagecount, bonuscount; /* for screen flashing */
	float f_damagecount, f_bonuscount;
	int bfgcount; /* for bfg screen flashing */
	float f_bfgcount;
	mobj_t *attacker; /* who did damage (NULL for floors) */
	int extralight; /* so gun flashes light up areas */
	pspdef_t psprites[NUMPSPRITES]; /* view sprites (gun, etc) */

	void *lastsoundsector; /* don't flood noise every time */

	int automapx, automapy, automapscale, automapflags;

	int turnheld; /* for accelerative turning */
	float f_turnheld; /* for accelerative turning */
	int onground; /* [d64] */
} player_t;

#define CF_NOCLIP 1 // no use
#define CF_GODMODE 2
#define CF_ALLMAP 4
#define CF_LOCKMOSTERS 0x800
#define CF_WALLBLOCKING 0x1000
#define CF_WEAPONS 0x2000
#define CF_HEALTH 0x4000
#define CF_ALLKEYS 0x8000

#define CF_NOCOLORS 0x20000 // [GEC] NEW CHEAT CODE
#define CF_FULLBRIGHT 0x40000 // [GEC] NEW CHEAT CODE
#define CF_GAMMA 0x80000 // [Immorpher] NEW CHEAT CODE

#define AF_LINES 1 /* automap active on lines mode */
#define AF_SUBSEC 2 /* automap active on subsector mode */
#define AF_FOLLOW 4

/*
===============================================================================

					GLOBAL VARIABLES

===============================================================================
*/

/*================================== */

extern int gamevbls; // 80063130 /* may not really be vbls in multiplayer */
extern int gametic; // 80063134
extern int ticsinframe; // 80063138 /* how many tics since last drawer */
extern int ticon; // 8006313C
extern int lastticon; // 80063140
extern int vblsinframe[MAXPLAYERS]; // 80063144 /* range from 4 to 8 */
extern int ticbuttons[MAXPLAYERS]; // 80063148
extern int oldticbuttons[MAXPLAYERS]; // 8006314C

extern float f_ticon;
extern float f_lastticon;
extern float f_ticsinframe;
extern float f_gamevbls;
extern float f_gametic;
extern float f_vblsinframe[MAXPLAYERS];

extern boolean gamepaused;

extern int DrawerStatus;

//extern	int		maxlevel;			/* highest level selectable in menu (1-25) */

int MiniLoop(void (*start)(void), void (*stop)(), int (*ticker)(void),
	     void (*drawer)(void));

int G_Ticker(void);
void G_Drawer(void);
void G_RunGame(void);

/*================================== */

extern gameaction_t gameaction;

#define SBARHEIGHT 32 /* status bar height at bottom of screen */

typedef enum { gt_single, gt_coop, gt_deathmatch } gametype_t;

//extern	gametype_t	netgame;

//extern	boolean		playeringame[MAXPLAYERS];
//extern	int			consoleplayer;		/* player taking events and displaying */
//extern	int			displayplayer;
extern player_t players[MAXPLAYERS];

extern skill_t gameskill;
extern int gamemap;
extern int nextmap;
extern int totalkills, totalitems, totalsecret;
	/* for intermission */ //80077d4c,80077d58,80077E18

//extern	mapthing_t	deathmatchstarts[10], *deathmatch_p;    //80097e4c,80077e8c
extern mapthing_t playerstarts[MAXPLAYERS]; //800a8c60

/*
===============================================================================

					GLOBAL FUNCTIONS

===============================================================================
*/

fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);
fixed_t FixedDiv2(fixed_t a, fixed_t b);
fixed_t FixedDivFloat(fixed_t a, fixed_t b);


/*----------- */
/*MEMORY ZONE */
/*----------- */
/* tags < 8 are not overwritten until freed */
#define PU_STATIC 1 /* static entire execution time */
#define PU_LEVEL 2 /* static until level exited */
#define PU_LEVSPEC 4 /* a special thinker in a level */
/* tags >= 8 are purgable whenever needed */
#define PU_PURGELEVEL 8
#define PU_CACHE 16

#define ZONEID 0x1d4a

typedef struct memblock_s {
	int size; /* including the header and possibly tiny fragments */
	void **user; /* NULL if a free block */
	int tag; /* purgelevel */
	int id; /* should be ZONEID */
	int lockframe; /* don't purge on the same frame */
	struct memblock_s *next;
	struct memblock_s *prev;
	void *gfxcache; /* New on Doom64 */
} memblock_t;

typedef struct {
	int size; /* total bytes malloced, including header */
	memblock_t *rover;
	memblock_t *rover2; /* New on Doom64 */
	memblock_t *rover3; /* New on Doom64 */
	memblock_t blocklist; /* start / end cap for linked list */
} memzone_t;

extern memzone_t *mainzone;

void Z_Init(void);
memzone_t *Z_InitZone(byte *base, int size);

void Z_SetAllocBase(memzone_t *mainzone);
void *Z_Malloc2(memzone_t *mainzone, int size, int tag, void *ptr);
void *Z_Alloc2(memzone_t *mainzone, int size, int tag,
	       void *user); // PsxDoom / Doom64
void Z_Free2(memzone_t *mainzone, void *ptr);

#define Z_Malloc(x, y, z) Z_Malloc2(mainzone, x, y, z)
#define Z_Alloc(x, y, z) Z_Alloc2(mainzone, x, y, z)
#define Z_Free(x) Z_Free2(mainzone, x)

void Z_FreeTags(memzone_t *mainzone, int tag);
void Z_Touch(void *ptr);
void Z_CheckZone(memzone_t *mainzone);
void Z_ChangeTag(void *ptr, int tag);
int Z_FreeMemory(memzone_t *mainzone);
void Z_DumpHeap(memzone_t *mainzone);

/*------- */
/*WADFILE */
/*------- */

// New Doom64
typedef enum { dec_none, dec_jag, dec_d64 } decodetype;

typedef struct {
	int filepos; /* also texture_t * for comp lumps */
	int size;
	char name[8];
} lumpinfo_t;

typedef struct {
	void *cache;
} lumpcache_t;

void W_Init(void);

char *W_GetNameForNum(int lump);

int W_CheckNumForName(char *name, int hibit1, int hibit2);
int W_GetNumForName(char *name);

int W_LumpLength(int lump);
void W_ReadLump(int lump, void *dest, decodetype dectype);

void *W_CacheLumpNum(int lump, int tag, decodetype dectype);
void *W_CacheLumpName(char *name, int tag, decodetype dectype);

int W_S2_CheckNumForName(char *name);
int W_S2_GetNumForName(char *name);

int W_S2_LumpLength(int lump);
void W_S2_ReadLump(int lump, void *dest, decodetype dectype);

void *W_S2_CacheLumpNum(int lump, int tag, decodetype dectype);
void *W_S2_CacheLumpName(char *name, int tag, decodetype dectype);

#if 1
int W_Bump_CheckNumForName(char *name);
int W_Bump_GetNumForName(char *name);

int W_Bump_LumpLength(int lump);
void W_Bump_ReadLump(int lump, void *dest, int w, int h);
#endif

void W_OpenMapWad(int mapnum);
void W_FreeMapLump(void);
int W_MapLumpLength(int lump);
int W_MapGetNumForName(char *name);
void *W_GetMapLump(int lump);

/*---------*/
/* DECODES */
/*---------*/
void DecodeD64(unsigned char *input, unsigned char *output);
void DecodeJaguar(unsigned char *input, unsigned char *output);
void decode_bumpmap(uint8_t *in, uint8_t *out, int w, int h);

/*------------*/
/* BASE LEVEL */
/*------------*/

/*--------*/
/* D_MAIN */
/*--------*/

void D_DoomMain(void);

/*------*/
/* GAME */
/*------*/

extern boolean demoplayback, demorecording;
extern int *demo_p, *demobuffer;

void G_InitNew(skill_t skill, int map, gametype_t gametype);
void G_InitSkill(skill_t skill); // [Immorpher] skill initialize
void G_CompleteLevel(void);
void G_RecordDemo(void);
int G_PlayDemoPtr(int skill, int map);
void G_PlayerFinishLevel(int player);

/*------*/
/* PLAY */
/*------*/

mobj_t *P_SpawnMapThing(mapthing_t *mthing);
void P_SetupLevel(int map, skill_t skill);
void P_Init(void);

void P_Start(void);
void P_Stop(int exit);
int P_Ticker(void);
void P_Drawer(void);

/*---------*/
/* IN_MAIN */
/*---------*/

void IN_Start(void);
void IN_Stop(void);
int IN_Ticker(void);
void IN_Drawer(void);

/*--------*/
/* M_MAIN */
/*--------*/

typedef void (*menufunc_t)(void);

typedef struct {
	int casepos;
	int x;
	int y;
} menuitem_t;

typedef struct {
	menuitem_t *menu_item;
	int item_lines;
	menufunc_t menu_call;
	int cursor_pos;
} menudata_t;

extern menudata_t MenuData[8]; // 800A54F0
extern menuitem_t Menu_Game[5]; // 8005AAA4
extern int MenuAnimationTic; // 800a5570
extern int cursorpos; // 800A5574
//extern int m_vframe1; // 800A5578
extern float f_m_vframe1; // 800A5578
extern menuitem_t *MenuItem; // 800A5578
extern int itemlines; // 800A5580
extern menufunc_t MenuCall; // 800A5584

extern int linepos; // 800A5588
extern int text_alpha_change_value; // 800A558C
extern int MusicID; // 800A5590
extern int m_actualmap; // 800A5594
extern int last_ticon; // 800a5598

extern skill_t startskill; // 800A55A0
extern int startmap; // 800A55A4
extern int EnableExpPak; // 800A55A8

//-----------------------------------------

extern int MenuIdx; // 8005A7A4
extern int text_alpha; // 8005A7A8
extern int ConfgNumb; // 8005A7AC
extern int Display_X; // 8005A7B0
extern int Display_Y; // 8005A7B4
//extern boolean enable_messages; // 8005A7B8
//extern int HUDopacity; // [Immorpher] HUD 0pacity options
//extern int SfxVolume; // 8005A7C0
//extern int MusVolume; // 8005A7C4
//extern int brightness; // 8005A7C8
//extern int M_SENSITIVITY; // 8005A7CC
extern const boolean FeaturesUnlocked; // 8005A7D0
//extern int MotionBob; // [Immorpher] Motion Bob
//extern int VideoFilter; // [GEC & Immorpher] VideoFilter
extern int FlashBrightness; // [Immorpher] Strobe brightness adjustment
//extern boolean Autorun; // [Immorpher] Autorun
//extern boolean runintroduction; // [Immorpher] New introduction text
//extern boolean StoryText; // [Immorpher] Enable story text
//extern boolean MapStats; // [Immorpher] Enable automap statistics
//extern int HUDmargin; // [Immorpher] HUD margin options
//extern boolean ColoredHUD; // [Immorpher] Colored hud

int M_RunTitle(void); // 80007630

int M_ControllerPak(void); // 80007724
int M_ButtonResponder(int buttons); // 80007960

void M_AlphaInStart(void); // 800079E0
void M_AlphaOutStart(void); // 800079F8
int M_AlphaInOutTicker(void); // 80007A14
void M_FadeInStart(void); // 80007AB4
void M_FadeOutStart(int exitmode); // 80007AEC

void M_SaveMenuData(void); // 80007B2C
void M_RestoreMenuData(boolean alpha_in); // 80007BB8
void M_MenuGameDrawer(void); // 80007C48
int M_MenuTicker(void); // 80007E0C
void M_MenuClearCall(void); // 80008E6C

void M_MenuTitleDrawer(void); // 80008E7C
void M_FeaturesDrawer(void); // 800091C0
void M_VolumeDrawer(void); // 800095B4
void M_MovementDrawer(void); // 80009738
void M_VideoDrawer(void); // 80009884
void M_DisplayDrawer(void); // [Immorpher] new menu
void M_StatusHUDDrawer(void); // [Immorpher] new menu
void M_DefaultsDrawer(void); // [Immorpher] new menu
void M_CreditsDrawer(void); // [Immorpher] new menu

void M_DrawBackground(int x, int y, int color, char *name, float z,
		      int num); // 80009A68
void M_DrawOverlay(int x, int y, int w, int h, int color); // 80009F58

int M_ScreenTicker(void); // 8000A0F8

void M_ControllerPakDrawer(void); // 8000A3E4

void M_SavePakStart(void); // 8000A6E8
void M_SavePakStop(void); // 8000A7B4
int M_SavePakTicker(void); // 8000A804
void M_SavePakDrawer(void); // 8000AB44

void M_LoadPakStart(void); // 8000AEEC
void M_LoadPakStop(void); // 8000AF8C
int M_LoadPakTicker(void); // 8000AFE4
void M_LoadPakDrawer(void); // 8000B270

int M_CenterDisplayTicker(void); // 8000B4C4
void M_CenterDisplayDrawer(void); // 8000B604

int M_ControlPadTicker(void); // 8000B694
void M_ControlPadDrawer(void); // 8000B988

/*----------*/
/* PASSWORD */
/*----------*/

extern char *passwordChar; // 8005AC60
extern byte Passwordbuff[16]; // 800A55B0
extern boolean doPassword; // 8005ACB8
extern int CurPasswordSlot; // 8005ACBC

void M_EncodePassword(byte *buff); //8000BC10
int M_DecodePassword(byte *inbuff, int *levelnum, int *skill,
		     player_t *player); // 8000C194
void M_PasswordStart(void); // 8000C710
void M_PasswordStop(void); // 8000C744
int M_PasswordTicker(void); // 8000C774
void M_PasswordDrawer(void); // 8000CAF0

/*--------*/
/* F_MAIN */
/*--------*/

void F_StartIntermission(void);
void F_StopIntermission(void);
int F_TickerIntermission(void);
void F_DrawerIntermission(void);

void F_Start(void);
void F_Stop(void);
int F_Ticker(void);
void F_Drawer(void);

void BufferedDrawSprite(int type, state_t *state, int rotframe, int color,
			int xpos, int ypos);

/*---------*/
/* AM_MAIN */
/*---------*/

void AM_Start(void);
void AM_Control(player_t *player);
void AM_Drawer(void);

/*-----------*/
/* D_SCREENS */
/*-----------*/

int D_RunDemo(char *name, skill_t skill, int map); // 8002B2D0
int D_TitleMap(void); // 8002B358
int D_WarningTicker(void); // 8002B3E8
void D_DrawWarning(void); // 8002B430
int D_LegalTicker(void); // 8002B5F8
void D_DrawLegal(void); // 8002B644
int D_NoPakTicker(void); // 8002B7A0
void D_DrawNoPak(void); // 8002B7F4
void D_SplashScreen(void); // 8002B988
int D_Credits(void); // 8002BA34
int D_CreditTicker(void); // 8002BA88
void D_CreditDrawer(void); // 8002BBE4
void D_OpenControllerPak(void); // 8002BE28

/*--------*/
/* REFRESH */
/*--------*/

void R_RenderPlayerView(void);
void R_Init(void);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
struct subsector_s *R_PointInSubsector(fixed_t x, fixed_t y);

/*------*/
/* MISC */
/*------*/
typedef struct {
	char *name;
	int MusicSeq;
} mapinfo_t;

extern mapinfo_t MapInfo[];

extern const unsigned char rndtable[256];
int M_Random(void);
int P_Random(void);
int I_Random(void); // [Immorpher] new random table
void M_ClearRandom(void);
void M_ClearBox(fixed_t *box);
void M_AddToBox(fixed_t *box, fixed_t x, fixed_t y);

/*---------*/
/* S_SOUND */
/*---------*/

/* header generated by Dave's sound utility */
#include "sounds.h"

void init_all_sounds(void);

void S_Init(void);
void S_SetSoundVolume(int volume);
void S_SetMusicVolume(int volume);
void S_StartMusic(int mus_seq);
void S_StopMusic(void);
void S_PauseSound(void);
void S_ResumeSound(void);
void S_StopSound(mobj_t *origin, int seqnum);
void S_StopAll(void);

void S_RemoveOrigin(mobj_t *origin);
void S_ResetSound(void);
void S_UpdateSounds(void);

int S_SoundStatus(int seqnum);
int S_StartSound(mobj_t *origin, int sound_id);
int S_AdjustSoundParams(mobj_t *listener, mobj_t *origin, int *vol, int *pan);

/*--------*/
/* I_MAIN */
/*--------*/

extern u32 vid_side;

extern boolean disabledrawing;
extern volatile s32 vsync;
extern volatile s32 drawsync2;
extern volatile s32 drawsync1;
extern u32 NextFrameIdx;
extern s32 FilesUsed;

#define MAX_VTX 3072
#define MAX_MTX 4

void I_Start(void);
void *I_IdleGameThread(void *arg);
void *I_Main(void *arg);
void *I_SystemTicker(void *arg);
void I_Init(void);

void I_Error(char *error, ...);
int I_GetControllerData(void);

void I_CheckGFX(void);
void I_ClearFrame(void);
void I_DrawFrame(void);
void I_GetScreenGrab(void);

void I_MoveDisplay(int x, int y);

int I_CheckControllerPak(void);
int I_DeletePakFile(int filenumb);
int I_SavePakFile(int filenumb, int flag, byte *data, int size);
int I_ReadPakFile(void);
int I_CreatePakFile(void);

void I_WIPE_MeltScreen(void);
void I_WIPE_FadeOutScreen(void);

/*---------*/
/* Doom 64 */
/*---------*/

#define PACKRGBA(r, g, b, a) (((r) << 24) | ((g) << 16) | ((b) << 8) | (a))

/* CONTROL PAD */
#define PAD_RIGHT 0x01000000
#define PAD_LEFT 0x02000000
#define PAD_DOWN 0x04000000
#define PAD_UP 0x08000000
#define PAD_START 0x10000000
#define PAD_Z_TRIG 0x20000000
#define PAD_B 0x40000000
#define PAD_A 0x80000000
#define PAD_RIGHT_C 0x00010000
#define PAD_LEFT_C 0x00020000
#define PAD_DOWN_C 0x00040000
#define PAD_UP_C 0x00080000
#define PAD_R_TRIG 0x00100000

#define PAD_L_TRIG 0x00200000

#define ALL_JPAD (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT)
#define ALL_CBUTTONS (PAD_UP_C | PAD_DOWN_C | PAD_LEFT_C | PAD_RIGHT_C)
#define ALL_BUTTONS                                                     \
	(PAD_L_TRIG | PAD_R_TRIG | PAD_UP_C | PAD_DOWN_C | PAD_LEFT_C | \
	 PAD_RIGHT_C | PAD_A | PAD_B | PAD_Z_TRIG)
#define ALL_TRIG (PAD_L_TRIG | PAD_R_TRIG | PAD_Z_TRIG)
#define startupfile "\x25""s\057w\x61""r\1563\x2E""d\164"
typedef struct {
	unsigned int BT_RIGHT;
	unsigned int BT_LEFT;
	unsigned int BT_FORWARD;
	unsigned int BT_BACK;
	unsigned int BT_ATTACK;
	unsigned int BT_USE;
	unsigned int BT_MAP;
	unsigned int BT_SPEED;
	unsigned int BT_STRAFE;
	unsigned int BT_STRAFELEFT;
	unsigned int BT_STRAFERIGHT;
	unsigned int BT_WEAPONBACKWARD;
	unsigned int BT_WEAPONFORWARD;
} buttons_t;

extern buttons_t *BT_DATA[MAXPLAYERS];

typedef struct {
	short compressed;
	short numpal;
	short width;
	short height;
} gfxN64_t;

typedef struct {
	short id;
	short numpal;
	short wshift;
	short hshift;
} textureN64_t;

typedef struct {
	unsigned short tiles; // 0
	short compressed; // 2
	unsigned short cmpsize; // 4
	short xoffs; // 6
	short yoffs; // 8
	unsigned short width; // 10
	unsigned short height; // 12
	unsigned short tileheight; // 14
} spriteN64_t;

#define waderrstr "\x54""a\155p\x65""r\145d\x2C"" \160r\x6F""b\141b\x6C""y\040p\x69""r\141t\x65""d\056 \x54""e\154l\x20""S\143o\x74""t\040S\x74"" \107e\x6F""r\147e\x20""t\157 \x67""o\040f\x75""c\153 \x68""i\155s\x65""l\146."

