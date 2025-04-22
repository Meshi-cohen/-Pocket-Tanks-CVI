#include <windows.h>
#include <formatio.h>
#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include <utility.h>	
#include <analysis.h>
#include <math.h>
#include "finel project.h"

#define G 9.81

#define MAX_SCORE 150

#define PI 3.1415

#define MOVE_PIXELS 10

#define TANK_HEIGHT 60;
#define TANK_WIDTH 75;

#define PIZUZ_NUM 20
#define PIZUZ_SIZE 5 
#define CANNON_SIZE 15 

#define SIND(a) (sin((a)/180.0 * PI))
#define COSD(a) (cos((a)/180.0 * PI))

double intervaldelay=0.05; //the interval of rocket

static int menuPanel, gamePanel;

int height,width; //size of canvas

//CmtThreadLockHandle mlock; //not used

Point points[12]= {{0,466},{0,450},{260,450},{310,420},{450,300},{505,200},{630,300},{680,350},{700,420},{760,450},{1000,450},{1000,466}}; //the points that build the mount


int flag =0; //flag pagaz 1  
int flag1 =0; //flag pagaz 2

int state=2; //state of the player ( state 0 = win | state 1= fire | state 2= wait for press fire | state 3= after reset mode )

int count1,count2; //Count the number of bullets that went outside the canvas frame

typedef struct {
	int bitmapid;
	int width;
	int height;
	int* pixelarr;
	int pixelsize;
	int bytesperrow;
	int pixeldepth;
	int bytesize;
}photoData; //struct of photo data

typedef  struct
	{
	double x;
	double y;
	} Point1; // struct of point - containts the exact calculation. the canvas contains double type so we use casting to int

typedef struct {
	
	Point1 curLocation;
	Point1 prevLocation;

	Point1 curVelocity;
	Point1 prevVelocity;

	Rect weaponFrame;
	
	int c; // flag pitzuz // check if the ball which out of the frame is count 
	
}Weapon; //struct of weapon - containts the iterative calculation  

typedef struct {
	char playerName[100];
	
	Weapon* pagaz; 
	Weapon* pagaz1;
	
	double cannonAngle; // the angle of the cannon
	
	double power; // velocity of tank
	int weaponType; //type of weapon single==0 of double==1
	int numofturns; //numbers of turns he did
	int flag; //flag for the parallel calculation 
	int score; // the score
	int turn; //the turn that need to play 

	Rect tankFrame; //the tanks frame rect (size of the photo)
	
	photoData imageData; //the tanks photos
	
	CmtThreadFunctionID threadid; //parallel calculation 
	
}Tank; // struct of tank

Tank t1,t2; //t1- player 1 | t2- player 2


Weapon singleW,doubleW;


Weapon piz[PIZUZ_NUM]; //for the explotion hit 
Weapon piz1[PIZUZ_NUM]; //for the explotion hit 


void calculate (Weapon* w);

void resetData(Tank* t);

void drawTank(Tank* t);

void savetofile();

void pitzuz(Point1 p1, int* x,int f);

void pitzuz1(Point1 p1, int* x,int f);

int hit(Tank* t,Point1 p);

void EndGame();

int mounthit(Point1 p1);

void fireData();

void draw();

void drawMount() ;

void drawPagaz();

//2
void initializeData() // initializing the initial information
{
	GetCtrlAttribute (gamePanel, GAME_PANEL_CANVAS, ATTR_HEIGHT, &height); 
	GetCtrlAttribute (gamePanel, GAME_PANEL_CANVAS, ATTR_WIDTH, &width);
	
	GetBitmapFromFileEx ("tank1+edit-right-crop.png", 0, &t1.imageData.bitmapid); //load tank 1 photo
	GetBitmapInfoEx (t1.imageData.bitmapid, NULL, &t1.imageData.bytesize,NULL, NULL);
	t1.imageData.pixelarr = malloc(t1.imageData.bytesize*4);
	GetBitmapDataEx (t1.imageData.bitmapid, &t1.imageData.bytesperrow, &t1.imageData.pixeldepth, &t1.imageData.width, &t1.imageData.height, NULL, (unsigned char*) t1.imageData.pixelarr, NULL, NULL);
	t1.imageData.pixelsize = t1.imageData.width*t1.imageData.height;
	
	t1.tankFrame.height=TANK_HEIGHT; // the height of t1
	t1.tankFrame.width=TANK_WIDTH; // the width of t1
	
	t1.tankFrame.left=width-t1.tankFrame.width;
	t1.tankFrame.top=450-t1.tankFrame.height;
	
	GetBitmapFromFileEx ("tank2+edit-left-crop.png", 0, &t2.imageData.bitmapid); //load tank 2 photo
	GetBitmapInfoEx (t2.imageData.bitmapid, NULL, &t2.imageData.bytesize,NULL, NULL);
	t2.imageData.pixelarr = malloc(t2.imageData.bytesize*4);
	GetBitmapDataEx (t2.imageData.bitmapid, &t2.imageData.bytesperrow, &t2.imageData.pixeldepth, &t2.imageData.width, &t2.imageData.height, NULL, (unsigned char*) t2.imageData.pixelarr, NULL,NULL);
	t2.imageData.pixelsize = t2.imageData.width*t2.imageData.height;
	
	t2.tankFrame.height=TANK_HEIGHT; // the height of t2
	t2.tankFrame.width=TANK_WIDTH; // the width of t2
	
	t2.tankFrame.left=0;
	t2.tankFrame.top=450-t2.tankFrame.height;
	
	t1.pagaz=&singleW; 
	t1.pagaz1=&doubleW;
	
	t2.pagaz=&singleW;
	t2.pagaz1=&doubleW;
	
	t1.weaponType=0;
	
	t2.weaponType=0;
	
	for(int i=0;i<PIZUZ_NUM;i++) // the explosion setting
	{
		piz[i].weaponFrame=MakeRect(0,0,PIZUZ_SIZE,PIZUZ_SIZE); //size of the expotion balls of single cannon
		piz1[i].weaponFrame=MakeRect(0,0,PIZUZ_SIZE,PIZUZ_SIZE); //size of the expotion balls of single cannon
	}
	
	t1.turn=1;
	t2.turn=0;
	singleW.weaponFrame=MakeRect(0,0,CANNON_SIZE,CANNON_SIZE); //size of single cannon
	doubleW.weaponFrame=MakeRect(0,0,CANNON_SIZE,CANNON_SIZE); //size of single cannon
}

//3
int CVICALLBACK threadfunction_fire(void *functionData) //the main function // responsible for the parallel calculation for tank 1
{
	int f1=0,f2=0; //flags check if the calculation is done
	//f1= responsible for stop the calc of t1
	//f2= responsible for stop the calc of t2
	int score=1,score1=1; //flags to hit the mount
	
	//CmtGetLock (mlock);
	
	while(t1.flag) //while the parallel calculation of tank 1 continuied
	{
		if(!f1) 
		{
			if(hit(&t2,t1.pagaz->prevLocation)) //if it was hit on the other tank
			{
				pitzuz(t1.pagaz->prevLocation,&f1,score); //do the pitzuz
				if(score) //check if the flag of t1 on
				{
					score=0; //reset the flag
					if(t1.weaponType) //if the weapon is the double cannon but one of the pagaz is hit
						t1.score+=15; //add to t1 score 15 points
					else 
						t1.score+=30; //the single cannon+the double cannon hit - add to t1 30 points
					SetCtrlVal (gamePanel, GAME_PANEL_SCORE1, t1.score); //display the score
				}			
			}
			else if(mounthit(t1.pagaz->prevLocation)) //the pagaz hit the mount
			{
				f1=1; //reset the flag of t1
			}
			else if((t1.pagaz->prevLocation.x>=width||t1.pagaz->prevLocation.x<=-15)) //if the pagaz out of the canvas boundry
					f1=1; //reset the flag of t1
			else 
				calculate(t1.pagaz); //keep calculate
		}
		
		if(t1.weaponType&&!f2) //if the weapon is the single cannon && flag of tank 2 off
		{
			if(hit(&t2,t1.pagaz1->prevLocation)) //if it was hit on the other tank
			{
				pitzuz1(t1.pagaz1->prevLocation,&f2,score1); //do the pitzuz
				if(score1) //the flag of t2 on
				{
					score1=0; //reset the flag
					t1.score+=15; //add to t1 score 15 points
					SetCtrlVal (gamePanel, GAME_PANEL_SCORE1, t1.score); //display the score
				}	
			}
			else if(mounthit(t1.pagaz1->prevLocation)) //the pagaz hit the mount
			{		
				f2=1; //reset the flag of t2
			}
			else if((t1.pagaz1->prevLocation.x>=width||t1.pagaz1->prevLocation.x<=-15)) //if the pagaz out of the canvas boundry
					f2=1; //reset the flag of t2
			else 
				calculate(t1.pagaz1); //keep calculate
		
		}
		else 
			f2=1; //reset the flag of t2
		
		draw();
		Sleep(1000*0.001);
		if(f1&&f2) //if the flags on 
		{
			t1.flag=0; //stop calculation 
		}
	}
	//CmtReleaseLock (mlock);
	
	state=2; //wait for fire press
	EndGame(); 
	return 0;
}

