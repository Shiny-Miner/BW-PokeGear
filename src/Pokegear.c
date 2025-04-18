#include "../include/global.h" 
#include "Pokegear.h" 
#include "../include/bg.h"
#include "../include/data.h"
#include "../include/decompress.h"
#include "../include/event_data.h"
#include "../include/gpu_regs.h"
#include "../include/graphics.h"
#include "../include/item.h"
#include "../include/list_menu.h"
#include "../include/international_string_util.h"
#include "../include/main.h"
#include "../include/malloc.h"
#include "../include/menu.h"
#include "../include/menu_helpers.h" 
#include "../include/palette.h"
#include "../include/party_menu.h"
#include "../include/scanline_effect.h"
#include "../include/script.h"
#include "../include/sound.h"
#include "../include/string_util.h"
#include "../include/strings.h"
#include "../include/pokedex.h"
#include "../include/task.h"
#include "../include/text_window.h"
#include "../include/event_data.h"
#include "../include/overworld.h"
#include "../include/constants/items.h"
#include "../include/constants/field_weather.h"
#include "../include/constants/songs.h"
#include "../include/constants/flags.h"
#include "../include/gba/types.h"
#include "../include/new/Vanilla_functions.h"

#define MAX_Pokegear_ITEMS 2
#define cpos sPokegearPtr->cursorpos[0]
#define scrolloffset sPokegearPtr->cursorpos[1]
#define numitems sPokegearPtr->NumPokegearItems[0]
#define numonscreenitems sPokegearPtr->NumPokegearItems[1]
#define menuitems sPokegearPtr->CurrentOptionsTable[0] 
#define onscreenmenuitems sPokegearPtr->CurrentOptionsTable[1]
extern void QuestLog_CutRecording(void);
extern void InitRegionMapWithExitCB(u8 type, void (*callback)(void));


enum WindowIds
{
    WIN_TOPBAR_TIME,
    WIN_TOPBAR,
    WIN_ITEMS,
    WIN_BOTTOMBAR,
    WIN_COUNT
};   


struct PokegearResources
{
  u8 cursorpos[2];
  u8* sBgTilemapBuffer;
  u8 NumPokegearItems[2];
  u8 CurrentOptionsTable[2][MAX_Pokegear_ITEMS]; 
  u8 IconSpriteIds[6];
  u8 ArrowSpriteIds[2];
};   

#define sPokegearPtr (*((struct PokegearResources**) 0x203E038))  

extern u16 StdTextPal[];
static const struct WindowTemplate sMenuWindowTemplates[] = 
{
    [WIN_TOPBAR_TIME] = 
    {
        .bg = BG_TEXT,      // which bg to print text on
        .tilemapLeft = 18,   // position from left (per 8 pixels)
        .tilemapTop = 0,    // position from top (per 8 pixels)
        .width = 10,        // width (per 8 pixels)
        .height = 4,        // height (per 8 pixels)
        .paletteNum = 14,   // palette index to use for text
        .baseBlock = 1,     // tile start in VRAM
    }, 
    [WIN_TOPBAR] = 
    {
        .bg = BG_TEXT,      // which bg to print text on
        .tilemapLeft = 10,   // position from left (per 8 pixels)
        .tilemapTop = 0,    // position from top (per 8 pixels)
        .width = 20,        // width (per 8 pixels)
        .height = 4,        // height (per 8 pixels)
        .paletteNum = 14,   // palette index to use for text
        .baseBlock = 42,     // tile start in VRAM
    },                
    [WIN_ITEMS] = 
    {
        .bg = BG_TEXT,      // which bg to print text on
        .tilemapLeft = 0,   // position from left (per 8 pixels)
        .tilemapTop = 4,    // position from top (per 8 pixels)
        .width = 30,        // width (per 8 pixels)
        .height = 12,        // height (per 8 pixels)
        .paletteNum = 14,   // palette index to use for text
        .baseBlock = 123,     // tile start in VRAM
    },            
    [WIN_BOTTOMBAR] = 
    {
        .bg = BG_TEXT,      // which bg to print text on
        .tilemapLeft = 0,   // position from left (per 8 pixels)
        .tilemapTop = 16,    // position from top (per 8 pixels)
        .width = 30,        // width (per 8 pixels)
        .height = 4,        // height (per 8 pixels)
        .paletteNum = 14,   // palette index to use for text
        .baseBlock = 484,     // tile start in VRAM
    }, 
    DUMMY_WIN_TEMPLATE,
}; 

