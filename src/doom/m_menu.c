//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2021 Julian Nechaevsky
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//


#include <stdlib.h>
#include <ctype.h>

#include "doomdef.h"
#include "doomkeys.h"
#include "d_main.h"
#include "deh_main.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "m_argv.h"
#include "m_controls.h"
#include "p_saveg.h"
#include "s_sound.h"
#include "doomstat.h"
#include "m_menu.h"
#include "st_stuff.h"
#include "v_trans.h"
#include "am_map.h"         // [JN] AM_initColors();

#include "rd_keybinds.h"
#include "rd_lang.h"
#include "rd_menu.h"
#include "crispy.h"
#include "jn.h"

#define LINEHEIGHT      16

void (*messageRoutine)(int response);

boolean inhelpscreens;
int InfoType = 0;
boolean menuactive;

// [JN] Save strings and messages 
int     quickSaveSlot;      // -1 = no quicksave slot picked!
int     messageToPrint;     // 1 = message to be printed
int     messageToBind;      // [JN] Indicates if we are binding key
int    *keyToBind;          // [Dasperal] Pointer to key_var to bind
char   *messageString;      // ...and here is the message string!
static boolean slottextloaded;
boolean saveStatus[8];
char    savegamestrings[10][SAVESTRINGSIZE];
char    saveOldString[SAVESTRINGSIZE];  // old save description before edit
char	endstring[160];
boolean messageNeedsInput;  // timed message = no input from user
boolean QuickSaveTitle;     // [JN] Extra title "БЫСТРОЕ СОХРАНЕНИЕ"

// message x & y
int     messx;
int     messy;
int     messageLastMenuActive;

// we are going to be entering a savegame string
int     saveStringEnter;              
int     saveSlot;           // which slot to save in
int     saveCharIndex;      // which char we're editing


static boolean opldev;
extern boolean sendpause;

short   skullAnimCounter;   // skull animation counter
short   whichSkull;         // which skull to draw

// graphic name of skulls
char   *skullName[2] = {"M_SKULL1", "M_SKULL2"};

// -----------------------------------------------------------------------------
// [JN] Custom RD menu: font writing prototypes
// -----------------------------------------------------------------------------
void M_WriteTextBigCentered_ENG(int y, char *string);
void M_WriteTextBigCentered_RUS(int y, char *string);
void M_WriteTextSmallCentered_ENG(int y, char *string);
void M_WriteTextSmallCentered_RUS(int y, char *string);


//
// PROTOTYPES
//

void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartMessage(char *string,void *routine,boolean input);
void M_RD_StartBinding(int* key_var);
void M_RD_StartBinding_Mouse(int* key_var);

// -----------------------------------------------------------------------------
// [JN] Custom RD menu prototypes
// -----------------------------------------------------------------------------

// Rendering
void M_RD_Draw_Rendering(void);
void M_RD_Change_Widescreen(Direction_t direction);
void M_RD_Change_VSync(Direction_t direction);
void M_RD_Change_Uncapped(Direction_t direction);
void M_RD_Change_FPScounter(Direction_t direction);
void M_RD_Change_DiskIcon(Direction_t direction);
void M_RD_Change_Smoothing(Direction_t direction);
void M_RD_Change_Wiping(Direction_t direction);
void M_RD_Change_Screenshots(Direction_t direction);
void M_RD_Change_ENDOOM(Direction_t direction);
void M_RD_Change_Renderer(Direction_t direction);
void M_RD_Change_PorchFlashing(Direction_t direction);

// Display
void M_RD_Draw_Display(void);
void M_RD_Change_ScreenSize(Direction_t direction);
void M_RD_Change_Gamma(Direction_t direction);
void M_RD_Change_LevelBrightness(Direction_t direction);
void M_RD_Change_MenuShading(Direction_t direction);
void M_RD_Change_Detail(Direction_t direction);
void M_RD_Change_HUD_Detail(Direction_t direction);
void M_RD_Change_LocalTime(Direction_t direction);

// Messages
void M_RD_Draw_MessagesSettings(void);
void M_RD_Change_Messages(Direction_t direction);
void M_RD_Change_Msg_Alignment(Direction_t direction);
void M_RD_Change_Msg_TimeOut(Direction_t direction);
void M_RD_Change_Msg_Fade(Direction_t direction);
void M_RD_Change_ShadowedText(Direction_t direction);
void M_RD_Change_Msg_Pickup_Color(Direction_t direction);
void M_RD_Change_Msg_Secret_Color(Direction_t direction);
void M_RD_Change_Msg_System_Color(Direction_t direction);
void M_RD_Change_Msg_Chat_Color(Direction_t direction);

// Automap
void M_RD_Draw_AutomapSettings(void);
void M_RD_Change_AutomapColor(Direction_t direction);
void M_RD_Change_AutomapAntialias(Direction_t direction);
void M_RD_Change_AutomapOverlay(Direction_t direction);
void M_RD_Change_AutomapRotate(Direction_t direction);
void M_RD_Change_AutomapFollow(Direction_t direction);
void M_RD_Change_AutomapGrid(Direction_t direction);
void M_RD_Change_AutomapGridSize(Direction_t direction);
void M_RD_Change_AutomapStats(Direction_t direction);
void M_RD_Change_AutomapLevelTime(Direction_t direction);
void M_RD_Change_AutomapTotalTime(Direction_t direction);
void M_RD_Change_AutomapCoords(Direction_t direction);

// Sound
void M_RD_Draw_Audio(void);
void M_RD_Change_SfxVol(Direction_t direction);
void M_RD_Change_MusicVol(Direction_t direction);
void M_RD_Change_SfxChannels(Direction_t direction);

// Sound system
void M_RD_Draw_Audio_System(void);
void M_RD_Change_SoundDevice(Direction_t direction);
void M_RD_Change_MusicDevice(Direction_t direction);
void M_RD_Change_Sampling(Direction_t direction);
void M_RD_Change_SndMode(Direction_t direction);
void M_RD_Change_PitchShifting(Direction_t direction);
void M_RD_Change_MuteInactive(Direction_t direction);

// Controls
void M_RD_Draw_Controls(void);
void M_RD_Change_MouseLook(Direction_t direction);
void M_RD_Change_InvertY(Direction_t direction);
void M_RD_Change_Novert(Direction_t direction);
void M_RD_Change_Sensitivity(Direction_t direction);
void M_RD_Change_Acceleration(Direction_t direction);
void M_RD_Change_Threshold(Direction_t direction);

// Key bindings (1)
void M_RD_Bind_Key(int choice);
void M_RD_Draw_Bindings();

// Mouse bindings
void M_RD_Draw_Mouse_Bindings(void);
void M_RD_Mouse_Bind_FireAttack(int choice);
void M_RD_Mouse_Bind_Use(int choice);
void M_RD_Mouse_Bind_MoveForward(int choice);
void M_RD_Mouse_Bind_MoveBackward(int choice);
void M_RD_Mouse_Bind_StrafeOn(int choice);
void M_RD_Mouse_Bind_StrafeLeft(int choice);
void M_RD_Mouse_Bind_StrafeRight(int choice);
void M_RD_Mouse_Bind_PrevWeapon(int choice);
void M_RD_Mouse_Bind_NextWeapon(int choice);

// Gameplay
void M_RD_Draw_Gameplay_1(void);
void M_RD_Draw_Gameplay_2(void);
void M_RD_Draw_Gameplay_3(void);
void M_RD_Draw_Gameplay_4(void);
void M_RD_Draw_Gameplay_5(void);

void M_RD_Change_Brightmaps(Direction_t direction);
void M_RD_Change_FakeContrast(Direction_t direction);
void M_RD_Change_Translucency(Direction_t direction);
void M_RD_Change_ImprovedFuzz(Direction_t direction);
void M_RD_Change_ColoredBlood(Direction_t direction);
void M_RD_Change_SwirlingLiquids(Direction_t direction);
void M_RD_Change_InvulSky(Direction_t direction);
void M_RD_Change_LinearSky(Direction_t direction);
void M_RD_Change_FlipWeapons(Direction_t direction);

void M_RD_Change_ExtraPlayerFaces(Direction_t direction);
void M_RD_Change_NegativeHealth(Direction_t direction);
void M_RD_Change_SBarColored(Direction_t direction);
void M_RD_Change_SBarHighValue(Direction_t direction);
void M_RD_Change_SBarNormalValue(Direction_t direction);
void M_RD_Change_SBarLowValue(Direction_t direction);
void M_RD_Change_SBarCriticalValue(Direction_t direction);
void M_RD_Change_SBarArmorType1(Direction_t direction);
void M_RD_Change_SBarArmorType2(Direction_t direction);
void M_RD_Change_SBarArmorType0(Direction_t direction);

void M_RD_Change_ZAxisSfx(Direction_t direction);
void M_RD_Change_ExitSfx(Direction_t direction);
void M_RD_Change_CrushingSfx(Direction_t direction);
void M_RD_Change_BlazingSfx(Direction_t direction);
void M_RD_Change_AlertSfx(Direction_t direction);
void M_RD_Change_SecretNotify(Direction_t direction);
void M_RD_Change_InfraGreenVisor(Direction_t direction);

void M_RD_Change_WalkOverUnder(Direction_t direction);
void M_RD_Change_Torque(Direction_t direction);
void M_RD_Change_Bobbing(Direction_t direction);
void M_RD_Change_SSGBlast(Direction_t direction);
void M_RD_Change_FlipCorpses(Direction_t direction);
void M_RD_Change_FloatPowerups(Direction_t direction);
void M_RD_Change_TossDrop(Direction_t direction);
void M_RD_Change_CrosshairDraw(Direction_t direction);
void M_RD_Change_CrosshairType(Direction_t direction);
void M_RD_Change_CrosshairScale(Direction_t direction);

void M_RD_Change_FixMapErrors(Direction_t direction);
void M_RD_Change_FlipLevels(Direction_t direction);
void M_RD_Change_LostSoulsQty(Direction_t direction);
void M_RD_Change_LostSoulsAgr(Direction_t direction);
void M_RD_Change_PistolStart(Direction_t direction);
void M_RD_Change_DemoTimer(Direction_t direction);
void M_RD_Change_DemoTimerDir(Direction_t direction);
void M_RD_Change_DemoBar(Direction_t direction);
void M_RD_Change_NoInternalDemos(Direction_t direction);

// Level select
void M_LevelSelect(int choice);

void M_RD_Draw_Level_1(void);
void M_RD_Draw_Level_2(void);

void M_RD_Change_Selective_Skill(Direction_t direction);
void M_RD_Change_Selective_Episode(Direction_t direction);
void M_RD_Change_Selective_Map(Direction_t direction);
void M_RD_Change_Selective_Health(Direction_t direction);
void M_RD_Change_Selective_Armor(Direction_t direction);
void M_RD_Change_Selective_ArmorType(Direction_t direction);

void M_RD_Change_Selective_WP_Chainsaw(Direction_t direction);
void M_RD_Change_Selective_WP_Shotgun(Direction_t direction);
void M_RD_Change_Selective_WP_SSgun(Direction_t direction);
void M_RD_Change_Selective_WP_Chaingun(Direction_t direction);
void M_RD_Change_Selective_WP_RLauncher(Direction_t direction);
void M_RD_Change_Selective_WP_Plasmagun(Direction_t direction);
void M_RD_Change_Selective_WP_BFG9000(Direction_t direction);

void M_RD_Change_Selective_Backpack(Direction_t direction);

void M_RD_Change_Selective_Ammo_0(Direction_t direction);
void M_RD_Change_Selective_Ammo_1(Direction_t direction);
void M_RD_Change_Selective_Ammo_2(Direction_t direction);
void M_RD_Change_Selective_Ammo_3(Direction_t direction);

void M_RD_Change_Selective_Key_0(Direction_t direction);
void M_RD_Change_Selective_Key_1(Direction_t direction);
void M_RD_Change_Selective_Key_2(Direction_t direction);
void M_RD_Change_Selective_Key_3(Direction_t direction);
void M_RD_Change_Selective_Key_4(Direction_t direction);
void M_RD_Change_Selective_Key_5(Direction_t direction);

void M_RD_Change_Selective_Fast(Direction_t direction);
void M_RD_Change_Selective_Respawn(Direction_t direction);

// Reset settings
void M_RD_Draw_Reset(void);
void M_RD_BackToDefaults_Recommended(int choice);
void M_RD_BackToDefaults_Original(int choice);

// Language hot-swapping
void M_RD_ChangeLanguage(int choice);

// -----------------------------------------------------------------------------
// [JN] Vanilla menu prototypes
// -----------------------------------------------------------------------------

void M_Vanilla_DrawOptions(void);
void M_Vanilla_DrawSound(void);

// -----------------------------------------------------------------------------
// HU_WriteTextBigCentered
//
// [JN] Write a centered string using the BIG hu_font_big. Only Y coord is set.
// -----------------------------------------------------------------------------

void M_WriteTextBigCentered_ENG (int y, char *string)
{
    char*   ch;
    int	    c;
    int	    cx;
    int	    cy;
    int	    w;
    int	    width;

    // find width
    ch = string;
    width = 0;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            width += 10;
            continue;
        }

        w = SHORT (hu_font_big_eng[c]->width);
        width += w;
    }

    // draw it
    cx = origwidth/2-width/2;
    ch = string;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            cx += 10;
            continue;
        }

        w = SHORT (hu_font_big_eng[c]->width);

        V_DrawShadowedPatchDoom(cx, cy, hu_font_big_eng[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// HU_WriteTextBigCentered
//
// [JN] Write a centered string using the BIG hu_font_big. Only Y coord is set.
// -----------------------------------------------------------------------------

void M_WriteTextBigCentered_RUS (int y, char *string)
{
    char*   ch;
    int	    c;
    int	    cx;
    int	    cy;
    int	    w;
    int	    width;

    // find width
    ch = string;
    width = 0;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            width += 10;
            continue;
        }

        w = SHORT (hu_font_big_rus[c]->width);
        width += w;
    }

    // draw it
    cx = origwidth/2-width/2;
    ch = string;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = c - HU_FONTSTART2;

        if (c < 0 || c> HU_FONTSIZE2)
        {
            cx += 10;
            continue;
        }

        w = SHORT (hu_font_big_rus[c]->width);

        V_DrawShadowedPatchDoom(cx, cy, hu_font_big_rus[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// M_WriteTextSmallCentered_ENG
// [JN] Write a centered string using a small hu_font_small_eng.
// -----------------------------------------------------------------------------

void M_WriteTextSmallCentered_ENG (int y, char *string)
{
    char *ch;
    int   c, cx, cy;
    int   w, width;

    // find width
    ch = string;
    width = 0;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            width += 4;
            continue;
        }

        w = SHORT (hu_font_small_eng[c]->width);
        width += w;
    }

    // draw it
    cx = origwidth/2-width/2;
    ch = string;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_small_eng[c]->width);

        V_DrawShadowedPatchDoom(cx, cy, hu_font_small_eng[c]);

        cx+=w;
    }
}


// -----------------------------------------------------------------------------
// M_WriteTextSmallCentered_RUS
// [JN] Write a centered string using a small hu_font_small_rus.
// -----------------------------------------------------------------------------

void M_WriteTextSmallCentered_RUS (int y, char *string)
{
    char *ch;
    int   c, cx, cy;
    int   w, width;

    // find width
    ch = string;
    width = 0;
    cy = y;

    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            width += 4;
            continue;
        }

        w = SHORT (hu_font_small_rus[c]->width);
        width += w;
    }

    // draw it
    cx = origwidth/2-width/2;
    ch = string;
    while (ch)
    {
        c = *ch++;

        if (!c)
        break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {
            cx += 4;
            continue;
        }

        w = SHORT (hu_font_small_rus[c]->width);

        V_DrawShadowedPatchDoom(cx, cy, hu_font_small_rus[c]);

        cx+=w;
    }
}

// -----------------------------------------------------------------------------
// M_RD_ColorTranslation
// [JN] Returns a color translation for given variable.
// -----------------------------------------------------------------------------

static Translation_CR_t M_RD_ColorTranslation (int color)
{
    switch (color)
    {
        case 1:   return CR_DARKRED;    break;
        case 2:   return CR_GREEN;      break;
        case 3:   return CR_DARKGREEN;  break;
        case 4:   return CR_OLIVE;      break;
        case 5:   return CR_BLUE2;      break;
        case 6:   return CR_DARKBLUE;   break;
        case 7:   return CR_YELLOW;     break;
        case 8:   return CR_ORANGE;     break;
        case 9:   return CR_WHITE;      break;
        case 10:  return CR_GRAY;       break;
        case 11:  return CR_DARKGRAY;   break;
        case 12:  return CR_TAN;        break;
        case 13:  return CR_BROWN;      break;
        case 14:  return CR_ALMOND;     break;
        case 15:  return CR_KHAKI;      break;
        case 16:  return CR_PINK;       break;
        case 17:  return CR_BURGUNDY;   break;
        default:  return CR_NONE;          break;
    }
}

// -----------------------------------------------------------------------------
// M_RD_ColorName_ENG
// [JN] Returns a color name for given variable, in English language.
// -----------------------------------------------------------------------------

static char *M_RD_ColorName_ENG (int color)
{
    switch (color)
    {
        case 1:   return "dark red";    break;
        case 2:   return "green";       break;
        case 3:   return "dark green";  break;
        case 4:   return "olive";       break;
        case 5:   return "blue";        break;
        case 6:   return "dark blue";   break;
        case 7:   return "yellow";      break;
        case 8:   return "orange";      break;
        case 9:   return "white";       break;
        case 10:  return "gray";        break;
        case 11:  return "dark gray";   break;
        case 12:  return "tan";         break;
        case 13:  return "brown";       break;
        case 14:  return "almond";      break;
        case 15:  return "khaki";       break;
        case 16:  return "pink";        break;
        case 17:  return "burgundy";    break;
        default:  return "red";         break;
    }
}

// -----------------------------------------------------------------------------
// M_RD_ColorName_RUS
// [JN] Returns a color name for given variable, in Russian language.
// -----------------------------------------------------------------------------

static char *M_RD_ColorName_RUS (int color)
{
    switch (color)
    {
        case 1:   return "ntvyj-rhfcysq";  break; // Тёмно-красный
        case 2:   return "ptktysq";        break; // Зеленый
        case 3:   return "ntvyj-ptktysq";  break; // Тёмно-зеленый
        case 4:   return "jkbdrjdsq";      break; // Оливковый
        case 5:   return "cbybq";          break; // Синий
        case 6:   return "ntvyj-cbybq";    break; // Тёмно-синий
        case 7:   return ";tknsq";         break; // Жёлтый
        case 8:   return "jhfy;tdsq";      break; // Оранжевый
        case 9:   return ",tksq";          break; // Белый
        case 10:  return "cthsq";          break; // Серый
        case 11:  return "ntvyj-cthsq";    break; // Тёмно-серый
        case 12:  return ",t;tdsq";        break; // Бежевый
        case 13:  return "rjhbxytdsq";     break; // Коричневый
        case 14:  return "vbylfkmysq";     break; // Миндальный
        case 15:  return "[frb";           break; // Хаки
        case 16:  return "hjpjdsq";        break; // Розовый
        case 17:  return ",jhljdsq";       break; // Бордовый
        default:  return "rhfcysq";        break; // Красный
    }
}

static Menu_t* MainMenu;
static Menu_t* EpisodeMenu;
static Menu_t NewGameMenu;
static Menu_t OptionsMenu;
static Menu_t RenderingMenu;
static Menu_t DisplayMenu;
static Menu_t MessagesMenu;
static Menu_t AutomapMenu;
static Menu_t SoundMenu;
static Menu_t SoundSysMenu;
static Menu_t ControlsMenu;
static Menu_t Bindings1Menu;
static Menu_t Bindings2Menu;
static Menu_t Bindings3Menu;
static Menu_t Bindings4Menu;
static const Menu_t* BindingsMenuPages[] = {&Bindings1Menu, &Bindings2Menu, &Bindings3Menu, &Bindings4Menu};
static Menu_t MouseBindingsMenu;
static Menu_t Gameplay1Menu;
static Menu_t Gameplay2Menu;
static Menu_t Gameplay3Menu;
static Menu_t Gameplay4Menu;
static Menu_t Gameplay5Menu;
static const Menu_t* GameplayMenuPages[] = {&Gameplay1Menu, &Gameplay2Menu, &Gameplay3Menu, &Gameplay4Menu, &Gameplay5Menu};
static Menu_t LevelSelect1Menu;
static Menu_t LevelSelect2Menu;
static const Menu_t* LevelSelectMenuPages[] = {&LevelSelect1Menu, &LevelSelect2Menu};
static Menu_t ResetMenu;
static Menu_t Options2Menu_Vanilla;
static Menu_t LoadMenu;
static Menu_t SaveMenu;

// =============================================================================
// DOOM MENU
// =============================================================================

static MenuItem_t DoomItems[] = {
    {ITT_EFUNC,   "nM_NGAME",  "yRD_NGAME", M_NewGame,    0},
    {ITT_SETMENU, "oM_OPTION", "yRD_OPTN",  &OptionsMenu, 0},
    {ITT_EFUNC,   "lM_LOADG",  "pRD_LOADG", M_LoadGame,   0},
    {ITT_EFUNC,   "sM_SAVEG",  "cRD_SAVEG", M_SaveGame,   0},
    {ITT_EFUNC,   "rM_RDTHIS", "bRD_INFO",  M_ReadThis,   0},
    {ITT_EFUNC,   "qM_QUITG",  "dRD_QUITG", M_QuitDOOM,   0}
};

static Menu_t DoomMenu = {
    97, 97,
    64,
    NULL, NULL, true,
    6, DoomItems, true,
    M_DrawMainMenu,
    NULL, 0,
    NULL,
    0
};

// -----------------------------------------------------------------------------
// [JN] Special menu for Commercial
// -----------------------------------------------------------------------------

static MenuItem_t Doom2Items[] = {
    {ITT_EFUNC,   "nM_NGAME",  "yRD_NGAME", M_NewGame,    0},
    {ITT_SETMENU, "oM_OPTION", "yRD_OPTN",  &OptionsMenu, 0},
    {ITT_EFUNC,   "lM_LOADG",  "pRD_LOADG", M_LoadGame,   0},
    {ITT_EFUNC,   "sM_SAVEG",  "cRD_SAVEG", M_SaveGame,   0},
    {ITT_EFUNC,   "qM_QUITG",  "dRD_QUITG", M_QuitDOOM,   0}
};

static Menu_t Doom2Menu = {
    97, 97,
    72,
    NULL, NULL, true,
    5, Doom2Items, true,
    M_DrawMainMenu,
    NULL, 0,
    NULL,
    0
};

// -----------------------------------------------------------------------------
// [JN] Special menu for Press Beta
// -----------------------------------------------------------------------------

static MenuItem_t MainMenuBetaItems[] = {
    {ITT_EFUNC,   "dM_BLVL1",  "eRD_BLVL1", M_Episode,    0},
    {ITT_EFUNC,   "dM_BLVL2",  "eRD_BLVL2", M_Episode,    1},
    {ITT_EFUNC,   "dM_BLVL3",  "eRD_BLVL3", M_Episode,    2},
    {ITT_SETMENU, "oM_OPTION", "yRD_OPTN",  &OptionsMenu, 0},
    {ITT_EFUNC,   "qM_QUITG",  "dRD_QUITG", M_QuitDOOM,   0}
};

static Menu_t MainMenuBeta = {
    97, 97,
    70,
    NULL, NULL, true,
    5, MainMenuBetaItems, true,
    M_DrawMainMenu,
    NULL, 0,
    NULL,
    0
};

// =============================================================================
// EPISODE SELECT
// =============================================================================

static MenuItem_t DoomEpisodeItems [] = {
    {ITT_EFUNC, "kM_EPI1", "gRD_EPI1", M_Episode, 0},
    {ITT_EFUNC, "tM_EPI2", "gRD_EPI2", M_Episode, 1},
    {ITT_EFUNC, "iM_EPI3", "bRD_EPI3", M_Episode, 2},
    {ITT_EFUNC, "sM_EPI5", "cRD_EPI5", M_Episode, 4} // [Dasperal]
};

static MenuItem_t UltimateEpisodeItems [] = {
    {ITT_EFUNC, "kM_EPI1", "gRD_EPI1",  M_Episode, 0},
    {ITT_EFUNC, "tM_EPI2", "gRD_EPI2",  M_Episode, 1},
    {ITT_EFUNC, "iM_EPI3", "bRD_EPI3",  M_Episode, 2},
    {ITT_EFUNC, "tM_EPI4", "nRD_EPI4",  M_Episode, 3},
    {ITT_EFUNC, "sM_EPI5", "cRD_EPI5", M_Episode, 4} // [crispy] Sigil
};

static Menu_t DoomEpisodeMenu = {
    48, 48,
    63,
    NULL, NULL, true,
    3, DoomEpisodeItems, true,
    M_DrawEpisode,
    NULL, 0,
    &DoomMenu,
    0
};

static Menu_t DoomSigilEpisodeMenu = {
    48, 48,
    63,
    NULL, NULL, true,
    4, DoomEpisodeItems, true,
    M_DrawEpisode,
    NULL, 0,
    &DoomMenu,
    0
};

static Menu_t UltimateEpisodeMenu = {
    48, 48,
    63,
    NULL, NULL, true,
    4, UltimateEpisodeItems, true,
    M_DrawEpisode,
    NULL, 0,
    &DoomMenu,
    0
};

static Menu_t UltimateSigilEpisodeMenu = {
    48, 48,
    63,
    NULL, NULL, true,
    5, UltimateEpisodeItems, true,
    M_DrawEpisode,
    NULL, 0,
    &DoomMenu,
    0
};

// =============================================================================
// NEW GAME
// =============================================================================

static MenuItem_t NewGameItems[] = {
    {ITT_EFUNC, "iM_JKILL",  "vRD_JKILL", M_ChooseSkill, 0},
    {ITT_EFUNC, "hM_ROUGH",  "'RD_ROUGH", M_ChooseSkill, 1},
    {ITT_EFUNC, "hM_HURT",   "cRD_HURT",  M_ChooseSkill, 2},
    {ITT_EFUNC, "uM_ULTRA",  "eRD_ULTRA", M_ChooseSkill, 3},
    {ITT_EFUNC, "nM_NMARE",  "rRD_NMARE", M_ChooseSkill, 4},
    {ITT_EFUNC, "uM_UNMARE", "eRD_UNMAR", M_ChooseSkill, 5}
};

static Menu_t NewGameMenu = {
    48, 48,
    63,
    NULL, NULL, true,
    6, NewGameItems, true,
    M_DrawNewGame,
    NULL, 0,
    &DoomEpisodeMenu,
    0
};

// =============================================================================
// [JN] NEW OPTIONS MENU: STRUCTURE
// =============================================================================

static MenuItem_t OptionsItems[] = {
    {ITT_SETMENU, "Rendering",        "Dbltj",          &RenderingMenu,      0},
    {ITT_SETMENU, "Display",          "\"rhfy",         &DisplayMenu,        0},
    {ITT_SETMENU, "Sound",            "Felbj",          &SoundMenu,          0},
    {ITT_SETMENU, "Controls",         "Eghfdktybt",     &ControlsMenu,       0},
    {ITT_SETMENU, "Gameplay",         "Utqvgktq",       &Gameplay1Menu,      0},
    {ITT_EFUNC,   "Level select",     "Ds,jh ehjdyz",   M_LevelSelect,       0},
    {ITT_EFUNC,   "End Game",         "Pfrjyxbnm buhe", M_EndGame,           0},
    {ITT_SETMENU, "Reset settings",   "C,hjc yfcnhjtr", &ResetMenu,          0},
    {ITT_EFUNC,   "Language:english", "Zpsr#heccrbq",   M_RD_ChangeLanguage, 0}
};

static Menu_t OptionsMenu = {
    60, 60,
    22,
    "OPTIONS", "YFCNHJQRB", false,
    9, OptionsItems, true,
    NULL,
    NULL, 0,
    &DoomMenu,
    0
};

// -----------------------------------------------------------------------------
// Video and Rendering
// -----------------------------------------------------------------------------

static MenuItem_t RenderingItems[] = {
    {ITT_TITLE,  "Rendering",                 "htylthbyu",                       NULL,                      0}, // Рендеринг
    {ITT_LRFUNC, "Display aspect ratio:",     "Cjjnyjitybt cnjhjy \'rhfyf:",     M_RD_Change_Widescreen,   0},
    {ITT_LRFUNC, "Vertical synchronization:", "Dthnbrfkmyfz cby[hjybpfwbz:",     M_RD_Change_VSync,        0},
    {ITT_LRFUNC, "Frame rate:",               "Rflhjdfz xfcnjnf:",               M_RD_Change_Uncapped,     0},
    {ITT_LRFUNC, "Show FPS counter:",         "Cxtnxbr rflhjdjq xfcnjns:",       M_RD_Change_FPScounter,   0},
    {ITT_LRFUNC, "Pixel scaling:",            "Gbrctkmyjt cukf;bdfybt:",         M_RD_Change_Smoothing,    0},
    {ITT_LRFUNC, "Porch palette changing:",   "Bpvtytybt gfkbnhs rhftd 'rhfyf:", M_RD_Change_PorchFlashing,0},
    {ITT_LRFUNC, "Video renderer:",           "J,hf,jnrf dbltj:",                M_RD_Change_Renderer,     0},
    {ITT_TITLE,  "Extra",                     "ljgjkybntkmyj",                   NULL,                      0}, // Дополнительно
    {ITT_LRFUNC, "Show disk icon:",           "Jnj,hf;fnm pyfxjr lbcrtns:",      M_RD_Change_DiskIcon,     0},
    {ITT_LRFUNC, "Screen wiping effect:",     "\'aatrn cvtys \'rhfyjd:",         M_RD_Change_Wiping,       0},
    {ITT_LRFUNC, "Screenshot format:",        "Ajhvfn crhbyijnjd:",              M_RD_Change_Screenshots,  0},
    {ITT_LRFUNC, "Show ENDOOM screen:",       "Gjrfpsdfnm \'rhfy",               M_RD_Change_ENDOOM,       0}
};

