#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <maxmod9.h>
#include "myDSiMode.h"

#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/flashcard.h"
#include "common/tonccpy.h"
#include "graphics/lodepng.h"
#include "graphics/graphics.h"
#include "graphics/color.h"
#include "sound.h"

//#include "logo_anniversary.h"
//#include "logoPhat_anniversary.h"
#include "icon_nds.h"
#include "icon_ndsl.h"
#include "icon_ndsi.h"
#include "icon_gba.h"
#include "iconPhat_gba.h"
#include "icon_gb.h"
#include "icon_a26.h"
#include "icon_int.h"
#include "icon_nes.h"
#include "icon_sms.h"
#include "icon_gg.h"
#include "icon_pce.h"
#include "icon_md.h"
#include "icon_snes.h"
#include "icon_present.h"

static inline const char* sm64dsReleaseDate(void) {
	using TRegion = TWLSettings::TRegion;
	int gameRegion = ms().getGameRegion();

	if (gameRegion == TRegion::ERegionKorea) {
		return "07/26";
	} else if (gameRegion == TRegion::ERegionChina) {
		return "06/21";
	} else if (gameRegion == TRegion::ERegionAustralia) {
		return "02/24";
	} else if (gameRegion == TRegion::ERegionEurope) {
		return "03/11";
	} else if (gameRegion == TRegion::ERegionUSA) {
		return "11/21";
	}
	return "12/02"; // Japan
}

static inline const char* styleSavvyReleaseDate(void) {
	using TRegion = TWLSettings::TRegion;
	int gameRegion = ms().getGameRegion();

	if (gameRegion == TRegion::ERegionKorea) {
		return "09/06";
	} else if (gameRegion == TRegion::ERegionAustralia) {
		return "11/19";
	} else if (gameRegion == TRegion::ERegionUSA) {
		return "11/02";
	}
	return "10/23"; // Japan
}

extern bool useTwlCfg;

//extern void loadROMselectAsynch(void);

extern u16 convertVramColorToGrayscale(u16 val);

extern u16 frameBuffer[2][256*192];
extern u16 frameBufferBot[2][256*192];
extern bool doubleBuffer;
extern bool doubleBufferTop;

extern bool fadeType;
extern bool fadeColor;
extern bool controlTopBright;

static int frameDelaySprite = 0;
static bool frameDelaySpriteEven = true;	// For 24FPS or 48FPS
static bool loadFrameSprite = true;
static bool longVersion = false;
static bool highFPS = false; // 75FPS

/*static int anniversaryTextYpos = -14;
static bool anniversaryTextYposMove = false;
static int anniversaryTextYposMoveSpeed = 9;
static int anniversaryTextYposMoveDelay = 0;
static bool anniversaryTextYposMoveDelayEven = true;	// For 24FPS */

static u16 twlColors[16] = {
	0x66F3,
	0x2193,
	0x359F,
	0x7A1F,
	0x323F,
	0x0B7F,
	0x0BF7,
	0x0BE2,
	0x270A,
	0x4351,
	0x7EEA,
	0x7DCB,
	0x7C42,
	0x7C53,
	0x7C58,
	0x5C17
};

static int zoomingIconXpos[11] = {-32, -32, 256, 256+16, -32, -32, 256, 256+16, -32, -32, 256+16};
static int zoomingIconYpos[11] = {-32, -48, -48, -32, 192+32, 192+48, 192+48, 192+32, -32, 192, -32};
static int gbaIconYpos = 44;