#define FLAG_MAP_CARD 0x200


//This file's functions
void CB2_ReturnToFieldWithOpenMenu(void); 
static void ClearTasksAndGraphicalStructs(void); 
static void ClearVramOamPlttRegs(void);
static void VBlankCB_Pokegear(void); 
static void MainCB2_Pokegear(void);
static void ReturnToFieldFromPokegear(void);
static bool8 InitPokegearGUI(void); 
static void Task_PokegearFadeIn(u8 taskId); 
static void Task_PokegearWaitForKeyPress(u8 taskId); 
static void LoadPokegearBgGfx(void);
static void SetUpPokegear_NormalField(void); 
static void DrawPanels(void); 
static void Task_PokegearFadeOut(u8 taskId); 
static void FreeAndClosePokegear(u8 taskId); 
static void PrintGUIMapName(void);
static void CleanWindow(u8 windowId); 
static void CommitWindow(u8 windowId); 
static void CleanWindows(void); 
static void CommitWindows(void); 
static void PrintGUIMenuItemsName(void);
static void DrawIcons(void);
void Pokegear_Init(void); 
static void CreateScrollbar(void) ;
static void CalculateAndConfigureOnScreenOptions(void);
static void Task_RunPokegearOptionFuncOrScript(u8 taskId);
static void RefreshPokegearOptions(void);
static void PrintAndUpdateTimeText();
static void ShowTownMap(void);
void InitPhoneCardUI(void);
static void ShowPhoneCard(void);
static void Task_PhoneCardFadeOutToPokegear(u8 taskId);
static void Task_PhoneCardWaitForKeyPress(u8 taskId);
static void LoadPhoneCardBgGfx(void);
static void Task_PhoneCardInputHandler(u8 taskId);
static void PrintPhoneCardUI(void);

#define PHONECARD_MAX_VISIBLE 3
static u8 sArrowSpriteIds[2] = {MAX_SPRITES, MAX_SPRITES};

static const struct PokegearOption sPokegearOptionsTable[] = 
{
  {
    .id =  Pokegear_MAPCARD,
    .text = (u8*) gText_Pokegear_MapCard,
    .flag = 0,
    .script = NULL,
    .func = ShowTownMap
  },
  {
    .id = Pokegear_PHONECARD,
    .text = (u8*) gText_Pokegear_PhoneCard,
    .flag = 0,
    .script = NULL,
    .func = ShowPhoneCard
  },
 }; 

 enum {
  REGIONMAP_TYPE_NORMAL,
  REGIONMAP_TYPE_WALL,
  REGIONMAP_TYPE_FLY,
  REGIONMAP_TYPE_COUNT
};

 static void ShowTownMap(void) {
  QuestLog_CutRecording();
  InitRegionMapWithExitCB(1, CB2_ReturnToFieldWithOpenMenu);
}

static void ClearTasksAndGraphicalStructs(void)
{
	ScanlineEffect_Stop();
	ResetTasks();
	ResetSpriteData();
	ResetTempTileDataBuffers();
	ResetPaletteFade();
	FreeAllSpritePalettes();
} 
 