//3
int CVICALLBACK threadfunction_fire2(void *functionData) //the main function // responsible for the parallel calculation for tank 2
{
	int f1=0,f2=0;
	int score=1,score1=1;
	
	//CmtGetLock (mlock);
	
	while(t2.flag)
	{
		if(!f1)
		{
		if(hit(&t1,t2.pagaz->prevLocation))
		{
			pitzuz(t2.pagaz->prevLocation,&f1,score);
			if(score)
			{
				score=0;
				if(t2.weaponType)
					t2.score+=15;
				else 
					t2.score+=30;
				SetCtrlVal (gamePanel, GAME_PANEL_SCORE2, t2.score);
			}			
		}
		else if(mounthit(t2.pagaz->prevLocation))
		{	
			f1=1;
		}
		else if((t2.pagaz->prevLocation.x>=width||t2.pagaz->prevLocation.x<=-15))
				f1=1;
		else 
			calculate(t2.pagaz);
		}
				
		if(t2.weaponType&&!f2)
		{
			if(hit(&t1,t2.pagaz1->prevLocation))
			{
				pitzuz1(t2.pagaz1->prevLocation,&f2,score1);
				if(score1)
				{
					score1=0;
					t2.score+=15;
					SetCtrlVal (gamePanel, GAME_PANEL_SCORE2, t2.score);
				}
			}
			else if(mounthit(t2.pagaz1->prevLocation))
			{
				
				f2=1;
			}
			else if((t2.pagaz1->prevLocation.x>=width||t2.pagaz1->prevLocation.x<=-15))
					f2=1;
			else 
				calculate(t2.pagaz1);
		}
		else 
			f2=1;
				
		draw();
		Sleep(1000*0.001);
		if(f1&&f2)
		{
			t2.flag=0;
		}
	}
	//CmtReleaseLock (mlock);
	
	state=2;
	EndGame();
	return 0;
}

//8
int CVICALLBACK moveTank (int panel, int event, void *callbackData,
                               int eventData1, int eventData2)
{
	Point1 p;
	if(t1.flag||t2.flag)//if the parallel calculation continued not to allow the move of the tanks 
		return 0;
    switch (event)
    {
        case EVENT_KEYPRESS: //communication with the keyboard
			switch(eventData1) 
			{
				case VAL_LEFT_ARROW_VKEY:
					t1.tankFrame.left=t1.tankFrame.left-MOVE_PIXELS; 
					p.x=t1.tankFrame.left;
					p.y=t1.tankFrame.top+t1.tankFrame.height;
					if(mounthit(p))
						t1.tankFrame.left=t1.tankFrame.left+MOVE_PIXELS;
					break;
					
				case VAL_RIGHT_ARROW_VKEY:
					t1.tankFrame.left=t1.tankFrame.left+MOVE_PIXELS;
					if(t1.tankFrame.left>=width-t1.tankFrame.width)
						t1.tankFrame.left=width-t1.tankFrame.width;
					break;
					//small caps
				case 'a':
					t2.tankFrame.left=t2.tankFrame.left-MOVE_PIXELS;
					if(t2.tankFrame.left<0) 
						t2.tankFrame.left=0;
					break;
				case 'd':
					t2.tankFrame.left=t2.tankFrame.left+MOVE_PIXELS;
					p.x=t2.tankFrame.left+t2.tankFrame.width;
					p.y=t2.tankFrame.top+t2.tankFrame.height;
					if(mounthit(p)) 
						t2.tankFrame.left=t2.tankFrame.left-MOVE_PIXELS;
					break;
					//caps lock
				case 'A':
					t2.tankFrame.left=t2.tankFrame.left-MOVE_PIXELS;
					if(t2.tankFrame.left<0)
						t2.tankFrame.left=0;
					break;
				case 'D':
					t2.tankFrame.left=t2.tankFrame.left+MOVE_PIXELS;
					p.x=t2.tankFrame.left+t2.tankFrame.width;
					p.y=t2.tankFrame.top+t2.tankFrame.height;
					if(mounthit(p))
						t2.tankFrame.left=t2.tankFrame.left-MOVE_PIXELS;
					break;		
			}
			draw();
            break;
    }
    return 0;
}

