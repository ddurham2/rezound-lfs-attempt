/* 
 * Copyright (C) 2002 - David W. Durham
 * 
 * This file is part of ReZound, an audio editing application.
 * 
 * ReZound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 * 
 * ReZound is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "FXRezWaveView.h"

#include "FXWaveScrollArea.h"
#include "FXPopupHint.h"

#include "CSoundFileManager.h"
#include "CSoundWindow.h"

#include "../backend/CActionParameters.h"
#include "../backend/Edits/CCueAction.h"

#include <string>
#include <algorithm>

#include <istring>

#include "settings.h"

// --- declaration of FXWaveRuler -----------------------------------------------

class FXWaveRuler : public FXComposite
{
	FXDECLARE(FXWaveRuler)
public:
	FXWaveRuler(FXComposite *p,FXRezWaveView *parent,CLoadedSound *_loadedSound);
	virtual ~FXWaveRuler();

	virtual void create();

	size_t getClickedCue(FXint x,FXint y);

	// events I get by message

	long onPaint(FXObject *object,FXSelector sel,void *ptr);

	long onPopupMenu(FXObject *object,FXSelector sel,void *ptr);

	long onFindStartPosition(FXObject *object,FXSelector sel,void *ptr);
	long onFindStopPosition(FXObject *object,FXSelector sel,void *ptr);

	long onSetPositionToCue(FXObject *object,FXSelector sel,void *ptr);
	long onAddCue(FXObject *object,FXSelector sel,void *ptr);
	long onRemoveCue(FXObject *object,FXSelector sel,void *ptr);
	long onEditCue(FXObject *object,FXSelector sel,void *ptr);
	long onShowCueList(FXObject *object,FXSelector sel,void *ptr);

	long onLeftBtnPress(FXObject *object,FXSelector sel,void *ptr);
	long onMouseMove(FXObject *object,FXSelector sel,void *ptr);
	long onLeftBtnRelease(FXObject *object,FXSelector sel,void *ptr);

	// methods get information about and for altering which cue is focused
	void focusNextCue();
	void focusPrevCue();

	enum
	{
		ID_FIND_START_POSITION=FXComposite::ID_LAST,
		ID_FIND_STOP_POSITION,

		ID_SET_START_POSITION,
		ID_SET_STOP_POSITION,

		ID_ADD_CUE,
		ID_ADD_CUE_AT_START_POSITION,
		ID_ADD_CUE_AT_STOP_POSITION,
		ID_REMOVE_CUE,
		ID_EDIT_CUE,

		ID_SHOW_CUE_LIST,

		ID_POPUP_MENU,

		ID_LAST
	};

protected:
	FXWaveRuler() {}

private:
	FXRezWaveView *parent;

	CLoadedSound *loadedSound;
	CSound *sound;

	FXFont *font;

	size_t cueClicked; // the index of the cue clicked on holding the value between the click event and the menu item event
	int cueClickedOffset; // used when dragging cues to know how far from the middle a cue was clicked
	sample_pos_t origCueClickedTime;
	sample_pos_t addCueTime; // the time in the audio where the mouse was clicked to add a cue if that's what they choose

	size_t focusedCueIndex; // 0xffff,ffff if none focused
	sample_pos_t focusedCueTime;

	FXPopupHint *dragCueHint;

	CMoveCueActionFactory *moveCueActionFactory;
};




// --- FXRezWaveView ---------------------------------------------------------

FXDEFMAP(FXRezWaveView) FXRezWaveViewMap[]=
{
	//Message_Type					ID				Message_Handler
	//FXMAPFUNC(SEL_TIMEOUT,			FXWindow::ID_AUTOSCROLL,	FXRezWaveView::onAutoScroll),
};

FXIMPLEMENT(FXRezWaveView,FXPacker,FXRezWaveViewMap,ARRAYNUMBER(FXRezWaveViewMap))

FXRezWaveView::FXRezWaveView(FXComposite* p,CLoadedSound *_loadedSound) :
	FXPacker(p,FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0),

	rulerPanel(new FXWaveRuler(this,this,_loadedSound)),
	waveScrollArea(new FXWaveScrollArea(this,_loadedSound))
{
	enable();
}

FXRezWaveView::~FXRezWaveView()
{
}

void FXRezWaveView::setHorzZoom(double v,FXWaveCanvas::HorzRecenterTypes horzRecenterType)
{
	waveScrollArea->setHorzZoom(v,horzRecenterType);
	updateRuler();
}

double FXRezWaveView::getHorzZoom() const
{
	return waveScrollArea->getHorzZoom();
}

void FXRezWaveView::setVertZoom(double v)
{
	waveScrollArea->setVertZoom(v);
}

void FXRezWaveView::drawPlayPosition(sample_pos_t dataPosition,bool justErasing,bool scrollToMakeVisible)
{
	waveScrollArea->drawPlayPosition(dataPosition,justErasing,scrollToMakeVisible);
}

void FXRezWaveView::redraw()
{
	update();
	waveScrollArea->redraw();
	rulerPanel->update();
}

void FXRezWaveView::centerStartPos()
{
	waveScrollArea->centerStartPos();
	rulerPanel->update();
}

void FXRezWaveView::centerStopPos()
{
	waveScrollArea->centerStopPos();
	rulerPanel->update();
}

void FXRezWaveView::showAmount(double seconds,sample_pos_t pos,int marginPixels)
{
	waveScrollArea->showAmount(seconds,pos,marginPixels);
}

void FXRezWaveView::updateFromSelectionChange(FXWaveCanvas::LastChangedPositions lastChangedPosition)
{
	waveScrollArea->updateFromSelectionChange(lastChangedPosition);
}

void FXRezWaveView::updateFromEdit()
{
	waveScrollArea->updateFromEdit();
	update();
	rulerPanel->update();
}

void FXRezWaveView::updateRuler()
{
	rulerPanel->update();
}

void FXRezWaveView::updateRulerFromScroll(int deltaX,FXEvent *event)
{
	FXEvent e(*event);
	e.last_x+=deltaX;
	rulerPanel->onMouseMove(NULL,0,&e);
}

void FXRezWaveView::getWaveSize(int &top,int &height)
{
	top=waveScrollArea->getY();
	height=waveScrollArea->getHeight()-waveScrollArea->horizontalScrollBar()->getHeight();
}






// --- FXWaveRuler -------------------------------------------------------

#include "../backend/CLoadedSound.h"
#include "../backend/CSound.h"
#include "../backend/CSoundPlayerChannel.h"

#warning I might try making the zoom action zoom in and center the focused cue; then I would probably want to make clicking an un-cued area on the ruler make no cue focused so it would behave as before (with the start and stop positions)

FXDEFMAP(FXWaveRuler) FXWaveRulerMap[]=
{
	//Message_Type				ID						Message_Handler
	FXMAPFUNC(SEL_RIGHTBUTTONPRESS,		0,						FXWaveRuler::onPopupMenu),
	
	FXMAPFUNC(SEL_PAINT,			0,						FXWaveRuler::onPaint),

	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_FIND_START_POSITION,		FXWaveRuler::onFindStartPosition),
	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_FIND_STOP_POSITION,		FXWaveRuler::onFindStopPosition),

	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_SET_START_POSITION,		FXWaveRuler::onSetPositionToCue),
	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_SET_STOP_POSITION,		FXWaveRuler::onSetPositionToCue),

	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_ADD_CUE,			FXWaveRuler::onAddCue),
	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_ADD_CUE_AT_START_POSITION,	FXWaveRuler::onAddCue),
	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_ADD_CUE_AT_STOP_POSITION,	FXWaveRuler::onAddCue),
	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_REMOVE_CUE,			FXWaveRuler::onRemoveCue),
	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_EDIT_CUE,			FXWaveRuler::onEditCue),

	FXMAPFUNC(SEL_COMMAND,			FXWaveRuler::ID_SHOW_CUE_LIST,			FXWaveRuler::onShowCueList),

	// these handle drag, mouse click for focus, and mouse double click for edit cue(s)
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,		0,						FXWaveRuler::onLeftBtnPress),
	FXMAPFUNC(SEL_MOTION,			0,						FXWaveRuler::onMouseMove),
	FXMAPFUNC(SEL_LEFTBUTTONRELEASE,	0,						FXWaveRuler::onLeftBtnRelease),
};

FXIMPLEMENT(FXWaveRuler,FXComposite,FXWaveRulerMap,ARRAYNUMBER(FXWaveRulerMap))


FXWaveRuler::FXWaveRuler(FXComposite *p,FXRezWaveView *_parent,CLoadedSound *_loadedSound) :
	FXComposite(p,FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT, 0,0,0,13+8),

	parent(_parent),

	loadedSound(_loadedSound),
	sound(_loadedSound->sound),

	font(getApp()->getNormalFont()),

	dragCueHint(new FXPopupHint(p->getApp())),

	moveCueActionFactory(NULL)
{
	enable();
	flags|=FLAG_SHOWN; // I have to do this, or it will not show up.. like height is 0 or something

	FXFontDesc d;
	font->getFontDesc(d);
	d.weight=FONTWEIGHT_LIGHT;
	d.size=65;
	font=new FXFont(getApp(),d);

	focusedCueIndex=0xffffffff;
	focusNextCue(); // to make it focus the first one if there is one

	dragCueHint->create();

	moveCueActionFactory=new CMoveCueActionFactory;
}

FXWaveRuler::~FXWaveRuler()
{
	delete font;
}

void FXWaveRuler::create()
{
	FXComposite::create();
	font->create();
}

#define CUE_Y ((height-1)-(1+CUE_RADIUS))

long FXWaveRuler::onPaint(FXObject *object,FXSelector sel,void *ptr)
{
	FXComposite::onPaint(object,sel,ptr);

	FXEvent *ev=(FXEvent*)ptr;
	FXDCWindow dc(this,ev);

	#define LABEL_TICK_FREQ 80
	/*
	 * Here, we draw the ruler
	 * 	We want to always put 1 labled position every LABELED_TICK_FREQ pixels 
	 * 	And draw ruler ticks also
	 */

	FXint lastX=parent->waveScrollArea->getCanvasWidth();

	const FXint left=0;
	const FXint right=left+lastX;

	FXint s=ev->rect.x;
	s-=LABEL_TICK_FREQ-1;

	FXint e=s+ev->rect.w;
	e+=LABEL_TICK_FREQ-1;

	if(e>lastX)
		e=lastX;

	dc.setForeground(FXRGB(20,20,20));
#if REZ_FOX_VERSION<10117
	dc.setTextFont(font);
#else
	dc.setFont(font);
#endif
	for(FXint x=s;x<=e;x++)
	{
		if((x%LABEL_TICK_FREQ)==0)
		{
			dc.drawLine(x,0,x,10);

			const string time=sound->getTimePosition(parent->waveScrollArea->getSamplePosForScreenX(x));

			dc.drawText(x+2,12,time.c_str(),time.length());
		}
		else if(((x+(LABEL_TICK_FREQ/2))%(LABEL_TICK_FREQ))==0)
			dc.drawLine(x,0,x,4); // half way tick between labled ticks
		else if((x%(LABEL_TICK_FREQ/10))==0)
			dc.drawLine(x,0,x,2); // small tenth ticks
	}

	// render cues
	const size_t cueCount=sound->getCueCount();
	for(size_t t=0;t<cueCount;t++)
	{
		// ??? I could figure out the min and max screen-visible time and make sure that is in range before testing every cue
		FXint cueXPosition=parent->waveScrollArea->getCueScreenX(t);

		if(cueXPosition!=CUE_OFF_SCREEN)
		{
			dc.setForeground(FXRGB(0,0,0));
			const string cueName=sound->getCueName(t);
			dc.drawText(cueXPosition+CUE_RADIUS+1,height-1,cueName.data(),cueName.size());

			dc.drawArc(cueXPosition-CUE_RADIUS,CUE_Y-CUE_RADIUS,CUE_RADIUS*2,CUE_RADIUS*2,0*64,360*64);

			dc.setForeground(sound->isCueAnchored(t) ? FXRGB(0,0,255) : FXRGB(255,0,0));
			dc.drawLine(cueXPosition-1,height-4,cueXPosition-1,CUE_Y-1);
			dc.drawLine(cueXPosition,height-1,cueXPosition,CUE_Y);
			dc.drawLine(cueXPosition+1,height-4,cueXPosition+1,CUE_Y-1);

			if(focusedCueIndex!=0xffffffff && focusedCueTime==sound->getCueTime(t))
			{	// draw focus rectangle
				const int textWidth=font->getTextWidth(cueName.data(),cueName.size());
				const int textHeight=font->getTextHeight(cueName.data(),cueName.size());

				const int x=cueXPosition-CUE_RADIUS;
				const int y=min(height-1-textHeight,CUE_Y-CUE_RADIUS);
				const int w=CUE_RADIUS*2+1+textWidth;
				const int h=height-y;
				dc.drawFocusRectangle(x-1,y+1,w+2,h-1);
			}
		}
	}

	return 1;
}

size_t FXWaveRuler::getClickedCue(FXint x,FXint y)
{
	const size_t cueCount=sound->getCueCount();
	size_t cueClicked=cueCount;
	if(cueCount>0)
	{
		// go in reverse order so that the more recent one created can be removed first
		// I mean, if you accidently create one, then want to remove it, and it's on top
		// of another one then it needs to find that more recent one first
		for(size_t t=cueCount;t>=1;t--)
		{
			FXint X=parent->waveScrollArea->getCueScreenX(t-1);
			if(X!=CUE_OFF_SCREEN)
			{
				FXint Y=CUE_Y;

				// check distance from clicked point to the cue's position
				if( ((x-X)*(x-X))+((y-Y)*(y-Y)) <= ((CUE_RADIUS+1)*(CUE_RADIUS+1)) )
				{
					cueClickedOffset=x-X;
					cueClicked=t-1;
					break;
				}
			}
		}
	}
	return(cueClicked);
}

long FXWaveRuler::onPopupMenu(FXObject *object,FXSelector sel,void *ptr)
{
	if(!underCursor())
		return(1);

	// ??? if the parent->target is NULL, I should pop up the windows

	FXEvent *event=(FXEvent*)ptr;

	// see if any cue was clicked on
	cueClicked=getClickedCue(event->win_x,event->win_y);

	if(cueClicked<sound->getCueCount())
	{
		FXMenuPane cueMenu(this);
			// ??? make sure that these get deleted when cueMenu is deleted

			const sample_fpos_t cueTime=sound->getCueTime(cueClicked);

			if(cueTime<loadedSound->channel->getStartPosition())
				new FXMenuCommand(&cueMenu,"Set Start Position to Cue",NULL,this,ID_SET_START_POSITION);
			else if(cueTime>loadedSound->channel->getStopPosition())
				new FXMenuCommand(&cueMenu,"Set Stop Position to Cue",NULL,this,ID_SET_STOP_POSITION);
			else
			{
				new FXMenuCommand(&cueMenu,"Set Start Position to Cue",NULL,this,ID_SET_START_POSITION);
				new FXMenuCommand(&cueMenu,"Set Stop Position to Cue",NULL,this,ID_SET_STOP_POSITION);
			}
		
			new FXMenuSeparator(&cueMenu);
			new FXMenuCommand(&cueMenu,"&Edit Cue",NULL,this,ID_EDIT_CUE);
			new FXMenuCommand(&cueMenu,"&Remove Cue",NULL,this,ID_REMOVE_CUE);
			new FXMenuSeparator(&cueMenu);
			new FXMenuCommand(&cueMenu,"Show Cues &List",NULL,this,ID_SHOW_CUE_LIST);

		cueMenu.create();
		cueMenu.popup(NULL,event->root_x,event->root_y);
		getApp()->runModalWhileShown(&cueMenu);
	}
	else
	{
		addCueTime=  parent->waveScrollArea->getCueTimeFromX(event->win_x);

		FXMenuPane gotoMenu(this);
			// ??? make sure that these get deleted when gotoMenu is deleted
			new FXMenuCommand(&gotoMenu,"Center Start Position",NULL,this,ID_FIND_START_POSITION);
			new FXMenuCommand(&gotoMenu,"Center Stop Position",NULL,this,ID_FIND_STOP_POSITION);
			new FXMenuSeparator(&gotoMenu);
			new FXMenuCommand(&gotoMenu,"Add Cue at This Position",NULL,this,ID_ADD_CUE);
			new FXMenuCommand(&gotoMenu,"Add Cue at Start Position",NULL,this,ID_ADD_CUE_AT_START_POSITION);
			new FXMenuCommand(&gotoMenu,"Add Cue at Stop Position",NULL,this,ID_ADD_CUE_AT_STOP_POSITION);
			new FXMenuSeparator(&gotoMenu);
			new FXMenuCommand(&gotoMenu,"Show Cues &List",NULL,this,ID_SHOW_CUE_LIST);

		gotoMenu.create();
		gotoMenu.popup(NULL,event->root_x,event->root_y);
		getApp()->runModalWhileShown(&gotoMenu);
	}


	return(0);
}

long FXWaveRuler::onFindStartPosition(FXObject *object,FXSelector sel,void *ptr)
{ 
	FXWaveScrollArea *wsa=parent->waveScrollArea;
	wsa->centerStartPos();
	return 1;
}

long FXWaveRuler::onFindStopPosition(FXObject *object,FXSelector sel,void *ptr)
{
	FXWaveScrollArea *wsa=parent->waveScrollArea;
	wsa->centerStopPos();
	return 1;
}

long FXWaveRuler::onSetPositionToCue(FXObject *object,FXSelector sel,void *ptr)
{
	switch(FXSELID(sel))
	{
	case ID_SET_START_POSITION:
		loadedSound->channel->setStartPosition((sample_pos_t)sound->getCueTime(cueClicked));
		break;

	case ID_SET_STOP_POSITION:
		loadedSound->channel->setStopPosition((sample_pos_t)sound->getCueTime(cueClicked));
		break;
	}

	parent->waveScrollArea->updateFromSelectionChange();

	return 1;
}

long FXWaveRuler::onAddCue(FXObject *object,FXSelector sel,void *ptr)
{
	switch(FXSELID(sel))
	{
	case ID_ADD_CUE:
		if(parent->target)
			parent->target->handle(parent,MKUINT(parent->message,FXRezWaveView::SEL_ADD_CUE),&addCueTime);
		break;
	case ID_ADD_CUE_AT_START_POSITION:
		if(parent->target)
			parent->target->handle(parent,MKUINT(parent->message,FXRezWaveView::SEL_ADD_CUE_AT_START_POSITION),NULL);
		break;
	case ID_ADD_CUE_AT_STOP_POSITION:
		if(parent->target)
			parent->target->handle(parent,MKUINT(parent->message,FXRezWaveView::SEL_ADD_CUE_AT_STOP_POSITION),NULL);
		break;
	}
	update();

	return 1;
}

long FXWaveRuler::onRemoveCue(FXObject *object,FXSelector sel,void *ptr)
{
	if(parent->target)
		parent->target->handle(parent,MKUINT(parent->message,FXRezWaveView::SEL_REMOVE_CUE),&cueClicked);
	update();
	return 1;
}

long FXWaveRuler::onEditCue(FXObject *object,FXSelector sel,void *ptr)
{
	if(parent->target)
		parent->target->handle(parent,MKUINT(parent->message,FXRezWaveView::SEL_EDIT_CUE),&cueClicked);
	update();
	return 1;
}

long FXWaveRuler::onShowCueList(FXObject *object,FXSelector sel,void *ptr)
{
	if(parent->target)
		parent->target->handle(parent,MKUINT(parent->message,FXRezWaveView::SEL_SHOW_CUE_LIST),NULL);
	update();
	return 1;
}

#warning make it so that if you press esc while dragging a cue that it moves back to the original location
long FXWaveRuler::onLeftBtnPress(FXObject *object,FXSelector sel,void *ptr)
{
	FXEvent* event=(FXEvent*)ptr;
	if(event->click_count==1)
	{
		cueClicked=getClickedCue(event->win_x,event->win_y); // setting data member
		if(cueClicked<sound->getCueCount())
		{
			origCueClickedTime=sound->getCueTime(cueClicked);

			dragCueHint->setText(sound->getTimePosition(origCueClickedTime,gSoundFileManager->getSoundWindow(loadedSound)->getZoomDecimalPlaces()).c_str());
			dragCueHint->show();
			dragCueHint->autoplace();
			
			// set focused cue
			focusedCueIndex=cueClicked;
			focusedCueTime=sound->getCueTime(focusedCueIndex);
			update();
		}
	}
	return 0;
}

long FXWaveRuler::onMouseMove(FXObject *object,FXSelector sel,void *ptr)
{
	FXEvent* event=(FXEvent*)ptr;
	if(event->state&LEFTBUTTONMASK && cueClicked<sound->getCueCount())
	{	// dragging a cue around


		const string cueName=sound->getCueName(cueClicked);
		const FXint cueTextWidth=font->getTextWidth(cueName.data(),cueName.length());

		// erase cue at old position
		update((event->last_x-cueClickedOffset)-CUE_RADIUS-1,0,CUE_RADIUS*2+1+cueTextWidth+2,getHeight());	

		// erase vertical cue line at old position
		if(gDrawVerticalCuePositions) 
			parent->waveScrollArea->redraw(event->last_x-cueClickedOffset,1); 

		if(object==NULL) // simulated event from auto-scrolling
		{
			// get where the dragCueHint appears on the wave view canvas
			FXint hint_win_x,hint_win_y;
			dragCueHint->getParent()->translateCoordinatesTo(hint_win_x,hint_win_y,this,dragCueHint->getX(),dragCueHint->getY());
	
			// redraw what was under the dragCueHint (necessary when auto-scrolling)
			parent->waveScrollArea->redraw(hint_win_x+(event->last_x-event->win_x),dragCueHint->getWidth());
		}



		sample_pos_t newTime=parent->waveScrollArea->getCueTimeFromX(event->win_x-cueClickedOffset);
		
		// update cue position
		sound->setCueTime(cueClicked,newTime);
		if(cueClicked==focusedCueIndex)
			focusedCueTime=newTime;

		// draw cue at new position
		update((event->win_x-cueClickedOffset)-CUE_RADIUS-1,0,CUE_RADIUS*2+1+cueTextWidth+2,getHeight());

		// draw vertical cue line at new position
		if(gDrawVerticalCuePositions) 
			parent->waveScrollArea->redraw(event->win_x-cueClickedOffset,1); 

		dragCueHint->setText(sound->getTimePosition(newTime,gSoundFileManager->getSoundWindow(loadedSound)->getZoomDecimalPlaces()).c_str());
		dragCueHint->autoplace();

		// have to call canvas->repaint() on real mouse moves because if while autoscrolling a real (that is, object!=NULL) mouse move event occurs, then the window may or may not have been blitted leftward or rightward yet and we will erase the vertical cue position at the wrong position (or something like that, I really had a hard time figuring out the problem that shows up if you omit this call to repaint() )
		if(object && gDrawVerticalCuePositions)
			parent->waveScrollArea->canvas->repaint();

		if(event->win_x<0 || event->win_x>=width)
		{ // scroll parent window leftwards or rightwards if mouse is beyond the window edges
#if REZ_FOX_VERSION>=10125
			parent->waveScrollArea->startAutoScroll(event);
#else
			parent->waveScrollArea->startAutoScroll(event->win_x,event->win_y);
#endif
		}

	}
	return 0;
}

long FXWaveRuler::onLeftBtnRelease(FXObject *object,FXSelector sel,void *ptr)
{
	FXComposite::handle(object,sel,ptr); // if FXComposite ever starts sending SEL_DOUBLECLICKED events, then the event will happen twice because I'm doing what's below.. detect the version and remove this handler

	FXEvent* event=(FXEvent*)ptr;
	if(event->click_count==1) //	<-- single click
	{

		if(cueClicked<sound->getCueCount() && sound->getCueTime(cueClicked)!=origCueClickedTime)
		{	// was dragging a cue around
			const sample_pos_t newCueTime=sound->getCueTime(cueClicked);

			// set back to the orig position
			sound->setCueTime(cueClicked,origCueClickedTime);

			// set cue to new position except use an AAction object so it goes on the undo stack
			CActionParameters actionParameters(NULL);
			actionParameters.addUnsignedParameter("index",cueClicked);
			actionParameters.addSamplePosParameter("position",newCueTime);
			moveCueActionFactory->performAction(loadedSound,&actionParameters,false);
		}

		dragCueHint->hide();
		parent->waveScrollArea->stopAutoScroll();
	}
	else if(event->click_count==2) //	<-- double click
	{
		cueClicked=getClickedCue(event->win_x,event->win_y); // setting data member
		if(cueClicked<sound->getCueCount())
			return onEditCue(object,sel,(void *)cueClicked);
		else
			return onShowCueList(object,sel,ptr);
	}
	return 0;
}

void FXWaveRuler::focusNextCue()
{
	if(focusedCueIndex==0xffffffff)
	{
		size_t dummy;
		if(!sound->findNearestCue(0,focusedCueIndex,dummy))
			focusedCueIndex=0xffffffff;
		else
			focusedCueTime=sound->getCueTime(focusedCueIndex);
	}
	else
	{
		if(sound->findNextCue(focusedCueTime,focusedCueIndex))
			focusedCueTime=sound->getCueTime(focusedCueIndex);
	}
}

void FXWaveRuler::focusPrevCue()
{
	if(focusedCueIndex==0xffffffff)
		focusNextCue(); // would implement the same thing here, so just call it
	else
	{
		size_t dummy;
		if(sound->findPrevCue(focusedCueTime,focusedCueIndex))
			focusedCueTime=sound->getCueTime(focusedCueIndex);
	}
}