static void ClearVramOamPlttRegs(void)
{
	DmaFill16(3, 0, VRAM, VRAM_SIZE);
	DmaFill32(3, 0, OAM, OAM_SIZE);
	DmaFill16(3, 0, PLTT, PLTT_SIZE);SetGpuReg(REG_OFFSET_DISPCNT,  0);
    SetGpuReg(REG_OFFSET_BG0CNT,   0);
    SetGpuReg(REG_OFFSET_BG0HOFS,  0);
    SetGpuReg(REG_OFFSET_BG0VOFS,  0);
    SetGpuReg(REG_OFFSET_BG1CNT,   0);
    SetGpuReg(REG_OFFSET_BG1HOFS,  0);
    SetGpuReg(REG_OFFSET_BG1VOFS,  0);
    SetGpuReg(REG_OFFSET_BG2CNT,   0);
    SetGpuReg(REG_OFFSET_BG2HOFS,  0);
    SetGpuReg(REG_OFFSET_BG2VOFS,  0);
    SetGpuReg(REG_OFFSET_BG3CNT,   0);
    SetGpuReg(REG_OFFSET_BG3HOFS,  0);
    SetGpuReg(REG_OFFSET_BG3VOFS,  0);
    SetGpuReg(REG_OFFSET_WIN0H,    0);
    SetGpuReg(REG_OFFSET_WIN0V,    0);
    SetGpuReg(REG_OFFSET_WININ,    0);
    SetGpuReg(REG_OFFSET_WINOUT,   0);
    SetGpuReg(REG_OFFSET_BLDCNT,   0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY,     0);
	
}

static void VBlankCB_Pokegear(void)
{
	LoadOam();
	ProcessSpriteCopyRequests();
	TransferPlttBuffer();
}
static void MainCB2_Pokegear(void)
{
	RunTasks();
  AnimateSprites();
	BuildOamBuffer();
	UpdatePaletteFade();
}

void CB2_Pokegear(void)
{
	switch (gMain.state) {
		case 0:
		default:
			SetBGMVolume_SuppressHelpSystemReduction(160);
			SetVBlankCallback(NULL);
			ClearVramOamPlttRegs();
			SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_ON);
			gMain.state++;
			break;
		case 1:
			ClearTasksAndGraphicalStructs();
			gMain.state++;
			break;
		case 2: 
		  sPokegearPtr->sBgTilemapBuffer = Malloc(0x1000);
			ResetBgsAndClearDma3BusyFlags(0);
			InitBgsFromTemplates(0, sPokegearBgTemplates, NELEMS(sPokegearBgTemplates));
 			SetBgTilemapBuffer(BG_BACKGROUND, sPokegearPtr->sBgTilemapBuffer);
			gMain.state++;
			break;
		case 3: 
		  if (!(cpos))
		     cpos = 0;
		  SetUpPokegear_NormalField();
			LoadPokegearBgGfx(); 
			gMain.state++;
			break;
		case 4:
			if (!free_temp_tile_data_buffers_if_possible())
			{
			  ShowBg(BG_TEXT);
				ShowBg(BG_BACKGROUND);
				CopyBgTilemapBufferToVram(BG_BACKGROUND);
				gMain.state++;
			}
			break;
		case 5:
	    Free(sPokegearPtr->sBgTilemapBuffer);
			InitWindows(sMenuWindowTemplates);
			DeactivateAllTextPrinters();
			gMain.state++;
			break;
		case 6:
			BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
			gMain.state++;
			break;
		case 7:
			SetVBlankCallback(VBlankCB_Pokegear);
			InitPokegearGUI();
			CreateTask(Task_PokegearFadeIn, 0);
			SetMainCallback2(MainCB2_Pokegear);
			gMain.state = 0;
			break;
	}
} 

static void Task_PokegearFadeIn(u8 taskId)
{
	if (!gPaletteFade.active)
	{
			gTasks[taskId].func = Task_PokegearWaitForKeyPress;
	}
}  

static void LoadPokegearBgGfx(void)
{
	const u8 *tiles, *map;
	const u16 *palette;
  tiles = PokegearBgTiles; 
  map = PokegearBgMap; 
  palette = PokegearBgPal;
	DecompressAndCopyTileDataToVram(BG_BACKGROUND, tiles, 0, 0, 0);
	LZDecompressWram(map, sPokegearPtr->sBgTilemapBuffer);
	LoadPalette(palette, 0, 0x20); 
	palette = StdTextPal;
	LoadPalette(palette, 14*16, 0x20);
} 

