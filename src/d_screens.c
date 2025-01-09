/* D_screens.c */

#include "i_main.h"
#include "doomdef.h"
#include "r_local.h"
#include "st_main.h"

extern int DefaultConfiguration[6][13];

static uint32_t Swap32(uint32_t val)
{
	return ((((val)&0xff000000) >> 24) | (((val)&0x00ff0000) >> 8) |
		(((val)&0x0000ff00) << 8) | (((val)&0x000000ff) << 24));
}

int D_RunDemo(char *name, skill_t skill, int map)
{
	int lump;
	int exit;

	demo_p = Z_Alloc(16000, PU_STATIC, NULL);
	memset(demo_p, 0, 16000);

	lump = W_GetNumForName(name);
	W_ReadLump(lump, demo_p, dec_d64);
	for (int i = 0; i < 4000; i++) {
		demo_p[i] = Swap32(demo_p[i]);
	}

	exit = G_PlayDemoPtr(skill, map);
	Z_Free(demo_p);

	return exit;
}

int D_TitleMap(void)
{
	int exit;

	D_OpenControllerPak();

	demo_p = Z_Alloc(16000, PU_STATIC, NULL);
	D_memset(demo_p, 0, 16000);
	D_memcpy(demo_p, DefaultConfiguration[0], 13 * sizeof(int));
	exit = G_PlayDemoPtr(sk_medium, 33);
	Z_Free(demo_p);

	return exit;
}

int D_WarningTicker(void)
{
	static int last_f_gametic = 0;

//	if ((gamevbls < gametic) && !(gametic & 7))
	if (((int)f_gamevbls < (int)f_gametic) && !(((int)f_gametic) & 7)) {
		if (last_f_gametic != (int)f_gametic) {
			last_f_gametic = (int)f_gametic;
			MenuAnimationTic = (MenuAnimationTic + 1) & 7;
		}
	}

	return 0;
}

void D_DrawWarning(void)
{
	I_ClearFrame();

	if (MenuAnimationTic & 1)
		ST_DrawString(-1, 30, "WARNING!", 0xc00000ff,1);

	ST_DrawString(-1, 60, "dreamcast controller", 0xffffffff,1);
	ST_DrawString(-1, 80, "is not connected.", 0xffffffff,1);
	ST_DrawString(-1, 120, "please turn off your", 0xffffffff,1);
	ST_DrawString(-1, 140, "dreamcast system.", 0xffffffff,1);
	ST_DrawString(-1, 180, "plug in your dreamcast", 0xffffffff,1);
	ST_DrawString(-1, 200, "controller and turn it on.", 0xffffffff,1);

	I_DrawFrame();
}

int D_LegalTicker(void)
{
	if ((ticon - last_ticon) >= 150) { // 5 * TICRATE
		text_alpha -= 8;
		if (text_alpha < 0) {
			text_alpha = 0;
			return 8;
		}
	}
	return 0;
}

void D_DrawLegal(void)
{
	I_ClearFrame();

	M_DrawBackground(27, 74, text_alpha, "USLEGAL", 0.00015f, 0);

	if (FilesUsed > -1) {
		ST_DrawString(-1, 200, "hold \x8d to manage vmu",
			      text_alpha | 0xffffff00,1);
	}

	I_DrawFrame();
}

int D_NoPakTicker(void)
{
	if ((ticon - last_ticon) >= 240) // 8 * TICRATE
		return 8;

	return 0;
}

void D_DrawNoPak(void)
{
	I_ClearFrame();

	ST_DrawString(-1, 40, "no vmu.", 0xffffffff,1);
	ST_DrawString(-1, 60, "your game cannot", 0xffffffff,1);
	ST_DrawString(-1, 80, "be saved.", 0xffffffff,1);
	ST_DrawString(-1, 120, "please turn off your", 0xffffffff,1);
	ST_DrawString(-1, 140, "dreamcast system", 0xffffffff,1);
	ST_DrawString(-1, 160, "before inserting a", 0xffffffff,1);
	ST_DrawString(-1, 180, "vmu.", 0xffffffff,1);

	I_DrawFrame();
}

void D_SplashScreen(void)
{
	// Check if any dreamcast controller is connected
	// if not connected, it will show the Warning screen
	maple_device_t *device = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
	if (!device) {
		MiniLoop(NULL, NULL, D_WarningTicker, D_DrawWarning);
	}

#if 1
	/* */
	/* Check if the n64 controller Pak is connected */
	/* */
	I_CheckControllerPak();

	/* */
	/* if not connected, it will show the NoPak screen */
	/* */
	if (FilesUsed < 0) {
		last_ticon = 0;
		MiniLoop(NULL, NULL, D_NoPakTicker, D_DrawNoPak);
	}
#endif

	// show the legals screen
	text_alpha = 0xff;
	last_ticon = 0;
	MiniLoop(NULL, NULL, D_LegalTicker, D_DrawLegal);
}

static int cred_step;
static int cred1_alpha;
static int cred2_alpha;
static int cred_next;

int D_Credits(void)
{
	int exit;

	cred_next = 0;
	cred1_alpha = 0;
	cred2_alpha = 0;
	cred_step = 0;
	exit = MiniLoop(NULL, NULL, D_CreditTicker, D_CreditDrawer);

	// if you exit while screens are up you can end up with colored background in main menu
	pvr_set_bg_color(0, 0, 0);

	return exit;
}

int D_CreditTicker(void)
{
	if (((u32)ticbuttons[0] >> 16) != 0)
		return ga_exit;

	if ((cred_next == 0) || (cred_next == 1)) {
		if (cred_step == 0) {
			cred1_alpha += 8;
			if (cred1_alpha >= 255) {
				cred1_alpha = 0xff;
				cred_step = 1;
			}
		} else if (cred_step == 1) {
			cred2_alpha += 8;
			if (cred2_alpha >= 255) {
				cred2_alpha = 0xff;
				last_ticon = ticon;
				cred_step = 2;
			}
		} else if (cred_step == 2) {
			if ((ticon - last_ticon) >= 180) // 6 * TICRATE
				cred_step = 3;
		} else {
			cred1_alpha -= 8;
			cred2_alpha -= 8;
			if (cred1_alpha < 0) {
				cred_next += 1;
				cred1_alpha = 0;
				cred2_alpha = 0;
				cred_step = 0;
			}
		}
	} else if (cred_next == 2)
		return ga_exitdemo;

	return ga_nothing;
}

void D_CreditDrawer(void)
{
	int color;

	I_ClearFrame();

	if (cred_next == 0) {
		// Set Background Color (Dark Blue)
		color = (cred1_alpha * 16) / 255;
		pvr_set_bg_color(0, 0, (float)color / 255.0f);

		M_DrawBackground(68, 21, cred1_alpha, "IDCRED1", 0.00015f, 0);
		M_DrawBackground(32, 41, cred2_alpha, "IDCRED2", 0.00016f, 1);
	} else {
		if ((cred_next == 1) || (cred_next == 2)) {
			// Set Background Color (Dark Grey)
			float fcol;
			color = (cred1_alpha * 30) / 255;
			fcol = (float)color / 255.0f;
			pvr_set_bg_color(fcol, fcol, fcol);

			M_DrawBackground(22, 82, cred1_alpha, "WMSCRED1",
					 0.00015f, 0);
			M_DrawBackground(29, 28, cred2_alpha, "WMSCRED2",
					 0.00016f, 1);
		}
	}

	I_DrawFrame();
}

void D_OpenControllerPak(void)
{
	unsigned int oldbuttons;

	oldbuttons = I_GetControllerData();

	if (((oldbuttons & 0xffff0000) == PAD_START) && (I_CheckControllerPak() == 0)) {
		MenuCall = M_ControllerPakDrawer;
		linepos = 0;
		cursorpos = 0;

		MiniLoop(M_FadeInStart, M_MenuClearCall, M_ScreenTicker, M_MenuGameDrawer);
		I_WIPE_FadeOutScreen();
	}
}