void twlMenuVideo_loadTopGraphics(void) {
	// Anniversary
	/*glDeleteTextures(1, &anniversaryTexID);
	
	icon_Pal = (u16*)(sys().isDSPhat() ? logoPhat_anniversaryPal : logo_anniversaryPal);
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	anniversaryTexID =
	glLoadTileSet(anniversaryText, // pointer to glImage array
				256, // sprite width
				64, // sprite height
				256, // bitmap width
				64, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Load our 16 color tiles palette
				(u8*) (sys().isDSPhat() ? logoPhat_anniversaryBitmap : logo_anniversaryBitmap) // image data generated by GRIT
				);*/

	char currentDate[16];
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	strftime(currentDate, sizeof(currentDate), "%m/%d", Time);

	bool december = (strncmp(currentDate, "12", 2) == 0
				   && strcmp(currentDate, "12/25") != 0
				   && strcmp(currentDate, "12/26") != 0
				   && strcmp(currentDate, "12/27") != 0
				   && strcmp(currentDate, "12/28") != 0
				   && strcmp(currentDate, "12/29") != 0
				   && strcmp(currentDate, "12/30") != 0
				   && strcmp(currentDate, "12/31") != 0);

	oamInit(&oamMain, SpriteMapping_1D_32, false);

	u16* gfx;

	// NDS
	u8* icon_Tiles = (u8*)icon_ndsTiles;
	int icon_TilesLen = icon_ndsTilesLen;
	u16* icon_Pal = (u16*)icon_ndsPal;
	int icon_PalLen = icon_ndsPalLen;
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else if (!sys().isRegularDS()) {
		icon_Tiles = (u8*)icon_ndsiTiles;
		icon_TilesLen = icon_ndsiTilesLen;
		icon_Pal = (u16*)icon_ndsiPal;
		icon_PalLen = icon_ndsiPalLen;
	} else if (!sys().isDSPhat()) {
		icon_Tiles = (u8*)icon_ndslTiles;
		icon_TilesLen = icon_ndslTilesLen;
		icon_Pal = (u16*)icon_ndslPal;
		icon_PalLen = icon_ndslPalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*7), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 7, zoomingIconXpos[7], zoomingIconYpos[7], 0, 7, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// GBA
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else {
		icon_Tiles = (u8*)(sys().isDSPhat() ? iconPhat_gbaTiles : icon_gbaTiles);
		icon_TilesLen = (sys().isDSPhat() ? iconPhat_gbaTilesLen : icon_gbaTilesLen);
		icon_Pal = (u16*)(sys().isDSPhat() ? iconPhat_gbaPal : icon_gbaPal);
		icon_PalLen = (sys().isDSPhat() ? iconPhat_gbaPalLen : icon_gbaPalLen);
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*3), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 3, zoomingIconXpos[3], zoomingIconYpos[3], 0, 3, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// GBC
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_gbTiles;
		icon_TilesLen = icon_gbTilesLen;
		icon_Pal = (u16*)icon_gbPal;
		icon_PalLen = icon_gbPalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*1), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 1, zoomingIconXpos[1], zoomingIconYpos[1], 0, 1, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// A26
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_a26Tiles;
		icon_TilesLen = icon_a26TilesLen;
		icon_Pal = (u16*)icon_a26Pal;
		icon_PalLen = icon_a26PalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*8), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 8, zoomingIconXpos[8], zoomingIconYpos[8], 0, 8, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// INT
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_intTiles;
		icon_TilesLen = icon_intTilesLen;
		icon_Pal = (u16*)icon_intPal;
		icon_PalLen = icon_intPalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*10), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 10, zoomingIconXpos[10], zoomingIconYpos[10], 0, 10, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// NES
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_nesTiles;
		icon_TilesLen = icon_nesTilesLen;
		icon_Pal = (u16*)icon_nesPal;
		icon_PalLen = icon_nesPalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE, icon_Pal, icon_PalLen);
	oamSet(&oamMain, 0, zoomingIconXpos[0], zoomingIconYpos[0], 0, 0, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// SMS
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_smsTiles;
		icon_TilesLen = icon_smsTilesLen;
		icon_Pal = (u16*)icon_smsPal;
		icon_PalLen = icon_smsPalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*4), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 4, zoomingIconXpos[4], zoomingIconYpos[4], 0, 4, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// GG
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_ggTiles;
		icon_TilesLen = icon_ggTilesLen;
		icon_Pal = (u16*)icon_ggPal;
		icon_PalLen = icon_ggPalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*6), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 6, zoomingIconXpos[6], zoomingIconYpos[6], 0, 6, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// PCE
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_pceTiles;
		icon_TilesLen = icon_pceTilesLen;
		icon_Pal = (u16*)icon_pcePal;
		icon_PalLen = icon_pcePalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*9), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 9, zoomingIconXpos[9], zoomingIconYpos[9], 0, 9, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// MD
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_mdTiles;
		icon_TilesLen = icon_mdTilesLen;
		icon_Pal = (u16*)icon_mdPal;
		icon_PalLen = icon_mdPalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*5), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 5, zoomingIconXpos[5], zoomingIconYpos[5], 0, 5, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);

	// SNES
	if (december) {
		icon_Tiles = (u8*)icon_presentTiles;
		icon_TilesLen = icon_presentTilesLen;
		icon_Pal = (u16*)icon_presentPal;
		icon_PalLen = icon_presentPalLen;
	} else 	{
		icon_Tiles = (u8*)icon_snesTiles;
		icon_TilesLen = icon_snesTilesLen;
		icon_Pal = (u16*)icon_snesPal;
		icon_PalLen = icon_snesPalLen;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < icon_PalLen/2; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gfx = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
	tonccpy(gfx, icon_Tiles, icon_TilesLen);
	tonccpy(SPRITE_PALETTE+(16*2), icon_Pal, icon_PalLen);
	oamSet(&oamMain, 2, zoomingIconXpos[2], zoomingIconYpos[2], 0, 2, SpriteSize_32x32, SpriteColorFormat_16Color, gfx, 0, false, false, false, false, false);
}