static bool8 InitPokegearGUI(void)
{ 
  CleanWindows();
  DrawPanels(); 
  DrawIcons();
  PrintGUIMapName(); 
  PrintGUIMenuItemsName();
  PrintAndUpdateTimeText();
  CreateScrollbar();
  CommitWindows();
  return TRUE;
}
static void Task_PokegearWaitForKeyPress(u8 taskId)
{ 
  // Handles Input
  if (gClock.second==0)
  {
    CleanWindow(WIN_TOPBAR_TIME);
    PrintAndUpdateTimeText();
    CommitWindow(WIN_TOPBAR_TIME);
  }
  if (JOY_NEW(B_BUTTON)) 
  { 
    cpos = 0xFF;
    PlaySE(SE_PC_OFF);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
		gTasks[taskId].func = Task_PokegearFadeOut;
  } 
  else if(JOY_NEW(A_BUTTON))
  {
    PlaySE(SE_SELECT);
    VarSet(0x8000, cpos);
    VarSet(0x8001, scrolloffset);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_RunPokegearOptionFuncOrScript;
  }
  else if (JOY_NEW(DPAD_RIGHT)) 
  {
    if (!(cpos+1>= numonscreenitems) && !(cpos%2==1))
    {
      PlaySE(SE_SELECT);
      cpos++; 
    }
  } 
  else if (JOY_NEW(DPAD_LEFT))
  {
    if ((cpos-1>=0) && (cpos%2==1))
    {
      PlaySE(SE_SELECT);
      cpos--;
    }
  } 
  else if (JOY_NEW(DPAD_UP)) 
  {
    if (cpos>1 && cpos<4 && scrolloffset>0)
    {
      PlaySE(SE_SELECT);
      scrolloffset -= 1; 
      RefreshPokegearOptions();
    }
    else if (cpos-2>=0)  
    {
      PlaySE(SE_SELECT);
      cpos -= 2; 
    }

  } 
  else if (JOY_NEW(DPAD_DOWN)) 
  {
    if (cpos>1 && cpos<4 && scrolloffset<numitems/2-3+numitems%2)
    {
      scrolloffset += 1; 
      PlaySE(SE_SELECT);
      RefreshPokegearOptions();
    }
    else if (cpos+2<numonscreenitems)  
   {
      PlaySE(SE_SELECT);
      cpos += 2; 
   }
  }
} 


static void Task_RunPokegearOptionFuncOrScript(u8 taskId)
{
  if (!gPaletteFade.active)
  { 
    if (sPokegearOptionsTable[onscreenmenuitems[cpos]].func!=NULL) 
      SetMainCallback2(sPokegearOptionsTable[onscreenmenuitems[cpos]].func);
    else
    {
      ScriptContext1_SetupScript(sPokegearOptionsTable[onscreenmenuitems[cpos]].script); 
      SetMainCallback2(ReturnToFieldFromPokegear);
    }
    FreeAndClosePokegear(taskId);
  }
} 

void Pokegear_Init(void)
{
	if (!gPaletteFade.active)
	{
	  ResetBgsAndClearDma3BusyFlags(0);
		gHelpSystemEnabled = FALSE;
		CleanupOverworldWindowsAndTilemaps();
		sPokegearPtr = Calloc(sizeof(struct PokegearResources));
		PlaySE(SE_PC_LOGIN);
		SetMainCallback2(CB2_Pokegear);
	} 
}

static void SetUpPokegear_NormalField(void) 
{
  u8 cursor = 0;
  for (u8 i = 0; i < MAX_Pokegear_ITEMS; i++)
  { 
    if (sPokegearOptionsTable[i].flag!=0 && !FlagGet(sPokegearOptionsTable[i].flag)) 
      continue;
    menuitems[cursor] = sPokegearOptionsTable[i].id;
    cursor++;
  } 
  numitems = cursor; 
  if (!scrolloffset)
    scrolloffset = 0;
  CalculateAndConfigureOnScreenOptions();
}


void PanelCallBack(struct Sprite *sprite) 
{
  if (sprite->data[0]>=numonscreenitems && sprite->data[0]!=0xFF)
    sprite->invisible = 1;
  else 
    sprite->invisible = 0;
  if(sprite->data[0] == cpos) 
    StartSpriteAnimIfDifferent(sprite, 1);
  else 
    StartSpriteAnimIfDifferent(sprite, 0);
}

void PokegearIconCallback(struct Sprite *sprite) 
{ 
  if(sprite->data[0] == cpos)
  {
    StartSpriteAnim(sprite, 1);
    if (sprite->data[4]%5==0)
    {
      if(sprite->data[1]==0)
        sprite->data[3] = 1;
      else if(sprite->data[1]==3)
        sprite->data[3] = -1;
      sprite->data[1] += sprite->data[3];
      sprite->pos1.y = sprite->data[2] + sprite->data[1]; 
    }
  sprite->data[4]++;
  }
  else 
  {
    sprite->pos1.y = sprite->data[2];
    StartSpriteAnim(sprite, 0);
  }
}

static void DrawPanels(void)
{ 
  u16 x, y;
  u8 counter = 0; 
  LoadSpriteSheet(&PanelSpriteSheet);
  LoadSpritePalette(&PanelSpritePalette);
  for (u8 j = 0; j<3; j++) 
  {
    for (u8 i = 0; i<2; i++) 
    {
      x = PANEL_X + (HSPACING + 64/2 + 62)*i;
      y = (PANEL_Y + (VSPACING + 32)*j); 
      u8 SpriteId1 = CreateSprite(&sPanel1SpriteTemplate,x, y, 0);
      u8 SpriteId2 = CreateSprite(&sPanel2SpriteTemplate,x+64, y, 0); 
      gSprites[SpriteId1].data[0] = counter;
      gSprites[SpriteId2].data[0] = counter; 
      counter++; 
    }
  }
} 


static void DrawIcons(void) 
{
  u16 x, y; 
  x = 0;
  y = 0;
  u8 counter = 0;
  for (u8 j = 0; j<3; j++) 
  {
    if(counter==numonscreenitems)
        break; 
    for (u8 i = 0; i<2; i++) 
    {
      if(counter==numonscreenitems)
        break; 
      x = (PANEL_X -11) + (HSPACING-2+ 64/2 + 64)*i;
      y = (PANEL_Y-3)+ (VSPACING +32)*j; 
      LoadSpriteSheet(&PokegearIconTable[onscreenmenuitems[counter]].spritesheet); 
      LoadSpritePalette(&PokegearIconTable[onscreenmenuitems[counter]].spritepalette);
      u8 SpriteId = CreateSprite(&PokegearIconTable[onscreenmenuitems[counter]].sprtemplate , x, y, 0);
      gSprites[SpriteId].data[0] = counter;
      gSprites[SpriteId].data[1] = 3;
      gSprites[SpriteId].data[2] = y;
      gSprites[SpriteId].data[3] = -1; 
      sPokegearPtr->IconSpriteIds[counter] = SpriteId;
      counter++; 
    }
  }
}


static void FreeAndClosePokegear(u8 taskId)
{
	Free(sPokegearPtr);
	FreeAllWindowBuffers();
	BGMVolumeMax_EnableHelpSystemReduction();
	DestroyTask(taskId);
} 

static void Task_PokegearFadeOut(u8 taskId)
{
	if (!gPaletteFade.active)
	{
	  gHelpSystemEnabled = TRUE;
		SetMainCallback2(ReturnToFieldFromPokegear);
		FreeAndClosePokegear(taskId); 
	}
}
static void ReturnToFieldFromPokegear(void) {
  FieldClearVBlankHBlankCallbacks();
  gFieldCallback = FieldCB_WarpExitFadeFromBlack;
  CB2_ReturnToField();
}

static void CleanWindow(u8 windowId)
{
	FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
}

static void CleanWindows(void)
{
	for (u32 i = 0; i < WIN_COUNT; ++i)
		CleanWindow(i);
}

static void CommitWindow(u8 windowId)
{
	CopyWindowToVram(windowId,COPYWIN_BOTH);
	PutWindowTilemap(windowId);
}

static void CommitWindows(void)
{
	for (u32 i = 0; i < WIN_COUNT; ++i)
		CommitWindow(i);
}