//9
int main (int argc, char *argv[]) 
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((menuPanel = LoadPanel (0, "finel project.uir", MENU_PANEL)) < 0)
		return -1;
	if ((gamePanel = LoadPanel (0, "finel project.uir", GAME_PANEL)) < 0)
		return -1;
	srand(clock()*17); //random of the  balls exsplotion
	initializeData(); 
	//CmtNewLock(NULL, 0, &mlock);

	//InstallPanelCallback (gamePanel, moveTank, 0);
	DisplayPanel (menuPanel);
	resetData(&t1); //reset data t1
	resetData(&t2); //reset data t2
	draw();
	RunUserInterface ();

	reset (0, 0, EVENT_COMMIT,NULL,0,0);
	
	DiscardBitmap(t1.imageData.bitmapid);
	DiscardBitmap(t2.imageData.bitmapid);
	free(t1.imageData.pixelarr); //free
	free(t2.imageData.pixelarr); //free
	
	DiscardPanel (menuPanel);
	DiscardPanel (gamePanel);
	//CmtDiscardLock (mlock);
	
	return 0;
}

//10
void draw() //draw function
{
	
	CanvasStartBatchDraw (gamePanel, GAME_PANEL_CANVAS);
	CanvasClear (gamePanel, GAME_PANEL_CANVAS, VAL_ENTIRE_OBJECT);
	drawMount(); //draw mount function
	drawTank(&t1); //draw tank 1
	drawTank(&t2); //draw tank 2
	for(int i=0; i<PIZUZ_NUM ;i++) //the exsplotion draw | always happens
	{
		SetCtrlAttribute (gamePanel, GAME_PANEL_CANVAS, ATTR_PEN_FILL_COLOR, VAL_RED);
		if(flag) 
			CanvasDrawOval (gamePanel, GAME_PANEL_CANVAS, piz[i].weaponFrame, VAL_DRAW_FRAME_AND_INTERIOR);
		if(flag1&&(t1.weaponType||t2.weaponType))
			CanvasDrawOval (gamePanel, GAME_PANEL_CANVAS, piz1[i].weaponFrame, VAL_DRAW_FRAME_AND_INTERIOR);
	}
	CanvasEndBatchDraw (gamePanel, GAME_PANEL_CANVAS);
}

//11
void drawTank(Tank* t)
{
	if(t1.flag||t2.flag) //if the flags of the parallel calculation is on  
		drawPagaz(t); // draw the pagaz
	
	CanvasDrawBitmap (gamePanel, GAME_PANEL_CANVAS, t->imageData.bitmapid, VAL_ENTIRE_OBJECT, t->tankFrame );	
}

//12
void drawMount() //draw mount function
{	
	SetCtrlAttribute (gamePanel, GAME_PANEL_CANVAS, ATTR_PEN_FILL_COLOR, VAL_GREEN);
	CanvasDrawPoly (gamePanel, GAME_PANEL_CANVAS,12, points, 1, VAL_DRAW_FRAME_AND_INTERIOR);
}

int CVICALLBACK exitmenu (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
	}
	return 0;
}

//13
int CVICALLBACK startgame (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2) // display the main panel // receiving data from the user (names of players)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (menuPanel, MENU_PANEL_NAME, t1.playerName);
			GetCtrlVal (menuPanel, MENU_PANEL_NAME_2, t2.playerName);
			DisplayPanel(gamePanel);
			SetCtrlVal (gamePanel, GAME_PANEL_PLAYER1, t1.playerName);
			SetCtrlVal (gamePanel, GAME_PANEL_PLAYER2, t2.playerName);

			drawMount();
			HidePanel(menuPanel);
			break;
	}
	return 0;
}

//14
int CVICALLBACK exitgame (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2) //exit game but
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(state==1) //if the pagaz is on the function cant happens
				break;
			
			reset(gamePanel,0, EVENT_COMMIT,NULL, 0, 0); 
			DisplayPanel(menuPanel);
			HidePanel(gamePanel);
			break;
	}
	return 0;
}

//15
void resetData(Tank* t) //reset data function | gets the tank palyer
{
	int a=180, x=0;
	//a= 180 angle or 0 variable on the two tanks 
	if(t==&t2) 
	{
		x=t2.tankFrame.width; 
		a=0; 
	}
	//the two tanks have a diffrent initials values
	t->pagaz->prevLocation.x=t->tankFrame.left+x-6;
	t->pagaz->prevLocation.y=t->tankFrame.top+6;
	t->pagaz->weaponFrame.left=t->pagaz->prevLocation.x;
	t->pagaz->weaponFrame.top=t->pagaz->prevLocation.y;
					
	t->pagaz->prevVelocity.x=t->power*COSD(t1.cannonAngle+a);
	t->pagaz->prevVelocity.y=t->power*SIND(t1.cannonAngle+180);
	t->pagaz->curVelocity.x=t->pagaz->prevVelocity.x;
	t->pagaz->curVelocity.y=t->pagaz->prevVelocity.y;
	
	if(t==&t2)
		t2.pagaz->prevLocation.y=t2.tankFrame.top+10;
	
	if(t->weaponType==1) 
	{
		t->pagaz1->prevLocation.x=t->tankFrame.left+x-6;
		t->pagaz1->prevLocation.y=t->tankFrame.top+6;
		t->pagaz1->weaponFrame.left=t->pagaz1->prevLocation.x;
		t->pagaz1->weaponFrame.top=t->pagaz1->prevLocation.y;
						
		t->pagaz1->prevVelocity.x=t->power*COSD(t->cannonAngle+a)*0.95;
		t->pagaz1->prevVelocity.y=t->power*SIND(t->cannonAngle+180)*0.95;
		t->pagaz1->curVelocity.x=t->pagaz1->prevVelocity.x;
		t->pagaz1->curVelocity.y=t->pagaz1->prevVelocity.y;
		
		if(t==&t2)
			t2.pagaz1->prevLocation.y=t2.tankFrame.top+10;
	}
}

//16
int CVICALLBACK fireBut (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2) //fire but | if there is a victory or there is a ball in the air, he will not continue to shoot a ball
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(state==0)
				break;
			
			if((t1.flag||t2.flag)&&state==1) 
				break;
			
			state=1;
			if(t1.turn)
			{
				fireData(&t1);
				resetData(&t1);
				t1.numofturns++; //count the turns
				t1.flag=1;
				count1=0;
				count2=0;
				
				CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, threadfunction_fire, NULL, &t1.threadid);
				CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, t1.threadid);
				flag=0;
				flag1=0;
				t2.turn=1;
				t1.turn=0;
			}
			else
			{
				fireData(&t2);	
				resetData(&t2);
				t2.numofturns++; //count the turns
				t2.flag=1;
				count1=0;
				count2=0;
				
				CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, threadfunction_fire2, NULL, &t2.threadid);
				CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, t2.threadid);

				flag=0;
				flag1=0;
				t1.turn=1;
				t2.turn=0;
			}
		
			break;
	}
	return 0;
}

//5
void EndGame() //cheak if the game end
{
	if(t1.score>=MAX_SCORE) // t1 win
	{
  		 MessagePopup("Win",t1.playerName);
		 state=0; //the reset mode
	}
	else if(t2.score>=MAX_SCORE) //t2 win
	{
		MessagePopup("Win",t2.playerName);
		state=0; //the reset mode
	}		
}

//1
void calculate (Weapon* w) //get a single weapon and makes an iterative calculation. gets the current location and calculate thee next location 
{
	w->curLocation.x=w->prevLocation.x+w->prevVelocity.x*intervaldelay;
	w->curLocation.y=w->prevLocation.y+w->prevVelocity.y*intervaldelay+0.5*G*pow(intervaldelay,2);
	w->curVelocity.y=w->prevVelocity.y+G*intervaldelay;
	w->prevVelocity.y=w->curVelocity.y;

	w->prevLocation.x=w->curLocation.x;
	w->prevLocation.y=w->curLocation.y;
	w->weaponFrame.left=w->curLocation.x;
	w->weaponFrame.top=w->curLocation.y;
}

//17
void drawPagaz(Tank* t) //draw pagaz 1 or draw 1+2 pagaz | gets tank 
{
	SetCtrlAttribute (gamePanel, GAME_PANEL_CANVAS, ATTR_PEN_FILL_COLOR, VAL_BLACK);
	if(!flag) //if flag=0 not drawing the ball | flag=1 only if fire
		CanvasDrawOval (gamePanel, GAME_PANEL_CANVAS, t->pagaz->weaponFrame, VAL_DRAW_FRAME_AND_INTERIOR);
	if(t->weaponType==1&&!flag1) //flag1 = if it was an explotion so to conceal the dark pagaz
	{
		CanvasDrawOval (gamePanel, GAME_PANEL_CANVAS, t->pagaz1->weaponFrame, VAL_DRAW_FRAME_AND_INTERIOR);
	}
}

//6
int hit(Tank* t, Point1 p) //check if it was a hit on the tank |gets the rival tank and location of the cannon
{	
	if((int)p.y>=t->tankFrame.top) 
	{
		if((int)p.x>=t->tankFrame.left && (int)p.x<=t->tankFrame.left+t->tankFrame.width)
		{
			return 1;
		}
	}
	return 0;
}

//7
int mounthit(Point1 p1) //check if it was a hit on the mountent | gets the cannon point
{
	Point p; //casting 
	p.x=(int)p1.x;
	p.y=(int)p1.y;
	
	int pc; //check the color of pixel of the mount
	CanvasGetPixel (gamePanel, GAME_PANEL_CANVAS, p, &pc);
	if(pc==VAL_GREEN)
		return 1;
	
	return 0;
}

//18
void fireData(Tank* t) //the fire data | gets tank 
{
	int temp; //the single of double weapon
	GetCtrlVal (gamePanel, GAME_PANEL_ANGLE, &t->cannonAngle);
	GetCtrlVal (gamePanel, GAME_PANEL_POWER_SLIDE, &t->power);
	GetCtrlVal (gamePanel, GAME_PANEL_WEAPON, &temp);
	switch(temp)
	{
		case 1:
			t1.weaponType=0;
			t2.weaponType=0;
		break;
		
		case 2:			
			t1.weaponType=1;
			t2.weaponType=1;
		break;
	}							
}

//19
int CVICALLBACK reset (int panel, int control, int event,
					   void *callbackData, int eventData1, int eventData2)
{
	int visible; // cheak if the panel is on mode 
	switch (event) 
	{
		case EVENT_COMMIT:
			if(state==3||state==1)
				break;
			
			t1.score=0;
			t2.score=0;
			t1.numofturns=0;
			t2.numofturns=0;
			if(t1.flag)
			{
				t1.flag=0;
			}
			
			if(t2.flag)
			{
				t2.flag=0;
			}
			GetPanelAttribute (gamePanel, ATTR_VISIBLE, &visible);
			if(visible)
			{
				SetCtrlVal (gamePanel, GAME_PANEL_SCORE1, t1.score);
				SetCtrlVal (gamePanel, GAME_PANEL_SCORE2, t2.score);
			}
			resetData(&t1);
			resetData(&t2);
				   
			flag=0;
			flag1=0;
			draw();
			state=3;//after reset mode
			t1.turn=1;
			t2.turn=0;
			
			break;
	}
	return 0;
}