extern char soundBank[];
extern bool soundBankInited;
mm_sound_effect bootJingle;

void twlMenuVideo_topGraphicRender(void) {
	if (!loadFrameSprite) {
		frameDelaySprite++;
		if (highFPS) {
			loadFrameSprite = (frameDelaySprite == (longVersion ? 3 : 2));
		} else {
			loadFrameSprite = (frameDelaySprite == (longVersion ? 2 : 1)+frameDelaySpriteEven);
		}
	}

	if (loadFrameSprite) {
		zoomingIconXpos[0] += 2;
		zoomingIconYpos[0] += 3;
		if (zoomingIconXpos[0] > 36) {
			zoomingIconXpos[0] = 36;
		}
		if (zoomingIconYpos[0] > 32) {
			zoomingIconYpos[0] = 32;
		}

		zoomingIconXpos[1] += 3;
		zoomingIconYpos[1] += 2;
		if (zoomingIconXpos[1] > 80) {
			zoomingIconXpos[1] = 80;
		}
		if (zoomingIconYpos[1] > 12) {
			zoomingIconYpos[1] = 12;
		}

		zoomingIconXpos[2] -= 3;
		zoomingIconYpos[2] += 2;
		if (zoomingIconXpos[2] < 154) {
			zoomingIconXpos[2] = 154;
		}
		if (zoomingIconYpos[2] > 12) {
			zoomingIconYpos[2] = 12;
		}

		zoomingIconXpos[3] -= 2;
		zoomingIconYpos[3] += 2;
		if (zoomingIconXpos[3] < 202) {
			zoomingIconXpos[3] = 202;
		}
		if (zoomingIconYpos[3] > gbaIconYpos) {
			zoomingIconYpos[3] = gbaIconYpos;
		}

		zoomingIconXpos[4] += 2;
		zoomingIconYpos[4] -= 3;
		if (zoomingIconXpos[4] > 32) {
			zoomingIconXpos[4] = 32;
		}
		if (zoomingIconYpos[4] < 120) {
			zoomingIconYpos[4] = 120;
		}

		zoomingIconXpos[5] += 3;
		zoomingIconYpos[5] -= 2;
		if (zoomingIconXpos[5] > 80) {
			zoomingIconXpos[5] = 80;
		}
		if (zoomingIconYpos[5] < 152) {
			zoomingIconYpos[5] = 152;
		}

		zoomingIconXpos[6] -= 3;
		zoomingIconYpos[6] -= 3;
		if (zoomingIconXpos[6] < 150) {
			zoomingIconXpos[6] = 150;
		}
		if (zoomingIconYpos[6] < 142) {
			zoomingIconYpos[6] = 142;
		}

		zoomingIconXpos[7] -= 2;
		zoomingIconYpos[7] -= 3;
		if (zoomingIconXpos[7] < 202) {
			zoomingIconXpos[7] = 202;
		}
		if (zoomingIconYpos[7] < 120) {
			zoomingIconYpos[7] = 120;
		}

		zoomingIconXpos[8] += 2;
		zoomingIconYpos[8] += 2;
		if (zoomingIconXpos[8] > 4) {
			zoomingIconXpos[8] = 4;
		}
		if (zoomingIconYpos[8] > 8) {
			zoomingIconYpos[8] = 8;
		}

		zoomingIconXpos[9] += 2;
		zoomingIconYpos[9] -= 2;
		if (zoomingIconXpos[9] > 8) {
			zoomingIconXpos[9] = 8;
		}
		if (zoomingIconYpos[9] < 192-40) {
			zoomingIconYpos[9] = 192-40;
		}

		zoomingIconXpos[10] -= 2;
		zoomingIconYpos[10] += 2;
		if (zoomingIconXpos[10] < 256-40) {
			zoomingIconXpos[10] = 256-40;
		}
		if (zoomingIconYpos[10] > 8) {
			zoomingIconYpos[10] = 8;
		}

		for (int i = 0; i < 11; i++) {
			oamSetXY(&oamMain, i, zoomingIconXpos[i], zoomingIconYpos[i]);
		}
		if (highFPS) {
			while (REG_VCOUNT < 88); // Fix/Hide screen tearing
		}
		oamUpdate(&oamMain);

		frameDelaySprite = 0;
		frameDelaySpriteEven = !frameDelaySpriteEven;
		loadFrameSprite = false;
	}
	
	/*if (rocketVideo_playVideo && rocketVideo_currentFrame >= 13) {
		if (!anniversaryTextYposMove) {
			anniversaryTextYposMoveDelay++;
			anniversaryTextYposMove = (anniversaryTextYposMoveDelay == 2+anniversaryTextYposMoveDelayEven);
		}

		if (anniversaryTextYposMove && anniversaryTextYpos < 40) {
			anniversaryTextYpos += anniversaryTextYposMoveSpeed;
			anniversaryTextYposMoveSpeed--;
			if (anniversaryTextYposMoveSpeed < 1) anniversaryTextYposMoveSpeed = 1;

			anniversaryTextYposMoveDelay = 0;
			anniversaryTextYposMoveDelayEven = !anniversaryTextYposMoveDelayEven;
			anniversaryTextYposMove = false;
		}
	}*/
}