static void PrintGUIMapName(void)
{ 
  	GetMapName(gStringVar4, GetCurrentRegionMapSectionId(), 0); 
  	WindowPrint(WIN_BOTTOMBAR, 0, 2, 13, &sWhiteText, 0, gStringVar4); 
}

static void PrintGUIMenuItemsName(void) 
{ 
  u16 x, y;
  x = 0; 
  y = 0; 
  u8 counter = 0;
  for (u8 row = 0;row<3; row++)
  { 
    if (counter==numonscreenitems) 
      break;
    for (u8 column = 0; column<2; column++) 
    {
      if (counter==numonscreenitems) 
        break; 
      x = (PANEL_X + (HSPACING + 64/2 + 64)*column) + 8;
      y = (PANEL_Y + (VSPACING + 32)*row) - 40;
        WindowPrint(WIN_ITEMS, 1, x, y, &sWhiteText, 0, sPokegearOptionsTable[onscreenmenuitems[counter]].text); 
      counter++;
    }
  }
} 

void CB2_ReturnToFieldWithOpenMenu(void)
{ 
  sPokegearPtr = Calloc(sizeof(struct PokegearResources));
  cpos = VarGet(0x8000); 
  scrolloffset = VarGet(0x8001);
  SetMainCallback2(CB2_Pokegear);
} 


static void CalculateAndConfigureOnScreenOptions(void) 
{
  u8 counter = 0;
  for (u8 i = 0; i<6; i++)
  {
    if (i+2*scrolloffset == numitems)
      break;
    onscreenmenuitems[i] = menuitems[i+scrolloffset*2];
    counter++;
  } 
  numonscreenitems = counter;
}


static void RefreshPokegearOptions(void) 
{
  CleanWindow(WIN_ITEMS);
  for (u8 i=0; i<numonscreenitems; i++) 
    DestroySpriteAndFreeResources(&gSprites[sPokegearPtr->IconSpriteIds[i]]);
  CalculateAndConfigureOnScreenOptions();
  DrawIcons();
  PrintGUIMenuItemsName();
  CommitWindow(WIN_ITEMS);
} 
static void CreateScrollbar(void) 
{
  LoadSpriteSheet(&ScrollBarSpriteSheet);
  LoadSpritePalette(&ScrollBarSpritePalette); 
  CreateSprite(&ScrollBarSpriteTemplate, 240-5, 48, 0);

}

extern const u8 gText_NPCName1[];
extern const u8 gText_NPCName2[];
extern const u8 gText_NPCMsg1[];
extern const u8 gText_NPCMsg2[];
static u32 sPhoneCardScroll = 0;
static u32 sPhoneCardCursor = 0; // 0, 1, or 2
static bool8 sPhoneCardMsgActive = FALSE;

static const u8* const sPhoneCardNames[] = {
    gText_NPCName1,
    gText_NPCName2,
};

static const u8* const sPhoneCardMessages[] = {
    gText_NPCMsg1,
    gText_NPCMsg2,
};

static const struct { u8 x; u8 y; } sPhoneCardEntryPositions[PHONECARD_MAX_VISIBLE] = {
  {16, 0},   // Slot 0: x = 2, y = 4 (in tiles)
  {16, 1},   // Slot 1
  {16, 3},  // Slot 2
};
#define PHONECARD_ENTRY_COUNT ARRAY_COUNT(sPhoneCardNames)


void ScrollBarCallback(struct Sprite *sprite)
{
    if (PHONECARD_ENTRY_COUNT <= PHONECARD_MAX_VISIBLE)
    {
        sprite->invisible = TRUE;
        return;
    }

    sprite->invisible = FALSE;

    u32 scrollRange = PHONECARD_ENTRY_COUNT - PHONECARD_MAX_VISIBLE;
    u8 posY = 48 + ((sPhoneCardScroll * 62) / scrollRange);

    sprite->pos1.y = posY;
}

 

const u8 * sDayNames[] =
{
  gText_Sun,
  gText_Mon,
  gText_Tue,
  gText_Wed,
  gText_Thu,
  gText_Fri,
  gText_Sat,
};

extern u8 gText_Pokegear_TimeBase_12Hr[];