static Menu_t RenderingMenu = {
    35, 35,
    25,
    "RENDERING OPTIONS", "YFCNHJQRB DBLTJ", false, // НАСТРОЙКИ ВИДЕО
    13, RenderingItems, false,
    M_RD_Draw_Rendering,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

static MenuItem_t DisplayItems[] = {
    {ITT_TITLE,   "Screen",                 "\'rhfy",                  NULL,                        0}, // Экран
    {ITT_LRFUNC,  "screen size",            "hfpvth buhjdjuj \'rhfyf", M_RD_Change_ScreenSize,      0},
    {ITT_EMPTY,   NULL,                     NULL,                      NULL,                        0},
    {ITT_LRFUNC,  "gamma-correction",       "ehjdtym ufvvf-rjhhtrwbb", M_RD_Change_Gamma,           0},
    {ITT_EMPTY,   NULL,                     NULL,                      NULL,                        0},
    {ITT_LRFUNC,  "level brightness",       "ehjdtym jcdtotyyjcnb",    M_RD_Change_LevelBrightness, 0},
    {ITT_EMPTY,   NULL,                     NULL,                      NULL,                        0},
    {ITT_LRFUNC,  "menu shading",           "pfntvytybt ajyf vty.",    M_RD_Change_MenuShading,     0},
    {ITT_EMPTY,   NULL,                     NULL,                      NULL,                        0},
    {ITT_LRFUNC,  "graphics detail:",       "ltnfkbpfwbz uhfabrb:",    M_RD_Change_Detail,          0},
    {ITT_LRFUNC,  "hud background detail:", "ltnfkbpfwbz ajyf",        M_RD_Change_HUD_Detail,      0},
    {ITT_TITLE,   "Interface",              "bynthatqc",               NULL,                        0}, // Интерфейс
    {ITT_SETMENU, "messages and texts",     "cjj,otybz b ntrcns",      &MessagesMenu,               0},
    {ITT_SETMENU, "automap and statistics", "rfhnf b cnfnbcnbrf",      &AutomapMenu,                0}
};

static Menu_t DisplayMenu = {
    35, 35,
    25,
    "DISPLAY OPTIONS", "YFCNHJQRB \"RHFYF", false, // НАСТРОЙКИ ЭКРАНА
    14, DisplayItems, false,
    M_RD_Draw_Display,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

static MenuItem_t MessagesItems[] = {
    {ITT_TITLE,  "General",             "jcyjdyjt",                 NULL,                         0}, // Основное
    {ITT_LRFUNC, "messages enabled:",   "jnj,hf;tybt cjj,otybq:",   M_RD_Change_Messages,         0}, // Отображение сообщений:
    {ITT_LRFUNC, "alignment:",          "dshfdybdfybt:",            M_RD_Change_Msg_Alignment,    0}, // Выравнивание:
    {ITT_LRFUNC, "message timeout",     "nfqvfen jnj,hf;tybz",      M_RD_Change_Msg_TimeOut,      0}, // Таймаут отображения
    {ITT_EMPTY,  NULL,                  NULL,                       NULL,                         0},
    {ITT_LRFUNC, "fading effect:",      "gkfdyjt bcxtpyjdtybt:",    M_RD_Change_Msg_Fade,         0}, // Плавное исчезновение:
    {ITT_LRFUNC, "text casts shadows:", "ntrcns jn,hfcsdf.n ntym:", M_RD_Change_ShadowedText,     0}, // Тексты отбрасывают тень:
    {ITT_TITLE,  "Misc.",               "hfpyjt",                   NULL,                         0}, // Разное
    {ITT_LRFUNC, "local time:",         "cbcntvyjt dhtvz:",         M_RD_Change_LocalTime,        0}, // Системное время:
    {ITT_TITLE,  "Colors",              "wdtnf",                    NULL,                         0}, // Цвета
    {ITT_LRFUNC, "item pickup:",        "gjkextybt ghtlvtnjd:",     M_RD_Change_Msg_Pickup_Color, 0}, // Получение предметов:
    {ITT_LRFUNC, "revealed secret:",    "j,yfhe;tybt nfqybrjd:",    M_RD_Change_Msg_Secret_Color, 0}, // Обнаружение тайников:
    {ITT_LRFUNC, "system message:",     "cbcntvyst cjj,otybz:",     M_RD_Change_Msg_System_Color, 0}, // Системные сообщения:
    {ITT_LRFUNC, "netgame chat:",       "xfn ctntdjq buhs:",        M_RD_Change_Msg_Chat_Color,   0}  // Чат сетевой игры:
};

static Menu_t MessagesMenu = {
    35, 35,
    25,
    "MESSAGES AND TEXTS", "CJJ<OTYBZ B NTRCNS", false, // СООБЩЕНИЯ И ТЕКСТЫ
    14, MessagesItems, false,
    M_RD_Draw_MessagesSettings,
    NULL, 0,
    &DisplayMenu,
    1
};

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

static MenuItem_t AutomapItems[] = {
    {ITT_TITLE,  "Automap",            "Rfhnf",              NULL,                          0}, // Карта
    {ITT_LRFUNC, "color scheme:",      "wdtnjdfz c[tvf:",    M_RD_Change_AutomapColor,     0}, // Цветовая схема:
    {ITT_LRFUNC, "line antialiasing:", "cukf;bdfybt kbybq:", M_RD_Change_AutomapAntialias, 0}, // Сглаживание линий:
    {ITT_LRFUNC, "overlay mode:",      "ht;bv yfkj;tybz:",   M_RD_Change_AutomapOverlay,   0}, // Режим наложения:
    {ITT_LRFUNC, "rotate mode:",       "ht;bv dhfotybz:",    M_RD_Change_AutomapRotate,    0}, // Режим вращения:
    {ITT_LRFUNC, "follow mode:",       "ht;bv cktljdfybz:",  M_RD_Change_AutomapFollow,    0}, // Режим следования:
    {ITT_LRFUNC, "grid:",              "ctnrf:",             M_RD_Change_AutomapGrid,      0}, // Сетка:
    {ITT_LRFUNC, "grid size:",         "hfpvth ctnrb:",      M_RD_Change_AutomapGridSize,  0}, // Размер сетки:
    {ITT_TITLE,  "Statistics",         "Cnfnbcnbrf",         NULL,                          0}, // Статистика
    {ITT_LRFUNC, "level stats:",       "cnfnbcnbrf ehjdyz:", M_RD_Change_AutomapStats,     0}, // Статистика уровня:
    {ITT_LRFUNC, "level time:",        "dhtvz ehjdyz:",      M_RD_Change_AutomapLevelTime, 0}, // Время уровня:
    {ITT_LRFUNC, "total time:",        "j,ott dhtvz:",       M_RD_Change_AutomapTotalTime, 0}, // Общее время:
    {ITT_LRFUNC, "player coords:",     "rjjhlbyfns buhjrf:", M_RD_Change_AutomapCoords,    0}  // Координаты игрока:
};

static Menu_t AutomapMenu = {
    70, 70,
    25,
    "AUTOMAP AND STATS", "RFHNF B CNFNBCNBRF", false, // КАРТА И СТАТИСТИКА
    13, AutomapItems, false,
    M_RD_Draw_AutomapSettings,
    NULL, 0,
    &DisplayMenu,
    1
};

// -----------------------------------------------------------------------------
// Sound and Music
// -----------------------------------------------------------------------------

static MenuItem_t SoundItems[] = {
    {ITT_TITLE,   "volume",                "uhjvrjcnm",                  NULL,                    0}, // Громкость
    {ITT_LRFUNC,  "sfx volume",            "pder",                       M_RD_Change_SfxVol,      0}, // Звук
    {ITT_EMPTY,   NULL,                    NULL,                         NULL,                    0},
    {ITT_LRFUNC,  "music volume",          "vepsrf",                     M_RD_Change_MusicVol,    0}, // Музыка
    {ITT_EMPTY,   NULL,                    NULL,                         NULL,                    0},
    {ITT_TITLE,   "channels",              "djcghjbpdtltybt",            NULL,                    0}, // Воспроизведение
    {ITT_LRFUNC,  "sound channels",        "Pderjdst rfyfks",            M_RD_Change_SfxChannels, 0}, // Звуковые каналы
    {ITT_EMPTY,   NULL,                    NULL,                         NULL,                    0},
    {ITT_TITLE,   "advanced",              "ljgjkybntkmyj",              NULL,                    0}, // Дополнительно
    {ITT_SETMENU, "sound system settings", "yfcnhjqrb pderjdjq cbcntvs", &SoundSysMenu,           0}  // Настройки звуковой системы...
};

static Menu_t SoundMenu = {
    35, 35,
    25,
    "SOUND OPTIONS", "YFCNHJQRB PDERF", false, // НАСТРОЙКИ ЗВУКА
    10, SoundItems, false,
    M_RD_Draw_Audio,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

static MenuItem_t SoundSysItems[] = {
    {ITT_TITLE,  "sound system",          "pderjdfz cbcntvf",           NULL,                       0}, // ЗВУКОВАЯ СИСТЕМА
    {ITT_LRFUNC, "sound effects:",        "pderjdst \'aatrns:",         M_RD_Change_SoundDevice,   0}, // Звуковые эффекты
    {ITT_LRFUNC, "music:",                "vepsrf:",                    M_RD_Change_MusicDevice,   0}, // Музыка
    {ITT_TITLE,  "quality",               "rfxtcndj pdexfybz",                 NULL,                       0}, // Качество звучания
    {ITT_LRFUNC, "sampling frequency:",   "xfcnjnf lbcrhtnbpfwbb:",     M_RD_Change_Sampling,      0}, // Частота дискретизации
    {ITT_TITLE,  "Miscellaneous",         "hfpyjt",                            NULL,                       0}, // Разное
    {ITT_LRFUNC, "sound effects mode:",   "Ht;bv pderjds[ \'aatrnjd:",  M_RD_Change_SndMode,       0}, // Режим звуковых эффектов
    {ITT_LRFUNC, "pitch-shifted sounds:", "ghjbpdjkmysq gbnx-ibanbyu:", M_RD_Change_PitchShifting, 0}, // Произвольный питч-шифтинг
    {ITT_LRFUNC, "mute inactive window:", "pder d ytfrnbdyjv jryt:",    M_RD_Change_MuteInactive,  0}  // Звук в неактивном окне
};

static Menu_t SoundSysMenu = {
    35, 35,
    25,
    "SOUND SYSTEM", "PDERJDFZ CBCNTVF", false, // ЗВУКОВАЯ СИСТЕМА
    9, SoundSysItems, false,
    M_RD_Draw_Audio_System,
    NULL, 0,
    &SoundMenu,
    1
};

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

static MenuItem_t ControlsItems[] = {
    {ITT_TITLE,   "Controls",               "eghfdktybt",                NULL,                       0}, // Управление
    {ITT_SETMENU, "keyboard bindings",      "yfcnhjqrb rkfdbfnehs",      &Bindings1Menu,              0}, // Настройки клавиатуры
    {ITT_SETMENU, "mouse bindings",         "yfcnhjqrb vsib",            &MouseBindingsMenu,         0}, // Настройки мыши
    {ITT_TITLE,   "mouse",                  "vsim",                      NULL,                       0}, // Мышь
    {ITT_LRFUNC,  "sensivity",              "crjhjcnm",                  M_RD_Change_Sensitivity,    0}, // Скорость
    {ITT_EMPTY,   NULL,                     NULL,                        NULL,                       0},
    {ITT_LRFUNC,  "acceleration",           "frctkthfwbz",               M_RD_Change_Acceleration,   0}, // Акселерация
    {ITT_EMPTY,   NULL,                     NULL,                        NULL,                       0},
    {ITT_LRFUNC,  "acceleration threshold", "gjhju frctkthfwbb",         M_RD_Change_Threshold,      0}, // Порог акселерации
    {ITT_EMPTY,   NULL,                     NULL,                        NULL,                       0},
    {ITT_LRFUNC,  "mouse look:",            "j,pjh vsim.:",              M_RD_Change_MouseLook,      0}, // Обзор мышью
    {ITT_LRFUNC,  "invert y axis:",         "dthnbrfkmyfz bydthcbz:",    M_RD_Change_InvertY,        0}, // Вертикальная инверсия
    {ITT_LRFUNC,  "vertical movement:",     "dthnbrfkmyjt gthtvtotybt:", M_RD_Change_Novert,         0}  // Вертикальное перемещение
};

static Menu_t ControlsMenu = {
    35, 35,
    25,
    "CONTROL SETTINGS", "EGHFDKTYBT", false, // УПРАВЛЕНИЕ
    13, ControlsItems, false,
    M_RD_Draw_Controls,
    NULL, 0,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (1)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings1Items[] = {
    {ITT_TITLE,   "Movement",      "ldb;tybt",        NULL,           0},
    {ITT_EFUNC,   "Move Forward",  "ldb;tybt dgthtl", M_RD_Bind_Key,  bk_forward},      // Движение вперед
    {ITT_EFUNC,   "Move Backward", "ldb;tybt yfpfl",  M_RD_Bind_Key,  bk_backward},     // Движение назад
    {ITT_EFUNC,   "Turn Left",     "gjdjhjn yfktdj",  M_RD_Bind_Key,  bk_turn_left},    // Поворот налево
    {ITT_EFUNC,   "Turn Right",    "gjdjhjn yfghfdj", M_RD_Bind_Key,  bk_turn_right},   // Поворот направо
    {ITT_EFUNC,   "Strafe Left",   ",jrjv dktdj",     M_RD_Bind_Key,  bk_strafe_left},  // Боком влево
    {ITT_EFUNC,   "Strafe Right",  ",jrjv dghfdj",    M_RD_Bind_Key,  bk_strafe_right}, // Боком вправо
    {ITT_EFUNC,   "Speed On",      ",tu",             M_RD_Bind_Key,  bk_speed},        // Бег
    {ITT_EFUNC,   "Strafe On",     "ldb;tybt ,jrjv",  M_RD_Bind_Key,  bk_strafe},       // Движение боком
    {ITT_TITLE,   "Action",        "ltqcndbt",        NULL,           0},
    {ITT_EFUNC,   "Fire/Attack",   "fnfrf*cnhtkm,f",  M_RD_Bind_Key,  bk_fire},         // Атака/стрельба
    {ITT_EFUNC,   "Use",           "bcgjkmpjdfnm",    M_RD_Bind_Key,  bk_use},          // Использовать
    {ITT_EMPTY,   NULL,            NULL,              NULL,           0},
    {ITT_SETMENU, NULL,            NULL,              &Bindings2Menu, 0},               // Далее >
    {ITT_SETMENU, NULL,            NULL,              &Bindings4Menu, 0},               // < Назад
    {ITT_EMPTY,   NULL,            NULL,              NULL,           0}
};

static Menu_t Bindings1Menu = {
    35, 35,
    25,
    "Keyboard bindings", "Yfcnhjqrb rkfdbfnehs", false, // Настройки клавиатуры
    16, Bindings1Items, false,
    M_RD_Draw_Bindings,
    BindingsMenuPages, 4,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (2)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings2Items[] = {
    {ITT_TITLE,   "Weapons",         "jhe;bt",            NULL,           0},
    {ITT_EFUNC,   "Weapon 1",        "jhe;bt 1",          M_RD_Bind_Key,  bk_weapon_1},    // Оружие 1
    {ITT_EFUNC,   "Weapon 2",        "jhe;bt 2",          M_RD_Bind_Key,  bk_weapon_2},    // Оружие 2
    {ITT_EFUNC,   "Weapon 3",        "jhe;bt 3",          M_RD_Bind_Key,  bk_weapon_3},    // Оружие 3
    {ITT_EFUNC,   "Weapon 4",        "jhe;bt 4",          M_RD_Bind_Key,  bk_weapon_4},    // Оружие 4
    {ITT_EFUNC,   "Weapon 5",        "jhe;bt 5",          M_RD_Bind_Key,  bk_weapon_5},    // Оружие 5
    {ITT_EFUNC,   "Weapon 6",        "jhe;bt 6",          M_RD_Bind_Key,  bk_weapon_6},    // Оружие 6
    {ITT_EFUNC,   "Weapon 7",        "jhe;bt 7",          M_RD_Bind_Key,  bk_weapon_7},    // Оружие 7
    {ITT_EFUNC,   "Weapon 8",        "jhe;bt 8",          M_RD_Bind_Key,  bk_weapon_8},    // Оружие 8
    {ITT_EFUNC,   "Previous weapon", "ghtlsleott jhe;bt", M_RD_Bind_Key,  bk_weapon_prev}, // Предыдущее оружие
    {ITT_EFUNC,   "Next weapon",     "cktle.ott jhe;bt",  M_RD_Bind_Key,  bk_weapon_next}, // Следующее оружие
    {ITT_EMPTY,   NULL,              NULL,                NULL,           0},
    {ITT_EMPTY,   NULL,              NULL,                NULL,           0},
    {ITT_SETMENU, NULL,              NULL,                &Bindings3Menu, 0},               // Далее >
    {ITT_SETMENU, NULL,              NULL,                &Bindings1Menu, 0},               // < Назад
    {ITT_EMPTY,   NULL,              NULL,                NULL,           0}
};

static Menu_t Bindings2Menu = {
    35, 35,
    25,
    "Keyboard bindings", "Yfcnhjqrb rkfdbfnehs", false, // Настройки клавиатуры
    16, Bindings2Items, false,
    M_RD_Draw_Bindings,
    BindingsMenuPages, 4,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (3)
// -----------------------------------------------------------------------------


static MenuItem_t Bindings3Items[] = {
    {ITT_TITLE,   "Shortcut keys",         ",scnhsq ljcneg",        NULL,           0},
    {ITT_EFUNC,   "Quick save",            ",scnhjt cj[hfytybt",    M_RD_Bind_Key,  bk_save},             // Быстрое сохранение
    {ITT_EFUNC,   "Quick load",            ",scnhfz pfuheprf",      M_RD_Bind_Key,  bk_load},             // Быстрая загрузка
    {ITT_EFUNC,   "Go to next level",      "cktle.obq ehjdtym",     M_RD_Bind_Key,  bk_nextlevel},        // Следующий уровень
    {ITT_EFUNC,   "Restart level/demo",    "gthtpfgecr ehjdyz",     M_RD_Bind_Key,  bk_reloadlevel},      // Перезапуск уровня
    {ITT_EFUNC,   "Save a screenshot",     "crhbyijn",              M_RD_Bind_Key,  bk_screenshot},       // Скриншот
    {ITT_EFUNC,   "Finish demo recording", "pfrjyxbnm pfgbcm ltvj", M_RD_Bind_Key,  bk_finish_demo},      // Закончить запись демо
    {ITT_TITLE,   "Toggleables",           "gthtrk.xtybt",          NULL,           0},
    {ITT_EFUNC,   "Mouse look",            "j,pjh vsim.",           M_RD_Bind_Key,  bk_toggle_mlook},     // Обзор мышью
    {ITT_EFUNC,   "Always run",            "gjcnjzyysq ,tu",        M_RD_Bind_Key,  bk_toggle_autorun},   // Постоянный бег
    {ITT_EFUNC,   "Crosshair",             "ghbwtk",                M_RD_Bind_Key,  bk_toggle_crosshair}, // Прицел
    {ITT_EFUNC,   "Level flipping",        "pthrfkbhjdfybt ehjdyz", M_RD_Bind_Key,  bk_toggle_fliplvls},  // Зеркалирование уровня
    {ITT_EMPTY,   NULL,                    NULL,                    NULL,           0},
    {ITT_SETMENU, NULL,                    NULL,                    &Bindings4Menu, 0},                   // Далее >
    {ITT_SETMENU, NULL,                    NULL,                    &Bindings2Menu, 0},                   // < Назад
    {ITT_EMPTY,   NULL,                    NULL,                    NULL,           0}
};

static Menu_t Bindings3Menu = {
    35, 35,
    25,
    "Keyboard bindings", "Yfcnhjqrb rkfdbfnehs", false, // Настройки клавиатуры
    16, Bindings3Items, false,
    M_RD_Draw_Bindings,
    BindingsMenuPages, 4,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Key bindings (4)
// -----------------------------------------------------------------------------

static MenuItem_t Bindings4Items[] = {
    {ITT_TITLE,   "Automap",          "rfhnf",             NULL,           0},
    {ITT_EFUNC,   "Toggle automap",   "jnrhsnm rfhne",     M_RD_Bind_Key,  bk_map_toggle},    // Открыть карту
    {ITT_EFUNC,   "Zoom in",          "ghb,kbpbnm",        M_RD_Bind_Key,  bk_map_zoom_in},   // Приблизить
    {ITT_EFUNC,   "Zoom out",         "jnlfkbnm",          M_RD_Bind_Key,  bk_map_zoom_out},  // Отдалить
    {ITT_EFUNC,   "Maximum zoom out", "gjkysq vfcinf,",    M_RD_Bind_Key,  bk_map_zoom_max},  // Полный масштаб
    {ITT_EFUNC,   "Follow mode",      "ht;bv cktljdfybz",  M_RD_Bind_Key,  bk_map_follow},    // Режим следования
    {ITT_EFUNC,   "Overlay mode",     "ht;bv yfkj;tybz",   M_RD_Bind_Key,  bk_map_overlay},   // Режим наложения
    {ITT_EFUNC,   "Rotate mode",      "ht;bv dhfotybz",    M_RD_Bind_Key,  bk_map_rotate},    // Режим вращения
    {ITT_EFUNC,   "Toggle grid",      "ctnrf",             M_RD_Bind_Key,  bk_map_grid},      // Сетка
    {ITT_EFUNC,   "Mark location",    "gjcnfdbnm jnvtnre", M_RD_Bind_Key,  bk_map_mark},      // Поставить отметку
    {ITT_EFUNC,   "Clear all marks",  "e,hfnm jnvtnrb",    M_RD_Bind_Key,  bk_map_clearmark}, // Убрать отметки
    {ITT_EMPTY,   NULL,               NULL,                NULL,           0},
    {ITT_EMPTY,   NULL,               NULL,                NULL,           0},
    {ITT_SETMENU, NULL,               NULL,                &Bindings1Menu, 0},                // Далее >
    {ITT_SETMENU, NULL,               NULL,                &Bindings3Menu, 0},                // < Назад
    {ITT_EMPTY,   NULL,               NULL,                NULL,           0}
};

static Menu_t Bindings4Menu = {
    35, 35,
    25,
    "Keyboard bindings", "Yfcnhjqrb rkfdbfnehs", false, // Настройки клавиатуры
    16, Bindings4Items, false,
    M_RD_Draw_Bindings,
    BindingsMenuPages, 4,
    &ControlsMenu,
    1
};


// -----------------------------------------------------------------------------
// Mouse bindings
// -----------------------------------------------------------------------------

static MenuItem_t MouseBindingsItems[] = {
    {ITT_TITLE, "Buttons",         "ryjgrb",            NULL,                          0}, // Кнопки
    {ITT_EFUNC, "Fire/Attack",     "fnfrf*cnhtkm,f",    M_RD_Mouse_Bind_FireAttack,   0}, // Атака/стрельба
    {ITT_EFUNC, "Use",             "bcgjkmpjdfnm",      M_RD_Mouse_Bind_Use,          0}, // Использовать
    {ITT_EFUNC, "Move Forward",    "ldb;tybt dgthtl",   M_RD_Mouse_Bind_MoveForward,  0}, // Движение вперед
    {ITT_EFUNC, "Move Backward",   "ldb;tybt yfpfl",    M_RD_Mouse_Bind_MoveBackward, 0}, // Движение назад
    {ITT_EFUNC, "Strafe On",       "ldb;tybt ,jrjv",    M_RD_Mouse_Bind_StrafeOn,     0}, // Движение боком
    {ITT_EFUNC, "Strafe Left",     ",jrjv dktdj",       M_RD_Mouse_Bind_StrafeLeft,   0}, // Боком влево
    {ITT_EFUNC, "Strafe Right",    ",jrjv dghfdj",      M_RD_Mouse_Bind_StrafeRight,  0}, // Боком вправо
    {ITT_EFUNC, "Previous Weapon", "ghtlsleott jhe;bt", M_RD_Mouse_Bind_PrevWeapon,   0}, // Предыдущее оружие
    {ITT_EFUNC, "Next Weapon",     "cktle.ott jhe;bt",  M_RD_Mouse_Bind_NextWeapon,   0}  // Следующее оружие
};

static Menu_t MouseBindingsMenu = {
    35, 35,
    25,
    "Mouse bindings", "Yfcnhjqrb vsib", false, // Настройки мыши
    10, MouseBindingsItems, false,
    M_RD_Draw_Mouse_Bindings,
    NULL, 0,
    &ControlsMenu,
    1
};

// -----------------------------------------------------------------------------
// Gameplay enhancements
// -----------------------------------------------------------------------------

static MenuItem_t Gameplay1Items[] = {
    {ITT_TITLE,   "Graphical",                    "uhfabrf",                        NULL,                       0}, // Графика
    {ITT_LRFUNC,  "Brightmaps:",                  ",hfqnvfggbyu:",                  M_RD_Change_Brightmaps,      0}, // Брайтмаппинг
    {ITT_LRFUNC,  "Fake contrast:",               "Bvbnfwbz rjynhfcnyjcnb:",        M_RD_Change_FakeContrast,    0}, // Имитация контрастности
    {ITT_LRFUNC,  "Translucency:",                "Ghjphfxyjcnm j,]trnjd:",         M_RD_Change_Translucency,    0}, // Прозрачность объектов
    {ITT_LRFUNC,  "Fuzz effect:",                 "\'aatrn ievf:",                  M_RD_Change_ImprovedFuzz,    0}, // Эффект шума
    {ITT_LRFUNC,  "Colored blood and corpses:",   "Hfpyjwdtnyfz rhjdm b nhegs:",    M_RD_Change_ColoredBlood,    0}, // Разноцветная кровь и трупы
    {ITT_LRFUNC,  "Swirling liquids:",            "ekexityyfz fybvfwbz ;blrjcntq:", M_RD_Change_SwirlingLiquids, 0}, // Улучшенная анимация жидкостей
    {ITT_LRFUNC,  "Invulnerability affects sky:", "ytezpdbvjcnm jrhfibdftn yt,j:",  M_RD_Change_InvulSky,        0}, // Неуязвимость окрашивает небо
    {ITT_LRFUNC,  "Sky drawing mode:",            "ht;bv jnhbcjdrb yt,f:",          M_RD_Change_LinearSky,       0}, // Режим отрисовки неба
    {ITT_LRFUNC,  "Flip weapons:",                "pthrfkmyjt jnhf;tybt jhe;bz:",   M_RD_Change_FlipWeapons,     0}, // Зеркальное отражение оружия
    {ITT_EMPTY,   NULL,                           NULL,                      NULL,                             0},
    {ITT_EMPTY,   NULL,                           NULL,                      NULL,                             0},
    {ITT_SETMENU, NULL, /* Next Page > */         NULL,                             &Gameplay2Menu,               0}, // Далее >
    {ITT_SETMENU, NULL, /* < Last Page */         NULL,                             &Gameplay5Menu,               0}  // < Назад
};

static Menu_t Gameplay1Menu = {
    35, 35,
    25,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    15, Gameplay1Items, false,
    M_RD_Draw_Gameplay_1,
    GameplayMenuPages, 5,
    &OptionsMenu,
    1
};

static MenuItem_t Gameplay2Items[] = {
    {ITT_TITLE,  "Status bar",            "cnfnec-,fh",                  NULL,                             0}, // Статус-бар
    {ITT_LRFUNC, "Extra player faces:",   "Ljgjkybntkmyst kbwf buhjrf:", M_RD_Change_ExtraPlayerFaces,  0}, // Дополнительные лица игрока
    {ITT_LRFUNC, "Show negative health:", "jnhbwfntkmyjt pljhjdmt:",     M_RD_Change_NegativeHealth,    0}, // Отрицательное здоровье
    {ITT_LRFUNC, "Colored elements:",     "Hfpyjwdtnyst 'ktvtyns:",      M_RD_Change_SBarColored,       0}, // Разноцветные элементы
    {ITT_TITLE,  "Coloring",              "wdtnf",                       NULL,                             0}, // Цвета
    {ITT_LRFUNC, NULL,                    NULL,                          M_RD_Change_SBarHighValue,     0}, // Высокое значение
    {ITT_LRFUNC, NULL,                    NULL,                          M_RD_Change_SBarNormalValue,   0}, // Нормальное значение
    {ITT_LRFUNC, NULL,                    NULL,                          M_RD_Change_SBarLowValue,      0}, // Низкое значение
    {ITT_LRFUNC, NULL,                    NULL,                          M_RD_Change_SBarCriticalValue, 0}, // Критическое значение
    {ITT_LRFUNC, NULL,                    NULL,                          M_RD_Change_SBarArmorType1,    0}, // Тип брони 1
    {ITT_LRFUNC, NULL,                    NULL,                          M_RD_Change_SBarArmorType2,    0}, // Тип брони 2
    {ITT_LRFUNC, NULL,                    NULL,                          M_RD_Change_SBarArmorType0,    0}, // Отсутствие брони
    {ITT_SETMENU,NULL, /* Next Page > */  NULL,                          &Gameplay3Menu,                0}, // Далее >
    {ITT_SETMENU,NULL, /* < Prev page */  NULL,                          &Gameplay1Menu,                0}  // < Назад
};

static Menu_t Gameplay2Menu = {
    35, 35,
    25,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    14, Gameplay2Items, false,
    M_RD_Draw_Gameplay_2,
    GameplayMenuPages, 5,
    &OptionsMenu,
    1
};

static MenuItem_t Gameplay3Items[] = {
    {ITT_TITLE,   "Audible",                         "Pder",                            NULL,                      0}, // Звук
    {ITT_LRFUNC,  "Sound attenuation axises:",       "pfne[fybt pderf gj jczv:",      M_RD_Change_ZAxisSfx,        0}, // Затухание звука по осям
    {ITT_LRFUNC,  "Play exit sounds:",               "Pderb ghb ds[jlt bp buhs:",     M_RD_Change_ExitSfx,         0}, // Звук при выходе из игры
    {ITT_LRFUNC,  "Sound of crushing corpses:",      "Pder hfplfdkbdfybz nhegjd:",    M_RD_Change_CrushingSfx,     0}, // Звук раздавливания трупов
    {ITT_LRFUNC,  "Single sound of blazing door:",   "Jlbyjxysq pder ,scnhjq ldthb:", M_RD_Change_BlazingSfx,      0}, // Одиночный звук быстрой двери
    {ITT_LRFUNC,  "Monster alert waking up others:", "J,ofz nhtdjuf e vjycnhjd:",     M_RD_Change_AlertSfx,        0}, // Общая тревога у монстров
    {ITT_TITLE,   "Tactical",                        "Nfrnbrf",                            NULL,                   0}, // Тактика
    {ITT_LRFUNC,  "Notify of revealed secrets:",     "Cjj,ofnm j yfqltyyjv nfqybrt:", M_RD_Change_SecretNotify,    0}, // Сообщать о найденном тайнике
    {ITT_LRFUNC,  "Infragreen light amp. visor:",    "Byahfptktysq dbpjh jcdtotybz:", M_RD_Change_InfraGreenVisor, 0}, // Инфразеленый визор освещения
    {ITT_EMPTY,   NULL,                              NULL,                            NULL,                        0},
    {ITT_EMPTY,   NULL,                              NULL,                            NULL,                        0},
    {ITT_EMPTY,   NULL,                              NULL,                            NULL,                        0},
    {ITT_SETMENU, NULL, /* Next page >   */          NULL,                            &Gameplay4Menu,              0}, // Далее >
    {ITT_SETMENU, NULL, /* < Prev page > */          NULL,                            &Gameplay2Menu,              0}  // < Назад
};

static Menu_t Gameplay3Menu = {
    35, 35,
    25,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    14, Gameplay3Items, false,
    M_RD_Draw_Gameplay_3,
    GameplayMenuPages, 5,
    &OptionsMenu,
    1
};

static MenuItem_t Gameplay4Items[] = {
    {ITT_TITLE,   "Physical",                                  "Abpbrf",                              NULL,                       0}, // Физика
    {ITT_LRFUNC,  "Walk over and under monsters:",       "Gthtvtotybt gjl*yfl vjycnhfvb:",  M_RD_Change_WalkOverUnder,  0}, // Перемещение над/под монстрами
    {ITT_LRFUNC,  "Corpses sliding from the ledges:",    "Nhegs cgjkpf.n c djpdsitybq:",    M_RD_Change_Torque,         0}, // Трупы сползают с возвышений
    {ITT_LRFUNC,  "Weapon bobbing while firing:",        "Ekexityyjt gjrfxbdfybt jhe;bz:",  M_RD_Change_Bobbing,        0}, // Улучшенное покачивание оружия
    {ITT_LRFUNC,  "Lethal pellet of a point-blank SSG:", "ldecndjkrf hfphsdftn dhfujd:",    M_RD_Change_SSGBlast,       0}, // Двустволка разрывает врагов
    {ITT_LRFUNC,  "Randomly mirrored corpses:",          "pthrfkbhjdfybt nhegjd:",          M_RD_Change_FlipCorpses,    0}, // Зеркалирование трупов
    {ITT_LRFUNC,  "Floating powerups:",                  "Ktdbnbhe.obt caths-fhntafrns:",   M_RD_Change_FloatPowerups,  0}, // Левитирующие сферы-артефакты
    {ITT_LRFUNC,  "Items are tossed when dropped:",      "Gjl,hfcsdfnm dsgfdibt ghtlvtns:", M_RD_Change_TossDrop,       0}, // Подбрасывать выпавшие предметы
    {ITT_TITLE,   "Crosshair",                           "Ghbwtk",                          NULL,                       0}, // Прицел
    {ITT_LRFUNC,  "Draw crosshair:",                     "Jnj,hf;fnm ghbwtk:",              M_RD_Change_CrosshairDraw,  0}, // Отображать прицел
    {ITT_LRFUNC,  "Indication:",                         "Bylbrfwbz:",                      M_RD_Change_CrosshairType,  0}, // Индикация
    {ITT_LRFUNC,  "Increased size:",                     "Edtkbxtyysq hfpvth:",             M_RD_Change_CrosshairScale, 0}, // Увеличенный размер
    {ITT_SETMENU, NULL, /* Next page >   */              NULL,                              &Gameplay5Menu,             0}, // Далее >
    {ITT_SETMENU, NULL, /* < Prev page > */              NULL,                              &Gameplay3Menu,             0}  // < Назад
};

static Menu_t Gameplay4Menu = {
    35, 35,
    25,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    14, Gameplay4Items, false,
    M_RD_Draw_Gameplay_4,
    GameplayMenuPages, 5,
    &OptionsMenu,
    1
};

static MenuItem_t Gameplay5Items[] = {
    {ITT_TITLE,   "Gameplay",                            "Utqvgktq",                        NULL,                        0}, // Геймплей
    {ITT_LRFUNC,  "Fix errors of vanilla maps:",         "ecnhfyznm jib,rb jhbu> ehjdytq:", M_RD_Change_FixMapErrors,    0}, // Устранять ошибки ориг. уровней
    {ITT_LRFUNC,  "Flip game levels:",                   "pthrfkmyjt jnhf;tybt ehjdytq:",   M_RD_Change_FlipLevels,      0}, // Зеркальное отражение уровней
    {ITT_LRFUNC,  "Pain Elemental without Souls limit:", "'ktvtynfkm ,tp juhfybxtybz lei:", M_RD_Change_LostSoulsQty,    0}, // Элементаль без ограничения душ
    {ITT_LRFUNC,  "More aggressive lost souls:",         "gjdsityyfz fuhtccbdyjcnm lei:",   M_RD_Change_LostSoulsAgr,    0}, // Повышенная агрессивность душ
    {ITT_LRFUNC,  "Pistol start game mode:",             NULL, /*[JN] Joint EN/RU string*/  M_RD_Change_PistolStart,     0}, // Режим игры "Pistol start"
    {ITT_TITLE,   "Demos",                               "Ltvjpfgbcb",                      NULL,                        0}, // Демозаписи
    {ITT_LRFUNC,  "Show demo timer:",                    "jnj,hf;fnm nfqvth:",              M_RD_Change_DemoTimer,       0}, // Отображать таймер
    {ITT_LRFUNC,  "timer direction:",                    "dhtvz nfqvthf:",                  M_RD_Change_DemoTimerDir,    0}, // Время таймера
    {ITT_LRFUNC,  "Show progress bar:",                  "irfkf ghjuhtccf:",                M_RD_Change_DemoBar,         0}, // Шкала прогресса
    {ITT_LRFUNC,  "Play internal demos:",                "Ghjbuhsdfnm ltvjpfgbcb:",         M_RD_Change_NoInternalDemos, 0}, // Проигрывать демозаписи
    {ITT_EMPTY,   NULL,                                  NULL,                              NULL,                        0},
    {ITT_SETMENU, NULL, /* First page > */               NULL,                              &Gameplay1Menu,              0}, // Далее >
    {ITT_SETMENU, NULL, /* < Prev page > */              NULL,                              &Gameplay4Menu,              0}  // < Назад
};

static Menu_t Gameplay5Menu = {
    35, 35,
    25,
    "GAMEPLAY FEATURES", "YFCNHJQRB UTQVGKTZ", false, // НАСТРОЙКИ ГЕЙМПЛЕЯ
    14, Gameplay5Items, false,
    M_RD_Draw_Gameplay_5,
    GameplayMenuPages, 5,
    &OptionsMenu,
    1
};

// -----------------------------------------------------------------------------
// Level select
// -----------------------------------------------------------------------------

static MenuItem_t LevelSelect1Items[] = {
    {ITT_LRFUNC,  "skill level",      "ckj;yjcnm",          M_RD_Change_Selective_Skill,        0}, // Сложность
    {ITT_LRFUNC,  "episode",          "\'gbpjl",            M_RD_Change_Selective_Episode,      0}, // Эпизод
    {ITT_LRFUNC,  "map",              "ehjdtym",            M_RD_Change_Selective_Map,          0}, // Уровень
    {ITT_TITLE,   "PLAYER",           "buhjr",              NULL,                               0}, // Игрок
    {ITT_LRFUNC,  "health",           "pljhjdmt",           M_RD_Change_Selective_Health,       0}, // Здоровье
    {ITT_LRFUNC,  "armor",            ",hjyz",              M_RD_Change_Selective_Armor,        0}, // Броня
    {ITT_LRFUNC,  "armor type",       "nbg ,hjyb",          M_RD_Change_Selective_ArmorType,    0}, // Тип брони
    {ITT_TITLE,   "WEAPONS",          "jhe;bt",             NULL,                               0}, // Оружие
    {ITT_LRFUNC,  "chainsaw",         ",typjgbkf",          M_RD_Change_Selective_WP_Chainsaw,  0}, // Бензопила
    {ITT_LRFUNC,  "shotgun",          "he;mt",              M_RD_Change_Selective_WP_Shotgun,   0}, // Ружье
    {ITT_LRFUNC,  "super shotgun",    "ldecndjkmyjt he;mt", M_RD_Change_Selective_WP_SSgun,     0}, // Двуствольное ружье
    {ITT_LRFUNC,  "chaingun",         "gektvtn",            M_RD_Change_Selective_WP_Chaingun,  0}, // Пулемет
    {ITT_LRFUNC,  "rocket launcher",  "hfrtnybwf",          M_RD_Change_Selective_WP_RLauncher, 0}, // Ракетница
    {ITT_LRFUNC,  "plasmagun",        "gkfpvtyyfz geirf",   M_RD_Change_Selective_WP_Plasmagun, 0}, // Плазменная пушка
    {ITT_LRFUNC,  "bfg9000",          "&9000",              M_RD_Change_Selective_WP_BFG9000,   0}, // BFG9000
    {ITT_EMPTY,   NULL,               NULL,                 NULL,                               0},
    {ITT_SETMENU, NULL,               NULL,                 &LevelSelect2Menu,                  0},
    {ITT_EFUNC,   NULL,               NULL,                 G_DoSelectiveGame,                  0}
};

static Menu_t LevelSelect1Menu = {
    75, 72,
    20,
    "LEVEL SELECT", "DS<JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    18, LevelSelect1Items, false,
    M_RD_Draw_Level_1,
    LevelSelectMenuPages, 2,
    &OptionsMenu,
    0
};

static MenuItem_t LevelSelect2Items[] = {
    {ITT_LRFUNC,  "backpack",            "h.rpfr",             M_RD_Change_Selective_Backpack, 0}, // Рюкзак
    {ITT_LRFUNC,  "bullets",             "gekb",               M_RD_Change_Selective_Ammo_0,   0}, // Пули
    {ITT_LRFUNC,  "shells",              "lhj,m",              M_RD_Change_Selective_Ammo_1,   0}, // Дробь
    {ITT_LRFUNC,  "rockets",             "hfrtns",             M_RD_Change_Selective_Ammo_3,   0}, // Ракеты
    {ITT_LRFUNC,  "cells",               "\'ythubz",           M_RD_Change_Selective_Ammo_2,   0}, // Энергия
    {ITT_TITLE,   "KEYS",                "rk.xb",              NULL,                           0}, // Ключи
    {ITT_LRFUNC,  "blue keycard",        "cbyzz rk.x-rfhnf",   M_RD_Change_Selective_Key_0,    0}, // Синяя ключ-карта
    {ITT_LRFUNC,  "yellow keycard",      ";tknfz rk.x-rfhnf",  M_RD_Change_Selective_Key_1,    0}, // Желтая ключ-карта
    {ITT_LRFUNC,  "red keycard",         "rhfcyfz rk.x-rfhnf", M_RD_Change_Selective_Key_2,    0}, // Красная ключ-карта
    {ITT_LRFUNC,  "blue skull key",      "cbybq rk.x-xthtg",   M_RD_Change_Selective_Key_3,    0}, // Синий ключ-череп
    {ITT_LRFUNC,  "yellow skull key",    ";tknsq rk.x-xthtg",  M_RD_Change_Selective_Key_4,    0}, // Желтый ключ-череп
    {ITT_LRFUNC,  "red skull key",       "rhfcysq rk.x-xthtg", M_RD_Change_Selective_Key_5,    0}, // Красный ключ-череп
    {ITT_TITLE,   "EXTRA",               "vjycnhs",            NULL,                           0}, // Монстры
    {ITT_LRFUNC,  "fast monsters",       "ecrjhtyyst",         M_RD_Change_Selective_Fast,     0}, // Ускоренные
    {ITT_LRFUNC,  "respawning monsters", "djcrhtif.obtcz",     M_RD_Change_Selective_Respawn,  0}, // Воскрешающиеся
    {ITT_EMPTY,   NULL,                  NULL,                 NULL,                           0},
    {ITT_SETMENU, NULL,                  NULL,                 &LevelSelect1Menu,              0},
    {ITT_EFUNC,   NULL,                  NULL,                 G_DoSelectiveGame,              0}
};

static Menu_t LevelSelect2Menu = {
    75, 72,
    20,
    "LEVEL SELECT", "DS<JH EHJDYZ", false, // ВЫБОР УРОВНЯ
    18, LevelSelect2Items, false,
    M_RD_Draw_Level_2,
    LevelSelectMenuPages, 2,
    &OptionsMenu,
    0
};

// -----------------------------------------------------------------------------
// Reset settings
// -----------------------------------------------------------------------------

static MenuItem_t ResetItems[] = {
    {ITT_EFUNC, "Recommended", "Htrjvtyljdfyysq", M_RD_BackToDefaults_Recommended, 0},
    {ITT_EFUNC, "Original",    "Jhbubyfkmysq",    M_RD_BackToDefaults_Original,    0}
};

static Menu_t ResetMenu = {
    115, 98,
    95,
    NULL, NULL, false,
    2, ResetItems, false,
    M_RD_Draw_Reset,
    NULL, 0,
    &OptionsMenu,
    0
};

// =============================================================================
// LOAD GAME MENU
// =============================================================================

static MenuItem_t LoadItems[] = {
    {ITT_EFUNC, NULL, NULL, M_LoadSelect, 0},
    {ITT_EFUNC, NULL, NULL, M_LoadSelect, 1},
    {ITT_EFUNC, NULL, NULL, M_LoadSelect, 2},
    {ITT_EFUNC, NULL, NULL, M_LoadSelect, 3},
    {ITT_EFUNC, NULL, NULL, M_LoadSelect, 4},
    {ITT_EFUNC, NULL, NULL, M_LoadSelect, 5},
    {ITT_EFUNC, NULL, NULL, M_LoadSelect, 6},
    {ITT_EFUNC, NULL, NULL, M_LoadSelect, 7}
};

static Menu_t LoadMenu = {
    67, 67,
    37,
    NULL, NULL, false,
    8, LoadItems, true,
    M_DrawLoad,
    NULL, 0,
    &DoomMenu,
    0
};

// =============================================================================
// SAVE GAME MENU
// =============================================================================

static MenuItem_t SaveItems[] = {
    {ITT_EFUNC, NULL, NULL, M_SaveSelect,  0},
    {ITT_EFUNC, NULL, NULL, M_SaveSelect,  1},
    {ITT_EFUNC, NULL, NULL, M_SaveSelect,  2},
    {ITT_EFUNC, NULL, NULL, M_SaveSelect,  3},
    {ITT_EFUNC, NULL, NULL, M_SaveSelect,  4},
    {ITT_EFUNC, NULL, NULL, M_SaveSelect,  5},
    {ITT_EFUNC, NULL, NULL, M_SaveSelect,  6},
    {ITT_EFUNC, NULL, NULL, M_SaveSelect,  7}
};

static Menu_t SaveMenu = {
    67, 67,
    37,
    NULL, NULL, false,
    8, SaveItems, true,
    M_DrawSave,
    NULL, 0,
    &DoomMenu,
    0
};

// =============================================================================
// [JN] VANILLA OPTIONS MENU
// =============================================================================

static MenuItem_t OptionsItems_Vanilla[] = {
    {ITT_EFUNC,   "eM_ENDGAM", "pRD_ENDGM", M_EndGame,               0}, // Закончить игру
    {ITT_EFUNC,   "mM_MESSG",  "cRD_MESSG", M_RD_Change_Messages,    0}, // Сообщения
    {ITT_EFUNC,   "gM_DETAIL", "lRD_DETL",  M_RD_Change_Detail,      0}, // Детализация:
    {ITT_LRFUNC,  "sM_SCRNSZ", "hRD_SCRSZ", M_RD_Change_ScreenSize,  0}, // Размер экрана
    {ITT_EMPTY,   NULL,       NULL,       NULL,                    0},
    {ITT_LRFUNC,  "mM_MSENS",  "cRD_MSENS", M_RD_Change_Sensitivity, 0}, // Скорость мыши
    {ITT_EMPTY,   NULL,       NULL,       NULL,                    0},
    {ITT_SETMENU, "sM_SVOL",   "uRD_SVOL",  &Options2Menu_Vanilla,   0}  // Громкость
};

static Menu_t OptionsMenu_Vanilla = {
    60, 60,
    37,
    NULL, NULL, true,
    8, OptionsItems_Vanilla, true,
    M_Vanilla_DrawOptions,
    NULL, 0,
    &DoomMenu,
    0
};

// -----------------------------------------------------------------------------
// Sound Menu
// -----------------------------------------------------------------------------

static MenuItem_t Options2Items_Vanilla[] = {
    {ITT_LRFUNC, "sM_SFXVOL", "pRD_SFXVL", M_RD_Change_SfxVol,   0}, // Звук
    {ITT_EMPTY,  NULL,       NULL,       NULL,                 0},
    {ITT_LRFUNC, "mM_MUSVOL", "vRD_MUSVL", M_RD_Change_MusicVol, 0}  // Музыка
};

static Menu_t Options2Menu_Vanilla = {
    80, 80,
    64,
    NULL, NULL, true,
    3, Options2Items_Vanilla, true,
    M_Vanilla_DrawSound,
    NULL, 0,
    &OptionsMenu_Vanilla,
    0
};

// =============================================================================
// [JN] VANILLA OPTIONS MENU: DRAWING
// =============================================================================

void M_Vanilla_DrawOptions(void)
{
    if (english_language)
    {
    // - "OPTIONS" title -------------------------------------------------------
    V_DrawPatch(108 + wide_delta, 15, 
                W_CacheLumpName(DEH_String("M_OPTTTL"), PU_CACHE));

    // - Messages --------------------------------------------------------------
    V_DrawPatch(179 + wide_delta, 53, 
                W_CacheLumpName(DEH_String(showMessages == 1 ?
                                           "M_MSGON" : "M_MSGOFF"), PU_CACHE));

    // - Graphic Detail --------------------------------------------------------
    V_DrawPatch(235 + wide_delta, 69, 
                W_CacheLumpName(DEH_String(detailLevel == 1 ?
                                           "M_GDLOW" : "M_GDHIGH"), PU_CACHE));
    }
    else
    {
    // - "НАСТРОЙКИ" title -----------------------------------------------------
    M_WriteTextBigCentered_RUS(15, "YFCNHJQRB");

    V_DrawPatch(180 + wide_delta, 53,
                W_CacheLumpName(DEH_String(showMessages == 1 ?
                "RD_MSGON" : "RD_MSGOF"), PU_CACHE));

    V_DrawPatch(235 + wide_delta, 69,
                W_CacheLumpName(DEH_String(detailLevel == 1 ?
                "RD_GDL" : "RD_GDH"), PU_CACHE));

    }

    // - Screen size slider ----------------------------------------------------
    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: only 6 sizes are available
        RD_Menu_DrawSlider(&OptionsMenu_Vanilla, 102, 6, screenblocks - 9);
    }
    else
    {
        RD_Menu_DrawSlider(&OptionsMenu_Vanilla, 102, 12, screenblocks - 3);
    }

    // - Mouse sensivity slider ------------------------------------------------
    RD_Menu_DrawSlider(&OptionsMenu_Vanilla, 134, 10, mouseSensitivity);
}

void M_Vanilla_DrawSound(void)
{
    if (english_language)
    {
    // - "Sound volume" title --------------------------------------------------
    V_DrawPatch (60 + wide_delta, 38, 
                 W_CacheLumpName(DEH_String("M_SVOL"), PU_CACHE));
    }
    else
    {
    // - "ГРОМКОСТЬ" title -----------------------------------------------------
    M_WriteTextBigCentered_RUS(38, "UHJVRJCNM");
    }

    // - Sfx volume slider -----------------------------------------------------
    RD_Menu_DrawSlider(&Options2Menu_Vanilla, 81, 16, sfxVolume);

    // - Music volume slider ---------------------------------------------------
    RD_Menu_DrawSlider(&Options2Menu_Vanilla, 113, 16, musicVolume);
}

// =============================================================================
// [JN] NEW OPTIONS MENU: DRAWING
// =============================================================================

// -----------------------------------------------------------------------------
// Rendering
// -----------------------------------------------------------------------------

void M_RD_Draw_Rendering(void)
{
    // [JN] Jaguar Doom: clear remainings of bottom strings from the status bar.
    if (gamemission == jaguar)
        inhelpscreens = true;

    if (english_language)
    {
        // Widescreen rendering
        RD_M_DrawTextSmallENG(aspect_ratio_temp == 1 ? "5:4" :
                              aspect_ratio_temp == 2 ? "16:9" :
                              aspect_ratio_temp == 3 ? "16:10" :
                              aspect_ratio_temp == 4 ? "21:9" : "4:3", 185 + wide_delta, 35, CR_NONE);
        // Informative message
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_WHITE];
            M_WriteTextSmallCentered_ENG(156, "Program must be restarted");
            dp_translation = NULL;
        }

        // Vertical synchronization
        if (force_software_renderer == 1)
        {
            RD_M_DrawTextSmallENG("n/a", 216 + wide_delta, 45, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(vsync ? "on" : "off", 216 + wide_delta, 45, CR_NONE);
        }

        // Frame rate
        RD_M_DrawTextSmallENG(uncapped_fps ? "uncapped" : "35 fps", 120 + wide_delta, 55, CR_NONE);

        // Show FPS counter
        RD_M_DrawTextSmallENG(show_fps ? "on" : "off", 162 + wide_delta, 65, CR_NONE);

        // Pixel scaling
        if (force_software_renderer == 1)
        {
            RD_M_DrawTextSmallENG("n/a", 135 + wide_delta, 75, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(smoothing ? "smooth" : "sharp", 135 + wide_delta, 75, CR_NONE);
        }

        // Porch palette changing
        RD_M_DrawTextSmallENG(vga_porch_flash ? "on" : "off", 207 + wide_delta, 85, CR_NONE);

        // Video renderer
        RD_M_DrawTextSmallENG(force_software_renderer ? "software (cpu)" : "hardware (gpu)", 146 + wide_delta, 95, CR_NONE);

        // Show disk icon
        RD_M_DrawTextSmallENG(show_diskicon ? "on" : "off", 138 + wide_delta, 115, CR_NONE);

        // Screen wiping effect
        RD_M_DrawTextSmallENG(screen_wiping == 1 ? "standard" :
                              screen_wiping == 2 ? "loading" :
                              "off", 187 + wide_delta, 125, CR_NONE);

        // Screenshot format
        RD_M_DrawTextSmallENG(png_screenshots ? "png" : "pcx", 174 + wide_delta, 135, CR_NONE);

        // Show ENDOOM screen
        RD_M_DrawTextSmallENG(show_endoom ? "on" : "off", 179 + wide_delta, 145, CR_NONE);
    }
    else
    {
        // Широкоформатный режим
        RD_M_DrawTextSmallRUS(aspect_ratio_temp == 1 ? "5:4" :
                              aspect_ratio_temp == 2 ? "16:9" :
                              aspect_ratio_temp == 3 ? "16:10" :
                              aspect_ratio_temp == 4 ? "21:9" :
                              "4:3", 238 + wide_delta, 35, CR_NONE);

        // Informative message: Необходим перезапуск программы
        if (aspect_ratio_temp != aspect_ratio)
        {
            dp_translation = cr[CR_WHITE];
            M_WriteTextSmallCentered_RUS(156, "ytj,[jlbv gthtpfgecr ghjuhfvvs");
            dp_translation = NULL;
        }

        // Вертикальная синхронизация
        if (force_software_renderer == 1)
        {
            RD_M_DrawTextSmallRUS("y*l", 249 + wide_delta, 45, CR_DARKRED); // Н/Д
        }
        else
        {
            RD_M_DrawTextSmallRUS(vsync ? "drk" : "dsrk", 249 + wide_delta, 45, CR_NONE);
        }

        // Кадровая частота
        if (uncapped_fps)
        {
            RD_M_DrawTextSmallRUS(",tp juhfybxtybz", 167 + wide_delta, 55, CR_NONE);
        }
        else
        {
            RD_M_DrawTextSmallENG("35 fps", 167 + wide_delta, 55, CR_NONE);
        }

        // Счетчик кадровой частоты
        RD_M_DrawTextSmallRUS(show_fps ? "drk" : "dsrk", 227 + wide_delta, 65, CR_NONE);

        // Пиксельное сглаживание
        if (force_software_renderer == 1)
        {
            RD_M_DrawTextSmallRUS("y*l", 219 + wide_delta, 75, CR_DARKRED); // Н/Д
        }
        else
        {
            RD_M_DrawTextSmallRUS(smoothing ? "drk" : "dsrk", 219 + wide_delta, 75, CR_NONE);
        }

        // Изменение палитры краёв экрана
        RD_M_DrawTextSmallRUS(vga_porch_flash ? "drk" : "dsrk", 274 + wide_delta, 85, CR_NONE);

        // Обработка видео
        RD_M_DrawTextSmallRUS(force_software_renderer ? "ghjuhfvvyfz" : "fggfhfnyfz", 160 + wide_delta, 95, CR_NONE);
        RD_M_DrawTextSmallENG(force_software_renderer ? "(cpu)" : "(gpu)",
                              (force_software_renderer ? 254 : 244) + wide_delta, 95, CR_NONE);

        // Отображать значок дискеты
        RD_M_DrawTextSmallRUS(show_diskicon ? "drk" : "dsrk", 241 + wide_delta, 115, CR_NONE);

        // Эффект смены экранов
        RD_M_DrawTextSmallRUS(screen_wiping == 1 ? "cnfylfhnysq" :
                              screen_wiping == 2 ? "pfuheprf" :
                              "dsrk", 202 + wide_delta, 125, CR_NONE);

        // Формат скриншотов
        RD_M_DrawTextSmallENG(png_screenshots ? "png" : "pcx", 180 + wide_delta, 135, CR_NONE);

        // Показывать экран ENDOOM
        RD_M_DrawTextSmallENG("ENDOOM:", 165 + wide_delta, 145, CR_NONE);
        RD_M_DrawTextSmallRUS(show_endoom ? "drk" : "dsrk", 222 + wide_delta, 145, CR_NONE);
    }
}

void M_RD_Change_Widescreen(Direction_t direction)
{
    // [JN] Widescreen: changing only temp variable here.
    // Initially it is set in M_Init and stored into config file in M_QuitResponse.
    RD_Menu_SpinInt(&aspect_ratio_temp, 0, 4, direction);
}

void M_RD_Change_VSync(Direction_t direction)
{
    // [JN] Disable "vsync" toggling in software renderer
    if (force_software_renderer == 1)
        return;

    vsync ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_Uncapped(Direction_t direction)
{
    uncapped_fps ^= 1;
}

void M_RD_Change_FPScounter(Direction_t direction)
{
    show_fps ^= 1;
}

void M_RD_Change_DiskIcon(Direction_t direction)
{
    show_diskicon ^= 1;
}

void M_RD_Change_Smoothing(Direction_t direction)
{
    // [JN] Disable "vsync" toggling in sofrware renderer
    if (force_software_renderer == 1)
        return;

    smoothing ^= 1;

    // Reinitialize graphics
    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);

    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_Wiping(Direction_t direction)
{
    RD_Menu_SpinInt(&screen_wiping, 0, 2, direction);
}

void M_RD_Change_Screenshots(Direction_t direction)
{
    png_screenshots ^= 1;
}

void M_RD_Change_ENDOOM(Direction_t direction)
{
    show_endoom ^= 1;
}

void M_RD_Change_Renderer(Direction_t direction)
{
    force_software_renderer ^= 1;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}

void M_RD_Change_PorchFlashing(Direction_t direction)
{
    vga_porch_flash ^= 1;

    // Update black borders
    I_DrawBlackBorders();
}

// -----------------------------------------------------------------------------
// Display settings
// -----------------------------------------------------------------------------

void M_RD_Draw_Display(void)
{
    static char num[4];

    // [JN] Jaguar Doom: clear remainings of bottom strings from the status bar.
    if (gamemission == jaguar)
        inhelpscreens = true;

    if (english_language)
    {
        // Graphics detail
        RD_M_DrawTextSmallENG(detailLevel ? "low" : "high", 150 + wide_delta, 115, CR_NONE);

        // HUD background detail
        RD_M_DrawTextSmallENG(hud_detaillevel ? "low" : "high", 199 + wide_delta, 125, CR_NONE);
    }
    else
    {
        // Детализация графики
        RD_M_DrawTextSmallRUS(detailLevel ? "ybprfz" : "dscjrfz", 195 + wide_delta, 115, CR_NONE);

        // Детализация фона HUD
        RD_M_DrawTextSmallENG("HUD: b", 167 + wide_delta, 125, CR_NONE);
        RD_M_DrawTextSmallRUS(hud_detaillevel ? "ybprfz" : "dscjrfz", 199 + wide_delta, 125, CR_NONE);

        //
        // Интерфейс
        //
        RD_M_DrawTextSmallRUS("bynthatqc", 35 + wide_delta, 135, CR_YELLOW);
    }

    // Screen size slider
    if (aspect_ratio >= 2)
    {
        // [JN] Wide screen: only 6 sizes are available
        RD_Menu_DrawSliderSmall(&DisplayMenu, 44, 6, screenblocks - 9);

        // Numerical representation of slider position
        M_snprintf(num, 4, "%3d", screenblocks);
        RD_M_DrawTextSmallENG(num, 96 + wide_delta, 45, CR_NONE);
    }
    else
    {
        RD_Menu_DrawSliderSmall(&DisplayMenu, 44, 12, screenblocks - 3);

        // Numerical representation of slider position
        M_snprintf(num, 4, "%3d", screenblocks);
        RD_M_DrawTextSmallENG(num, 145 + wide_delta, 45, CR_NONE);
    }

    // Gamma-correction slider
    RD_Menu_DrawSliderSmall(&DisplayMenu, 64, 18, usegamma);

    // Level brightness slider
    RD_Menu_DrawSliderSmall(&DisplayMenu, 84, 5, level_brightness / 16);

    // Level brightness slider
    RD_Menu_DrawSliderSmall(&DisplayMenu, 104, 7, menu_shading / 4);
}

void M_RD_Change_ScreenSize(Direction_t direction)
{
    extern void EnableLoadingDisk();

    RD_Menu_SlideInt(&screenblocks, 0, 14, direction);

    if (aspect_ratio >= 2)
    {
        // Wide screen: don't allow unsupported views
        // screenblocks - config file variable
        if (screenblocks < 9)
            screenblocks = 9;
        if (screenblocks > 14)
            screenblocks = 14;

        // Reinitialize fps and time widget's horizontal offset
        if (gamestate == GS_LEVEL)
        {
            HU_Start();
        }
        
        EnableLoadingDisk();
    }

    R_SetViewSize (screenblocks, detailLevel);
}

void M_RD_Change_Gamma(Direction_t direction)
{
    RD_Menu_SlideInt(&usegamma, 0, 17, direction);

    I_SetPalette ((byte *)W_CacheLumpName(DEH_String(usegamma <= 8 ?
                                                     "PALFIX" :
                                                     "PLAYPAL"),
                                                     PU_CACHE) + 
                                                     st_palette * 768);
    players[consoleplayer].message_system = DEH_String(english_language ? 
                                                       gammamsg[usegamma] :
                                                       gammamsg_rus[usegamma]);
}

void M_RD_Change_LevelBrightness(Direction_t direction)
{
    RD_Menu_SlideInt_Step(&level_brightness, 0, 64, 16, direction);
}

void M_RD_Change_MenuShading(Direction_t direction)
{
    RD_Menu_SlideInt_Step(&menu_shading, 0, 24, 4, direction);
}

void M_RD_Change_Detail(Direction_t direction)
{
    detailLevel ^= 1;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
    players[consoleplayer].message_system = DEH_String(english_language ?
                                            DETAILHI : DETAILHI_RUS);
    else
    players[consoleplayer].message_system = DEH_String(english_language ?
                                            DETAILLO : DETAILLO_RUS);
}

void M_RD_Change_HUD_Detail(Direction_t direction)
{
    extern boolean setsizeneeded;

    hud_detaillevel ^= 1;

    // [JN] Update screen border.
    setsizeneeded = true;

    // [JN] Refresh status bar.
    inhelpscreens = true;
}

void M_RD_Change_LocalTime(Direction_t direction)
{
    RD_Menu_SpinInt(&local_time, 0, 4, direction);

    // Reinitialize time widget's horizontal offset
    if (gamestate == GS_LEVEL)
    {
        HU_Start();
    }
}

// -----------------------------------------------------------------------------
// Messages settings
// -----------------------------------------------------------------------------

void M_RD_Draw_MessagesSettings(void)
{
    if (english_language)
    {
        // Messages
        RD_M_DrawTextSmallENG(showMessages ? "on" : "off", 165 + wide_delta, 35, CR_NONE);

        // Message alignment
        RD_M_DrawTextSmallENG(messages_alignment == 1 ? "centered" :
                              messages_alignment == 2 ? "left edge of the status bar" :
                              "left edge of the screen", 112 + wide_delta, 45, CR_NONE);

        // Message timeout. Print "second" or "seconds", depending of ammount.
        // [JN] Note: using M_StringJoin could be a smarter way,
        // but using it will make a notable delay in drawing routine, so here:
        RD_M_DrawTextSmallENG(messages_timeout == 1 ? "1 second" :
                              messages_timeout == 2 ? "2 seconds" :
                              messages_timeout == 3 ? "3 seconds" :
                              messages_timeout == 4 ? "4 seconds" :
                              messages_timeout == 5 ? "5 seconds" :
                              messages_timeout == 6 ? "6 seconds" :
                              messages_timeout == 7 ? "7 seconds" :
                              messages_timeout == 8 ? "8 seconds" :
                              messages_timeout == 9 ? "9 seconds" :
                              "10 seconds", 133 + wide_delta, 65, CR_NONE);

        // Fading effect
        RD_M_DrawTextSmallENG(message_fade ? "on" : "off", 139 + wide_delta, 75, CR_NONE);

        // Text casts shadows
        RD_M_DrawTextSmallENG(draw_shadowed_text ? "on" : "off", 177 + wide_delta, 85, CR_NONE);

        // Local time
        RD_M_DrawTextSmallENG(local_time == 1 ? "12-hour (hh:mm)" :
                              local_time == 2 ? "12-hour (hh:mm:ss)" :
                              local_time == 3 ? "24-hour (hh:mm)" :
                              local_time == 4 ? "24-hour (hh:mm:ss)" :
                              "off", 116 + wide_delta, 105, CR_NONE);

        // Item pickup
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 120 + wide_delta, 125, CR_NONE);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(message_pickup_color), 120 + wide_delta, 125,
                                  M_RD_ColorTranslation(message_pickup_color));
        }

        // Revealed secret
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 157 + wide_delta, 135, CR_NONE);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(message_secret_color), 157 + wide_delta, 135,
                                  M_RD_ColorTranslation(message_secret_color));
        }

        // System message
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 149 + wide_delta, 145, CR_NONE);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(message_system_color), 149 + wide_delta, 145,
                                  M_RD_ColorTranslation(message_system_color));
        }

        // Netgame chat
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 136 + wide_delta, 155, CR_NONE);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(message_chat_color), 136 + wide_delta, 155,
                                  M_RD_ColorTranslation(message_chat_color));
        }
    }
    else
    {
        // Отображение сообщений
        RD_M_DrawTextSmallRUS(showMessages ? "drk" : "dsrk", 214 + wide_delta, 35, CR_NONE);

        // Выравнивание
        RD_M_DrawTextSmallRUS(messages_alignment == 1 ? "gj wtynhe" :             // по центру
                              messages_alignment == 2 ? "gj rhf. cnfnec-,fhf" :   // по краю статус-бара
                              "gj rhf. \'rhfyf", 141 + wide_delta, 45, CR_NONE); // по краю экрана

        // Таймаут отображения. Печатать секунд(а/ы) в зависимости от количества.
        RD_M_DrawTextSmallRUS(messages_timeout == 1 ? "1 ctreylf" :
                              messages_timeout == 2 ? "2 ctreyls" :
                              messages_timeout == 3 ? "3 ctreyls" :
                              messages_timeout == 4 ? "4 ctreyls" :
                              messages_timeout == 5 ? "5 ctreyl"  :
                              messages_timeout == 6 ? "6 ctreyl"  :
                              messages_timeout == 7 ? "7 ctreyl"  :
                              messages_timeout == 8 ? "8 ctreyl"  :
                              messages_timeout == 9 ? "9 ctreyl"  :
                              "10 ctreyl", 133 + wide_delta, 65, CR_NONE);

        // Плавное исчезновение
        RD_M_DrawTextSmallRUS(message_fade ? "drk" : "dsrk", 198 + wide_delta, 75, CR_NONE);

        // Тексты отбрасывают тень
        RD_M_DrawTextSmallRUS(draw_shadowed_text ? "drk" : "dsrk", 226 + wide_delta, 85, CR_NONE);

        // Системное время
        RD_M_DrawTextSmallRUS(local_time == 1 ? "12-xfcjdjt (xx:vv)" :
                              local_time == 2 ? "12-xfcjdjt (xx:vv:cc)" :
                              local_time == 3 ? "24-xfcjdjt (xx:vv)" :
                              local_time == 4 ? "24-xfcjdjt (xx:vv:cc)" :
                              "dsrk", 161 + wide_delta, 105, CR_NONE);

        // Получение предметов
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 193 + wide_delta, 125, CR_NONE); // н/д
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(message_pickup_color), 193 + wide_delta, 125,
                                  M_RD_ColorTranslation(message_pickup_color));
        }

        // Обнаружение тайников
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 203 + wide_delta, 135, CR_NONE); // н/д
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(message_secret_color), 203 + wide_delta, 135,
                                  M_RD_ColorTranslation(message_secret_color));
        }

        // Системные сообщения
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 197 + wide_delta, 145, CR_NONE); // н/д
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(message_system_color), 197 + wide_delta, 145,
                                  M_RD_ColorTranslation(message_system_color));
        }

        // Чат сетевой игры
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 164 + wide_delta, 155, CR_NONE); // н/д
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(message_chat_color), 164 + wide_delta, 155,
                                  M_RD_ColorTranslation(message_chat_color));
        }
    }

    // Message timeout slider
    RD_Menu_DrawSliderSmall(&MessagesMenu, 64, 10, messages_timeout - 1);
}