//4
void pitzuz1(Point1 p1, int* x,int f) //pitzuz for pagaz 2
{
	flag1=1;
	Point p;
	p.x=(int)p1.x;
	p.y=(int)p1.y;
	if(f) //the initial data- if f==0 
	{
		for(int i=0;i<PIZUZ_NUM;i++)
		{
			piz1[i].prevLocation.x=p.x;
			piz1[i].prevLocation.y=p.y;
			piz1[i].prevVelocity.x=rand()%13;
			piz1[i].prevVelocity.y=rand()%13;
			
			if(i%2==0) //even number to make the explotion better  
			{ //calculate the velocity of the balls in the explotion
				piz1[i].prevVelocity.x=-piz1[i].prevVelocity.x;
				piz1[i].prevVelocity.y=-piz1[i].prevVelocity.y;
			}	
			piz1[i].c=1; 
		}
	}

	for(int j=0;j<PIZUZ_NUM;j++)
	{
				calculate(&piz1[j]); 
				if(piz1[j].c&&(piz1[j].prevLocation.x>=width||piz1[j].prevLocation.x<=-5||piz1[j].prevLocation.y>height||piz1[j].prevLocation.y<-5))
				{
					piz1[j].c=0;
					count2++; //count the num of balls which outside the canvas 
				}
	}
	if(count2>=PIZUZ_NUM) //if the number of balles which outside the canvas are the number of the balls pizuz number
	{
		*x=1; //flag on and the function is done
	}

}

//4
void pitzuz(Point1 p1, int *f1,int f) //pitzuz for pagaz 1
{
	flag=1;
	Point p;
	p.x=(int)p1.x;
	p.y=(int)p1.y;

	if(f)
	{
		for(int i=0;i<PIZUZ_NUM;i++)
		{
			piz[i].prevLocation.x=p.x;
			piz[i].prevLocation.y=p.y;
			piz[i].prevVelocity.x=rand()%13;
			piz[i].prevVelocity.y=rand()%13;
			
			if(i%2==0)
			{
				piz[i].prevVelocity.x=-piz[i].prevVelocity.x;
				piz[i].prevVelocity.y=-piz[i].prevVelocity.y;
			}	
			piz[i].c=1;
		}
	}

	for(int j=0;j<PIZUZ_NUM;j++)
	{
				calculate(&piz[j]);
				if(piz[j].c&&(piz[j].prevLocation.x>=width||piz[j].prevLocation.x<=-5||piz[j].prevLocation.y>height||piz[j].prevLocation.y<-5))
				{
					piz[j].c=0;
					count1++;
				}
	}
	if(count1>=PIZUZ_NUM)
	{
		*f1=1;
	}
}

//20
int CVICALLBACK savewinner (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2) //save winner function
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(t1.score<MAX_SCORE&&t2.score<MAX_SCORE)
			{
				MessagePopup("ERROR","can't save, No winner yet");
				break;
			}
				
			savetofile(); //save to file

			break;
	}
	return 0;
}

//21
void savetofile() //save to file function
{
	FILE* f;
	f = fopen ("tankscore.txt", "a+"); //open a new file or append mode

	if(t1.score>t2.score) //if tank 1 win
	{
		fprintf(f,"%s, ",t1.playerName); //save the win name
		fprintf(f,"%d\n",t1.numofturns); //save the numbers of turns which get him to win
	}
	else if(t1.score<t2.score) //if tank 2 win
	{
		fprintf(f,"%s, ",t2.playerName); //save the win name
		fprintf(f,"%d\n",t2.numofturns); //save the numbers of turns which get him to win
	}
	/*
	else
	{
		fprintf(f,"%s, ",t1.playerName);
		fprintf(f,"%d | ",t1.numofturns);
		fprintf(f,"%s, ",t2.playerName);
		fprintf(f,"%d\n",t2.numofturns);
	}*/
	fclose(f);	
}

//22
int CVICALLBACK timerf (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2) //timer responsible the apperance of the turns that display on the main panel
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			if(state==1) //fire mode
				break;
			if(t1.turn)
				SetCtrlVal(gamePanel,GAME_PANEL_TURNNAME,t1.playerName);
			else
				SetCtrlVal(gamePanel,GAME_PANEL_TURNNAME,t2.playerName);
			break;
	}
	return 0;
}