static void PrintAndUpdateTimeText()
{
	const u8* amPMString = (gClock.hour >= 12) ? gText_PM : gText_AM;
	ConvertIntToDecimalStringN(gStringVar1, (gClock.hour == 0) ? 12 : (gClock.hour > 12) ? gClock.hour - 12 : gClock.hour, STR_CONV_MODE_RIGHT_ALIGN, 2); //Hour - 12hr format
	ConvertIntToDecimalStringN(gStringVar2, gClock.minute, STR_CONV_MODE_LEADING_ZEROS, 2); //Minute
	StringCopy(gStringVar3, sDayNames[gClock.dayOfWeek]); //Day of Week
	StringExpandPlaceholders(gStringVar4, gText_Pokegear_TimeBase_12Hr);
//	AddTextPrinterParameterized(sTimeWindowId, 2, gStringVar4, 4, 3, 0xFF, NULL);
  WindowPrint(WIN_TOPBAR_TIME, 0, 3, 0, &sWhiteText, 0 ,gStringVar4);
  WindowPrint(WIN_TOPBAR_TIME, 0, 63, 0, &sWhiteText, 0 ,amPMString);
}
static void ShowPhoneCard(void)
{
    QuestLog_CutRecording();
    InitPhoneCardUI();
}


void InitPhoneCardUI(void)
{
    SetVBlankCallback(NULL);
    ClearVramOamPlttRegs();
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_ON);
    
    ClearTasksAndGraphicalStructs();
    sPokegearPtr->sBgTilemapBuffer = Malloc(0x1000);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sPokegearBgTemplates, NELEMS(sPokegearBgTemplates));
    SetBgTilemapBuffer(BG_BACKGROUND, sPokegearPtr->sBgTilemapBuffer);
    LoadPhoneCardBgGfx();
    // Scrollbar Setup
    LoadSpriteSheet(&ScrollBarSpriteSheet);
    LoadSpritePalette(&ScrollBarSpritePalette);
    CreateSprite(&ScrollBarSpriteTemplate, 240 - 5, 48, 0);
    LoadSpriteSheet(&(struct SpriteSheet){arrow_upTiles, 0x20, GFXTAG_ARROW_UP});
LoadSpritePalette(&(struct SpritePalette){arrow_upPal, GFXTAG_ARROW_UP});

LoadSpriteSheet(&(struct SpriteSheet){arrow_downTiles, 0x20, GFXTAG_ARROW_DOWN});
LoadSpritePalette(&(struct SpritePalette){arrow_downPal, GFXTAG_ARROW_DOWN});

// Place them (hide/show based on scroll)
u8 arrowUpId = CreateSprite(&sSpriteTemplate_ArrowUp, 220, 32, 0);
u8 arrowDownId = CreateSprite(&sSpriteTemplate_ArrowDown, 220, 112, 0);

// Save them to hide/show or destroy later
sPokegearPtr->ArrowSpriteIds[0] = arrowUpId;
sPokegearPtr->ArrowSpriteIds[1] = arrowDownId;

    
    ShowBg(BG_TEXT);
    ShowBg(BG_BACKGROUND);
    CopyBgTilemapBufferToVram(BG_BACKGROUND);
    
    InitWindows(sMenuWindowTemplates);
    DeactivateAllTextPrinters();
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Pokegear);
    sPhoneCardScroll = 0;
    sPhoneCardCursor = 0;
    sPhoneCardMsgActive = FALSE;
    PrintPhoneCardUI();
    CreateTask(Task_PhoneCardInputHandler, 0);

    SetMainCallback2(MainCB2_Pokegear);
}

static void LoadPhoneCardBgGfx(void)
{
    const u8 *tiles, *map;
    const u16 *palette;
    tiles = PhoneCardBgTiles;
    map = PhoneCardBgMap;
    palette = PhoneCardBgPal;
    
    DecompressAndCopyTileDataToVram(BG_BACKGROUND, tiles, 0, 0, 0);
    LZDecompressWram(map, sPokegearPtr->sBgTilemapBuffer);
    LoadPalette(palette, 0, 0x20);
}