void M_RD_Change_Messages(Direction_t direction)
{
    showMessages ^= 1;

    if (!showMessages)
        players[consoleplayer].message_system = DEH_String(english_language ?
                                            MSGOFF : MSGOFF_RUS);
    else
        players[consoleplayer].message_system = DEH_String(english_language ?
                                            MSGON : MSGON_RUS);

    message_dontfuckwithme = true;
}

void M_RD_Change_Msg_Alignment(Direction_t direction)
{
    RD_Menu_SpinInt(&messages_alignment, 0, 2, direction);
}

void M_RD_Change_Msg_TimeOut(Direction_t direction)
{
    RD_Menu_SlideInt(&messages_timeout, 1, 10, direction);
}

void M_RD_Change_Msg_Fade(Direction_t direction)
{
    message_fade ^= 1;
}

void M_RD_Change_ShadowedText(Direction_t direction)
{
    draw_shadowed_text ^= 1;
}

void M_RD_Define_Msg_Color(MessageType_t messageType, int color)
{
    Translation_CR_t * colorVar;
    switch (messageType)
    {
        case msg_pickup: // Item pickup.
            colorVar = &messages_pickup_color_set;
            break;
        case msg_secret: // Revealed secret
            colorVar = &messages_secret_color_set;
            break;
        case msg_system: // System message
            colorVar = &messages_system_color_set;
            break;
        case msg_chat: // Netgame chat
            colorVar = &messages_chat_color_set;
            break;
        default:
            return;
    }

    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        *colorVar = CR_NONE;
    }
    else
    {
        switch (color)
        {
            case 1:   *colorVar = CR_DARKRED;    break;
            case 2:   *colorVar = CR_GREEN;      break;
            case 3:   *colorVar = CR_DARKGREEN;  break;
            case 4:   *colorVar = CR_OLIVE;      break;
            case 5:   *colorVar = CR_BLUE2;      break;
            case 6:   *colorVar = CR_DARKBLUE;   break;
            case 7:   *colorVar = CR_YELLOW;     break;
            case 8:   *colorVar = CR_ORANGE;     break;
            case 9:   *colorVar = CR_WHITE;      break;
            case 10:  *colorVar = CR_GRAY;       break;
            case 11:  *colorVar = CR_DARKGRAY;   break;
            case 12:  *colorVar = CR_TAN;        break;
            case 13:  *colorVar = CR_BROWN;      break;
            case 14:  *colorVar = CR_ALMOND;     break;
            case 15:  *colorVar = CR_KHAKI;      break;
            case 16:  *colorVar = CR_PINK;       break;
            case 17:  *colorVar = CR_BURGUNDY;   break;
            default:  *colorVar = CR_NONE;       break;
        }
    }
}