void twlMenuVideo(void) {
	std::vector<unsigned char> image;
	unsigned width, height;

	char currentDate[16], logoPath[256];
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	strftime(currentDate, sizeof(currentDate), "%m/%d", Time);
	bool starship = (strcmp(currentDate, "04/01") == 0);

	if (starship) {
		// Load Starship Menu++ BG
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppStarship.png");
		longVersion = ms().longSplashJingle;
	} else if (strncmp(currentDate, "12", 2) == 0) {
		// Load christmas BG for December
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppXmas.png");
		longVersion = ms().longSplashJingle;
	} else if (strcmp(currentDate, "10/31") == 0) {
		// Load orange BG for Halloween
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppOrange.png");
		longVersion = ms().longSplashJingle;
	} else if (strcmp(currentDate, styleSavvyReleaseDate()) == 0) {
		// Load Style Savvy BG
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppFashion.png");
		gbaIconYpos -= 8;
	} else if (ms().getGameRegion() == 0 ? (strcmp(currentDate, "07/21") == 0) : (strcmp(currentDate, "08/14") == 0)) {
		// Load Virtual Boy BG
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppVirtualBoy.png");
		longVersion = ms().longSplashJingle;
	} else if (strcmp(currentDate, "02/14") == 0) {
		// Load heart-shaped BG for Valentine's Day
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppHeart.png");
		longVersion = ms().longSplashJingle;
	} else if (strcmp(currentDate, "03/10") == 0 || strcmp(currentDate, sm64dsReleaseDate()) == 0) {
		// Load Mario-themed BG & logo for MAR10 Day
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppMario.png");
	} else if (strcmp(currentDate, "03/17") == 0 || strcmp(currentDate, "04/22") == 0) {
		// Load green BG for St. Patrick's Day, or Earth Day
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppGreen.png");
		longVersion = ms().longSplashJingle;
	} else {
		// Load normal BG
		sprintf(logoPath, "nitro:/graphics/logo_twlmenupp.png");
		longVersion = ms().longSplashJingle;
	}

	// Load TWLMenu++ logo
	lodepng::decode(image, width, height, logoPath);
	bool alternatePixel = false;
	for (unsigned i=0;i<image.size()/4;i++) {
		image[(i*4)+3] = 0;
		if (alternatePixel) {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
				image[(i*4)+3] |= BIT(0);
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
				image[(i*4)+3] |= BIT(1);
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
				image[(i*4)+3] |= BIT(2);
			}
		}
		frameBuffer[0][i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (alternatePixel) {
			if (image[(i*4)+3] & BIT(0)) {
				image[(i*4)] += 0x4;
			}
			if (image[(i*4)+3] & BIT(1)) {
				image[(i*4)+1] += 0x4;
			}
			if (image[(i*4)+3] & BIT(2)) {
				image[(i*4)+2] += 0x4;
			}
		} else {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
			}
		}
		frameBuffer[1][i] = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if ((i % 256) == 255) alternatePixel = !alternatePixel;
		alternatePixel = !alternatePixel;
	}
	image.clear();

	doubleBuffer = true;
	doubleBufferTop = true;

	if (!starship) {
		lodepng::decode(image, width, height, "nitro:/graphics/TWL.png");

		u16* twlTextBuffer = new u16[60*14];
		int x = 19;
		int y = 81;
		for (int i=0; i<60*14; i++) {
			if (x >= 79) {
				x = 19;
				y++;
			}
			if (image[(i*4)+3] > 0) {
				const u16 color = twlColors[(int)(useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme)];
				const u16 bgColor = frameBuffer[0][y*256+x];
				twlTextBuffer[i] = alphablend(color, bgColor, image[(i*4)+3]);
			} else {
				twlTextBuffer[i] = 0;
			}
			x++;
		}

		u16* src = twlTextBuffer;
		x = 19;
		y = 81;
		for (int i=0; i<60*14; i++) {
			if (x >= 79) {
				x = 19;
				y++;
			}
			u16 val = *(src++);
			if (image[(i*4)+3] > 0) {
				frameBuffer[0][y*256+x] = val;
				frameBuffer[1][y*256+x] = val;
			}
			x++;
		}

		delete[] twlTextBuffer;

		image.clear();
	}

	if (ms().colorMode == 1) {
		for (int i=0; i<256*192; i++) {
			frameBuffer[0][i] = convertVramColorToGrayscale(frameBuffer[0][i]);
			frameBuffer[1][i] = convertVramColorToGrayscale(frameBuffer[1][i]);
		}
	}

	highFPS = (sys().isRegularDS() || ((dsiFeatures() || sdFound()) && ms().consoleModel < 2));

	if (highFPS) {
		*(u32*)(0x2FFFD0C) = 0x43535046;
	}
	fadeType = true;

	for (int i = 0; i < 15; i++)
	{
		swiWaitForVBlank();
	}

	snd();
	snd().beginStream();

	extern bool twlMenuSplash;
	twlMenuSplash = true;

	for (int i = 0; i < (highFPS ? (longVersion ? ((75 * 6) + 30) : (75 * 3)) : (longVersion ? ((60 * 6) + 35) : (60 * 3))); i++)
	{
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT) || (keysHeld() & KEY_TOUCH)) return;
		//loadROMselectAsynch();
		snd().updateStream();
		twlMenuVideo_topGraphicRender();
		swiWaitForVBlank();
	}
}