//void loadTankPhoto(Tank* t) 
//{
//	
//		
//	int asize1;
//	mColor y;
//
//	//if(&pd.pixelarr!=NULL)
//	//	free(pd.pixelarr);
//	////if(!FileSelectPopupEx ("", ".PNG", ".PNG", "", VAL_LOAD_BUTTON, 0, 0, pathname)) 
//		//return;
//	
//		GetBitmapFromFileEx (pathname, 0, &pd.bitmapid);			
//		GetBitmapInfoEx (pd.bitmapid, NULL, &pd.bytesize, NULL, &asize1);
//		pd.pixelarr = malloc(pd.bytesize*4);
//		unsigned char* alpha = malloc(asize1);
//		GetBitmapDataEx (pd.bitmapid, &pd.bytesperrow, &pd.pixeldepth, &pd.width, &pd.height, NULL, (unsigned char*) pd.pixelarr, NULL, alpha);
//		pd.pixelsize = pd.width*pd.height;
//			
//			
//			for (int i = 0; i < pd.height; i++) {
//			    for (int j = 0; j < pd.width; j++) {
//			        // Check if the pixel is within the frame boundary
//			        if (i < 150 || i >= height - 150 || j < 150 || j >= width - 150) {
//			            // Set the pixel to black (0x00000000)
//			            pixelarr[i * width + j] = 0xFFFFFFFF;
//			        }
//			    }
//			}
//			
//			
//			
//			int bitmapid2;
//				NewBitmapEx (bytesperrow, pixeldepth, width, height, NULL,(unsigned char*) rotatedImage, NULL, NULL, &bitmapid2);
//				CanvasDrawBitmap (gamePanel, GAME_PANEL_CANVAS, bitmapid2, VAL_ENTIRE_OBJECT, VAL_ENTIRE_OBJECT);
//			*/
//			//int* canvasArr=malloc(307200*4);
//			
//			//CanvasDrawRect (gamePanel, GAME_PANEL_CANVAS, MakeRect(20,20,300,300), VAL_DRAW_INTERIOR);
//			//CanvasGetPixels (gamePanel,GAME_ PANEL_CANVAS,MakeRect(0,0,639,479) ,canvasArr);
//		//	for(int i =0;i<asize1;i++)
//				//alpha[i]=0;
//			
//	for(int i=0;i<pd.height;i++)
//		{
//			for (int j = 0; j < pd.width; j++)
//			{
//					y.rgb=pd.pixelarr[i*pd.width+j];
//					pd.pixelarr[i*pd.width+j]=0x00000000;	
//					//pd.pixelarr[i*pd.width+j]=y.rgb;
//			}
//		}
//		
//		int bitmapid2;
//		NewBitmapEx (pd.bytesperrow, pd.pixeldepth, pd.width, pd.height, NULL, (unsigned char*) pd.pixelarr, NULL, alpha, &bitmapid2);
//		CanvasDrawBitmap (gamePanel, GAME_PANEL_CANVAS, bitmapid2, VAL_ENTIRE_OBJECT, VAL_ENTIRE_OBJECT);
//		//CanvasDrawBitmap (gamePanel, GAME_PANEL_CANVAS, bitmapid2, VAL_ENTIRE_OBJECT, t1.tankFrame);
//			//SaveBitmapToPNGFile (bitmapid2, "background.png");
//			
//			
//			CanvasDrawBitmap (gamePanel, GAME_PANEL_CANVAS, pd.bitmapid, t1.tankFrame, VAL_ENTIRE_OBJECT);
//}
//void rotatePhoto(photoData* pd)
//{
//	//unsigned int Default_Pixel=0xFFFFFFFF;
//	unsigned int* rotatedImage = malloc(pd->pixelsize*4);
//	double sinx = sin(pd->angle);  // -degrees
//	double cosx = cos(pd->angle);
//	
//	int xCenter = pd->height/2;        // Rotate image by its center.
//	int yCenter = pd->width/2;
//	for(int x=0; x<pd->height; x++)
// 	{
//		int xt = x - xCenter;
//		double xt_cosx = xt*cosx;
//		double xt_sinx = xt*sinx;
//		for(int y=0; y<pd->width; y++)
//		{
//			int yt = y - yCenter;
//			long xRotate = lround(xt_cosx - (yt*sinx)) + xCenter;
//			long yRotate = lround((yt*cosx) + xt_sinx) + yCenter;   
//			
//			pd->rotate.x= xRotate;
//			pd->rotate.y= yRotate;
//			if( (xRotate >= 0) && (xRotate < pd->height) && (yRotate >= 0) && (yRotate < pd->width) ) 
//			{
//				rotatedImage[x*pd->width+ y] = pd->pixelarr[xRotate*pd->width+yRotate];
//			} 
//			else 
//			{
//				//rotatedImage[x*pd->width+ y] = Default_Pixel; //ALPHA?
//				//add alpha arr!!!!!!
//			}
//		}
//	}
//}