void M_RD_Change_Msg_Pickup_Color(Direction_t direction)
{
    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&message_pickup_color, 0, 17, direction);

    // [JN] Redefine pickup message color.
    M_RD_Define_Msg_Color(msg_pickup, message_pickup_color);
}

void M_RD_Change_Msg_Secret_Color(Direction_t direction)
{
    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&message_secret_color, 0, 17, direction);

    // [JN] Redefine revealed secret message color.
    M_RD_Define_Msg_Color(msg_secret, message_secret_color);
}

void M_RD_Change_Msg_System_Color(Direction_t direction)
{
    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&message_system_color, 0, 17, direction);

    // [JN] Redefine system message color.
    M_RD_Define_Msg_Color(msg_system, message_system_color);
}

void M_RD_Change_Msg_Chat_Color(Direction_t direction)
{
    // [JN] Disable colored messages toggling in Jaguar
    if (gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&message_chat_color, 0, 17, direction);

    // [JN] Redefine netgame chat message color.
    M_RD_Define_Msg_Color(msg_chat, message_chat_color);
}

// -----------------------------------------------------------------------------
// Automap settings
// -----------------------------------------------------------------------------

void M_RD_Draw_AutomapSettings(void)
{
    static char num[4];

    M_snprintf(num, 4, "%d", automap_grid_size);

    if (english_language)
    {
        // Automap colors (English only names, different placement)
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 170 + wide_delta, 35, CR_NONE);
        }
        else
        {
            RD_M_DrawTextSmallENG(automap_color == 1 ? "boom" :
                                  automap_color == 2 ? "jaguar" :
                                  automap_color == 3 ? "raven"  :
                                  automap_color == 4 ? "strife" :
                                  automap_color == 5 ? "unity"  :
                                  "doom", 170 + wide_delta, 35, CR_NONE);
        }

        // Line antialiasing
        RD_M_DrawTextSmallENG(automap_antialias ? "on" : "off", 193 + wide_delta, 45,
                              automap_antialias ? CR_GREEN : CR_DARKRED);


        // Overlay mode
        RD_M_DrawTextSmallENG(automap_overlay ? "on" : "off", 170 + wide_delta, 55,
                              automap_overlay ? CR_GREEN : CR_DARKRED);

        // Rotate mode
        RD_M_DrawTextSmallENG(automap_rotate ? "on" : "off", 163 + wide_delta, 65,
                              automap_rotate ? CR_GREEN : CR_DARKRED);

        // Follow mode
        RD_M_DrawTextSmallENG(automap_follow ? "on" : "off", 164 + wide_delta, 75,
                              automap_follow ? CR_GREEN : CR_DARKRED);

        // Grid
        RD_M_DrawTextSmallENG(automap_grid ? "on" : "off", 106 + wide_delta, 85,
                             automap_grid ? CR_GREEN : CR_DARKRED);

        // Grid size
        RD_M_DrawTextSmallENG(num, 136 + wide_delta, 95,
                              automap_grid_size == 128 ? CR_DARKRED : CR_GREEN);

        // Level stats

        RD_M_DrawTextSmallENG(automap_stats == 1 ? "in automap" :
                              automap_stats == 2 ? "always" : "off",
                              159 + wide_delta, 115,
                              automap_stats ? CR_GREEN : CR_DARKRED);


        // Level time
        RD_M_DrawTextSmallENG(automap_level_time == 1 ? "in automap" :
                              automap_level_time == 2 ? "always" : "off",
                              150 + wide_delta, 125,
                              automap_level_time ? CR_GREEN : CR_DARKRED);

        // Total time
        RD_M_DrawTextSmallENG(automap_total_time == 1 ? "in automap" :
                              automap_total_time == 2 ? "always" : "off",
                              151 + wide_delta, 135,
                              automap_total_time ? CR_GREEN : CR_DARKRED);

        // Player coords
        RD_M_DrawTextSmallENG(automap_coords == 1 ? "in automap" :
                              automap_coords == 2 ? "always" : "off",
                              177 + wide_delta, 145,
                              automap_coords ? CR_GREEN : CR_DARKRED);
    }
    else
    {
        // Automap colors (English only names, different placement)
        if (gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 191 + wide_delta, 35, CR_NONE); // н/д
        }
        else
        {
            RD_M_DrawTextSmallENG(automap_color == 1 ? "boom" :
                                  automap_color == 2 ? "jaguar" :
                                  automap_color == 3 ? "raven"  :
                                  automap_color == 4 ? "strife" :
                                  automap_color == 5 ? "unity"  :
                                  "doom", 189 + wide_delta, 35, CR_NONE);
        }

        // Сглаживание линий
        RD_M_DrawTextSmallRUS(automap_antialias ? "drk" : "dsrk", 214 + wide_delta, 45,
                              automap_antialias ? CR_GREEN : CR_DARKRED);

        // Режим наложения
        RD_M_DrawTextSmallRUS(automap_overlay ? "drk" : "dsrk", 203 + wide_delta, 55,
                              automap_overlay ? CR_GREEN : CR_DARKRED);

        // Режим вращения
        RD_M_DrawTextSmallRUS(automap_rotate ? "drk" : "dsrk", 194 + wide_delta, 65,
                              automap_rotate ? CR_GREEN : CR_DARKRED);

        // Режим следования
        RD_M_DrawTextSmallRUS(automap_follow ? "drk" : "dsrk", 208 + wide_delta, 75,
                              automap_follow ? CR_GREEN : CR_DARKRED);

        // Сетка
        RD_M_DrawTextSmallRUS(automap_grid ? "drk" : "dsrk", 118 + wide_delta, 85,
                              automap_grid ? CR_GREEN : CR_DARKRED);

        // Размер сетки
        RD_M_DrawTextSmallENG(num, 171 + wide_delta, 95,
                             automap_grid_size == 128 ? CR_DARKRED : CR_GREEN);

        // Статистика уровня
        RD_M_DrawTextSmallRUS(automap_stats == 1 ? "yf rfhnt" :
                              automap_stats == 2 ? "dctulf" :
                              "dsrk", 210 + wide_delta, 115,
                              automap_stats ? CR_GREEN : CR_DARKRED);

        // Время уровня
        RD_M_DrawTextSmallRUS(automap_level_time == 1 ? "yf rfhnt" :
                              automap_level_time == 2 ? "dctulf" :
                              "dsrk", 171 + wide_delta, 125,
                              automap_level_time ? CR_GREEN : CR_DARKRED);

        // Общее время
        RD_M_DrawTextSmallRUS(automap_total_time == 1 ? "yf rfhnt" :
                              automap_total_time == 2 ? "dctulf" :
                              "dsrk", 166 + wide_delta, 135,
                              automap_total_time ? CR_GREEN : CR_DARKRED);

        // Координаты игрока
        RD_M_DrawTextSmallRUS(automap_coords == 1 ? "yf rfhnt" :
                              automap_coords == 2 ? "dctulf" :
                              "dsrk", 213 + wide_delta, 145,
                              automap_coords ? CR_GREEN : CR_DARKRED);
    }
}

void M_RD_Change_AutomapColor(Direction_t direction)
{
    // [JN] Disable automap colors changing in Jaguar
    if (gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&automap_coords, 0, 2, direction);

    // [JN] Reinitialize automap color scheme.
    AM_initColors();
}

void M_RD_Change_AutomapAntialias(Direction_t direction)
{
    automap_antialias ^= 1;
}

void M_RD_Change_AutomapOverlay(Direction_t direction)
{
    automap_overlay ^= 1;
}

void M_RD_Change_AutomapRotate(Direction_t direction)
{
    automap_rotate ^= 1;
}

void M_RD_Change_AutomapFollow(Direction_t direction)
{
    automap_follow ^= 1;
}

void M_RD_Change_AutomapGrid(Direction_t direction)
{
    automap_grid ^= 1;
}

void M_RD_Change_AutomapGridSize(Direction_t direction)
{
    RD_Menu_ShiftSlideInt(&automap_grid_size, 32, 512, direction);
}

void M_RD_Change_AutomapStats(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_stats, 0, 2, direction);
}

void M_RD_Change_AutomapLevelTime(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_level_time, 0, 2, direction);
}

void M_RD_Change_AutomapTotalTime(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_total_time, 0, 2, direction);
}

void M_RD_Change_AutomapCoords(Direction_t direction)
{
    RD_Menu_SpinInt(&automap_coords, 0, 2, direction);
}

// -----------------------------------------------------------------------------
// Sound
// -----------------------------------------------------------------------------

void M_RD_Draw_Audio(void)
{
    static char num[4];

    // SFX volume slider
    RD_Menu_DrawSliderSmall(&SoundMenu, 44, 16, sfxVolume);
    // Numerical representation of SFX volume
    M_snprintf(num, 4, "%3d", sfxVolume);
    RD_M_DrawTextSmallENG(num,  177 + wide_delta, 45,CR_NONE);

    // Music volume slider
    RD_Menu_DrawSliderSmall(&SoundMenu, 64, 16, musicVolume);
    // Numerical representation of music volume
    M_snprintf(num, 4, "%3d", musicVolume);
    RD_M_DrawTextSmallENG(num,  177 + wide_delta, 65, CR_NONE);

    // SFX channels slider
    RD_Menu_DrawSliderSmall(&SoundMenu, 94, 16, snd_channels / 4 - 1);
    // Numerical representation of channels
    M_snprintf(num, 4, "%3d", snd_channels);
    RD_M_DrawTextSmallENG(num,  177 + wide_delta, 95, CR_NONE);
}

void M_RD_Change_SfxVol(Direction_t direction)
{
    RD_Menu_SlideInt(&sfxVolume, 0, 15, direction);

    S_SetSfxVolume(sfxVolume * 8);
}

void M_RD_Change_MusicVol(Direction_t direction)
{
    RD_Menu_SlideInt(&musicVolume, 0, 15, direction);

    S_SetMusicVolume(musicVolume * 8);
}

void M_RD_Change_SfxChannels(Direction_t direction)
{
    RD_Menu_SlideInt_Step(&snd_channels, 4, 64, 4, direction);

    // Reallocate sound channels
    S_ChannelsRealloc();
}

// -----------------------------------------------------------------------------
// Sound system
// -----------------------------------------------------------------------------

void M_RD_Draw_Audio_System(void)
{
    static char snd_frequency[16];

    if (english_language)
    {
        // Sound effects
        if (snd_sfxdevice == 0)
        {
            
            RD_M_DrawTextSmallENG("disabled", 141 + wide_delta, 35, CR_DARKRED);
        }
        else if (snd_sfxdevice == 1)
        {
            RD_M_DrawTextSmallENG("pc speaker", 141 + wide_delta, 35, CR_NONE);
        }
        else if (snd_sfxdevice == 3)
        {
            RD_M_DrawTextSmallENG("digital sfx", 141 + wide_delta, 35, CR_NONE);
        }

        // Music
        if (snd_musicdevice == 0)
        {   
            RD_M_DrawTextSmallENG("disabled", 79 + wide_delta, 45, CR_DARKRED);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            RD_M_DrawTextSmallENG("opl2 synth", 79 + wide_delta, 45, CR_NONE);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            RD_M_DrawTextSmallENG("opl3 synth", 79 + wide_delta, 45, CR_NONE);
        }
        else if (snd_musicdevice == 5)
        {
            RD_M_DrawTextSmallENG("gus emulation", 79 + wide_delta, 45, CR_NONE);
        }
        else if (snd_musicdevice == 8)
        {
            RD_M_DrawTextSmallENG("MIDI/MP3/OGG/FLAC/TRACKER", 79 + wide_delta, 45, CR_NONE);
        }

        // Sampling frequency
        sprintf(snd_frequency, "%d HZ", snd_samplerate);
        RD_M_DrawTextSmallENG(snd_frequency, 179 + wide_delta, 65, CR_NONE);

        // Sfx mode
        RD_M_DrawTextSmallENG(snd_monomode ? "mono" : "stereo", 178 + wide_delta, 85, CR_NONE);

        // Pitch-shifted sounds
        RD_M_DrawTextSmallENG(snd_pitchshift ? "on" : "off", 186 + wide_delta, 95, CR_NONE);

        // Mute inactive window
        RD_M_DrawTextSmallENG(mute_inactive_window ? "on" : "off", 185 + wide_delta, 105, CR_NONE);
    }
    else
    {
        // Звуковые эффекты
        if (snd_sfxdevice == 0)
        {
            // Отключены
            RD_M_DrawTextSmallRUS("jnrk.xtys", 175 + wide_delta, 35, CR_DARKRED);
        }
        else if (snd_sfxdevice == 1)
        {
            // Динамик ПК
            RD_M_DrawTextSmallRUS("lbyfvbr gr", 175 + wide_delta, 35, CR_NONE);
        }
        else if (snd_sfxdevice == 3)
        {
            // Цифровые
            RD_M_DrawTextSmallRUS("wbahjdst", 175 + wide_delta, 35, CR_NONE);
        }

        // Музыка
        if (snd_musicdevice == 0)
        {
            // Отключена
            RD_M_DrawTextSmallRUS("jnrk.xtyf", 94 + wide_delta, 45, CR_DARKRED);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
        {
            // Синтез OPL2
            RD_M_DrawTextSmallRUS("cbyntp", 94 + wide_delta, 45, CR_NONE);
            RD_M_DrawTextSmallENG("opl2", 146 + wide_delta, 45, CR_NONE);
        }
        else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
        {
            // Синтез OPL3
            RD_M_DrawTextSmallRUS("cbyntp", 94 + wide_delta, 45, CR_NONE);
            RD_M_DrawTextSmallENG("opl3", 146 + wide_delta, 45, CR_NONE);
        }
        else if (snd_musicdevice == 5)
        {
            // Эмуляция GUS
            RD_M_DrawTextSmallRUS("\'vekzwbz", 94 + wide_delta, 45, CR_NONE);
            RD_M_DrawTextSmallENG("gus", 164 + wide_delta, 45, CR_NONE);
        }
        else if (snd_musicdevice == 8)
        {
            RD_M_DrawTextSmallENG("MIDI/MP3/OGG/FLAC/TRACKER", 94 + wide_delta, 45, CR_NONE);
        }

        // Частота дискретизации (ГЦ)
        sprintf(snd_frequency, "%d UW", snd_samplerate);
        RD_M_DrawTextSmallRUS(snd_frequency, 208 + wide_delta, 65, CR_NONE);

        // Режим звука
        RD_M_DrawTextSmallRUS(snd_monomode ? "vjyj" : "cnthtj", 231 + wide_delta, 85, CR_NONE);

        // Произвольный питч-шифтинг
        RD_M_DrawTextSmallRUS(snd_pitchshift ? "drk" : "dsrk", 242 + wide_delta, 95, CR_NONE);

        // Звук в неактивном окне
        RD_M_DrawTextSmallRUS(mute_inactive_window ? "dsrk" : "drk", 208 + wide_delta, 105, CR_NONE);
    }
}

void M_RD_Change_SoundDevice(Direction_t direction)
{
    switch(direction)
    {
        case LEFT_DIR:
            if (snd_sfxdevice == 0)
                snd_sfxdevice = 3;
            else if (snd_sfxdevice == 3)
                snd_sfxdevice = 1;
            else if (snd_sfxdevice == 1)
                snd_sfxdevice = 0;
            break;
        case RIGHT_DIR:
            if (snd_sfxdevice == 0)
                snd_sfxdevice = 1;
            else if (snd_sfxdevice == 1)
                snd_sfxdevice = 3;
            else if (snd_sfxdevice == 3)
                snd_sfxdevice = 0;
        default:
            break;
    }

    // Reinitialize SFX module
    InitSfxModule(snd_sfxdevice);

    // Call sfx device changing routine
    S_RD_Change_SoundDevice();

    // Reinitialize sound volume
    S_SetSfxVolume(sfxVolume * 8);
}

void M_RD_Change_MusicDevice(Direction_t direction)
{
    switch(direction)
    {
        case LEFT_DIR:
            if (snd_musicdevice == 0)
            {
                snd_musicdevice = 8;    // Set to MIDI/MP3/OGG/FLAC
            }
            else if (snd_musicdevice == 8)
            {
                snd_musicdevice = 5;    // Set to GUS
            }
            else if (snd_musicdevice == 5)
            {
                snd_musicdevice = 3;    // Set to OPL3
                snd_dmxoption = "-opl3";
            }
            else if (snd_musicdevice == 3  && !strcmp(snd_dmxoption, "-opl3"))
            {
                snd_musicdevice = 3;    // Set to OPL2
                snd_dmxoption = "";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
            {
                snd_musicdevice = 0;    // Disable
            }
            break;
        case RIGHT_DIR:
            if (snd_musicdevice == 0)
            {
                snd_musicdevice  = 3;   // Set to OPL2
                snd_dmxoption = "";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, ""))
            {
                snd_musicdevice  = 3;   // Set to OPL3
                snd_dmxoption = "-opl3";
            }
            else if (snd_musicdevice == 3 && !strcmp(snd_dmxoption, "-opl3"))
            {
                snd_musicdevice  = 5;
            }
            else if (snd_musicdevice == 5)
            {
                snd_musicdevice  = 8;   // Set to MIDI/MP3/OGG/FLAC
            }
            else if (snd_musicdevice == 8)
            {
                snd_musicdevice  = 0;   // Disable
            }
        default:
            break;
    }

    // Shut down current music
    S_StopMusic();

    // Shut down music system
    S_Shutdown();
    
    // Start music system
    I_InitSound(true);

    // Reinitialize music volume
    S_SetMusicVolume(musicVolume * 8);

    // Restart current music
    S_ChangeMusic(music_num_rd, true);
}

void M_RD_Change_Sampling(Direction_t direction)
{
    RD_Menu_ShiftSpinInt(&snd_samplerate, 11025, 44100, direction);

    // Shut down sound system
    I_ShutdownSound();

    // Reinitialize SFX module
    InitSfxModule(snd_sfxdevice);

    // Call sfx device changing routine
    S_RD_Change_SoundDevice();

    // Reinitialize sound volume
    S_SetSfxVolume(sfxVolume * 8);

    // Shut down current music
    S_StopMusic();

    // Shut down music system
    S_Shutdown();
    
    // Start music system
    I_InitSound(true);

    // Reinitialize music volume
    S_SetMusicVolume(musicVolume * 8);

    // Restart current music
    S_ChangeMusic(music_num_rd, true);
}

void M_RD_Change_SndMode(Direction_t direction)
{
    snd_monomode ^= 1;

    // Update stereo separation
    S_UpdateStereoSeparation();
}

void M_RD_Change_PitchShifting(Direction_t direction)
{
    snd_pitchshift ^= 1;
}

void M_RD_Change_MuteInactive(Direction_t direction)
{
    mute_inactive_window ^= 1;
}

// -----------------------------------------------------------------------------
// Keyboard and Mouse
// -----------------------------------------------------------------------------

void M_RD_Draw_Controls(void)
{
    static char num[4];

    if (english_language)
    {
        // Mouse look
        RD_M_DrawTextSmallENG(mlook ? "on" : "off", 119 + wide_delta, 125, CR_NONE);

        // Invert Y axis
        RD_M_DrawTextSmallENG(mouse_y_invert ? "on" : "off", 130 + wide_delta, 135,
                              !mlook ? CR_DARKRED : CR_NONE);

        // Vertical movement
        RD_M_DrawTextSmallENG(!novert ? "on" : "off", 171 + wide_delta, 145,
                              mlook ? CR_DARKRED : CR_NONE);
    }
    else
    {
        // Обзор мышью
        RD_M_DrawTextSmallRUS(mlook ? "drk" : "dsrk", 135 + wide_delta, 125, CR_NONE);

        // Вертикальная инверсия
        RD_M_DrawTextSmallRUS(mouse_y_invert ? "drk" : "dsrk", 207 + wide_delta, 135,
                              !mlook ? CR_DARKRED : CR_NONE);

        // Вертикальное перемещение
        RD_M_DrawTextSmallRUS(!novert ? "drk" : "dsrk", 235 + wide_delta, 145,
                              mlook ? CR_DARKRED : CR_NONE);
    }

    // Mouse sensivity slider
    RD_Menu_DrawSliderSmall(&ControlsMenu, 74, 17, mouseSensitivity);
    // Numerical representation
    M_snprintf(num, 4, "%3d", mouseSensitivity);
    RD_M_DrawTextSmallENG(num, 189 + wide_delta, 75, CR_NONE);

    // Acceleration slider
    RD_Menu_DrawSliderSmall(&ControlsMenu, 94, 17, mouse_acceleration * 4 - 4);
    // Numerical representation
    M_snprintf(num, 4, "%f", mouse_acceleration);
    RD_M_DrawTextSmallENG(num, 189 + wide_delta, 95, CR_NONE);

    // Acceleration threshold slider
    RD_Menu_DrawSliderSmall(&ControlsMenu, 114, 17, mouse_threshold / 2);
    // Numerical representation
    M_snprintf(num, 4, "%3d", mouse_threshold);
    if (mouse_acceleration < 1.1)
        RD_M_DrawTextSmallENG(num, 189 + wide_delta, 115, CR_DARKRED);
    else
        RD_M_DrawTextSmallENG(num, 189 + wide_delta, 115, CR_NONE);
}

void M_RD_Change_MouseLook(Direction_t direction)
{
    mlook ^= 1;

    if (!mlook)
        players[consoleplayer].centering = true;
}

void M_RD_Change_InvertY(Direction_t direction)
{
    mouse_y_invert ^= 1;
}

void M_RD_Change_Novert(Direction_t direction)
{
    novert ^= 1;
}

void M_RD_Change_Sensitivity(Direction_t direction)
{
    RD_Menu_SlideInt(&mouseSensitivity, 0, 255, direction); // [crispy] extended range
}

void M_RD_Change_Acceleration(Direction_t direction)
{
    RD_Menu_SlideFloat_Step(&mouse_acceleration, 1.1F, 5.0F, 0.1F, direction);
}

void M_RD_Change_Threshold(Direction_t direction)
{
    RD_Menu_SlideInt(&mouse_threshold, 0, 32, direction);
}

// -----------------------------------------------------------------------------
// Key bindings
// -----------------------------------------------------------------------------

void M_RD_Bind_Key(int choice)
{
    M_RD_StartBinding(BK_getKeyDescriptor(choice)->key_var);
}

void M_RD_Draw_Bindings()
{
    int x = (english_language ? 209 : 210);

    // [JN] Erase the entire screen to a tiled background.
    inhelpscreens = true;
    V_FillFlat ("FLOOR4_8");

    if (english_language)
    {
        RD_M_DrawTextSmallENG(CurrentMenu == &Bindings4Menu ? "first page >" : "next page >", 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallENG(CurrentMenu == &Bindings1Menu ? "< last page" : "< prev page", 35 + wide_delta, 165, CR_WHITE);
        //RD_M_DrawTextSmallENG(keyPage->pageNumber_eng, x + wide_delta, 165, CR_WHITE); // [Dasperal] TODO replace with paging system

        RD_M_DrawTextSmallENG("enter to change, del to clear", 55 + wide_delta, 180, CR_DARKRED);
        RD_M_DrawTextSmallENG("pgup/pgdn to turn pages", 75 + wide_delta, 189, CR_DARKRED);
    }
    else
    {
        RD_M_DrawTextSmallRUS(RD_NEXT_RUS, 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallRUS(RD_PREV_RUS, 35 + wide_delta, 165, CR_WHITE);
        //RD_M_DrawTextSmallRUS(keyPage->pageNumber_rus, x + wide_delta, 165, CR_WHITE); // [Dasperal] TODO replace with paging system

        RD_M_DrawTextSmallENG("enter =", 44 + wide_delta, 180, CR_DARKRED);
        RD_M_DrawTextSmallRUS("= yfpyfxbnm<", 88 + wide_delta, 180, CR_DARKRED);
        RD_M_DrawTextSmallENG("del =", 176 + wide_delta, 180, CR_DARKRED);
        RD_M_DrawTextSmallRUS("jxbcnbnm", 213 + wide_delta, 180, CR_DARKRED);

        RD_M_DrawTextSmallENG("pgup/pgdn =", 55 + wide_delta, 189, CR_DARKRED);
        RD_M_DrawTextSmallRUS("kbcnfnm cnhfybws", 139 + wide_delta, 189, CR_DARKRED);
    }

    for (int i = 0; i < CurrentMenu->itemCount; ++i)
    {
        if (CurrentMenu->items->option != 0)
        {
            boolean bindingThis = messageToBind && i == CurrentItPos;

            RD_M_DrawTextSmallENG(bindingThis ? "?" : BK_getBoundKeysString(CurrentMenu->items->option),
                                  x + wide_delta, i * 10 + 35,
                                  bindingThis ? CR_WHITE : BK_KeyHasNoBinds(CurrentMenu->items->option) ?
                                  CR_DARKRED : CR_NONE);
        }
    }
}

// -----------------------------------------------------------------------------
// M_RD_MouseBtnDrawer
// [JN] Returns button name for mouse bindings.
// -----------------------------------------------------------------------------

static char *M_RD_MouseBtnDrawer (int i)
{
    switch (i)
    {
        case -1:    return "---";
        case  0:    return "LEFT BUTTON";
        case  1:    return "RIGHT BUTTON";
        case  2:    return "MIDDLE BUTTON";
        case  3:    return "BUTTON #3";
        case  4:    return "BUTTON #4";
        default:    return "?"; // [JN] Unknown key
    }
}

// -----------------------------------------------------------------------------
// Mouse bindings
// -----------------------------------------------------------------------------

void M_RD_Draw_Mouse_Bindings(void)
{
    int x = 186;
    
    // [JN] Erase the entire screen to a tiled background.
    inhelpscreens = true;
    V_FillFlat ("FLOOR4_8");

    if (english_language)
    {
        //
        // Footer
        //
        RD_M_DrawTextSmallENG("enter to change, del to clear", 55 + wide_delta, 180, CR_DARKRED);
    }
    else
    {
        //
        // Footer
        //
        RD_M_DrawTextSmallENG("enter =", 44 + wide_delta, 180, CR_DARKRED);
        RD_M_DrawTextSmallRUS("= yfpyfxbnm<", 88 + wide_delta, 180, CR_DARKRED);
        RD_M_DrawTextSmallENG("del =", 176 + wide_delta, 180, CR_DARKRED);
        RD_M_DrawTextSmallRUS("jxbcnbnm", 213 + wide_delta, 180, CR_DARKRED);
    }

    // Fire/Attack
    if (messageToBind && CurrentItPos == 1)
    {
        RD_M_DrawTextSmallENG("?", x + wide_delta, 35, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebfire), x + wide_delta, 35,
                             mousebfire == -1 ? CR_DARKRED : CR_NONE);
    }

    // Use
    if (messageToBind && CurrentItPos == 2)
    {
        RD_M_DrawTextSmallENG("?", x + wide_delta, 45, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebuse), x + wide_delta, 45,
                              mousebuse == -1 ? CR_DARKRED : CR_NONE);
    }

    // Move Forward
    if (messageToBind && CurrentItPos == 3)
    {

        RD_M_DrawTextSmallENG("?", x + wide_delta, 55, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebforward), x + wide_delta, 55,
                              mousebforward == -1 ? CR_DARKRED : CR_NONE);
    }

    // Move Backward
    if (messageToBind && CurrentItPos == 4)
    {
        RD_M_DrawTextSmallENG("?", x + wide_delta, 65, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebbackward), x + wide_delta, 65,
                              mousebbackward == -1 ? CR_DARKRED : CR_NONE);
    }

    // Strafe On
    if (messageToBind && CurrentItPos == 5)
    {
        RD_M_DrawTextSmallENG("?", x + wide_delta, 75, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebstrafe), x + wide_delta, 75,
                              mousebstrafe == -1 ? CR_DARKRED : CR_NONE);

    }

    // Strafe Left
    if (messageToBind && CurrentItPos == 6)
    {
        RD_M_DrawTextSmallENG("?",  x + wide_delta, 85, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebstrafeleft), x + wide_delta, 85,
                              mousebstrafeleft == -1 ? CR_DARKRED : CR_NONE);
    }

    // Strafe Right
    if (messageToBind && CurrentItPos == 7)
    {
        RD_M_DrawTextSmallENG("?", x + wide_delta, 95, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebstraferight), x + wide_delta, 95,
                              mousebstraferight == -1 ? CR_DARKRED : CR_NONE);
    }

    // Previous Weapon
    if (messageToBind && CurrentItPos == 8)
    {
        RD_M_DrawTextSmallENG("?", x + wide_delta, 105, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebprevweapon), x + wide_delta, 105,
                              mousebprevweapon == -1 ? CR_DARKRED : CR_NONE);
    }

    // Next Weapon
    if (messageToBind && CurrentItPos == 9)
    {
        RD_M_DrawTextSmallENG("?", x + wide_delta, 115, CR_WHITE);
    }
    else
    {
        RD_M_DrawTextSmallENG(M_RD_MouseBtnDrawer(mousebnextweapon), x + wide_delta, 115,
                              mousebnextweapon == -1 ? CR_DARKRED : CR_NONE);
    }
}

//
// Fire/Attack
//
void M_RD_Mouse_Bind_FireAttack (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebfire);
}

//
// Use
//
void M_RD_Mouse_Bind_Use (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebuse);
}

//
// Move Forward
//
void M_RD_Mouse_Bind_MoveForward (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebforward);
}

//
// Move Backward
//
void M_RD_Mouse_Bind_MoveBackward (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebbackward);
}

//
// Strafe On
//
void M_RD_Mouse_Bind_StrafeOn (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebstrafe);
}

//
// Strafe Left
//
void M_RD_Mouse_Bind_StrafeLeft (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebstrafeleft);
}

//
// Strafe Right
//
void M_RD_Mouse_Bind_StrafeRight (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebstraferight);
}

//
// Previous Weapon
//
void M_RD_Mouse_Bind_PrevWeapon (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebprevweapon);
}

//
// Previous Weapon
//
void M_RD_Mouse_Bind_NextWeapon (int choice) 
{
    M_RD_StartBinding_Mouse(&mousebnextweapon);
}


// -----------------------------------------------------------------------------
// Gameplay features
// -----------------------------------------------------------------------------

void M_RD_Draw_Gameplay_1(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        // Brightmaps
        RD_M_DrawTextSmallENG(brightmaps ? RD_ON : RD_OFF, 119 + wide_delta, 35,
                              brightmaps ? CR_GREEN : CR_DARKRED);


        // Fake contrast
        RD_M_DrawTextSmallENG(fake_contrast ? RD_ON : RD_OFF,142 + wide_delta, 45,
                              fake_contrast ? CR_GREEN : CR_DARKRED);

        // Translucency
        RD_M_DrawTextSmallENG(translucency ? RD_ON : RD_OFF, 138 + wide_delta, 55,
                              translucency ? CR_GREEN : CR_DARKRED);

        // Fuzz effect
        RD_M_DrawTextSmallENG(improved_fuzz == 0 ? "Original" :
                              improved_fuzz == 1 ? "Original (b&w)" :
                              improved_fuzz == 2 ? "Improved" :
                              improved_fuzz == 3 ? "Improved (b&w)" :
                              "Translucent", 125 + wide_delta, 65,
                              improved_fuzz > 0 ? CR_GREEN : CR_DARKRED);

        // Colored blood and corpses
        RD_M_DrawTextSmallENG(colored_blood ? RD_ON : RD_OFF, 229 + wide_delta, 75,
                             colored_blood ? CR_GREEN : CR_DARKRED);

        // Swirling liquids
        RD_M_DrawTextSmallENG(swirling_liquids ? RD_ON : RD_OFF, 150 + wide_delta, 85,
                             swirling_liquids ? CR_GREEN : CR_DARKRED);

        // Invulnerability affects sky
        RD_M_DrawTextSmallENG(invul_sky ? RD_ON : RD_OFF, 237 + wide_delta, 95,
                             invul_sky ? CR_GREEN : CR_DARKRED);

        // Horizontally linear sky drawing
        RD_M_DrawTextSmallENG(linear_sky ? "linear" : "original", 160 + wide_delta, 105,
                             linear_sky ? CR_GREEN : CR_DARKRED);

        // Flip weapons
        RD_M_DrawTextSmallENG(flip_weapons ? RD_ON : RD_OFF, 131 + wide_delta, 115,
                             flip_weapons ? CR_GREEN : CR_DARKRED);

        //
        // Footer
        //
        RD_M_DrawTextSmallENG("next page >", 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallENG("< last page", 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallENG("page 1/5", 231 + wide_delta, 155, CR_WHITE);
    }
    else
    {
        // Брайтмаппинг
        RD_M_DrawTextSmallRUS(brightmaps ? RD_ON_RUS : RD_OFF_RUS, 140 + wide_delta, 35,
                              brightmaps ? CR_GREEN : CR_DARKRED);

        // Имитация контрастности
        RD_M_DrawTextSmallRUS(fake_contrast ? RD_ON_RUS : RD_OFF_RUS, 217 + wide_delta, 45,
                              fake_contrast ? CR_GREEN : CR_DARKRED);

        // Прозрачность объектов
        RD_M_DrawTextSmallRUS(translucency ? RD_ON_RUS : RD_OFF_RUS, 207 + wide_delta, 55,
                              translucency ? CR_GREEN : CR_DARKRED);

        // Эффект шума
        RD_M_DrawTextSmallRUS(improved_fuzz == 0 ? "Jhbubyfkmysq" :
                              improved_fuzz == 1 ? "Jhbubyfkmysq (x*,)" :
                              improved_fuzz == 2 ? "Ekexityysq" :
                              improved_fuzz == 3 ? "Ekexityysq (x*,)" :
                              "Ghjphfxysq", 134 + wide_delta, 65,
                              improved_fuzz > 0 ? CR_GREEN : CR_DARKRED);

        // Разноцветная кровь и трупы
        RD_M_DrawTextSmallRUS(colored_blood ? RD_ON_RUS : RD_OFF_RUS, 242 + wide_delta, 75,
                              colored_blood ? CR_GREEN : CR_DARKRED);

        // Улучшенная анимация жидкостей
        RD_M_DrawTextSmallRUS(swirling_liquids ? RD_ON_RUS : RD_OFF_RUS, 275 + wide_delta, 85,
                              swirling_liquids ? CR_GREEN : CR_DARKRED);

        // Неуязвимость окрашивает небо
        RD_M_DrawTextSmallRUS(invul_sky ? RD_ON_RUS : RD_OFF_RUS, 262 + wide_delta, 95,
                              invul_sky ? CR_GREEN : CR_DARKRED);

        // Режим отрисовки неба
        RD_M_DrawTextSmallRUS(linear_sky ? "kbytqysq" : "jhbubyfkmysq", 200 + wide_delta, 105,
                              linear_sky ? CR_GREEN : CR_DARKRED);

        // Зеркальное отражение оружия
        RD_M_DrawTextSmallRUS(flip_weapons ? RD_ON_RUS : RD_OFF_RUS, 259 + wide_delta, 115,
                              flip_weapons ? CR_GREEN : CR_DARKRED);

        //
        // Footer
        //
        RD_M_DrawTextSmallRUS(RD_NEXT_RUS, 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallRUS(RD_PREV_RUS, 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallRUS("cnhfybwf 1*5", 197 + wide_delta, 155, CR_WHITE);
    }
}

void M_RD_Draw_Gameplay_2(void)
{
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        // Extra player faces
        RD_M_DrawTextSmallENG(extra_player_faces ? RD_ON : RD_OFF, 179 + wide_delta, 35,
                              extra_player_faces ? CR_GREEN : CR_DARKRED);

        // Show negative health
        RD_M_DrawTextSmallENG(negative_health ? RD_ON : RD_OFF, 190 + wide_delta, 45,
                              negative_health ? CR_GREEN : CR_DARKRED);

        // Colored elements
        RD_M_DrawTextSmallENG(sbar_colored == 1 ? "ON (NO %)"  :
                              sbar_colored == 2 ? "ON" : "OFF", 167 + wide_delta, 55,
                              sbar_colored ? CR_GREEN : CR_DARKRED);

        RD_M_DrawTextSmallENG("High value:",35 + wide_delta, 75,
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallENG("Normal value:",35 + wide_delta, 85,
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallENG("Low value:",35 + wide_delta, 95,
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallENG("Critical value:",35 + wide_delta, 105,
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallENG("Armor type 1:",35 + wide_delta, 115,
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallENG("Armor type 2:",35 + wide_delta, 125,
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallENG("No armor:", 35 + wide_delta, 135,
                              sbar_colored == 0 || gamemission == jaguar? CR_DARKRED : CR_NONE);
        
        // High Value
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 114 + wide_delta, 75,
                                 sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(sbar_color_high), 114 + wide_delta, 75,
                                  M_RD_ColorTranslation(sbar_color_high));
        }

        // Normal Value
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 135 + wide_delta, 85, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(sbar_color_normal), 135 + wide_delta, 85,
                                  M_RD_ColorTranslation(sbar_color_normal));
        }

        // Low Value
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 111 + wide_delta, 95, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(sbar_color_low), 111 + wide_delta, 95,
                                 M_RD_ColorTranslation(sbar_color_low));
        }

        // Critical value
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 142 + wide_delta, 105, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(sbar_color_critical), 142 + wide_delta, 105,
                                  M_RD_ColorTranslation(sbar_color_critical));
        }

        // Armor type 1
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 129 + wide_delta, 115, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(sbar_color_armor_1), 129 + wide_delta, 115,
                                  M_RD_ColorTranslation(sbar_color_armor_1));
        }

        // Armor type 2
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 132 + wide_delta, 125, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(sbar_color_armor_2), 132 + wide_delta, 125,
                                  M_RD_ColorTranslation(sbar_color_armor_2));
        }

        // Armor type 0
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallENG("n/a", 104 + wide_delta, 135, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(M_RD_ColorName_ENG(sbar_color_armor_0), 104 + wide_delta, 135,
                                  M_RD_ColorTranslation(sbar_color_armor_0));
        }

        //
        // Footer
        //
        RD_M_DrawTextSmallENG("next page >", 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallENG("< prev page", 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallENG("page 2/5", 231 + wide_delta, 155, CR_WHITE);
    }
    else
    {
        // Дополнительные лица игрока
        RD_M_DrawTextSmallRUS(extra_player_faces ? "DRK" : "DSRK", 247 + wide_delta, 35,
                              extra_player_faces ? CR_GREEN : CR_DARKRED);

        // Отрицательное здоровье
        RD_M_DrawTextSmallRUS(negative_health ? "DRK" : "DSRK", 217 + wide_delta, 45,
                              negative_health ? CR_GREEN : CR_DARKRED);

        // Разноцветные элементы
        RD_M_DrawTextSmallRUS(sbar_colored == 1 ? "DRK (,TP %)"  :
                              sbar_colored == 2 ? "DRK" : "DSRK", 213 + wide_delta, 55,
                              sbar_colored ? CR_GREEN : CR_DARKRED);

        // Высокое значение
        RD_M_DrawTextSmallRUS("Dscjrjt pyfxtybt:", 35 + wide_delta, 75,      // Высокое значение
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallRUS("Yjhvfkmyjt pyfxtybt:", 35 + wide_delta, 85,   // Нормальное значение
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallRUS("Ybprjt pyfxtybt:", 35 + wide_delta, 95,       // Низкое значение
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallRUS("Rhbnbxtcrjt pyfxtybt:", 35 + wide_delta, 105, // Критическое значение
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallRUS("Nbg ,hjyb 1:", 35 + wide_delta, 115,          // Тип брони 1
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallRUS("Nbg ,hjyb 2:", 35 + wide_delta, 125,          // Тип брони 2
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        RD_M_DrawTextSmallRUS("Jncencndbt ,hjyb:", 35 + wide_delta, 135,     // Отсутствие брони
                              sbar_colored == 0 || gamemission == jaguar ? CR_DARKRED : CR_NONE);
        
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 168 + wide_delta, 75, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(sbar_color_high), 168 + wide_delta, 75,
                                 M_RD_ColorTranslation(sbar_color_high));
        }

        // Нормальное значение
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 191 + wide_delta, 85, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(sbar_color_normal), 191 + wide_delta, 85,
                                 M_RD_ColorTranslation(sbar_color_normal));
        }

        // Низкое значение
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 158 + wide_delta, 95, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(sbar_color_low), 158 + wide_delta, 95,
                                  M_RD_ColorTranslation(sbar_color_low));
        }

        // Низкое значение
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 197 + wide_delta, 105, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(sbar_color_critical), 197 + wide_delta, 105,
                                  M_RD_ColorTranslation(sbar_color_critical));
        }

        // Тип брони 1
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 120 + wide_delta, 115, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(sbar_color_armor_1), 120 + wide_delta, 115,
                                  M_RD_ColorTranslation(sbar_color_armor_1));
        }

        // Тип брони 2
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 123 + wide_delta, 125, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(sbar_color_armor_2), 123 + wide_delta, 125,
                                  M_RD_ColorTranslation(sbar_color_armor_2));
        }

        // Отсутствие брони
        if (sbar_colored == 0 || gamemission == jaguar)
        {
            RD_M_DrawTextSmallRUS("y*l", 167 + wide_delta, 135, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(M_RD_ColorName_RUS(sbar_color_armor_0), 167 + wide_delta, 135,
                                  M_RD_ColorTranslation(sbar_color_armor_0));
        }

        //
        // Footer
        //
        RD_M_DrawTextSmallRUS(RD_NEXT_RUS, 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallRUS(RD_PREV_RUS, 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallRUS("cnhfybwf 2*5", 197 + wide_delta, 155, CR_WHITE);
    }
}

void M_RD_Draw_Gameplay_3(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        // Sound attenuation axises
        RD_M_DrawTextSmallENG(z_axis_sfx ? "x/y/z" : "x/y", 217 + wide_delta, 35,
                              z_axis_sfx ? CR_GREEN : CR_DARKRED);

        // Play exit sounds
        RD_M_DrawTextSmallENG(play_exit_sfx ? RD_ON : RD_OFF, 158 + wide_delta, 45,
                              play_exit_sfx ? CR_GREEN : CR_DARKRED);

        // Sound of crushing corpses
        RD_M_DrawTextSmallENG(crushed_corpses_sfx ? RD_ON : RD_OFF, 223 + wide_delta, 55,
                              crushed_corpses_sfx ? CR_GREEN : CR_DARKRED);

        // Single sound of closing blazing door
        RD_M_DrawTextSmallENG(blazing_door_fix_sfx ? RD_ON : RD_OFF,240 + wide_delta, 65,
                              blazing_door_fix_sfx ? CR_GREEN : CR_DARKRED);

        // Monster alert waking up other monsters
        RD_M_DrawTextSmallENG(noise_alert_sfx ? RD_ON : RD_OFF,263 + wide_delta, 75,
                              noise_alert_sfx ? CR_GREEN : CR_DARKRED);

        // Notify of revealed secrets
        RD_M_DrawTextSmallENG(secret_notification ? RD_ON : RD_OFF,232 + wide_delta, 95,
                               secret_notification ? CR_GREEN : CR_DARKRED);

        // Infragreen light amp. visor
        RD_M_DrawTextSmallENG(infragreen_visor ? RD_ON : RD_OFF,230 + wide_delta, 105,
                              infragreen_visor ? CR_GREEN : CR_DARKRED);

        //
        // Footer
        //
        RD_M_DrawTextSmallENG("next page >", 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallENG("< prev page", 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallENG("page 3/5", 231 + wide_delta, 155, CR_WHITE);
    }
    else
    {
        // Затухание звука по осям
        RD_M_DrawTextSmallENG(z_axis_sfx ? "x/y/z" : "x/y", 217 + wide_delta, 35,
                              z_axis_sfx ? CR_GREEN : CR_DARKRED);

        // Звуки при выходе из игры
        RD_M_DrawTextSmallRUS(play_exit_sfx ? RD_ON_RUS : RD_OFF_RUS, 225 + wide_delta, 45,
                              play_exit_sfx ? CR_GREEN : CR_DARKRED);

        // Звук раздавливания трупов
        RD_M_DrawTextSmallRUS(crushed_corpses_sfx ? RD_ON_RUS : RD_OFF_RUS, 236 + wide_delta, 55,
                              crushed_corpses_sfx ? CR_GREEN : CR_DARKRED);

        // Одиночный звук быстрой двери
        RD_M_DrawTextSmallRUS(blazing_door_fix_sfx ? RD_ON_RUS : RD_OFF_RUS, 260 + wide_delta, 65,
                              blazing_door_fix_sfx ? CR_GREEN : CR_DARKRED);

        // Общая тревога у монстров
        RD_M_DrawTextSmallRUS(noise_alert_sfx ? RD_ON_RUS : RD_OFF_RUS, 227 + wide_delta, 75,
                              noise_alert_sfx ? CR_GREEN : CR_DARKRED);

        // Сообщать о найденном тайнике
        RD_M_DrawTextSmallRUS(secret_notification ? RD_ON_RUS : RD_OFF_RUS, 260 + wide_delta, 95,
                              secret_notification ? CR_GREEN : CR_DARKRED);

        // Инфразеленый визор освещения
        RD_M_DrawTextSmallRUS(infragreen_visor ? RD_ON_RUS : RD_OFF_RUS, 266 + wide_delta, 105,
                              infragreen_visor ? CR_GREEN : CR_DARKRED);

        // Footer
        RD_M_DrawTextSmallRUS(RD_NEXT_RUS, 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallRUS(RD_PREV_RUS, 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallRUS("cnhfybwf 3*5", 197 + wide_delta, 155, CR_WHITE);
    }
}

void M_RD_Draw_Gameplay_4(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        // Walk over and under monsters
        RD_M_DrawTextSmallENG(over_under ? RD_ON : RD_OFF, 250 + wide_delta, 35,
                              over_under ? CR_GREEN : CR_DARKRED);

        // Corpses sliding from the ledges
        RD_M_DrawTextSmallENG(torque ? RD_ON : RD_OFF, 264 + wide_delta, 45,
                              torque ? CR_GREEN : CR_DARKRED);

        // Weapon bobbing while firing
        RD_M_DrawTextSmallENG(weapon_bobbing ? RD_ON : RD_OFF, 233 + wide_delta, 55,
                              weapon_bobbing ? CR_GREEN : CR_DARKRED);

        // Lethal pellet of a point-blank SSG
        RD_M_DrawTextSmallENG(ssg_blast_enemies ? RD_ON : RD_OFF, 287 + wide_delta, 65,
                              ssg_blast_enemies ? CR_GREEN : CR_DARKRED);

        // Randomly mirrored corpses
        RD_M_DrawTextSmallENG(randomly_flipcorpses ? RD_ON : RD_OFF, 231 + wide_delta, 75,
                              randomly_flipcorpses ? CR_GREEN : CR_DARKRED);

        // Floating powerups
        RD_M_DrawTextSmallENG(floating_powerups ? RD_ON : RD_OFF, 171 + wide_delta, 85,
                              floating_powerups ? CR_GREEN : CR_DARKRED);

        // Items are tossed when dropped
        RD_M_DrawTextSmallENG(toss_drop ? RD_ON : RD_OFF, 254 + wide_delta, 95,
                              toss_drop ? CR_GREEN : CR_DARKRED);

        // Draw crosshair
        RD_M_DrawTextSmallENG(crosshair_draw ? RD_ON : RD_OFF, 146 + wide_delta, 115,
                              crosshair_draw ? CR_GREEN : CR_DARKRED);

        // Indication
        RD_M_DrawTextSmallENG(crosshair_type == 1 ? "Health" :
                              crosshair_type == 2 ? "Target highlighting" :
                              crosshair_type == 3 ? "Target highlighting+Health" :
                              "Static", 111 + wide_delta, 125,
                              crosshair_type ? CR_GREEN : CR_DARKRED);

        // Increased size
        RD_M_DrawTextSmallENG(crosshair_scale ? RD_ON : RD_OFF, 140 + wide_delta, 135,
                              crosshair_scale ? CR_GREEN : CR_DARKRED);

        //
        // Footer
        //
        RD_M_DrawTextSmallENG("next page >", 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallENG("< prev page", 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallENG("page 4/5", 231 + wide_delta, 155, CR_WHITE);
    }
    else
    {
        // Перемещение под/над монстрами
        RD_M_DrawTextSmallRUS(over_under ? RD_ON_RUS : RD_OFF_RUS, 274 + wide_delta, 35,
                              over_under ? CR_GREEN : CR_DARKRED);

        // Трупы сползают с возвышений
        RD_M_DrawTextSmallRUS(torque ? RD_ON_RUS : RD_OFF_RUS, 256 + wide_delta, 45,
                              torque ? CR_GREEN : CR_DARKRED);

        // Улучшенное покачивание оружия
        RD_M_DrawTextSmallRUS(weapon_bobbing ? RD_ON_RUS : RD_OFF_RUS, 271 + wide_delta, 55,
                              weapon_bobbing ? CR_GREEN : CR_DARKRED);

        // Двустволка разрывает врагов
        RD_M_DrawTextSmallRUS(ssg_blast_enemies ? RD_ON_RUS : RD_OFF_RUS, 254 + wide_delta, 65,
                              ssg_blast_enemies ? CR_GREEN : CR_DARKRED);

        // Зеркалирование трупов
        RD_M_DrawTextSmallRUS(randomly_flipcorpses ? RD_ON_RUS : RD_OFF_RUS, 207 + wide_delta, 75,
                              randomly_flipcorpses ? CR_GREEN : CR_DARKRED);

        // Левитирующие сферы-артефакты
        RD_M_DrawTextSmallRUS(floating_powerups ? RD_ON_RUS : RD_OFF_RUS, 275 + wide_delta, 85,
                              floating_powerups ? CR_GREEN : CR_DARKRED);

        // Подбрасывать выпавшие предметы
        RD_M_DrawTextSmallRUS(toss_drop ? RD_ON_RUS : RD_OFF_RUS, 285 + wide_delta, 95,
                              toss_drop ? CR_GREEN : CR_DARKRED);

        // Отображать прицел
        RD_M_DrawTextSmallRUS(crosshair_draw ? RD_ON_RUS : RD_OFF_RUS, 180 + wide_delta, 115,
                              crosshair_draw ? CR_GREEN : CR_DARKRED);

        // Индикация
        RD_M_DrawTextSmallRUS(crosshair_type == 1 ? "Pljhjdmt" :       // Здоровье
                              crosshair_type == 2 ? "Gjlcdtnrf wtkb" : // Подсветка цели
                              crosshair_type == 3 ? "Gjlcdtnrf wtkb+pljhjdmt" : // Подсветка цели + здоровье
                              "Cnfnbxyfz", 117 + wide_delta, 125, // Статичная
                              crosshair_type ? CR_GREEN : CR_DARKRED);

        // Увеличенный размер
        RD_M_DrawTextSmallRUS(crosshair_scale ? RD_ON_RUS : RD_OFF_RUS, 185 + wide_delta, 135,
                              crosshair_scale ? CR_GREEN : CR_DARKRED);

        //
        // Footer
        //
        RD_M_DrawTextSmallRUS(RD_NEXT_RUS, 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallRUS(RD_PREV_RUS, 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallRUS("cnhfybwf 4*5", 197 + wide_delta, 155, CR_WHITE);
    }
}

void M_RD_Draw_Gameplay_5(void)
{   
    // Jaguar: hide game background, don't draw lines over the HUD
    if (gamemission == jaguar)
    {
        inhelpscreens = true;
        V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("INTERPIC"), PU_CACHE), false);
    }

    if (english_language)
    {
        // Fix errors of vanilla maps
        RD_M_DrawTextSmallENG(fix_map_errors ? RD_ON : RD_OFF, 226 + wide_delta, 35,
                              fix_map_errors ? CR_GREEN : CR_DARKRED);

        // Flip game levels
        RD_M_DrawTextSmallENG(flip_levels ? RD_ON : RD_OFF, 158 + wide_delta, 45,
                              flip_levels ? CR_GREEN : CR_DARKRED);

        // Pain Elemental without Souls limit
        RD_M_DrawTextSmallENG(unlimited_lost_souls ? RD_ON : RD_OFF, 284 + wide_delta, 55,
                              unlimited_lost_souls ? CR_GREEN : CR_DARKRED);

        // More agressive lost souls
        RD_M_DrawTextSmallENG(agressive_lost_souls ? RD_ON : RD_OFF, 230 + wide_delta, 65,
                              agressive_lost_souls ? CR_GREEN : CR_DARKRED);

        // Pistol start
        RD_M_DrawTextSmallENG(pistol_start ? RD_ON : RD_OFF, 203 + wide_delta, 75,
                              pistol_start ? CR_GREEN : CR_DARKRED);

        // Show demo timer
        RD_M_DrawTextSmallENG(demotimer == 1 ? "playback" :
                              demotimer == 2 ? "recording" :
                              demotimer == 3 ? "always" :
                              "off", 153 + wide_delta, 95,
                              demotimer > 0 ? CR_GREEN : CR_DARKRED);

        // Timer direction
        RD_M_DrawTextSmallENG(demotimerdir ? "backward" : "forward", 148 + wide_delta, 105,
                              demotimer > 0 ? CR_GREEN : CR_DARKRED);

        // Show progress bar 
        RD_M_DrawTextSmallENG(demobar ? RD_ON : RD_OFF, 169 + wide_delta, 115,
                              demobar ? CR_GREEN : CR_DARKRED);

        // Play internal demos
        RD_M_DrawTextSmallENG(no_internal_demos ? RD_OFF : RD_ON, 183 + wide_delta, 125,
                              no_internal_demos ? CR_DARKRED : CR_GREEN);

        //
        // Footer
        //
        RD_M_DrawTextSmallENG("first page >", 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallENG("< prev page", 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallENG("page 5/5", 231 + wide_delta, 155, CR_WHITE);
    }
    else
    {
        // Устранять ошибки ориг. уровней
        RD_M_DrawTextSmallRUS(fix_map_errors ? RD_ON_RUS : RD_OFF_RUS, 269 + wide_delta, 35,
                              fix_map_errors ? CR_GREEN : CR_DARKRED);

        // Устранять ошибки ориг. уровней
        RD_M_DrawTextSmallRUS(flip_levels ? RD_ON_RUS : RD_OFF_RUS, 263 + wide_delta, 45,
                              flip_levels ? CR_GREEN : CR_DARKRED);

        // Элементаль без ограничения Душ
        RD_M_DrawTextSmallRUS(unlimited_lost_souls ? RD_ON_RUS : RD_OFF_RUS, 274 + wide_delta, 55,
                              unlimited_lost_souls ? CR_GREEN : CR_DARKRED);

        // Повышенная агрессивность Душ
        RD_M_DrawTextSmallRUS(agressive_lost_souls ? RD_ON_RUS : RD_OFF_RUS, 266 + wide_delta, 65,
                              agressive_lost_souls ? CR_GREEN : CR_DARKRED);

        // Режим игры "Pistol start"
        RD_M_DrawTextSmallRUS("ht;bv buhs ^", 35 + wide_delta, 75, CR_NONE);
        RD_M_DrawTextSmallENG("\"Pistol start\":", 121 + wide_delta, 75, CR_NONE);
        RD_M_DrawTextSmallRUS(pistol_start ? RD_ON_RUS : RD_OFF_RUS, 229 + wide_delta, 75,
                              pistol_start ? CR_GREEN : CR_DARKRED);

        // Отображать таймер
        RD_M_DrawTextSmallRUS(demotimer == 1 ? "ghb ghjbuhsdfybb" :
                              demotimer == 2 ? "ghb pfgbcb" :
                              demotimer == 3 ? "dctulf" :
                              "dsrk", 180 + wide_delta, 95,
                              demotimer > 0 ? CR_GREEN : CR_DARKRED);

        // Время таймера
        RD_M_DrawTextSmallRUS(demotimerdir ? "jcnfdittcz" : "ghjitlitt", 145 + wide_delta, 105,
                              demotimer > 0 ? CR_GREEN : CR_DARKRED);

        // Шкала прогресса
        RD_M_DrawTextSmallRUS(demobar ? RD_ON_RUS : RD_OFF_RUS, 161 + wide_delta, 115,
                              demobar ? CR_GREEN : CR_DARKRED);

        // Проигрывать демозаписи
        RD_M_DrawTextSmallRUS(no_internal_demos ? RD_OFF_RUS : RD_ON_RUS, 219 + wide_delta, 125,
                              no_internal_demos ? CR_DARKRED : CR_GREEN);

        //
        // Footer
        //
        RD_M_DrawTextSmallRUS(RD_NEXT_RUS, 35 + wide_delta, 145, CR_WHITE);
        RD_M_DrawTextSmallRUS(RD_PREV_RUS, 35 + wide_delta, 155, CR_WHITE);
        RD_M_DrawTextSmallRUS("cnhfybwf 5*5", 197 + wide_delta, 155, CR_WHITE);
    }
}

void M_RD_Change_Brightmaps(Direction_t direction)
{
    brightmaps ^= 1;
}

void M_RD_Change_FakeContrast(Direction_t direction)
{
    fake_contrast ^= 1;
}

void M_RD_Change_Translucency(Direction_t direction)
{
    translucency ^= 1;
}

void M_RD_Change_ImprovedFuzz(Direction_t direction)
{
    RD_Menu_SpinInt(&improved_fuzz, 0, 4, direction);

    // Redraw game screen
    R_ExecuteSetViewSize();
}

void M_RD_Change_ColoredBlood(Direction_t direction)
{
    colored_blood ^= 1;
}

void M_RD_Change_SwirlingLiquids(Direction_t direction)
{
    swirling_liquids ^= 1;
}

void M_RD_Change_InvulSky(Direction_t direction)
{
    invul_sky ^= 1;
}

void M_RD_Change_LinearSky(Direction_t direction)
{
    linear_sky ^= 1;
}

void M_RD_Change_FlipWeapons(Direction_t direction)
{
    flip_weapons ^= 1;

    // [JN] Skip weapon bobbing interpolation for next frame.
    skippsprinterp = true;
}

//
// Gameplay: Status Bar
//

void M_RD_Change_ExtraPlayerFaces(Direction_t direction)
{
    extra_player_faces ^= 1;
}

void M_RD_Change_NegativeHealth(Direction_t direction)
{
    negative_health ^= 1;
}

void M_RD_Change_SBarColored(Direction_t direction)
{
    RD_Menu_SpinInt(&sbar_colored, 0, 2, direction);
    
    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }
}

void M_RD_Define_SBarColorValue(byte** sbar_color_set, int color)
{
    // [JN] No coloring in vanilla or Jaguar Doom.
    if (vanillaparm || gamemission == jaguar)
    {
        *sbar_color_set = NULL;
    }
    else
    {
        switch (color)
        {
            case 1:   *sbar_color_set = cr[CR_DARKRED];    break;
            case 2:   *sbar_color_set = cr[CR_GREEN];      break;
            case 3:   *sbar_color_set = cr[CR_DARKGREEN];  break;
            case 4:   *sbar_color_set = cr[CR_OLIVE];      break;
            case 5:   *sbar_color_set = cr[CR_BLUE2];      break;
            case 6:   *sbar_color_set = cr[CR_DARKBLUE];   break;
            case 7:   *sbar_color_set = cr[CR_YELLOW];       break;
            case 8:   *sbar_color_set = cr[CR_ORANGE];     break;
            case 9:   *sbar_color_set = cr[CR_WHITE];      break;
            case 10:  *sbar_color_set = cr[CR_GRAY];       break;
            case 11:  *sbar_color_set = cr[CR_DARKGRAY];   break;
            case 12:  *sbar_color_set = cr[CR_TAN];        break;
            case 13:  *sbar_color_set = cr[CR_BROWN];    break;
            case 14:  *sbar_color_set = cr[CR_ALMOND];      break;
            case 15:  *sbar_color_set = cr[CR_KHAKI];  break;
            case 16:  *sbar_color_set = cr[CR_PINK];      break;
            case 17:  *sbar_color_set = cr[CR_BURGUNDY];  break;
            default:  *sbar_color_set = NULL;              break;
        }
    }
}

void M_RD_Change_SBarHighValue(Direction_t direction)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&sbar_color_high, 0, 17, direction);

    // [JN] Redefine system message color.
    M_RD_Define_SBarColorValue(&sbar_color_high_set, sbar_color_high);

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_SBarNormalValue(Direction_t direction)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&sbar_color_normal, 0, 17, direction);

    // [JN] Redefine system message color.
    M_RD_Define_SBarColorValue(&sbar_color_normal_set, sbar_color_normal);

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_SBarLowValue(Direction_t direction)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&sbar_color_low, 0, 17, direction);

    // [JN] Redefine system message color.
    M_RD_Define_SBarColorValue(&sbar_color_low_set, sbar_color_low);

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_SBarCriticalValue(Direction_t direction)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&sbar_color_critical, 0, 17, direction);

    // [JN] Redefine system message color.
    M_RD_Define_SBarColorValue(&sbar_color_critical_set, sbar_color_critical);

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_SBarArmorType1(Direction_t direction)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&sbar_color_armor_1, 0, 17, direction);

    // [JN] Redefine system message color.
    M_RD_Define_SBarColorValue(&sbar_color_armor_1_set, sbar_color_armor_1);

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_SBarArmorType2(Direction_t direction)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&sbar_color_armor_2, 0, 17, direction);

    // [JN] Redefine system message color.
    M_RD_Define_SBarColorValue(&sbar_color_armor_2_set, sbar_color_armor_2);

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_SBarArmorType0(Direction_t direction)
{
    // [JN] Disallow changing if not appropriate.
    if (sbar_colored == 0 || gamemission == jaguar)
        return;

    RD_Menu_SpinInt(&sbar_color_armor_0, 0, 17, direction);

    // [JN] Redefine system message color.
    M_RD_Define_SBarColorValue(&sbar_color_armor_0_set, sbar_color_armor_0);

    // [JN] Routine №3: play sound only if necessary.
    S_StartSound(NULL,sfx_stnmov);
}

void M_RD_Change_ZAxisSfx(Direction_t direction)
{
    z_axis_sfx ^= 1;
}

void M_RD_Change_ExitSfx(Direction_t direction)
{
    play_exit_sfx ^= 1;
}

void M_RD_Change_CrushingSfx(Direction_t direction)
{
    crushed_corpses_sfx ^= 1;
}

void M_RD_Change_BlazingSfx(Direction_t direction)
{
    blazing_door_fix_sfx ^= 1;
}

void M_RD_Change_AlertSfx(Direction_t direction)
{
     noise_alert_sfx ^= 1;
}

void M_RD_Change_SecretNotify(Direction_t direction)
{
    secret_notification ^= 1;
}

void M_RD_Change_InfraGreenVisor(Direction_t direction)
{
    infragreen_visor ^= 1;

    // [JN] Update current COLORMAP
    if (players[consoleplayer].powers[pw_infrared])
    {
        if (players[consoleplayer].powers[pw_invulnerability])
        {
            // Set B&W colormap
            players[consoleplayer].fixedcolormap = 32;
        }
        else
        {
            // Set full bright colormap
            players[consoleplayer].fixedcolormap = 1;
        }
    }
}

void M_RD_Change_WalkOverUnder(Direction_t direction)
{
    over_under ^= 1;
}

void M_RD_Change_Torque(Direction_t direction)
{
    torque ^= 1;
}

void M_RD_Change_Bobbing(Direction_t direction)
{
    weapon_bobbing ^= 1;
}

void M_RD_Change_SSGBlast(Direction_t direction)
{
    ssg_blast_enemies ^= 1;
}

void M_RD_Change_FlipCorpses(Direction_t direction)
{
    randomly_flipcorpses ^= 1;
}

void M_RD_Change_FloatPowerups(Direction_t direction)
{
    floating_powerups ^= 1;
}

void M_RD_Change_TossDrop(Direction_t direction)
{
    toss_drop ^= 1;
}

void M_RD_Change_CrosshairDraw(Direction_t direction)
{
    crosshair_draw ^= 1;
}

void M_RD_Change_CrosshairType(Direction_t direction)
{
    RD_Menu_SpinInt(&crosshair_type, 0, 3, direction);
}

void M_RD_Change_CrosshairScale(Direction_t direction)
{
    crosshair_scale ^= 1;
}

void M_RD_Change_FixMapErrors(Direction_t direction)
{
    fix_map_errors ^= 1;
}

void M_RD_Change_FlipLevels(Direction_t direction)
{
    flip_levels ^= 1;

    // Redraw game screen
    R_ExecuteSetViewSize();

    // Update stereo separation
    S_UpdateStereoSeparation();
}

void M_RD_Change_LostSoulsQty(Direction_t direction)
{
    unlimited_lost_souls ^= 1;
}

void M_RD_Change_LostSoulsAgr(Direction_t direction)
{
    agressive_lost_souls ^= 1;
}

void M_RD_Change_PistolStart(Direction_t direction)
{
    pistol_start ^= 1;
}

void M_RD_Change_DemoTimer(Direction_t direction)
{
    RD_Menu_SpinInt(&demotimer, 0, 3, direction);
}

void M_RD_Change_DemoTimerDir(Direction_t direction)
{
    demotimerdir ^= 1;
}

void M_RD_Change_DemoBar(Direction_t direction)
{
    demobar ^= 1;

    // Refresh status bar
    if (gamestate == GS_LEVEL)
    {
        ST_doRefresh();
    }
}

void M_RD_Change_NoInternalDemos(Direction_t direction)
{
    no_internal_demos ^= 1;
}

// -----------------------------------------------------------------------------
// Level select
// -----------------------------------------------------------------------------

void M_LevelSelect(int choice)
{
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(english_language ?
                       NEWGAME : NEWGAME_RUS), NULL,false);
        return;
    }
    SetMenu(&LevelSelect1Menu);
}

void M_RD_Draw_Level_1(void)
{
    static char num[4];

    // [JN] Erase the entire screen to a tiled background.
    inhelpscreens = true;
    V_FillFlat ("FLOOR4_8");

    if (english_language)
    {
        // Skill level
        M_snprintf(num, 4, "%d", selective_skill+1);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 20, CR_NONE);

        // Episode
        if (logical_gamemission != doom)
        {
            RD_M_DrawTextSmallENG("N/A", 226 + wide_delta, 30, CR_DARKRED);
        }
        else
        {
            if (gamemode == shareware)
            {
                RD_M_DrawTextSmallENG("1", 226 + wide_delta, 30, CR_DARKRED);
            }
            else
            {
                M_snprintf(num, 4, "%d", selective_episode);
                RD_M_DrawTextSmallENG( num, 226 + wide_delta, 30,CR_NONE);
            }
        }

        // Map
        if (gamemode == pressbeta)
        {
            RD_M_DrawTextSmallENG("1", 226 + wide_delta, 40, CR_DARKRED);
        }
        else
        {
            M_snprintf(num, 4, "%d", selective_map);
            RD_M_DrawTextSmallENG(num, 226 + wide_delta, 40, CR_NONE);
        }

        // Health
        M_snprintf(num, 4, "%d", selective_health);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 60,
                              selective_health > 100 ? CR_BLUE2 :
                              selective_health >= 67 ? CR_GREEN :
                              selective_health >= 34 ? CR_YELLOW :
                                                      CR_RED);

        // Armor
        M_snprintf(num, 4, "%d", selective_armor);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 70,
                              selective_armor == 0 ? CR_RED :
                              selective_armortype == 1 ? CR_GREEN :
                                                        CR_BLUE2);

        // Armor type
        M_snprintf(num, 4, "%d", selective_armortype);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 80,
                              selective_armortype == 1 ? CR_GREEN : CR_BLUE2);

        // Chainsaw
        RD_M_DrawTextSmallENG(selective_wp_chainsaw ? "YES" : "NO", 226 + wide_delta, 100,
                              selective_wp_chainsaw ? CR_GREEN : CR_DARKRED);

        // Shotgun
        RD_M_DrawTextSmallENG(selective_wp_shotgun ? "YES" : "NO", 226 + wide_delta, 110,
                              selective_wp_shotgun ? CR_GREEN : CR_DARKRED);

        // Super Shotgun
        if (logical_gamemission == doom || gamemission == jaguar)
        {
            // Not available in Doom 1 and Jaguar
            RD_M_DrawTextSmallENG("N/A", 226 + wide_delta, 120, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(selective_wp_supershotgun ? "YES" : "NO", 226 + wide_delta, 120,
                                  selective_wp_supershotgun ? CR_GREEN : CR_DARKRED);
        }

        // Chaingun
        RD_M_DrawTextSmallENG(selective_wp_chaingun ? "YES" : "NO", 226 + wide_delta, 130,
                              selective_wp_chaingun ? CR_GREEN : CR_DARKRED);

        // Rocket Launcher
        RD_M_DrawTextSmallENG(selective_wp_missile ? "YES" : "NO", 226 + wide_delta, 140,
                              selective_wp_missile ? CR_GREEN : CR_DARKRED);

        // Plasma Gun
        if (gamemode == shareware)
        {
            // Not available in shareware
            RD_M_DrawTextSmallENG("N/A", 226 + wide_delta, 150, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(selective_wp_plasma ? "YES" : "NO", 226 + wide_delta, 150,
                                  selective_wp_plasma ? CR_GREEN : CR_DARKRED);
        }

        // BFG9000
        if (gamemode == shareware)
        {
            // Not available in shareware
            RD_M_DrawTextSmallENG("N/A", 226 + wide_delta, 160, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallENG(selective_wp_bfg ? "YES" : "NO", 226 + wide_delta, 160,
                                  selective_wp_bfg ? CR_GREEN : CR_DARKRED);
        }

        // Next page
        RD_M_DrawTextSmallENG("NEXT PAGE", 75 + wide_delta, 180, CR_WHITE);

        // Start game
        RD_M_DrawTextSmallENG("START GAME", 75 + wide_delta, 190, CR_WHITE);
    }
    else
    {
        // Сложность
        M_snprintf(num, 4, "%d", selective_skill+1);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 20, CR_NONE);

        // Эпизод
        if (logical_gamemission != doom)
        {
            RD_M_DrawTextSmallRUS("y*l", 226 + wide_delta, 30, CR_DARKRED);
        }
        else
        {
            if (gamemode == shareware)
            {
                RD_M_DrawTextSmallENG("1", 226 + wide_delta, 30, CR_DARKRED);
            }
            else
            {
                M_snprintf(num, 4, "%d", selective_episode);
                RD_M_DrawTextSmallENG(num, 226 + wide_delta, 30, CR_NONE);
            }
        }

        // Уровень
        if (gamemode == pressbeta)
        {
            RD_M_DrawTextSmallENG("1", 226 + wide_delta, 40, CR_DARKRED);
        }
        else
        {
            M_snprintf(num, 4, "%d", selective_map);
            RD_M_DrawTextSmallENG(num, 226 + wide_delta, 40, CR_NONE);
        }

        // Здоровье
        M_snprintf(num, 4, "%d", selective_health);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 60,
                              selective_health > 100 ? CR_BLUE2 :
                              selective_health >= 67 ? CR_GREEN :
                              selective_health >= 34 ? CR_YELLOW :
                              CR_RED);

        // Броня
        M_snprintf(num, 4, "%d", selective_armor);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 70,
                              selective_armor == 0 ? CR_RED :
                              selective_armortype == 1 ? CR_GREEN :
                              CR_BLUE2);

        // Тип брони
        M_snprintf(num, 4, "%d", selective_armortype);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 80,
                              selective_armortype == 1 ? CR_GREEN : CR_BLUE2);

        // Бензопила
        RD_M_DrawTextSmallRUS(selective_wp_chainsaw ? "lf" : "ytn", 226 + wide_delta, 100,
                              selective_wp_chainsaw ? CR_GREEN : CR_DARKRED);

        // Ружье
        RD_M_DrawTextSmallRUS(selective_wp_shotgun ? "lf" : "ytn", 226 + wide_delta, 110,
                              selective_wp_shotgun ? CR_GREEN : CR_DARKRED);

        // Двуствольное ружье
        if (logical_gamemission == doom || gamemission == jaguar)
        {
            // Not available in Doom 1 and Jaguar
            RD_M_DrawTextSmallRUS("y*l", 226 + wide_delta, 120, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(selective_wp_supershotgun ? "lf" : "ytn", 226 + wide_delta, 120,
                                  selective_wp_supershotgun ? CR_GREEN : CR_DARKRED);
        }

        // Пулемет
        RD_M_DrawTextSmallRUS(selective_wp_chaingun ? "lf" : "ytn", 226 + wide_delta, 130,
                              selective_wp_chaingun ? CR_GREEN : CR_DARKRED);

        // Ракетница
        RD_M_DrawTextSmallRUS(selective_wp_missile ? "lf" : "ytn", 226 + wide_delta, 140,
                              selective_wp_missile ? CR_GREEN : CR_DARKRED);

        // Плазменная пушка
        if (gamemode == shareware)
        {
            // Not available in shareware
            RD_M_DrawTextSmallRUS("y*l", 226 + wide_delta, 150, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(selective_wp_plasma ? "lf" : "ytn", 226 + wide_delta, 150,
                                  selective_wp_plasma ? CR_GREEN : CR_DARKRED);
        }

        // BFG9000
        if (gamemode == shareware)
        {
            // Not available in shareware
            RD_M_DrawTextSmallRUS("y*l", 226 + wide_delta, 160, CR_DARKRED);
        }
        else
        {
            RD_M_DrawTextSmallRUS(selective_wp_bfg ? "lf" : "ytn", 226 + wide_delta, 160,
                                  selective_wp_bfg ? CR_GREEN : CR_DARKRED);
        }

        // Следующая страница
        RD_M_DrawTextSmallRUS("cktle.ofz cnhfybwf", 72 + wide_delta, 180, CR_WHITE);

        // Начать игру
        RD_M_DrawTextSmallRUS("yfxfnm buhe", 72 + wide_delta, 190, CR_WHITE);
    }
}

void M_RD_Draw_Level_2(void)
{
    static char num[4];

    // [JN] Erase the entire screen to a tiled background.
    inhelpscreens = true;
    V_FillFlat ("FLOOR4_8");

    if (english_language)
    {
        // Backpack
        RD_M_DrawTextSmallENG(selective_backpack ? "YES" : "NO", 226 + wide_delta, 20,
                              selective_backpack ? CR_GREEN : CR_DARKRED);

        // Bullets
        M_snprintf(num, 4, "%d", selective_ammo_0);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 30,
                              selective_ammo_0 >   200 ? CR_BLUE2 :
                              selective_ammo_0 >=  100 ? CR_GREEN :
                              selective_ammo_0 >=  50  ? CR_YELLOW :
                              CR_RED);

        // Shells
        M_snprintf(num, 4, "%d", selective_ammo_1);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 40,
                              selective_ammo_1 >   50 ? CR_BLUE2 :
                              selective_ammo_1 >=  25 ? CR_GREEN :
                              selective_ammo_1 >=  12 ? CR_YELLOW :
                              CR_RED);

        // Rockets
        M_snprintf(num, 4, "%d", selective_ammo_3);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 50,
                              selective_ammo_3 >   50 ? CR_BLUE2 :
                              selective_ammo_3 >=  25 ? CR_GREEN :
                              selective_ammo_3 >=  12 ? CR_YELLOW :
                              CR_RED);

        // Cells
        M_snprintf(num, 4, "%d", selective_ammo_2);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 60,
                              selective_ammo_2 >  300 ? CR_BLUE2 :
                              selective_ammo_2 >= 150 ? CR_GREEN :
                              selective_ammo_2 >=  75 ? CR_YELLOW :
                              CR_RED);

        // Blue keycard
        RD_M_DrawTextSmallENG(selective_key_0 ? "YES" : "NO", 226 + wide_delta, 80,
                              selective_key_0 ? CR_GREEN : CR_DARKRED);

        // Yellow keycard
        RD_M_DrawTextSmallENG(selective_key_1 ? "YES" : "NO", 226 + wide_delta, 90,
                              selective_key_1 ? CR_GREEN : CR_DARKRED);

        // Red keycard
        RD_M_DrawTextSmallENG(selective_key_2 ? "YES" : "NO", 226 + wide_delta, 100,
                               selective_key_2 ? CR_GREEN : CR_DARKRED);

        // Blue skull key
        RD_M_DrawTextSmallENG(selective_key_3 ? "YES" : "NO", 226 + wide_delta, 110,
                               selective_key_3 ? CR_GREEN : CR_DARKRED);

        // Yellow skull key
        RD_M_DrawTextSmallENG(selective_key_4 ? "YES" : "NO", 226 + wide_delta, 120,
                               selective_key_4 ? CR_GREEN : CR_DARKRED);

        // Red skull key
        RD_M_DrawTextSmallENG(selective_key_5 ? "YES" : "NO", 226 + wide_delta, 130,
                               selective_key_5 ? CR_GREEN : CR_DARKRED);

        // Fast monsters
        RD_M_DrawTextSmallENG(selective_fast ? "YES" : "NO", 226 + wide_delta, 150,
                               selective_fast ? CR_GREEN : CR_DARKRED);

        // Respawning monsters
        RD_M_DrawTextSmallENG(selective_respawn ? "YES" : "NO", 226 + wide_delta, 160,
                               selective_respawn ? CR_GREEN : CR_DARKRED);

        // Previous page
        RD_M_DrawTextSmallENG("PREVIOUS PAGE", 75 + wide_delta, 180, CR_WHITE);

        // Start game
        RD_M_DrawTextSmallENG("START GAME", 75 + wide_delta, 190, CR_WHITE);
    }
    else
    {

        // Рюкзак
        RD_M_DrawTextSmallRUS(selective_backpack ? "lf" : "ytn", 226 + wide_delta, 20,
                              selective_backpack ? CR_GREEN : CR_DARKRED);

        // Пули
        M_snprintf(num, 4, "%d", selective_ammo_0);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 30,
                              selective_ammo_0 >  200 ? CR_BLUE2 :
                              selective_ammo_0 >= 100 ? CR_GREEN :
                              selective_ammo_0 >= 50  ? CR_YELLOW :
                                                       CR_RED);

        // Дробь
        M_snprintf(num, 4, "%d", selective_ammo_1);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 40,
                              selective_ammo_1 >  50 ? CR_BLUE2 :
                              selective_ammo_1 >= 25 ? CR_GREEN :
                              selective_ammo_1 >= 12 ? CR_YELLOW :
                                                      CR_RED);

        // Ракеты
        M_snprintf(num, 4, "%d", selective_ammo_3);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 50,
                              selective_ammo_3 >  50 ? CR_BLUE2 :
                              selective_ammo_3 >= 25 ? CR_GREEN :
                              selective_ammo_3 >= 12 ? CR_YELLOW :
                                                      CR_RED);

        // Энергия
        M_snprintf(num, 4, "%d", selective_ammo_2);
        RD_M_DrawTextSmallENG(num, 226 + wide_delta, 60,
                              selective_ammo_2 >  300 ? CR_BLUE2 :
                              selective_ammo_2 >= 150 ? CR_GREEN :
                              selective_ammo_2 >=  75 ? CR_YELLOW :
                                                       CR_RED);

        // Синяя ключ-карта
        RD_M_DrawTextSmallRUS(selective_key_0 ? "lf" : "ytn", 226 + wide_delta, 80,
                              selective_key_0 ? CR_GREEN : CR_DARKRED);

        // Желтая ключ-карта
        RD_M_DrawTextSmallRUS(selective_key_1 ? "lf" : "ytn", 226 + wide_delta, 90,
                              selective_key_1 ? CR_GREEN : CR_DARKRED);

        // Красная ключ-карта
        RD_M_DrawTextSmallRUS(selective_key_2 ? "lf" : "ytn", 226 + wide_delta, 100,
                              selective_key_2 ? CR_GREEN : CR_DARKRED);

        // Синий ключ-череп
        RD_M_DrawTextSmallRUS(selective_key_3 ? "lf" : "ytn", 226 + wide_delta, 110,
                              selective_key_3 ? CR_GREEN : CR_DARKRED);

        // Желтый ключ-череп
        RD_M_DrawTextSmallRUS(selective_key_4 ? "lf" : "ytn", 226 + wide_delta, 120,
                              selective_key_4 ? CR_GREEN : CR_DARKRED);

        // Красный ключ-череп
        RD_M_DrawTextSmallRUS(selective_key_5 ? "lf" : "ytn", 226 + wide_delta, 130,
                              selective_key_5 ? CR_GREEN : CR_DARKRED);

        // Ускоренные
        RD_M_DrawTextSmallRUS(selective_fast ? "lf" : "ytn", 226 + wide_delta, 150,
                              selective_fast ? CR_GREEN : CR_DARKRED);

        // Воскрешающиеся
        RD_M_DrawTextSmallRUS(selective_respawn ? "lf" : "ytn", 226 + wide_delta, 160,
                              selective_respawn ? CR_GREEN : CR_DARKRED);

        // Предыдущая страница
        RD_M_DrawTextSmallRUS("ghtlsleofz cnhfybwf", 72 + wide_delta, 180, CR_WHITE);

        // Начать игру
        RD_M_DrawTextSmallRUS("yfxfnm buhe", 72 + wide_delta, 190, CR_WHITE);
    }
}

void M_RD_Change_Selective_Skill(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_skill, 0, 5, direction);
}

void M_RD_Change_Selective_Episode(Direction_t direction)
{
    int epiWas;

    // [JN] Shareware have only 1 episode,
    // Doom 2 doest not have episodes at all.
    if (gamemode == shareware || gamemode == commercial)
        return;

    epiWas = selective_episode;
    RD_Menu_SlideInt(&selective_episode, 1,
                     (gamemode == pressbeta || (gamemode == registered && !sgl_loaded) ? 3 :
                     (gamemode == retail && sgl_loaded) ? 5 : 4), direction);
    // [Dasperal] Skip 4 episode for Registered with Sigil
    if(gamemode == registered && sgl_loaded && selective_episode == 4)
    {
        if(epiWas == 5)
            selective_episode = 3;
        else
            selective_episode = 5;
    }
}

void M_RD_Change_Selective_Map(Direction_t direction)
{
    // [JN] There are three episoder with one map for each in Press Beta.
    if (gamemode == pressbeta)
        return;

    RD_Menu_SlideInt(&selective_map, 1,
                    (logical_gamemission == doom ? 9  :
                    gamemission == jaguar ? 25 : 32), direction);
}

void M_RD_Change_Selective_Health(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_health, 1, 200, direction);
}

void M_RD_Change_Selective_Armor(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_armor, 0, 200, direction);
}

void M_RD_Change_Selective_ArmorType(Direction_t direction)
{
    selective_armortype++;
    
    if (selective_armortype > 2)
        selective_armortype = 1;
}

void M_RD_Change_Selective_WP_Chainsaw(Direction_t direction)
{
    selective_wp_chainsaw ^= 1;
}

void M_RD_Change_Selective_WP_Shotgun(Direction_t direction)
{
    selective_wp_shotgun ^= 1;
}

void M_RD_Change_Selective_WP_SSgun(Direction_t direction)
{
    // Not available in Doom 1 and Jaguar
    if (logical_gamemission == doom || gamemission == jaguar)
        return;

    selective_wp_supershotgun ^= 1;
}

void M_RD_Change_Selective_WP_Chaingun(Direction_t direction)
{
    selective_wp_chaingun ^= 1;
}

void M_RD_Change_Selective_WP_RLauncher(Direction_t direction)
{
    selective_wp_missile ^= 1;
}

void M_RD_Change_Selective_WP_Plasmagun(Direction_t direction)
{
    // Not available in shareware
    if (gamemode == shareware)
        return;

    selective_wp_plasma ^= 1;
}

void M_RD_Change_Selective_WP_BFG9000(Direction_t direction)
{
    // Not available in shareware
    if (gamemode == shareware)
        return;

    selective_wp_bfg ^= 1;
}

void M_RD_Change_Selective_Backpack(Direction_t direction)
{
    selective_backpack ^= 1;

    if (!selective_backpack)
    {
        if (selective_ammo_0 > 200)
            selective_ammo_0 = 200;
        if (selective_ammo_1 > 50)
            selective_ammo_1 = 50;
        if (selective_ammo_3 > 50)
            selective_ammo_3 = 50;
        if (selective_ammo_2 > 300)
            selective_ammo_2 = 300;
    }
}

void M_RD_Change_Selective_Ammo_0(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_0, 0, selective_backpack ? 400 : 200, direction);
}

void M_RD_Change_Selective_Ammo_1(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_1, 0, selective_backpack ? 100 : 50, direction);
}

void M_RD_Change_Selective_Ammo_2(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_2, 0, selective_backpack ? 600 : 300, direction);
}

void M_RD_Change_Selective_Ammo_3(Direction_t direction)
{
    RD_Menu_SlideInt(&selective_ammo_3, 0, selective_backpack ? 100 : 50, direction);
}

void M_RD_Change_Selective_Key_0(Direction_t direction)
{
    selective_key_0 ^= 1;
}

void M_RD_Change_Selective_Key_1(Direction_t direction)
{
    selective_key_1 ^= 1;
}

void M_RD_Change_Selective_Key_2(Direction_t direction)
{
    selective_key_2 ^= 1;
}

void M_RD_Change_Selective_Key_3(Direction_t direction)
{
    selective_key_3 ^= 1;
}

void M_RD_Change_Selective_Key_4(Direction_t direction)
{
    selective_key_4 ^= 1;
}

void M_RD_Change_Selective_Key_5(Direction_t direction)
{
    selective_key_5 ^= 1;
}

void M_RD_Change_Selective_Fast(Direction_t direction)
{
    selective_fast ^= 1;
}

void M_RD_Change_Selective_Respawn(Direction_t direction)
{
    selective_respawn ^= 1;
}

// -----------------------------------------------------------------------------
// Back to Defaults
// -----------------------------------------------------------------------------

void M_RD_Draw_Reset(void)
{   
    // [JN] Jaguar Doom: clear remainings of bottom strings from the status bar.
    if (gamemission == jaguar)
        inhelpscreens = true;

    if (english_language)
    {
        M_WriteTextSmallCentered_ENG(65, "Graphical, audible and gameplay settings");
        M_WriteTextSmallCentered_ENG(75, "will be reset to their default values.");
        M_WriteTextSmallCentered_ENG(85, "Which level of values to use?");

        // Explanations
        if (CurrentItPos == 0)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmallCentered_ENG(145, "Settings will be reset to");
            M_WriteTextSmallCentered_ENG(155, "default port's values");
            dp_translation = NULL;
        }
        else
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmallCentered_ENG(145, "Settings will be reset to");
            M_WriteTextSmallCentered_ENG(155, "original Doom values");
            dp_translation = NULL;
        }
    }
    else
    {
        M_WriteTextSmallCentered_RUS(65, "Yfcnhjqrb uhfabrb< pderf b utqvgktz");     // Настройки графики, звука и геймплея
        M_WriteTextSmallCentered_RUS(75, ",elen c,hjitys yf cnfylfhnyst pyfxtybz>"); // Будут сброшены на стандартные значения.
        M_WriteTextSmallCentered_RUS(85, "Ds,thbnt ehjdtym pyfxtybq:");              // Выберите уровень значений:

        // Пояснения
        if (CurrentItPos == 0)
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmallCentered_RUS(145, ",elen bcgjkmpjdfys pyfxtybz");       // Будут использованы значения
            M_WriteTextSmallCentered_RUS(155, "htrjvtyletvst gjhnjv");              // рекомендуемые портом
            dp_translation = NULL;
        }
        else
        {
            dp_translation = cr[CR_DARKRED];
            M_WriteTextSmallCentered_RUS(145, ",elen bcgjkmpjdfys pyfxtybz");       // Будут использованы значения
            dp_translation = NULL;
            RD_M_DrawTextSmallRUS("jhbubyfkmyjuj", 85 + wide_delta, 155, CR_DARKRED); // оригинального Doom
            RD_M_DrawTextSmallENG("Doom", 193 + wide_delta, 155, CR_DARKRED);
        }
    }
}

void M_RD_BackToDefaults_Recommended(int choice)
{
    static char resetmsg[24];

    // Rendering
    vsync                   = 1;
    aspect_ratio_correct    = 1;
    uncapped_fps            = 1;
    show_fps                = 0;
    smoothing               = 0;
    vga_porch_flash         = 0;
    force_software_renderer = 0;
    show_diskicon           = 1;
    screen_wiping           = 1;
    png_screenshots         = 1;

    // Display
    screenblocks          = 10;
    usegamma              = 4;
    level_brightness      = 0;
    menu_shading          = 0;
    detailLevel           = 0;
    hud_detaillevel       = 0;

    // Messages
    showMessages          = 1;
    messages_alignment    = 0;
    messages_timeout      = 4;
    message_fade          = 0;
    draw_shadowed_text    = 1;
    local_time            = 0;
    message_pickup_color  = 0;
    message_secret_color  = 7;
    message_system_color  = 0;
    message_chat_color    = 2;

    // Automap
    automap_color     = 0;
    automap_antialias = 1;
    automap_stats     = 1;
    automap_level_time = 1;
    automap_total_time = 0;
    automap_coords    = 0;
    automap_overlay   = 0;
    automap_rotate    = 0;
    automap_follow    = 1;
    automap_grid      = 0;
    automap_grid_size = 128;

    // Audio
    snd_samplerate = 44100;
    snd_sfxdevice        = 3;
        I_ShutdownSound();                  // Shut down sound system
        InitSfxModule(snd_sfxdevice);       // Reinitialize SFX module
        S_RD_Change_SoundDevice();          // Call sfx device changing routine
    snd_musicdevice      = 3;
    snd_dmxoption        = "-opl3";
        S_StopMusic();                      // Shut down current music
        S_Shutdown();                       // Shut down music system
        I_InitSound(true);                  // Start music system
        S_ChangeMusic(music_num_rd, true);  // Restart current music
    sfxVolume            = 8;
        S_SetSfxVolume(sfxVolume * 8);
    musicVolume          = 8;
        S_SetMusicVolume(musicVolume * 8);
    snd_channels         = 32;
        S_ChannelsRealloc();
    snd_monomode         = 0;
    snd_pitchshift       = 0;
    mute_inactive_window = 0;

    // Controls
    joybspeed        = 29;
    mlook            = 0;  players[consoleplayer].centering = true;
    mouseSensitivity = 5;
    mouse_acceleration = 2.0F;
    mouse_threshold  = 10;
    novert           = 1;

    // Gameplay: Graphical
    brightmaps       = 1;
    fake_contrast    = 0;
    translucency     = 1;
    improved_fuzz    = 2;
    colored_blood    = 1;
    swirling_liquids = 1;
    invul_sky        = 1;
    linear_sky       = 1;
    flip_weapons     = 0;

    // Gameplay: Status Bar
    sbar_colored        = 0;
    sbar_color_high     = 5;
    sbar_color_normal   = 2;
    sbar_color_low      = 7;
    sbar_color_critical = 0;
    sbar_color_armor_1  = 2;
    sbar_color_armor_2  = 5;
    sbar_color_armor_0  = 0;

    // Gameplay: Audible
    z_axis_sfx           = 0;
    play_exit_sfx        = 0;
    crushed_corpses_sfx  = 1;
    blazing_door_fix_sfx = 1;
    noise_alert_sfx      = 0;
    // correct_endlevel_sfx = 0; (hidden variable)

    // Gameplay: Tactical
    secret_notification = 1;
    negative_health     = 0;
    infragreen_visor    = 0;

    // Gameplay: Physical
    over_under           = 0;
    torque               = 1;
    weapon_bobbing       = 1;
    ssg_blast_enemies    = 1;
    randomly_flipcorpses = 1;
    floating_powerups    = 1;
    toss_drop            = 1;

    // Gameplay: Crosshair
    crosshair_draw   = 0;
    crosshair_type   = 1;
    crosshair_scale  = 0;

    // Gameplay: Gameplay
    fix_map_errors       = 1;
    flip_levels          = 0;
    extra_player_faces   = 1;
    unlimited_lost_souls = 1;
    agressive_lost_souls = 0;
    pistol_start         = 0;
    fast_quickload       = 1;

    // Gameplay: Demos
    demotimer         = 0;
    demotimerdir      = 0;
    demobar           = 0;
    no_internal_demos = 0;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update screen size and fuzz effect
    R_SetViewSize (screenblocks, detailLevel);

    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }

    // Reset Automap color scheme
    AM_initColors();

    // Print informative message
    M_snprintf(resetmsg, sizeof(resetmsg), english_language ? 
                                           "Settings reset" :
                                           "Yfcnhjqrb c,hjitys");
    players[consoleplayer].message_system = resetmsg;
}

void M_RD_BackToDefaults_Original(int choice)
{
    static char resetmsg[24];

    // Rendering
    vsync                   = 1;
    aspect_ratio_correct    = 1;
    uncapped_fps            = 0;
    show_fps                = 0;
    smoothing               = 0;
    vga_porch_flash         = 0;
    force_software_renderer = 0;
    show_diskicon           = 1;
    screen_wiping           = 1;
    png_screenshots         = 1;

    // Display
    screenblocks          = 10;
    usegamma              = 9;
    level_brightness      = 0;
    menu_shading          = 0;
    detailLevel           = 1;
    hud_detaillevel       = 1;

    // Messages
    showMessages          = 1;
    messages_alignment    = 0;
    messages_timeout      = 4;
    message_fade          = 0;
    draw_shadowed_text    = 0;
    local_time            = 0;
    message_pickup_color  = 0;
    message_secret_color  = 0;
    message_system_color  = 0;
    message_chat_color    = 0;

    // Automap
    automap_color     = 0;
    automap_antialias = 0;
    automap_stats     = 0;
    automap_level_time = 0;
    automap_total_time = 0;
    automap_coords    = 0;
    automap_overlay   = 0;
    automap_rotate    = 0;
    automap_follow    = 1;
    automap_grid      = 0;
    automap_grid_size = 128;

    // Audio
    snd_samplerate = 44100;
    snd_sfxdevice        = 3;
        I_ShutdownSound();                  // Shut down sound system
        InitSfxModule(snd_sfxdevice);       // Reinitialize SFX module
        S_RD_Change_SoundDevice();          // Call sfx device changing routine
    snd_musicdevice      = 3;
    snd_dmxoption        = "-opl3";
        S_StopMusic();                      // Shut down current music
        S_Shutdown();                       // Shut down music system
        I_InitSound(true);                  // Start music system
        S_ChangeMusic(music_num_rd, true);  // Restart current music
    sfxVolume            = 8;
        S_SetSfxVolume(sfxVolume * 8);
    musicVolume          = 8;
        S_SetMusicVolume(musicVolume * 8);
    snd_channels         = 8;
        S_ChannelsRealloc();
    snd_monomode         = 0;
    snd_pitchshift       = 0;
    mute_inactive_window = 0;

    // Controls
    joybspeed          = 29;
    mlook              = 0;  players[consoleplayer].centering = true;
    mouseSensitivity   = 5;
    mouse_acceleration = 2.0F;
    mouse_threshold    = 10;
    novert             = 1;

    // Gameplay: Graphical
    brightmaps       = 0;
    fake_contrast    = 1;
    translucency     = 0;
    improved_fuzz    = 0;
    colored_blood    = 0;
    swirling_liquids = 0;
    invul_sky        = 0;
    linear_sky       = 0;
    flip_weapons     = 0;

    // Gameplay: Status Bar
    sbar_colored        = 0;
    sbar_color_high     = 5;
    sbar_color_normal   = 2;
    sbar_color_low      = 7;
    sbar_color_critical = 0;
    sbar_color_armor_1  = 2;
    sbar_color_armor_2  = 5;
    sbar_color_armor_0  = 0;

    // Gameplay: Audible
    z_axis_sfx           = 0;
    play_exit_sfx        = 1;
    crushed_corpses_sfx  = 0;
    blazing_door_fix_sfx = 0;
    noise_alert_sfx      = 0;
    // correct_endlevel_sfx = 0; (hidden variable)

    // Gameplay: Tactical
    secret_notification = 0;
    negative_health     = 0;
    infragreen_visor    = 0;

    // Gameplay: Physical
    over_under           = 0;
    torque               = 0;
    weapon_bobbing       = 0;
    ssg_blast_enemies    = 0;
    randomly_flipcorpses = 0;
    floating_powerups    = 0;
    toss_drop            = 0;

    // Gameplay: Crosshair
    crosshair_draw   = 0;
    crosshair_type   = 0;
    crosshair_scale  = 0;

    // Gameplay: Gameplay
    fix_map_errors       = 0;
    flip_levels          = 0;
    extra_player_faces   = 0;
    unlimited_lost_souls = 0;
    agressive_lost_souls = 0;
    pistol_start         = 0;
    fast_quickload       = 1;

    // Gameplay: Demos
    demotimer         = 0;
    demotimerdir      = 0;
    demobar           = 0;
    no_internal_demos = 0;

    // Do a full graphics reinitialization
    I_InitGraphics();

    // Update screen size and fuzz effect
    R_SetViewSize (screenblocks, detailLevel);

    // Update background of classic HUD and player face 
    if (gamestate == GS_LEVEL)
    {
        ST_refreshBackground();
        ST_drawWidgets(true);
    }

    // Reset Automap color scheme
    AM_initColors();

    // Print informative message
    M_snprintf(resetmsg, sizeof(resetmsg), english_language ? 
                                           "Settings reset" :
                                           "Yfcnhjqrb c,hjitys");
    players[consoleplayer].message_system = resetmsg;
}


// -----------------------------------------------------------------------------
// Language hot-swapping
// -----------------------------------------------------------------------------

void M_RD_ChangeLanguage(int choice)
{
    extern void ST_createWidgetsJaguar(void);
    extern void F_CastDrawer(void);
    extern void F_CastDrawerJaguar(void);
    extern void F_StartFinale(void);
    extern int  demosequence;
    extern int  finalestage;

    english_language ^= 1;

    // Update messages
    RD_DefineLanguageStrings();

    // Update TITLEPIC/CREDIT screens in live mode
    if (gamestate == GS_DEMOSCREEN)
    {
        if (demosequence == 0
        ||  demosequence == 2
        ||  demosequence == 4)
        {
            demosequence--;
            D_DoAdvanceDemo();
        }
    }

    if (gamestate == GS_LEVEL)
    {
        // Update HUD system
        HU_Start();

        // Update status bar
        ST_doRefresh();
        // Update ARMS/FRAGS widget
        if (gamemission == jaguar)
        ST_createWidgetsJaguar();
        else
        ST_createWidgets(); 
    }

    // Update finale sequence
    if (gamestate == GS_FINALE)
    {
        if (finalestage == 2)
        {
            // Just redraw monster's names
            F_CastDrawer();
        }
        else if (finalestage == 3)
        {
            // Just redraw monster's names (Jaguar)
            F_CastDrawerJaguar();
        }
        else
        {
            // Restart finale text output
            F_StartFinale();
        }
    }

    // Update window title
    I_SetWindowTitle(english_language ? gamedescription_eng : gamedescription_rus);
    I_InitWindowTitle();
}


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//

void M_ReadSaveStrings(void)
{
    FILE    *handle;
    int     i;
    char    name[256];

    for (i = 0;i < 8;i++)
    {
        M_StringCopy(name, P_SaveGameFile(i), sizeof(name));

        handle = fopen(name, "rb");
        if (handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING, SAVESTRINGSIZE);
            saveStatus[i] = false;
            continue;
        }

        fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        saveStatus[i] = true;
    }
    slottextloaded = true;
}


//
// M_LoadGame & Cie.
//
static int LoadDef_x = 72, LoadDef_y = 28;  // [JN] from Crispy Doom
void M_DrawLoad(void)
{
    int i;
    int x;

    x = english_language ? LoadMenu.x_eng : LoadMenu.x_rus;

    if (english_language)
    {
        V_DrawShadowedPatchDoom(LoadDef_x + wide_delta, LoadDef_y,
                           W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));
    }
    else
    {
        // ЗАГРУЗИТЬ ИГРУ
        M_WriteTextBigCentered_RUS(LoadDef_y, "PFUHEPBNM BUHE");
    }

    for (i = 0;i < 8; i++)
    {
        M_DrawSaveLoadBorder(x + wide_delta,LoadMenu.y+LINEHEIGHT*i);

        // [crispy] shade empty savegame slots
        if (!saveStatus[i] && !vanillaparm)
            dp_translation = cr[CR_DARKRED];

        RD_M_DrawTextA(savegamestrings[i], x + wide_delta,LoadMenu.y+LINEHEIGHT*i);

        dp_translation = NULL;
    }
}


//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int i;

    V_DrawShadowedPatchDoom(x - 8, y + 8, W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));

    for (i = 0 ; i < 24 ; i++)
    {
        V_DrawShadowedPatchDoom(x, y + 8, W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
        x += 8;
    }

    V_DrawShadowedPatchDoom(x, y + 8, W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
}


//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    char    name[256];

    M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));

    G_LoadGame (name);
    M_ClearMenus ();
}


//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(english_language ?
                                  LOADNET : LOADNET_RUS),
                                  NULL,false);
        
        return;
    }

    SetMenu(&LoadMenu);
    if (!slottextloaded)
        M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
static int SaveDef_x = 72, SaveDef_y = 28;  // [JN] from Crispy Doom
void M_DrawSave(void)
{
    int i;
    int x;

    x = english_language ? LoadMenu.x_eng : LoadMenu.x_rus;

    if (english_language)
    {
        // [JN] Use standard centered title "M_SAVEG"
        V_DrawShadowedPatchDoom(SaveDef_x + wide_delta, SaveDef_y, 
                                W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
    }
    else
    {
        if (QuickSaveTitle) // БЫСТРОЕ СОХРАНЕНИЕ
            M_WriteTextBigCentered_RUS(SaveDef_y, "<SCNHJT CJ{HFYTYBT");
        else                // СОХРАНИТЬ ИГРУ
            M_WriteTextBigCentered_RUS(SaveDef_y, "CJ{HFYBNM BUHE");
    }

    for (i = 0;i < 8; i++)
    {
        M_DrawSaveLoadBorder(x + wide_delta,LoadMenu.y+LINEHEIGHT*i);
        RD_M_DrawTextA(savegamestrings[i], x + wide_delta,LoadMenu.y+LINEHEIGHT*i);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[saveSlot]);
        RD_M_DrawTextA("_", x + i + wide_delta,LoadMenu.y+LINEHEIGHT*saveSlot);
    }
}


//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    saveStatus[slot] = true;
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
        quickSaveSlot = slot;
}


//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;

    saveSlot = choice;
    M_StringCopy(saveOldString,savegamestrings[choice], SAVESTRINGSIZE);

    if (!strcmp(savegamestrings[choice], EMPTYSTRING))
        savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}


//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    if (!usergame)
    {
        M_StartMessage(DEH_String(english_language ?
                                  SAVEDEAD : SAVEDEAD_RUS),
                                  NULL,false);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    SetMenu(&SaveMenu);
    if (!slottextloaded)
        M_ReadSaveStrings();
}


//
// M_QuickSave
//
char tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(NULL,sfx_swtchx);
    }
}

void M_QuickSave(void)
{
    if (!usergame)
    {
        S_StartSound(NULL,sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        if (!slottextloaded)
            M_ReadSaveStrings();
        SetMenu(&SoundMenu);
        quickSaveSlot = -2;	// means to pick a slot now
        return;
    }

    if (fast_quickload)
    {
        M_DoSave(quickSaveSlot);
    }
    else
    {
        DEH_snprintf(tempstring, 80, english_language ?
                                     QSPROMPT : QSPROMPT_RUS,
                                     savegamestrings[quickSaveSlot]);
        M_StartMessage(tempstring,M_QuickSaveResponse,true);
    }
}


//
// M_QuickLoad
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL,sfx_swtchx);
    }
}


void M_QuickLoad(void)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(english_language ?
                                  QLOADNET : QLOADNET_RUS),
                                  NULL,false);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(DEH_String(english_language ?
                                  QSAVESPOT : QSAVESPOT_RUS),
                                  NULL,false);
        return;
    }

    if (fast_quickload)
    {
        M_LoadSelect(quickSaveSlot);
    }
    else
    {
        DEH_snprintf(tempstring, 80, english_language ?
                                     QLPROMPT : QLPROMPT_RUS,
                                     savegamestrings[quickSaveSlot]);
        M_StartMessage(tempstring,M_QuickLoadResponse,true);
    }
}


//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    char *lumpname = "CREDIT";
    int skullx = 330, skully = 175;

    inhelpscreens = true;

    // [JN] Различные экраны помощи и скорректированное положение M_SKULL для разных версий игры

    switch (gameversion)
    {
        case exe_doom_1_666:    // [JN] Needed for Shareware 1.6 / 1.666
        case exe_doom_1_8:      // [JN] Needed for Shareware 1.8
        case exe_doom_1_9:
        case exe_hacx:
            if (gamemode == commercial )
            {
                // Doom 2
                if (english_language)
                    lumpname = "HELP";
                else
                    lumpname = "HELPR";

                skullx = 330;
                skully = 162;
            }
            else
            {
                // Doom 1
                // HELP2 is the first screen shown in Doom 1
                if (english_language)
                    lumpname = "HELP2";
                else
                    lumpname = "HELP2R";

                skullx = 280;
                skully = 185;
            }
            break;

        case exe_ultimate:
        case exe_chex:

            // Ultimate Doom always displays "HELP1".

            // Chex Quest version also uses "HELP1", even though it is based
            // on Final Doom.

            if (english_language)
                lumpname = "HELP1";
            else
                lumpname = "HELP1R";
            break;
        case exe_final:
        case exe_final2:

            // Final Doom always displays "HELP".
            // [JN] Иконка черепа сдвинута чуть выше, по аналогии Doom 2,
            // чтобы не загораживать фразу "джойстика 2".

            if (english_language)
                lumpname = "HELP";
            else
                lumpname = "HELPR";
    
            skullx = 330;
            skully = 165;
            break;

        default:
            I_Error(english_language ?
                    "Unknown game version" :
                    "Версия игры не определена");
            break;
    }

    // [JN] Для обоих Стадий Freedoom используется одинаковое положение
    // черепа. Так, чтобы он не перекрывал надписb "ПКМ" и "E".
    if (gamevariant == freedoom)
    {
        skullx = 323;
        skully = 183;
    }

    // [JN] Pixel-perfect position for skull in Press Beta
    if (gamemode == pressbeta)
    {
            skullx = 330;
            skully = 175;
    }

    lumpname = DEH_String(lumpname);

    V_DrawPatchFullScreen (W_CacheLumpName(lumpname, PU_CACHE), false);

    V_DrawShadowedPatchDoom(skullx + -32 + wide_delta, skully - 5,
                            W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));
}


//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    V_DrawPatchFullScreen(W_CacheLumpName(DEH_String
               (english_language ? "HELP1" : "HELP1R"), PU_CACHE), false);

    V_DrawShadowedPatchDoom(298 + wide_delta, 175 - 5,
                            W_CacheLumpName(DEH_String(skullName[whichSkull]), PU_CACHE));
}


//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    inhelpscreens = true;

    if (english_language)
    {
        // [JN] Always draw original "M_DOOM" in English language
        V_DrawPatch(94+wide_delta, 2, 
                    W_CacheLumpName(DEH_String("M_DOOM"), PU_CACHE));
    }
    else
    {
        // [JN] Draw translated titles for Plutonia and TNT
        V_DrawPatch(94+wide_delta, 2, W_CacheLumpName
                    (logical_gamemission == pack_plut ? "RD_MPLUT" :
                      logical_gamemission == pack_tnt ? "RD_MTNT" :
                                                        "M_DOOM", PU_CACHE));
    }
}


//
// M_NewGame
//
void M_DrawNewGame(void)
{
    inhelpscreens = true;

    if (english_language)
    {
        V_DrawShadowedPatchDoom(96 + wide_delta, 13, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
        V_DrawShadowedPatchDoom(54 + wide_delta, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
    }
    else
    {
        // НОВАЯ ИГРА
        M_WriteTextBigCentered_RUS(13, "YJDFZ BUHF");
        // Уровень сложности:
        M_WriteTextBigCentered_RUS(38, "Ehjdtym ckj;yjcnb#");
    }
}

void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(english_language ?
                                  NEWGAME : NEWGAME_RUS),
                                  NULL,false);
        return;
    }

    // Chex Quest disabled the episode select screen, as did Doom II.

    if (gamemode == commercial || gameversion == exe_chex)
        SetMenu(&NewGameMenu);
    else
        SetMenu(EpisodeMenu);
}


//
// M_Episode
//
int epi;

void M_DrawEpisode(void)
{
    inhelpscreens = true;

    if (english_language)
    {
        V_DrawShadowedPatchDoom(96 + wide_delta, 13, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
        V_DrawShadowedPatchDoom(54 + wide_delta, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
    }
    else
    {
        // НОВАЯ ИГРА
        M_WriteTextBigCentered_RUS(13, "YJDFZ BUHF");
        // Какой эпизод?
        M_WriteTextBigCentered_RUS(38, "Rfrjq \'gbpjl?");
    }
}

void M_VerifyNightmare(int key)
{
    if (key != key_menu_confirm)
        return;

    G_DeferedInitNew(4,epi+1,1);
    M_ClearMenus ();
}

void M_VerifyUltraNightmare(int key)
{
    if (key != key_menu_confirm)
        return;

    G_DeferedInitNew(5,epi+1,1);
    M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
    if (choice == 4)
    {
        M_StartMessage(DEH_String(english_language ?
                                  NIGHTMARE : NIGHTMARE_RUS),
                                  M_VerifyNightmare,true);
        return;
    }
    if (choice == 5)
    {
        M_StartMessage(DEH_String(english_language ?
                                  ULTRANM : ULTRANM_RUS),
                                  M_VerifyUltraNightmare,true);
        return;
    }

    G_DeferedInitNew(choice,epi+1,1);
    M_ClearMenus ();
}

void M_Episode(int choice)
{
    if ((gamemode == shareware) && choice)
    {
        M_StartMessage(DEH_String(english_language ?
                                  SWSTRING : SWSTRING_RUS),
                                  NULL,false);
        // [JN] Return to Episode menu.
        SetMenu(EpisodeMenu);
        return;
    }

    // Yet another hack...
    if ( (gamemode == registered) && (choice == 3))
    {
        fprintf (stderr, english_language ?
                        "M_Episode: fourth episode available only in Ultimate DOOM\n" :
                        "M_Episode: четвертый эпизод доступен только в Ultimate DOOM\n");
        choice = 0;
    }

    epi = choice;
    SetMenu(&NewGameMenu);
}

//
// M_EndGame
//
void M_EndGameResponse(int key)
{
    if (key != key_menu_confirm)
        return;

    CurrentMenu->lastOn = CurrentItPos;
    M_ClearMenus ();
    D_StartTitle ();
}

void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame)
    {
        S_StartSound(NULL,sfx_oof);
        return;
    }

    if (netgame)
    {
        M_StartMessage(DEH_String(english_language ?
                                  NETEND : NETEND_RUS),NULL,false);
        return;
    }

    M_StartMessage(DEH_String(english_language ?
                              ENDGAME : ENDGAME_RUS),
                              M_EndGameResponse,true);
}

//
// M_ReadThis2
//
void M_ReadThis(int choice)
{
    InfoType = 1;
}

void M_ReadThis2(int choice)
{
    // Doom 1.9 had two menus when playing Doom 1
    // All others had only one
    //
    // [JN] Show second screen also 1.6, 1.666 and 1.8 Sharewares

    if ((gameversion == exe_doom_1_9 && gamemode != commercial)
    || (gameversion == exe_doom_1_666 && gamemode == shareware)
    || (gameversion == exe_doom_1_8 && gamemode == shareware))
    {
        InfoType = 2;
    }
    else
    {
        // Close the menu
        M_FinishReadThis(0);
    }
}

void M_FinishReadThis(int choice)
{
    InfoType = 0;
    SetMenu(MainMenu);
}


//
// M_QuitDOOM
//
int quitsounds[8] =
{
    sfx_pldeth,
    sfx_dmpain,
    sfx_popain,
    sfx_slop,
    sfx_telept,
    sfx_posit1,
    sfx_posit3,
    sfx_sgtatk
};

int quitsounds2[8] =
{
    sfx_vilact,
    sfx_getpow,
    sfx_boscub,
    sfx_slop,
    sfx_skeswg,
    sfx_kntdth,
    sfx_bspact,
    sfx_sgtatk
};


void M_QuitResponse(int key)
{
    if (key != key_menu_confirm)
        return;

    // [JN] Опциональное проигрывание звука при выходе из игры
    if ((!netgame && play_exit_sfx && sfxVolume > 0) || vanillaparm)
    {
        if (gamemode == commercial && gamemission != jaguar)
            S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
        else
            S_StartSound(NULL,quitsounds[(gametic>>2)&7]);

        I_WaitVBL(105);
    }

    // [JN] Widescreen: remember choosen widescreen variable before quit.
    aspect_ratio = aspect_ratio_temp;

    I_Quit ();
}


static char *M_SelectEndMessage(void)
{
    char **endmsg;

    if (logical_gamemission == doom)
    {
        endmsg = english_language ? doom1_endmsg : doom1_endmsg_rus; // Doom 1
    }
    else
    {
        endmsg = english_language ? doom2_endmsg : doom2_endmsg_rus; // Doom 2
    }

    return endmsg[gametic % NUM_QUITMESSAGES];
}


