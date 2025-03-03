#include "../include/global.h" 
#include "../include/sprite.h" 
#include "../include/bg.h" 

enum GfxTags
{
  GFXTAG_PANEL,
  GFXTAG_MAPCARD,
  GFXTAG_PHONECARD,
  GFXTAG_SCROLLBAR,
};



//Gfx Data
extern const u8 panelTiles[]; 
extern const u16 panelPal[];  

// BG Gfx data
extern const u8 StartMenuBgTiles[]; 
extern const u8 StartMenuBgMap[];
extern const u16 StartMenuBgPal[];

// Options Icon Gfx data
extern const u8 TownMapTiles[];
extern const u16 TownMapPal[];

// Scrollbar Gfx data
extern const u8 scrollbarTiles[];
extern const u16 scrollbarPal[];

// Phone card
extern const u8 phonecardTiles[];
extern const u16 phonecardPal[];

// Phone card bg
extern const u8 PhoneCardBgTiles[]; 
extern const u16 PhoneCardBgPal[]; 
extern const u8 PhoneCardBgMap[];


//================================================================================================
void PanelCallBack(struct Sprite *sprite);
void StartMenuIconCallback(struct Sprite *sprite);
void ScrollBarCallback(struct Sprite *sprite);

//=================================================================================================
static const struct SpriteSheet PanelSpriteSheet = {panelTiles, (64*64*4)/2, GFXTAG_PANEL };
static const struct SpritePalette PanelSpritePalette = {panelPal, GFXTAG_PANEL};
extern const struct SpriteTemplate Sprite1Template;
extern const struct SpriteTemplate Sprite2Template;

static const struct OamData sPanelOam =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_BLEND,
	.shape = SPRITE_SHAPE(64x64),
	.size = SPRITE_SIZE(64x64),
	.priority = 1, //Above other sprites
};

static const union AnimCmd sAnimCmdPanelUnselected1[] =
{
	ANIMCMD_FRAME(0, 0),
	ANIMCMD_END
}; 

static const union AnimCmd sAnimCmdPanelUnselected2[] =
{
	ANIMCMD_FRAME(64, 0),
	ANIMCMD_END
};  
static const union AnimCmd sAnimCmdPanelSelected1[] =
{
	ANIMCMD_FRAME(128, 0),
	ANIMCMD_END
}; 

static const union AnimCmd sAnimCmdPanelSelected2[] =
{
	ANIMCMD_FRAME(192, 0),       
	ANIMCMD_JUMP(0) 
};  


static const union AnimCmd *const sAnimCmdTable_Panel1[] =
{
	sAnimCmdPanelUnselected1,
	sAnimCmdPanelSelected1,
	
}; 

static const union AnimCmd *const sAnimCmdTable_Panel2[] =
{
	sAnimCmdPanelUnselected2,
	sAnimCmdPanelSelected2,
	
};  

static const struct SpriteTemplate sPanel1SpriteTemplate =
{
	.tileTag = GFXTAG_PANEL,
	.paletteTag = GFXTAG_PANEL,
	.oam = &sPanelOam,
	.anims = sAnimCmdTable_Panel1,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = PanelCallBack,
}; 
static const struct SpriteTemplate sPanel2SpriteTemplate =
{
	.tileTag = GFXTAG_PANEL,
	.paletteTag = GFXTAG_PANEL,
	.oam = &sPanelOam,
	.anims = sAnimCmdTable_Panel2,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = PanelCallBack,
}; 

// Pokedex Sprite

static const struct OamData sIconOamData =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_NORMAL,
	.shape = SPRITE_SHAPE(32x32),
	.size = SPRITE_SIZE(32x32),
	.priority = 1, //Above other sprites
};
static const struct OamData ScrollBarOamData =
{
	.affineMode = ST_OAM_AFFINE_OFF,
	.objMode = ST_OAM_OBJ_NORMAL,
	.shape = SPRITE_SHAPE(32x32),
	.size = SPRITE_SIZE(32x32),
	.priority = 1, //Above other sprites
};

static const union AnimCmd sAnimCmdIcon[] =
{
	ANIMCMD_FRAME(0, 0),
	ANIMCMD_END
};
static const union AnimCmd sAnimCmdIconSelected[] =
{
	ANIMCMD_FRAME(16, 0),
	ANIMCMD_END
}; 
static const union AnimCmd *const sAnimCmdTable_Icon[] =
{
	sAnimCmdIcon,
	sAnimCmdIconSelected
};   

static const struct SpriteSheet ScrollBarSpriteSheet = {scrollbarTiles, 32*16, GFXTAG_SCROLLBAR};
static const struct SpritePalette ScrollBarSpritePalette = {scrollbarPal, GFXTAG_SCROLLBAR};

static const union AnimCmd AnimCmdScrollbar[] = 
{
  ANIMCMD_FRAME(0, 0),
  ANIMCMD_END,
}; 
static const union AnimCmd *const sAnimCmdTable_Scrollbar[] =
{
	AnimCmdScrollbar
};   


static const struct SpriteTemplate ScrollBarSpriteTemplate =
{
  .tileTag = GFXTAG_SCROLLBAR,
	.paletteTag = GFXTAG_SCROLLBAR,
	.oam = &ScrollBarOamData,
	.anims = sAnimCmdTable_Scrollbar,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = ScrollBarCallback,
};
static const struct OamData sSprite1Oam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1, // Above other sprites
};

static const union AnimCmd sSprite1Anim[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sSprite1AnimTable[] =
{
    sSprite1Anim
};

const struct SpriteTemplate Sprite1Template =
{
    .tileTag = 0,
    .paletteTag = 0,
    .oam = &sSprite1Oam,
    .anims = sSprite1AnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
extern const u8 Sprite2Tiles[];
extern const u16 Sprite2Pal[];

static const struct OamData sSprite2Oam =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1, // Above other sprites
};

static const union AnimCmd sSprite2Anim[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sSprite2AnimTable[] =
{
    sSprite2Anim
};

const struct SpriteTemplate Sprite2Template =
{
    .tileTag = 1,  // Different from Sprite1
    .paletteTag = 1,  // Different from Sprite1
    .oam = &sSprite2Oam,
    .anims = sSprite2AnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