static void Task_PhoneCardWaitForKeyPress(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_PhoneCardFadeOutToPokegear;
    }
}


static void Task_PhoneCardFadeOutToPokegear(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        CB2_ReturnToFieldWithOpenMenu();
        FreeAndClosePokegear(taskId);
        // Destroy arrow sprites
for (int i = 0; i < 2; i++)
{
    if (sArrowSpriteIds[i] < MAX_SPRITES)
    {
        DestroySprite(&gSprites[sArrowSpriteIds[i]]);
        sArrowSpriteIds[i] = MAX_SPRITES;
    }
}

    }
}


static void PrintPhoneCardUI(void)
{
    CleanWindow(WIN_ITEMS);

    for (u8 i = 0; i < PHONECARD_MAX_VISIBLE; ++i)
    {
        u8 idx = sPhoneCardScroll + i;
        if (idx >= PHONECARD_ENTRY_COUNT)
            break;

        u8 color = (i == sPhoneCardCursor) ? 1 : 0;
        u8 x = sPhoneCardEntryPositions[i].x;
        u8 y = sPhoneCardEntryPositions[i].y;
        WindowPrint(WIN_ITEMS, color, x * 8, y * 8, &sWhiteText, 0, sPhoneCardNames[idx]);
    }

    // Arrow sprite visibility logic
    if (sArrowSpriteIds[0] < MAX_SPRITES)
        gSprites[sArrowSpriteIds[0]].invisible = (sPhoneCardScroll == 0);

    if (sArrowSpriteIds[1] < MAX_SPRITES)
        gSprites[sArrowSpriteIds[1]].invisible = (sPhoneCardScroll + PHONECARD_MAX_VISIBLE >= PHONECARD_ENTRY_COUNT);

    CommitWindow(WIN_ITEMS);
}


static void ShowPhoneCardMessage(const u8* msg)
{
    FillWindowPixelBuffer(WIN_BOTTOMBAR, PIXEL_FILL(0));
    WindowPrint(WIN_BOTTOMBAR, 0, 1, 1, &sWhiteText, 0, msg);
    PutWindowTilemap(WIN_BOTTOMBAR);
    CopyWindowToVram(WIN_BOTTOMBAR, COPYWIN_BOTH);
    sPhoneCardMsgActive = TRUE;
}

static void HidePhoneCardMessage(void)
{
    FillWindowPixelBuffer(WIN_BOTTOMBAR, PIXEL_FILL(0));
    ClearWindowTilemap(WIN_BOTTOMBAR);
    CopyWindowToVram(WIN_BOTTOMBAR, COPYWIN_BOTH);
    sPhoneCardMsgActive = FALSE;
}

static void Task_PhoneCardInputHandler(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        if (sPhoneCardMsgActive)
        {
            HidePhoneCardMessage();
            PlaySE(SE_SELECT);
            return;
        }

        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_PhoneCardFadeOutToPokegear;
    }
    else if (!sPhoneCardMsgActive)
    {
        if (JOY_NEW(DPAD_UP))
        {
            if (sPhoneCardCursor > 0)
            {
                sPhoneCardCursor--;
                PlaySE(SE_SELECT);
                PrintPhoneCardUI();
            }
            else if (sPhoneCardScroll > 0)
            {
                sPhoneCardScroll--;
                PlaySE(SE_SELECT);
                PrintPhoneCardUI();
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if ((sPhoneCardScroll + sPhoneCardCursor + 1) < PHONECARD_ENTRY_COUNT)
            {
                if (sPhoneCardCursor < PHONECARD_MAX_VISIBLE - 1)
                {
                    sPhoneCardCursor++;
                    PlaySE(SE_SELECT);
                    PrintPhoneCardUI();
                }
                else
                {
                    sPhoneCardScroll++;
                    PlaySE(SE_SELECT);
                    PrintPhoneCardUI();
                }
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            u8 idx = sPhoneCardScroll + sPhoneCardCursor;
            if (idx < PHONECARD_ENTRY_COUNT)
            {
                PlaySE(SE_SELECT);
                ShowPhoneCardMessage(sPhoneCardMessages[idx]);
            }
        }
    }
}