void M_QuitDOOM(int choice)
{
    DEH_snprintf(endstring, sizeof(endstring),
                (english_language ? ("%s\n\n" DOSY) : ("%s\n\n" DOSY_RUS)),
                 DEH_String(M_SelectEndMessage()));

    M_StartMessage(endstring,M_QuitResponse,true);
}

//
// Menu Functions
//

void
M_StartMessage
( char*     string,
  void*     routine,
  boolean   input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageToBind = 0;  // [JN] NOT binding key now
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
}

void M_RD_Key_Binding_Routine (int key)
{
    messageToBind = 0;

    if (keyToBind)
    {
        if (key == *keyToBind)
        {
            *keyToBind = 0;
        }
        else if (!(key == KEY_ESCAPE))
        {
            *keyToBind = key;
        }

        keyToBind = 0;
    }
}

//
// M_StartBinding
// [JN] Used for key binding routine.
//
void M_RD_StartBinding (int* key_var)
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageToBind = 1;  // [JN] Binding key now
    keyToBind = key_var; // [Dasperal] Specify key to bind
    messageRoutine = M_RD_Key_Binding_Routine;
    messageNeedsInput = false;
    menuactive = true;
}

void M_RD_MouseKey_Binding_Routine (int key)
{
    messageToBind = 0;

    switch (key)
    {
        case 13:
        {
            if (*keyToBind == 0 && key == 13)
                *keyToBind = -1;
            else
                *keyToBind = 0;
            break;
        }
        case 127:
        {
            if (*keyToBind == 1 && key == 127)
                *keyToBind = -1;
            else
                *keyToBind = 1;
            break;
        }
        case 4:
        {
            if (*keyToBind == 2 && key == 4)
                *keyToBind = -1;
            else
                *keyToBind = 2;
            break;
        }
        case 8:
        case 16:
        case 173:
        {
            if ((*keyToBind == 3 && key == 8)
                ||  (*keyToBind == 3 && key == 16)
                ||  (*keyToBind == 3 && key == 173))
                *keyToBind = -1;
            else
                *keyToBind = 3;

            break;
        }
        case 175:
        {
            if (*keyToBind == 4 && key == 175)
                *keyToBind = -1;
            else
                *keyToBind = 4;
            break;
        }
        case KEY_ESCAPE:
        default:
        {
            break;
        }
    }
}

void M_RD_StartBinding_Mouse (int* key_var)
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageToBind = 1;  // [JN] Binding key now
    keyToBind = key_var; // [Dasperal] Specify key to bind
    messageRoutine = M_RD_MouseKey_Binding_Routine;
    messageNeedsInput = false;
    menuactive = true;
}

//
// Find string width from hu_font chars
//
int M_StringWidth(char* string)
{
    size_t  i;
    int     w = 0;
    int     c;

    for (i = 0;i < strlen(string);i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
            w += 4;
        else
        {
            if (english_language || CurrentMenu == &SaveMenu)
            {
                w += SHORT (hu_font[c]->width);
            }
            else
            {
                w += SHORT (hu_font_small_rus[c]->width);    
            }
        }
    }

    return w;
}


//
// Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    size_t  i;
    int     h;
    int     height = SHORT(hu_font[0]->height);

    h = height;
    for (i = 0;i < strlen(string);i++)
        if (string[i] == '\n')
            h += height;

    return h;
}

// [crispy] reload current level / go to next level
// adapted from prboom-plus/src/e6y.c:369-449
static int G_ReloadLevel(void)
{
    int result = false;

    if (gamestate == GS_LEVEL)
    {
        // [crispy] restart demos from the map they were started
        if (demorecording)
        {
            gamemap = startmap;
        }
        G_DeferedInitNew(gameskill, gameepisode, gamemap);
        result = true;
    }

    return result;
}

static int G_GotoNextLevel(void)
{
    byte doom_next[5][9] = {
    {12, 13, 19, 15, 16, 17, 18, 21, 14},
    {22, 23, 24, 25, 29, 27, 28, 31, 26},
    {32, 33, 34, 35, 36, 39, 38, 41, 37},
    {42, 49, 44, 45, 46, 47, 48, 51, 43},
    {52, 53, 54, 55, 56, 59, 58, 11, 57},
    };

    byte doom2_next[33] = {
     2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
    32, 16, 3
    };

    byte jaguar_next[26] = {
     2,  3, 24,  5,  6,  7,  8,  9, 10, 11,
    12, 13, 25, 15, 16, 17, 18, 19, 20, 21,
    26, 23,  1,  4, 14, 22
    };

    byte nerve_next[9] = {
    2, 3, 4, 9, 6, 7, 8, 1, 5
  };

    int changed = false;

    if (gamemode == commercial)
    {
        /*
        if (crispy->havemap33)
        doom2_next[1] = 33;
        */

        if (W_CheckNumForName("map31") < 0)
        {
            doom2_next[14] = 16;
        }

        if (gamemission == pack_hacx)
        {
            doom2_next[30] = 16;
            doom2_next[20] = 1;
        }

        /*
        if (gamemission == pack_master)
        {
            doom2_next[1] = 3;
            doom2_next[14] = 16;
            doom2_next[20] = 1;
        }
        */
    }
    else
    {
        if (gamemode == shareware)
        {
            doom_next[0][7] = 11;
        }

        if (gamemode == registered)
        {
            doom_next[2][7] = 11;
        }

        if (!sgl_loaded)
        {
            doom_next[3][7] = 11;
        }

        if (gamemode == pressbeta)
        {
            if (gameepisode == 1)
                doom_next[0][0] = 21;
            if (gameepisode == 2)
                doom_next[1][0] = 31;
            if (gameepisode == 3)
                doom_next[2][0] = 11;
        }

        if (gameversion == exe_chex)
        {
            doom_next[0][2] = 14;
            doom_next[0][4] = 11;
        }
    }

    if (gamestate == GS_LEVEL)
    {
        int epsd, map;

        if (gamemode == commercial)
        {
            epsd = gameepisode;

            if (gamemission == pack_nerve)
            {
                map = nerve_next[gamemap-1];
            }
            else if (gamemission == jaguar)
            {
                map = jaguar_next[gamemap-1];
            }
            else
            {
                map = doom2_next[gamemap-1];
            }
        }
        else
        {
            epsd = doom_next[gameepisode-1][gamemap-1] / 10;
            map = doom_next[gameepisode-1][gamemap-1] % 10;
        }

        /*
        // [crispy] special-casing for E1M10 "Sewers" support
        if (crispy->havee1m10 && gameepisode == 1)
        {
            if (gamemap == 1)
            {
                map = 10;
            }
            else if (gamemap == 10)
            {
                epsd = 1;
                map = 2;
            }
        }
        */

        G_DeferedInitNew(gameskill, epsd, map);
        changed = true;
    }

    return changed;
}


//
// CONTROL PANEL
//

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    int             ch;
    int             key;
    int             i;
    static int      joywait = 0;
    static int      mousewait = 0;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (ev->type == ev_quit || (ev->type == ev_keydown
        && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // [JN] Remember game settings and quit immediately
    // in devparm mode by pressing "close" button on window.
    if (devparm && ev->type == ev_quit)
    {
        M_QuitDOOM(0);
    }

    // "close" button pressed on window?
    if (ev->type == ev_quit)
    {
        // First click on close button = bring up quit confirm message.
        // Second click on close button = confirm quit

        if (menuactive && messageToPrint && messageRoutine == M_QuitResponse)
        {
            M_QuitResponse(key_menu_confirm);
        }
        else
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed

    ch = 0;
    key = -1;

    if (ev->type == ev_joystick && joywait < I_GetTime())
    {

        // [JN] Disallow to use joystick keys while binding keyboard keys
        if (CurrentMenu == &Bindings1Menu && messageToBind)
        {
            for (i = 0 ; i < 15 ; i++)
                if (CurrentItPos == i)
                    return false;
        }
        if (CurrentMenu == &Bindings2Menu && messageToBind)
        {
            for (i = 0 ; i < 15 ; i++)
                if (CurrentItPos == i)
                    return false;
        }
        if (CurrentMenu == &Bindings3Menu && messageToBind)
        {
            for (i = 0 ; i < 15 ; i++)
                if (CurrentItPos == i)
                    return false;
        }
        if (CurrentMenu == &Bindings4Menu && messageToBind)
        {
            for (i = 0 ; i < 15 ; i++)
                if (CurrentItPos == i)
                    return false;
        }

        if (ev->data3 < 0)
        {
            key = key_menu_up;
            joywait = I_GetTime() + 5;
        }
        else if (ev->data3 > 0)
        {
            key = key_menu_down;
            joywait = I_GetTime() + 5;
        }

        if (ev->data2 < 0)
        {
            key = key_menu_left;
            joywait = I_GetTime() + 2;
        }
        else if (ev->data2 > 0)
        {
            key = key_menu_right;
            joywait = I_GetTime() + 2;
        }

        if (ev->data1&1)
        {
            key = key_menu_forward;
            joywait = I_GetTime() + 5;
        }
        if (ev->data1&2)
        {
            key = key_menu_back;
            joywait = I_GetTime() + 5;
        }
        if (joybmenu >= 0 && (ev->data1 & (1 << joybmenu)) != 0)
        {
            key = key_menu_activate;
            joywait = I_GetTime() + 5;
        }
    }
    else
    {
        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            // [FG] disable menu control by mouse
            /*
            mousey += ev->data3;
            if (mousey < lasty-30)
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else if (mousey > lasty+30)
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            mousex += ev->data2;

            if (mousex < lastx-30)
            {
                key = key_menu_left;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx+30)
            {
                key = key_menu_right;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }
            */

            // [JN] Disallow to use mouse keys while binding keyboard keys
            if (CurrentMenu == &Bindings1Menu && messageToBind)
            {
                for (i = 0 ; i < 15 ; i++)
                    if (CurrentItPos == i)
                        return false;
            }
            if (CurrentMenu == &Bindings2Menu && messageToBind)
            {
                for (i = 0 ; i < 15 ; i++)
                    if (CurrentItPos == i)
                        return false;
            }
            if (CurrentMenu == &Bindings3Menu && messageToBind)
            {
                for (i = 0 ; i < 15 ; i++)
                    if (CurrentItPos == i)
                        return false;
            }
            if (CurrentMenu == &Bindings4Menu && messageToBind)
            {
                for (i = 0 ; i < 15 ; i++)
                    if (CurrentItPos == i)
                        return false;
            }

            // [JN] Catch all incoming data1 mouse events. Makes middle mouse button 
            // working for message interruption and for binding ability.
            if (ev->data1)
            {
                key = ev->data1;
                mousewait = I_GetTime() + 5;
            }

            if (ev->data1&1)
            {
                key = key_menu_forward;
                mousewait = I_GetTime() + 15;
            }

            if (ev->data1&2)
            {
                key = key_menu_back;
                mousewait = I_GetTime() + 15;
            }
            
            // [crispy] scroll menus with mouse wheel
            // [JN] it also affecting mouse side buttons (forward/backward)
            if (mousebprevweapon >= 0 && ev->data1 & (1 << mousebprevweapon))
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 1;
            }
            else
            if (mousebnextweapon >= 0 && ev->data1 & (1 << mousebnextweapon))
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 1;
            }
        }
        else
        {
            if (ev->type == ev_keydown)
            {
                key = ev->data1;
                ch = ev->data2;
            }
        }
    }

    if (key == -1)
        return false;

    // Save Game string input
    if (saveStringEnter)
    {
        switch(key)
        {
            case KEY_BACKSPACE:
            if (saveCharIndex > 0)
            {
                saveCharIndex--;
                savegamestrings[saveSlot][saveCharIndex] = 0;
            }
            break;

            case KEY_ESCAPE:
                saveStringEnter = 0;
                M_StringCopy(savegamestrings[saveSlot], saveOldString, SAVESTRINGSIZE);
                break;

            case KEY_ENTER:
                saveStringEnter = 0;
                if (savegamestrings[saveSlot][0])
                    M_DoSave(saveSlot);
                break;

            default:
            // This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using 'data1'. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation: just
            // use the correct 'data2'.

            if (vanilla_keyboard_mapping)
            {
                ch = key;
            }

            ch = toupper(ch);

            if (ch != ' ' && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

            if (ch >= 32 && ch <= 127 &&
                saveCharIndex < SAVESTRINGSIZE-1 &&
                M_StringWidth(savegamestrings[saveSlot]) <
                (SAVESTRINGSIZE-2)*8)
            {
                savegamestrings[saveSlot][saveCharIndex++] = ch;
                savegamestrings[saveSlot][saveCharIndex] = 0;
            }
            break;
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint)
    {
        if (messageNeedsInput)
        {
            if (key != ' ' && key != KEY_ESCAPE && key != key_menu_confirm && key != key_menu_abort)
            {
                return false;
            }
        }

        menuactive = messageLastMenuActive;
        messageToPrint = 0;

        if (messageRoutine)
            messageRoutine(key);

        // [JN] Do not close Save/Load menu after deleting a savegame.
        if (CurrentMenu != &SaveMenu
        &&  CurrentMenu != &LoadMenu
        // [JN] Do not close Episode menu after closing "purchase entire trilogy" message in Shareware.
        &&  (CurrentMenu != EpisodeMenu && gamemode == shareware)
        // [JN] Do not close Options menu after pressing "N" in End Game.
        &&  CurrentMenu != &OptionsMenu
        // [JN] Do not close bindings menu after binding key / mouse button.
        &&  CurrentMenu != &Bindings1Menu
        &&  CurrentMenu != &Bindings2Menu
        &&  CurrentMenu != &Bindings3Menu
        &&  CurrentMenu != &Bindings4Menu
        &&  CurrentMenu != &MouseBindingsMenu)
        {
            menuactive = false;
        }
        S_StartSound(NULL,sfx_swtchx);
        return true;
    }

    if ((devparm && key == key_menu_help) || (key != 0 && key == key_menu_screenshot))
    {
        G_ScreenShot ();
        return true;
    }

    // [JN] Crosshair toggling. Disalloved in vanilla mode (no crosshair there).
    if (key == key_togglecrosshair && !vanillaparm)
    {
        static char crosshairmsg[24];

        crosshair_draw ^= 1;

        M_snprintf(crosshairmsg, sizeof(crosshairmsg),
                   crosshair_draw ? ststr_crosshair_on : ststr_crosshair_off);
        players[consoleplayer].message_system = crosshairmsg;
        S_StartSound(NULL,sfx_swtchn);

        return true;
    }

    // [JN] Toggle level flipping.
    if (key == key_togglefliplvls)
    {
        flip_levels ^= 1;
        R_ExecuteSetViewSize();         // Redraw game screen
        S_UpdateStereoSeparation();     // Update stereo separation
        S_StartSound(NULL, sfx_stnmov); // Play quiet sound

        return true;
    }

    // F-Keys
    if (!menuactive)
    {
        if (key == key_menu_help)     // Help key
        {
            M_StartControlPanel ();

            if ( gamemode == retail )
                InfoType = 2;
            else
                InfoType = 1;

            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        else if (key == key_menu_save)     // Save
        {
            QuickSaveTitle = false;
            M_StartControlPanel();
            S_StartSound(NULL,sfx_swtchn);
            M_SaveGame(0);
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            M_StartControlPanel();
            S_StartSound(NULL,sfx_swtchn);
            M_LoadGame(0);
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            if (vanillaparm)
            {
                CurrentMenu = &Options2Menu_Vanilla;
            }
            else
            {
                CurrentMenu = &SoundMenu;
            }
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            QuickSaveTitle = true;
            S_StartSound(NULL,sfx_swtchn);
            M_QuickSave();
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(NULL,sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuickLoad();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
            return true;
        }

        // [crispy] those two can be considered as shortcuts for the IDCLEV cheat
        // and should be treated as such, i.e. add "if (!netgame)"
        else if (!netgame && key != 0 && key == key_menu_reloadlevel)
        {
            if (G_ReloadLevel())
            return true;
        }
        else if (!netgame && key != 0 && key == key_menu_nextlevel)
        {
            if (G_GotoNextLevel())
            return true;
        }
    }

    // [JN] Allow detail toggle even while active menu.
    if (key == key_menu_detail)
    {
        M_RD_Change_Detail(0);
        S_StartSound(NULL,sfx_swtchn);
        return true;
    }
    // [JN] Allow to toggle messages even while active menu.
    else if (key == key_menu_messages)
    {
        M_RD_Change_Messages(0);
        S_StartSound(NULL,sfx_swtchn);
        return true;
    }
    // [JN] Allow gamma toggling even while active menu.
    else if (key == key_menu_gamma)
    {
        usegamma++;
        if (usegamma > 17)
            usegamma = 0;

        I_SetPalette ((byte *)W_CacheLumpName(DEH_String(usegamma <= 8 ?
                                              "PALFIX" : "PLAYPAL"), PU_CACHE) +
                                              st_palette * 768);

        players[consoleplayer].message_system = DEH_String(english_language ? 
                                                           gammamsg[usegamma] : 
                                                           gammamsg_rus[usegamma]);
        return true;
    }

    // [JN] Allow screen decreasing even while active menu.
	if (key == key_menu_decscreen)      // Screen size down
	{
		if (automapactive || chat_on)
		    return false;
		M_RD_Change_ScreenSize(0);
		return true;
	}

    // [JN] Allow screen increasing even while active menu.
	if (key == key_menu_incscreen) // Screen size up
	{
		if (automapactive || chat_on)
		    return false;
		M_RD_Change_ScreenSize(1);
		return true;
	}

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == key_menu_activate)
        {
            M_StartControlPanel ();
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        return false;
    }

    // [crispy] delete a savegame
    // [JN] Also used for clearing keyboard bindings
    if (key == KEY_DEL)
    {
        // [JN] Save/load menu
        if (CurrentMenu == &LoadMenu
        ||  CurrentMenu == &SaveMenu)
        {
            if (saveStatus[CurrentItPos])
            {
                CurrentMenu->lastOn = CurrentItPos;
                M_ConfirmDeleteGame();
                return true;
            }
            else
            {
                return true;
            }
        }

        //[Dasperal] Key bindings menus
        if(CurrentMenu == &Bindings1Menu ||
           CurrentMenu == &Bindings2Menu ||
           CurrentMenu == &Bindings3Menu ||
           CurrentMenu == &Bindings4Menu)
        {
            BK_ClearBinds(CurrentMenu->items[CurrentItPos].option);
            S_StartSound(NULL,sfx_stnmov);
            return true;
        }

        // [JN] Mouse bindings menu
        if (CurrentMenu == &MouseBindingsMenu)
        {
            if (CurrentItPos == 1) { mousebfire = -1; }
            if (CurrentItPos == 2) { mousebuse = -1; }
            if (CurrentItPos == 3) { mousebforward = -1; }
            if (CurrentItPos == 4) { mousebbackward = -1; }
            if (CurrentItPos == 5) { mousebstrafe = -1; }
            if (CurrentItPos == 6) { mousebstrafeleft = -1; }
            if (CurrentItPos == 7) { mousebstraferight = -1; }
            if (CurrentItPos == 8) { mousebprevweapon = -1; }
            if (CurrentItPos == 9) { mousebnextweapon = -1; }

            S_StartSound(NULL,sfx_stnmov);
            return true;
        }
    }

    if (InfoType)
    {
        if(key != key_menu_back)
        {
            RD_Menu_StartSound(MENU_SOUND_CLICK);
            if(InfoType == 1)
                M_ReadThis2(0);
            else
                M_FinishReadThis(0);
        }
        else
        {
            RD_Menu_StartSound(MENU_SOUND_BACK);
            if(InfoType == 1)
                M_FinishReadThis(0);
            else
                M_ReadThis(0);
        }
        return true;
    }

    // Keys usable within menu
    return RD_Menu_Responder(key, ch);
}


//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;

    menuactive = 1;
    CurrentMenu = MainMenu;         // JDC
    CurrentItPos = CurrentMenu->lastOn;   // JDC
}

void MN_ActivateMenu(void)
{
    M_StartControlPanel();
}

// Display OPL debug messages - hack for GENMIDI development.

static void M_DrawOPLDev(void)
{
    extern void I_OPL_DevMessages(char *, size_t);
    char        debug[1024];
    char        *curr, *p;
    int         line;

    I_OPL_DevMessages(debug, sizeof(debug));
    curr = debug;
    line = 0;

    for (;;)
    {
        p = strchr(curr, '\n');

        if (p != NULL)
        {
            *p = '\0';
        }

        RD_M_DrawTextSmallENG(curr, 0, line * 8, CR_NONE);
        ++line;

        if (p == NULL)
        {
            break;
        }

        curr = p + 1;
    }
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static int      x;
    static int      y;
    unsigned int    i;
    char            string[80];
    int             start;

    inhelpscreens = false;

    // Horiz. & Vertically center string and print it.
    // [JN] Do not invoke if we are binding key.
    if (messageToPrint && !messageToBind)
    {
        start = 0;
        y = ORIGHEIGHT/2 - M_StringHeight(messageString) / 2;
        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start, sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start, sizeof(string));
                start += strlen(string);
            }

            x = 160 - M_StringWidth(string) / 2;

            if (english_language)
            {
                RD_M_DrawTextA(string, x + wide_delta, y);
                y += SHORT(hu_font[0]->height);
            }
            else
            {
                RD_M_DrawTextSmallRUS(string, x + wide_delta, y, CR_NONE);
                y += SHORT(hu_font_small_rus[0]->height);                
            }
        }

        return;
    }

    if (opldev)
    {
        M_DrawOPLDev();
    }

    if (!menuactive)
        return;

    if (InfoType == 1)
    {
        M_DrawReadThis1();
        return;
    }
    else if (InfoType == 2)
    {
        M_DrawReadThis2();
        return;
    }

    RD_Menu_DrawMenu(CurrentMenu, MenuTime, CurrentItPos);
}


//
// M_ClearMenus
//
void M_ClearMenus (void)
{
    menuactive = 0;
    RD_Menu_StartSound(MENU_SOUND_DEACTIVATE);
}

void MN_DeactivateMenu(void)
{
    M_ClearMenus();
}

//
// M_Ticker
//
void M_Ticker (void)
{
    if (--skullAnimCounter <= 0)
    {
        whichSkull ^= 1;
        skullAnimCounter = 8;
    }
    MenuTime++;
}

//
// M_Init
//
void M_Init (void)
{
    // [crispy] rearrange Load Game and Save Game menus
    const patch_t *patchl, *patchs, *patchm;
    short captionheight, vstep;

    // [Dasperal] Init menu
    RD_M_InitFonts(// [JN] Original English fonts
                   DEH_String("STCFN033"),
                   DEH_String("FNTBE033"),
                   // [JN] Small special font used for time/fps widget
                   DEH_String("STCFG033"),
                   // [JN] Unchangable English fonts
                   DEH_String("FNTSE033"),
                   DEH_String("FNTBE033"),
                   // [JN] Unchangable Russian fonts
                   DEH_String("FNTSR033"),
                   DEH_String("FNTBR033"));

    RD_Menu_InitMenu(16, 10);

    RD_Menu_InitSliders(// [Dasperal] Big slider
                        DEH_String("M_THERML"),
                        DEH_String("M_THERMM"),
                        NULL,
                        DEH_String("M_THERMR"),
                        DEH_String("M_THERMO"),
                        // [Dasperal] Small slider
                        DEH_String("RD_THRML"),
                        DEH_String("RD_THRMM"),
                        DEH_String("RD_THRMR"),
                        DEH_String("RD_THRMW"),
                        // [Dasperal] Gem translations
                        CR_BLUE2,
                        CR_DARKGRAY,
                        CR_NONE);

    RD_Menu_InitCursor(// [Dasperal] Big cursor
                       DEH_String(skullName[0]),
                       DEH_String(skullName[1]),
                       // [Dasperal] Small cursor
                       DEH_String("FNTSE062"),
                       // [Dasperal] Use FNTBE034 as empty patch for cursor in jaguar
                       // and use RD_CURD (dark vwrsion of FNTSE062) for generic doom
                       DEH_String(gamemission == jaguar ?
                                                 "FNTBE034" : "RD_CURD"),
                       -5, 0, -32, -8);

    // [JN] Init message colors.
    M_RD_Define_Msg_Color(msg_pickup, message_pickup_color);
    M_RD_Define_Msg_Color(msg_secret, message_secret_color);
    M_RD_Define_Msg_Color(msg_system, message_system_color);
    M_RD_Define_Msg_Color(msg_chat, message_chat_color);

    // [JN] Widescreen: set temp variable for rendering menu.
    aspect_ratio_temp = aspect_ratio;

    menuactive = 0;
    whichSkull = 0;
    skullAnimCounter = 10;

    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

    switch ( gamemode )
    {
        case commercial:
            // Commercial has no "read this" entry.
            MainMenu = &Doom2Menu;
            NewGameMenu.prevMenu = MainMenu;
            break;
        case pressbeta:
            // [JN] Use special menu for Press Beta
            MainMenu = &MainMenuBeta;
            break;
        case shareware:
            // Episode 2 and 3 are handled,
            // branching to an ad screen.
        case registered:
        case retail:
            // We are fine.
        default:
            MainMenu = &DoomMenu;
            break;
    }

    // [JN] Move up Jaguar options menu to don't draw it over status bar
    if (gamemission == jaguar)
    {
        OptionsMenu.y -= 6;
    }

    // Versions of doom.exe before the Ultimate Doom release only had
    // three episodes; if we're emulating one of those then don't try
    // to show episode four. If we are, then do show episode four
    // (should crash if missing).
    if (gameversion < exe_ultimate)
    {
        // [Dasperal] Sigil
        if (sgl_loaded)
            EpisodeMenu = &DoomSigilEpisodeMenu;
        else
            EpisodeMenu = &DoomEpisodeMenu;
    }
    else
    {
        // [crispy] & [JN] Sigil
        if (sgl_loaded)
            EpisodeMenu = &UltimateSigilEpisodeMenu;
        else
            EpisodeMenu = &UltimateEpisodeMenu;
    }

    // [JN] Correct return to previous menu
    if (NewGameMenu.prevMenu != MainMenu)
        NewGameMenu.prevMenu = EpisodeMenu;
    OptionsMenu.prevMenu = MainMenu;
    LoadMenu.prevMenu = MainMenu;
    SaveMenu.prevMenu = MainMenu;
    OptionsMenu_Vanilla.prevMenu = MainMenu;

    CurrentMenu = MainMenu;
    CurrentItPos = CurrentMenu->lastOn;

	// [crispy] rearrange Load Game and Save Game menus
	patchl = W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE);
	patchs = W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE);
	patchm = W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE);

	LoadDef_x = (ORIGWIDTH - SHORT(patchl->width)) / 2 + SHORT(patchl->leftoffset);
	SaveDef_x = (ORIGWIDTH - SHORT(patchs->width)) / 2 + SHORT(patchs->leftoffset);
	LoadMenu.x_eng = LoadMenu.x_rus = SaveMenu.x_eng = SaveMenu.x_rus =
	        (ORIGWIDTH - 24 * 8) / 2 + SHORT(patchm->leftoffset); // [crispy] see M_DrawSaveLoadBorder()

	captionheight = MAX(SHORT(patchl->height), SHORT(patchs->height));

	vstep = ORIGHEIGHT - 32; // [crispy] ST_HEIGHT
	vstep -= captionheight;
	vstep -= (8 - 1) * LINEHEIGHT + SHORT(patchm->height);
	vstep /= 3;

	if (vstep > 0)
	{
		LoadDef_y = vstep + captionheight - SHORT(patchl->height) + SHORT(patchl->topoffset);
		SaveDef_y = vstep + captionheight - SHORT(patchs->height) + SHORT(patchs->topoffset);
	    LoadMenu.y = SaveMenu.y = vstep + captionheight + vstep + SHORT(patchm->topoffset) - 7; // [crispy] see M_DrawSaveLoadBorder()
	}

    opldev = M_CheckParm("-opldev") > 0;
}

// [from crispy] Возможность удаления сохраненных игр
static char *savegwarning;
static void M_ConfirmDeleteGameResponse (int key)
{
    free(savegwarning);

    if (key == key_menu_confirm)
    {
        char name[256];

        M_StringCopy(name, P_SaveGameFile(CurrentItPos), sizeof(name));
        remove(name);
        M_ReadSaveStrings();
    }
}

void M_ConfirmDeleteGame ()
{
    // [JN] Do not print savegame name in Russian language because of
    // lack of English symbols in message.
    if (english_language)
    {
        savegwarning =
            M_StringJoin("are you sure you want to\ndelete saved game\n\n\"",
            savegamestrings[CurrentItPos], "\"?\n\n", PRESSYN, NULL);
    }
    else
    {
        savegwarning =
            M_StringJoin("ds ltqcndbntkmyj [jnbnt\nelfkbnm cj[hfytyye. buhe?\n\n",
            PRESSYN_RUS, NULL);
    }

    M_StartMessage(savegwarning, M_ConfirmDeleteGameResponse, true);
    messageToPrint = 2;
    S_StartSound(NULL,sfx_swtchn);
}

boolean SCNetCheck(int option)
{
    if (!netgame)
    {                           // okay to go into the menu
        return true;
    }
    switch (option)
    {
        case 1:
            M_StartMessage(DEH_String(english_language ?
                           NEWGAME : NEWGAME_RUS), NULL,false);
            break;
        case 2:
            M_StartMessage(DEH_String(english_language ?
                           LOADNET : LOADNET_RUS), NULL,false);
            break;
        default:
            break;
    }
    menuactive = false;

    return false;
}

void SetMenu(const Menu_t* menu)
{
    CurrentMenu->lastOn = CurrentItPos;
    CurrentMenu = (Menu_t*) menu;
    CurrentItPos = CurrentMenu->lastOn;

    // [JN] Force to use vanilla options menu in -vanilla game mode.
    if (vanillaparm)
    {
        if (CurrentMenu == &OptionsMenu)
        {
            CurrentMenu = &OptionsMenu_Vanilla;
        }
    }
}

void RD_Menu_StartSound(MenuSound_t sound)
{
    switch (sound)
    {
        case MENU_SOUND_CURSOR_MOVE:
            S_StartSound(NULL,sfx_pstop);
            break;
        case MENU_SOUND_SLIDER_MOVE:
            S_StartSound(NULL,sfx_stnmov);
            break;
        case MENU_SOUND_CLICK:
        case MENU_SOUND_PAGE:
            S_StartSound(NULL,sfx_pistol);
            break;
        case MENU_SOUND_DEACTIVATE:
            S_StartSound(NULL,sfx_swtchx);
            break;
        case MENU_SOUND_BACK:
        case MENU_SOUND_ACTIVATE:
            S_StartSound(NULL,sfx_swtchn);
            break;
        default:
            break;
    }
}
