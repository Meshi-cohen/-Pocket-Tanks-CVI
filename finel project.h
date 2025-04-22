/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  GAME_PANEL                       1       /* callback function: moveTank */
#define  GAME_PANEL_EXITGAME              2       /* control type: command, callback function: exitgame */
#define  GAME_PANEL_POWER_SLIDE           3       /* control type: scale, callback function: (none) */
#define  GAME_PANEL_RESET                 4       /* control type: command, callback function: reset */
#define  GAME_PANEL_FIRE                  5       /* control type: command, callback function: fireBut */
#define  GAME_PANEL_WEAPON                6       /* control type: ring, callback function: (none) */
#define  GAME_PANEL_ANGLE                 7       /* control type: scale, callback function: (none) */
#define  GAME_PANEL_WINNER_BUT            8       /* control type: command, callback function: savewinner */
#define  GAME_PANEL_SPLITTER              9       /* control type: splitter, callback function: (none) */
#define  GAME_PANEL_SPLITTER_2            10      /* control type: splitter, callback function: (none) */
#define  GAME_PANEL_CANVAS                11      /* control type: canvas, callback function: (none) */
#define  GAME_PANEL_SCORE2                12      /* control type: numeric, callback function: (none) */
#define  GAME_PANEL_SCORE1                13      /* control type: numeric, callback function: (none) */
#define  GAME_PANEL_WINNERMSG             14      /* control type: textMsg, callback function: (none) */
#define  GAME_PANEL_PLAYER2               15      /* control type: textMsg, callback function: (none) */
#define  GAME_PANEL_PLAYER1               16      /* control type: textMsg, callback function: (none) */
#define  GAME_PANEL_TURNNAME              17      /* control type: string, callback function: (none) */
#define  GAME_PANEL_TIMER                 18      /* control type: timer, callback function: timerf */

#define  MENU_PANEL                       2
#define  MENU_PANEL_CANVAS                2       /* control type: canvas, callback function: (none) */
#define  MENU_PANEL_EXITMENU              3       /* control type: command, callback function: exitmenu */
#define  MENU_PANEL_STARTGAME             4       /* control type: command, callback function: startgame */
#define  MENU_PANEL_NAME_2                5       /* control type: string, callback function: (none) */
#define  MENU_PANEL_NAME                  6       /* control type: string, callback function: (none) */
#define  MENU_PANEL_TXTBOX                7       /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK exitgame(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK exitmenu(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK fireBut(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK moveTank(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK reset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK savewinner(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK startgame(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK timerf(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif