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

#include "CMainWindow.h"
#include "CActionButton.h"
#include "CActionMenuCommand.h"

#include <stdexcept>
#include <string>

#include "CSoundFileManager.h"

#include "settings.h"

#include "../images/images.h"

#include "CAboutDialog.h"

#include "../backend/main_controls.h"

#include "../backend/Effects/EffectActions.h"
#include "../backend/Looping/LoopingActions.h"
#include "../backend/Remaster/RemasterActions.h"

#include "../backend/CLoadedSound.h"
#include "../backend/AAction.h"
#include "../backend/ASoundClipboard.h"

#include "../backend/CSoundPlayerChannel.h"

#include "CEditToolbar.h"

#include "CSoundWindow.h"

#include "CUserNotesDialog.h"
#include "CCrossfadeEdgesDialog.h"

#include "rememberShow.h"


/* TODO:
 * 	- make a button that brings back the editing toolbar incase they closed it
 *
 * 	- put gap after repeating and other toggles
 * 
 *	- remove all the data members for controls that don't need to have their value saved for any reason
 *
 */

FXDEFMAP(CMainWindow) CMainWindowMap[]=
{
	//Message_Type				ID						Message_Handler

	FXMAPFUNC(SEL_CLOSE,			0,						CMainWindow::onQuit),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_QUIT_MENUITEM,		CMainWindow::onQuit),
	
		// file actions
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_NEW_MENUITEM,		CMainWindow::onFileAction),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_OPEN_MENUITEM,		CMainWindow::onFileAction),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_REOPEN_MENUITEM,		CMainWindow::onFileAction),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_SAVE_MENUITEM,		CMainWindow::onFileAction),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_SAVE_AS_MENUITEM,		CMainWindow::onFileAction),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_CLOSE_MENUITEM,		CMainWindow::onFileAction),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_REVERT_MENUITEM,		CMainWindow::onFileAction),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FILE_RECORD_MENUITEM,		CMainWindow::onFileAction),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_ABOUT_MENUITEM,			CMainWindow::onFileAction),


		// play controls
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_PLAY_ALL_ONCE_BUTTON,		CMainWindow::onPlayControlButton),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_PLAY_ALL_LOOPED_BUTTON,		CMainWindow::onPlayControlButton),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_PLAY_SELECTION_ONCE_BUTTON,	CMainWindow::onPlayControlButton),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_PLAY_SELECTION_LOOPED_BUTTON,	CMainWindow::onPlayControlButton),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_STOP_BUTTON,			CMainWindow::onPlayControlButton),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_PAUSE_BUTTON,			CMainWindow::onPlayControlButton),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_JUMP_TO_BEGINNING_BUTTON,	CMainWindow::onPlayControlButton),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_JUMP_TO_START_POSITION_BUTTON,	CMainWindow::onPlayControlButton),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_JUMP_TO_NEXT_CUE_BUTTON,	CMainWindow::onPlayControlButton),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_JUMP_TO_PREV_CUE_BUTTON,	CMainWindow::onPlayControlButton),

	FXMAPFUNC(SEL_LEFTBUTTONRELEASE,	CMainWindow::ID_SHUTTLE_DIAL,			CMainWindow::onShuttleReturn),
	FXMAPFUNC(SEL_CHANGED,			CMainWindow::ID_SHUTTLE_DIAL,			CMainWindow::onShuttleChange),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_SEEK_NORMAL,			CMainWindow::onShuttleReturn),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_SEEK_LEFT,			CMainWindow::onKeyboardSeek),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_SEEK_MODIFY,			CMainWindow::onKeyboardSeek),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_SEEK_RIGHT,			CMainWindow::onKeyboardSeek),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_CENTER_START_POS,		CMainWindow::onViewKey),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_CENTER_STOP_POS,		CMainWindow::onViewKey),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_REDRAW_BUTTON,			CMainWindow::onRedrawButton),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_NOTES_MENUITEM,			CMainWindow::onUserNotesButton),
	
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_DEFRAG_MENUITEM,		CMainWindow::onDefragButton),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_PRINT_SAT_MENUITEM,		CMainWindow::onPrintSATButton),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_FOLLOW_PLAY_POSITION_BUTTON,	CMainWindow::onFollowPlayPositionButton),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_CROSSFADE_EDGES_COMBOBOX,	CMainWindow::onCrossfadeEdgesComboBox),
	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_CROSSFADE_EDGES_SETTINGS,	CMainWindow::onCrossfadeEdgesSettings),

	FXMAPFUNC(SEL_COMMAND,			CMainWindow::ID_CLIPBOARD_COMBOBOX,		CMainWindow::onClipboardComboBox),
};

FXIMPLEMENT(CMainWindow,FXMainWindow,CMainWindowMap,ARRAYNUMBER(CMainWindowMap))

#include <fox/fxkeys.h>

CMainWindow::CMainWindow(FXApp* a) :
	FXMainWindow(a,"ReZound",NULL,NULL,DECOR_ALL,0,0,800,0)
{
	menubar=new FXMenubar(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

	contents=new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 1,1,1,1, 1,1);

	#define BUTTON_STYLE FRAME_RAISED|LAYOUT_EXPLICIT

	FXPacker *t;

	playControlsFrame=new FXPacker(new FXPacker(contents,FRAME_RIDGE|LAYOUT_FILL_Y,0,0,0,0, 6,6,6,6),LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
		#define PLAY_CONTROLS_BUTTON_STYLE BUTTON_STYLE
		new FXButton(playControlsFrame,"\tPlay All Once",new FXGIFIcon(getApp(),play_all_once_gif),this,ID_PLAY_ALL_ONCE_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 0,0,32,32);
		new FXButton(playControlsFrame,"\tPlay Selection Once",new FXGIFIcon(getApp(),play_selection_once_gif),this,ID_PLAY_SELECTION_ONCE_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 32,0,32,32);
		new FXButton(playControlsFrame,"\tPlay All Looped",new FXGIFIcon(getApp(),play_all_looped_gif),this,ID_PLAY_ALL_LOOPED_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 0,32,32,32);
		new FXButton(playControlsFrame,"\tPlay Selection Looped",new FXGIFIcon(getApp(),play_selection_looped_gif),this,ID_PLAY_SELECTION_LOOPED_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 32,32,32,32);

		new FXButton(playControlsFrame,"\tStop",new FXGIFIcon(getApp(),stop_gif),this,ID_STOP_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 32+32,0,32,32),
		new FXButton(playControlsFrame,"\tPause",new FXGIFIcon(getApp(),pause_gif),this,ID_PAUSE_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 32+32,32,32,32),

		new FXButton(playControlsFrame,"||<<\tJump to Beginning",NULL,this,ID_JUMP_TO_BEGINNING_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 0,32+32,32+32,16);
		new FXButton(playControlsFrame,"|<<\tJump to Start Position",NULL,this,ID_JUMP_TO_START_POSITION_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 32+32,32+32,32+32,16);

		new FXButton(playControlsFrame,"Q<<\tJump to Previous Cue",NULL,this,ID_JUMP_TO_PREV_CUE_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 0,32+32+16,32+32,16);
		new FXButton(playControlsFrame,">>Q\tJump to Next Cue",NULL,this,ID_JUMP_TO_NEXT_CUE_BUTTON,PLAY_CONTROLS_BUTTON_STYLE, 32+32,32+32+16,32+32,16);

		shuttleDial=new FXDial(playControlsFrame,this,ID_SHUTTLE_DIAL,DIAL_HORIZONTAL|DIAL_HAS_NOTCH|LAYOUT_EXPLICIT, 0,32+32+16+16,32+32+32+32,20);
		shuttleDial->setRange(-((shuttleDial->getWidth())/2),(shuttleDial->getWidth())/2);
		shuttleDial->setRevolutionIncrement(shuttleDial->getWidth()*2-1);
		shuttleDial->setTipText("Shuttle Seek While Playing\n(Hint: try the mouse wheel as well as dragging)");

	miscControlsFrame=new FXPacker(new FXPacker(contents,FRAME_RIDGE|LAYOUT_FILL_Y,0,0,0,0, 6,6,6,6),LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 3,2);
		t=new FXHorizontalFrame(miscControlsFrame,0, 0,0,0,0, 0,0,0,0);
				// ??? this and redo should probably be the top two buttons on the edit panel
		new FXButton(miscControlsFrame,"&Redraw",NULL,this,ID_REDRAW_BUTTON,FRAME_RAISED);
		followPlayPositionButton=new FXCheckButton(miscControlsFrame,"Follow Play Position",this,ID_FOLLOW_PLAY_POSITION_BUTTON);
		t=new FXHorizontalFrame(miscControlsFrame,0, 0,0,0,0, 0,0,0,0);
			//new FXLabel(t,"Crossfade Edges: ");
			crossfadeEdgesComboBox=new FXComboBox(t,8,3, this,ID_CROSSFADE_EDGES_COMBOBOX, FRAME_SUNKEN|FRAME_THICK | COMBOBOX_NORMAL|COMBOBOX_STATIC | LAYOUT_CENTER_Y);
				crossfadeEdgesComboBox->setTipText("After Most Actions a Crossfade can be Performed at the Start and Stop \nPositions to Give a Smoother Transition in to and out of the Modified Selection");
				crossfadeEdgesComboBox->appendItem("No Crossfade");
				crossfadeEdgesComboBox->appendItem("Crossfade Inner Edges");
				crossfadeEdgesComboBox->appendItem("Crossfade Outer Edges");
				crossfadeEdgesComboBox->setCurrentItem(0);
			new FXButton(t,"...\tChange Crossfade Times",NULL,this,ID_CROSSFADE_EDGES_SETTINGS, BUTTON_NORMAL & ~FRAME_THICK);
		clipboardComboBox=new FXComboBox(miscControlsFrame,8,8, this,ID_CLIPBOARD_COMBOBOX, FRAME_SUNKEN|FRAME_THICK | COMBOBOX_NORMAL|COMBOBOX_STATIC);

}

void CMainWindow::show()
{
	rememberShow(this);
	FXMainWindow::show();

	followPlayPositionButton->setCheck(gFollowPlayPosition);

	if(gCrossfadeEdges>=cetNone && gCrossfadeEdges<=cetOuter)
		crossfadeEdgesComboBox->setCurrentItem((FXint)gCrossfadeEdges);
	else
		crossfadeEdgesComboBox->setCurrentItem(0);


	// populate combo box to select clipboard
	for(size_t t=0;t<AAction::clipboards.size();t++)
		clipboardComboBox->appendItem(AAction::clipboards[t]->getDescription().c_str());

	if(gWhichClipboard>=AAction::clipboards.size())
		gWhichClipboard=1;

	clipboardComboBox->setCurrentItem(gWhichClipboard);

}

void CMainWindow::hide()
{
	rememberHide(this);
	FXMainWindow::hide();
}

static void setupFXWindow(FXWindow *w)
{
	w->create();
	w->recalc();
}

/*
   This is the class for the reopen submenu.  It intercepts calls to FXMenuPane::popup ()
   so it can create the menu items which can change between each popup.
*/
class CReopenPopup : public FXMenuPane
{
public:
	CReopenPopup(FXWindow *owner) :
		FXMenuPane(owner)
	{
	}

	virtual ~CReopenPopup()
	{
	}

	virtual void popup(FXWindow* grabto, FXint x, FXint y, FXint w=0, FXint h=0)
	{
		// clear from previous popup 
		// I can't do this on popdown because the event won't have happened yet needing the menu item's text for the filename)
		for(size_t t=0;t<items.size();t++)
			delete items[t];
		items.clear();

		// create menu items
		size_t reopenSize=gSoundFileManager->getReopenHistorySize();
		if(reopenSize<=0)
			return;
		for(size_t t=0;t<reopenSize;t++)
		{
			FXMenuCommand *item=new FXMenuCommand(this,gSoundFileManager->getReopenHistoryItem(t).c_str(),NULL,getOwner(),CMainWindow::ID_FILE_REOPEN_MENUITEM);
			item->create();
			items.push_back(item);
		}

		FXMenuPane::popup(grabto,x,y,w,h);
	}

	vector<FXMenuCommand *> items;
};

// ??? perhaps this could be a function placed else where that wouldn't require a recompile of CMainWindow everytime we change a dialog
#include "CChannelSelectDialog.h"
#include "EffectActionDialogs.h"
#include "RemasterActionDialogs.h"
void CMainWindow::createToolbars()
{
	// build the drop-down menus
	FXMenuPane *fileMenu=new FXMenuPane(this);
	setupFXWindow(fileMenu);
	setupFXWindow(new FXMenuTitle(menubar,"&File",NULL,fileMenu));
		setupFXWindow(new FXMenuCommand(fileMenu,"&New",NULL,this,ID_FILE_NEW_MENUITEM));
		setupFXWindow(new FXMenuCommand(fileMenu,"&Open",NULL,this,ID_FILE_OPEN_MENUITEM));
			setupFXWindow(reopenSubmenu=new CReopenPopup(this));
		setupFXWindow(new FXMenuCascade(fileMenu,"&Reopen",NULL,reopenSubmenu));
		setupFXWindow(new FXMenuCommand(fileMenu,"&Save",NULL,this,ID_FILE_SAVE_MENUITEM));
		setupFXWindow(new FXMenuCommand(fileMenu,"Save &As",NULL,this,ID_FILE_SAVE_AS_MENUITEM));
		setupFXWindow(new FXMenuCommand(fileMenu,"&Close",NULL,this,ID_FILE_CLOSE_MENUITEM));
		setupFXWindow(new FXMenuCommand(fileMenu,"Re&vert",NULL,this,ID_FILE_REVERT_MENUITEM));

		setupFXWindow(new FXMenuSeparator(fileMenu));
		setupFXWindow(new FXMenuCommand(fileMenu,"Record",NULL,this,ID_FILE_RECORD_MENUITEM));

		setupFXWindow(new FXMenuSeparator(fileMenu));
		setupFXWindow(new FXMenuCommand(fileMenu,"User No&tes"/*\tUser notes about the sound (and preserved in the file if the format supports it)"*/,NULL,this,ID_NOTES_MENUITEM));

		setupFXWindow(new FXMenuSeparator(fileMenu));
		setupFXWindow(new FXMenuCommand(fileMenu,"&About ReZound",NULL,this,ID_ABOUT_MENUITEM));

		// just for testing ???
		setupFXWindow(new FXMenuSeparator(fileMenu));
		setupFXWindow(new FXMenuCaption(fileMenu,"- Just for testing"));
		setupFXWindow(new FXMenuCommand(fileMenu,"Defrag",NULL,this,ID_DEFRAG_MENUITEM));
		setupFXWindow(new FXMenuCommand(fileMenu,"PrintSAT",NULL,this,ID_PRINT_SAT_MENUITEM));

		setupFXWindow(new FXMenuSeparator(fileMenu));
		setupFXWindow(new FXMenuCommand(fileMenu,"&Quit\tCtrl-Q",NULL,this,ID_FILE_QUIT_MENUITEM));

			// ??? I might be able to get the name of the effect from the action rather than having to type it here

	FXMenuPane *effectsMenu=new FXMenuPane(this);
	setupFXWindow(effectsMenu);
	setupFXWindow(new FXMenuTitle(menubar,"&Effects",NULL,effectsMenu));
		setupFXWindow(new CActionMenuCommand(new CReverseEffectFactory(gChannelSelectDialog),effectsMenu,"Re&verse"));
		setupFXWindow(new CActionMenuCommand(new CChangeAmplitudeEffectFactory(gChannelSelectDialog,new CNormalAmplitudeChangeDialog(this),new CAdvancedAmplitudeChangeDialog(this)),effectsMenu,"Change &Amplitude"));
		setupFXWindow(new CActionMenuCommand(new CChangeRateEffectFactory(gChannelSelectDialog,new CNormalRateChangeDialog(this),new CAdvancedRateChangeDialog(this)),effectsMenu,"Change &Rate"));
		setupFXWindow(new CActionMenuCommand(new CFlangeEffectFactory(gChannelSelectDialog,new CFlangeDialog(this)),effectsMenu,"&Flange"));
		setupFXWindow(new CActionMenuCommand(new CSimpleDelayEffectFactory(gChannelSelectDialog,new CSimpleDelayDialog(this)),effectsMenu,"&Delay (Echo)"));
		setupFXWindow(new CActionMenuCommand(new CStaticReverbEffectFactory(gChannelSelectDialog),effectsMenu,"Reverb"));
		setupFXWindow(new CActionMenuCommand(new CVariedRepeatEffectFactory(gChannelSelectDialog,new CVariedRepeatDialog(this)),effectsMenu,"Variable Repeat"));

		setupFXWindow(new CActionMenuCommand(new CTestEffectFactory(gChannelSelectDialog),effectsMenu,"test"));

	FXMenuPane *loopingMenu=new FXMenuPane(this);
	setupFXWindow(loopingMenu);
	setupFXWindow(new FXMenuTitle(menubar,"&Looping",NULL,loopingMenu));
		setupFXWindow(new CActionMenuCommand(new CMakeSymetricActionFactory(gChannelSelectDialog),loopingMenu,"Make &Symetric"));

	FXMenuPane *remasterMenu=new FXMenuPane(this);
	setupFXWindow(remasterMenu);
	setupFXWindow(new FXMenuTitle(menubar,"&Remaster",NULL,remasterMenu));
		setupFXWindow(new CActionMenuCommand(new CUnclipActionFactory(gChannelSelectDialog),remasterMenu,"&Unclip"));
		setupFXWindow(new CActionMenuCommand(new CRemoveDCActionFactory(gChannelSelectDialog),remasterMenu,"Remove &DC"));
		setupFXWindow(new CActionMenuCommand(new CNoiseGateActionFactory(gChannelSelectDialog,new CNoiseGateDialog(this)),remasterMenu,"&Noise Gate"));
		setupFXWindow(new CActionMenuCommand(new CCompressorActionFactory(gChannelSelectDialog,new CCompressorDialog(this)),remasterMenu,"Dynamic Range &Compressor"));

}


long CMainWindow::onQuit(FXObject *sender,FXSelector sel,void *ptr)
{
	if(getApp()->modalModality()==MODAL_FOR_WINDOW)
	{ // don't allow a quit if there is a modal window showing
		gStatusComm->beep();
		return(1);
	}

	if(exitReZound(gSoundFileManager))
	{
		gEditToolbar->hide();
		hide();
		getApp()->exit(0);
	}
	return(1);
}

long CMainWindow::onFollowPlayPositionButton(FXObject *sender,FXSelector sel,void *ptr)
{
	gFollowPlayPosition=followPlayPositionButton->getCheck();
	return 1;
}

long CMainWindow::onCrossfadeEdgesComboBox(FXObject *sender,FXSelector sel,void *ptr)
{
	gCrossfadeEdges=(CrossfadeEdgesTypes)crossfadeEdgesComboBox->getCurrentItem();
	return 1;
}

long CMainWindow::onCrossfadeEdgesSettings(FXObject *sender,FXSelector sel,void *ptr)
{
	gCrossfadeEdgesDialog->showIt();
	return 1;
}


long CMainWindow::onClipboardComboBox(FXObject *sender,FXSelector sel,void *ptr)
{
	gWhichClipboard=clipboardComboBox->getCurrentItem();
	return 1;
}


// file action events
long CMainWindow::onFileAction(FXObject *sender,FXSelector sel,void *ptr)
{
	switch(SELID(sel))
	{
	case ID_FILE_NEW_MENUITEM:
		newSound(gSoundFileManager);
		break;
	
	case ID_FILE_OPEN_MENUITEM:
		openSound(gSoundFileManager);
		break;

	case ID_FILE_REOPEN_MENUITEM:
		openSound(gSoundFileManager,dynamic_cast<FXMenuCommand *>(sender)->getText().text());
		break;
	
	case ID_FILE_SAVE_MENUITEM:
		saveSound(gSoundFileManager);
		break;

	case ID_FILE_SAVE_AS_MENUITEM:
		saveAsSound(gSoundFileManager);
		break;

	case ID_FILE_CLOSE_MENUITEM:
		closeSound(gSoundFileManager);
		break;

	case ID_FILE_REVERT_MENUITEM:
		revertSound(gSoundFileManager);
		break;

	case ID_FILE_RECORD_MENUITEM:
		recordSound(gSoundFileManager);
		break;

	case ID_ABOUT_MENUITEM:
		gAboutDialog->execute(PLACEMENT_SCREEN);
		break;

	default:
		throw(runtime_error(string(__func__)+" -- unhandled file button selector"));
	}
	return 1;
}

// play control events
long CMainWindow::onPlayControlButton(FXObject *sender,FXSelector sel,void *ptr)
{
	switch(SELID(sel))
	{
	case ID_PLAY_ALL_ONCE_BUTTON:
		play(gSoundFileManager,false,false);
		break;

	case ID_PLAY_ALL_LOOPED_BUTTON:
		play(gSoundFileManager,true,false);
		break;

	case ID_PLAY_SELECTION_ONCE_BUTTON:
		play(gSoundFileManager,false,true);
		break;

	case ID_PLAY_SELECTION_LOOPED_BUTTON:
		play(gSoundFileManager,true,true);
		break;

	case ID_STOP_BUTTON:
		stop(gSoundFileManager);
		break;

	case ID_PAUSE_BUTTON:
		pause(gSoundFileManager);
		break;

	case ID_JUMP_TO_BEGINNING_BUTTON:
		jumpToBeginning(gSoundFileManager);
		break;

	case ID_JUMP_TO_START_POSITION_BUTTON:
		jumpToStartPosition(gSoundFileManager);
		break;

	case ID_JUMP_TO_PREV_CUE_BUTTON:
		jumpToPreviousCue(gSoundFileManager);
		break;

	case ID_JUMP_TO_NEXT_CUE_BUTTON:
		jumpToNextCue(gSoundFileManager);
		break;

	default:
		throw(runtime_error(string(__func__)+" -- unhandled play button selector"));
	}
	return 1;
}

long CMainWindow::onRedrawButton(FXObject *sender,FXSelector sel,void *ptr)
{
	CLoadedSound *s=gSoundFileManager->getActive();
	if(s!=NULL)
	{
		for(unsigned t=0;t<s->getSound()->getChannelCount();t++)
			s->getSound()->invalidateAllPeakData();
		gSoundFileManager->updateAfterEdit();
	}
	else
		getApp()->beep();
	
	return(1);
}

long CMainWindow::onUserNotesButton(FXObject *sender,FXSelector sel,void *ptr)
{
	try
	{
		CLoadedSound *s=gSoundFileManager->getActive();
		if(s!=NULL)
			gUserNotesDialog->show(s,PLACEMENT_CURSOR);
		else
			getApp()->beep();
	}
	catch(exception &e)
	{
		Error(e.what());
	}
	return(1);
}

long CMainWindow::onShuttleReturn(FXObject *sender,FXSelector sel,void *ptr)
{
	shuttleDial->setValue(0);
	onShuttleChange(sender,sel,ptr); // I think I need to call this ???
	return 1;
}

long CMainWindow::onShuttleChange(FXObject *sender,FXSelector sel,void *ptr)
{
	// ??? if the active window changes.... I need to set the channel's speak back to 1x
	CLoadedSound *s=gSoundFileManager->getActive();
	if(s!=NULL)
	{
		FXint minValue,maxValue;
		shuttleDial->getRange(minValue,maxValue);
		const FXint shuttlePos=shuttleDial->getValue();
		float playSpeed;

		if(shuttlePos==0)
			playSpeed=1.0;
		else if(shuttlePos>0)
			playSpeed=(pow((float)shuttlePos/(float)maxValue,2.0)*100.0)+1.0;
		else if(shuttlePos<0)
			playSpeed=(pow((float)shuttlePos/(float)minValue,2.0)*-100.0)-1.0;

		s->channel->setPlaySpeed(playSpeed);	
	}

	return 1;
}

long CMainWindow::onKeyboardSeek(FXObject *sender,FXSelector sel,void *ptr)
{
	FXint lo,hi;
	shuttleDial->getRange(lo,hi);

	FXint inc= (hi-lo)/14; // 7 positions surrounding 0 

	FXint pos=shuttleDial->getValue();

	if(pos==0 && SELID(sel)==ID_SEEK_LEFT)
	{
		shuttleDial->setValue(pos-inc);
		onShuttleChange(sender,sel,ptr);
	}
	else if(pos==0 && SELID(sel)==ID_SEEK_RIGHT)
	{
		shuttleDial->setValue(pos+inc);
		onShuttleChange(sender,sel,ptr);
	}
	else if(pos!=0 && SELID(sel)==ID_SEEK_MODIFY)
	{
		if(pos<0)
		{ // go more leftward
			shuttleDial->setValue(pos-inc);
		}
		else if(pos>0)
		{ // go more rightward
			shuttleDial->setValue(pos+inc);
		}
		onShuttleChange(sender,sel,ptr);
	}


	return 1;
}

long CMainWindow::onViewKey(FXObject *sender,FXSelector sel,void *ptr)
{
	switch(SELID(sel))
	{
	case ID_CENTER_START_POS:
		if(gSoundFileManager->getActiveWindow())
			gSoundFileManager->getActiveWindow()->centerStartPos();
		break;

	case ID_CENTER_STOP_POS:
		if(gSoundFileManager->getActiveWindow())
			gSoundFileManager->getActiveWindow()->centerStopPos();
		break;
	}


	return 1;
}

long CMainWindow::onDefragButton(FXObject *sender,FXSelector sel,void *ptr)
{
	CLoadedSound *s=gSoundFileManager->getActive();
	if(s!=NULL)
	{
		s->getSound()->defragPoolFile();
		gSoundFileManager->updateAfterEdit();
	}
	else
		getApp()->beep();
	
	return(1);
}

long CMainWindow::onPrintSATButton(FXObject *sender,FXSelector sel,void *ptr)
{
	CLoadedSound *s=gSoundFileManager->getActive();
	if(s!=NULL)
		s->getSound()->printSAT();
	else
		getApp()->beep();
	
	return(1);
}


