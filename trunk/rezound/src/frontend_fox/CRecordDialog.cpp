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

#include "CRecordDialog.h"

#include "../backend/ASoundRecorder.h"

#include <istring>

CRecordDialog *gRecordDialog;

FXDEFMAP(CRecordDialog) CRecordDialogMap[]=
{
//	Message_Type			ID					Message_Handler
	//FXMAPFUNC(SEL_COMMAND,		CRecordDialog::ID_OKAY_BUTTON,	CRecordDialog::onOkayButton),
};
		

FXIMPLEMENT(CRecordDialog,FXModalDialogBox,CRecordDialogMap,ARRAYNUMBER(CRecordDialogMap))



// ----------------------------------------

CRecordDialog::CRecordDialog(FXWindow *mainWindow) :
	FXModalDialogBox(mainWindow,"Record",310,350,FXModalDialogBox::ftVertical)//,

{
	getFrame()->setHSpacing(1);
	getFrame()->setVSpacing(1);

	FXPacker *frame1;
	FXPacker *frame2;
	FXPacker *frame3;

	frame1=new FXHorizontalFrame(getFrame(),LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 1,1);
		frame2=new FXVerticalFrame(frame1,LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 1,1);
			frame3=new FXVerticalFrame(frame2,FRAME_RAISED | LAYOUT_FILL_X|LAYOUT_FILL_Y);
				new FXButton(frame3,"Add Cue",NULL,this,ID_ADD_CUE_BUTTON);
				new FXButton(frame3,"Add Anchored Cue",NULL,this,ID_ADD_ANCHORED_CUE_BUTTON);
					// but what name shall I give the cues
				//surroundWithCuesButton=new FXCheckButton(frame3,"Surround With Cues");
			frame3=new FXVerticalFrame(frame2,FRAME_RAISED | LAYOUT_FILL_X|LAYOUT_FILL_Y);
				lengthStatusLabel=new FXLabel(frame3,"Length: ",NULL);
				locationStatusLabel=new FXLabel(frame3,"Location: ",NULL);
				sizeStatusLabel=new FXLabel(frame3,"Size: ",NULL);
		frame2=new FXVerticalFrame(frame1,FRAME_RAISED | LAYOUT_FILL_X|LAYOUT_FILL_Y);
			frame3=new FXHorizontalFrame(frame2);
				new FXButton(frame3,"Reset",NULL,this,ID_CLEAR_CLIP_COUNT_BUTTON);
				clipCountLabel=new FXLabel(frame3,"Clip Count: 0",NULL);
			meterFrame=new FXHorizontalFrame(frame2,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	frame1=new FXVerticalFrame(getFrame(),FRAME_RAISED | LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 1,1);
		frame2=new FXHorizontalFrame(frame1,LAYOUT_CENTER_X);
			new FXButton(frame2,"Record",NULL,this,ID_START_BUTTON);
			new FXButton(frame2,"Stop",NULL,this,ID_STOP_BUTTON);
			new FXButton(frame2,"Redo",NULL,this,ID_REDO_BUTTON);
		frame2=new FXHorizontalFrame(frame1,LAYOUT_CENTER_X);
			setDurationButton=new FXCheckButton(frame2,"Limit Duration to ");
			durationEdit=new FXTextField(frame2,12);
			durationEdit->setText("MM:SS.sss");
				
		
}

void CRecordDialog::cleanupMeters()
{
	recorder->removePeakReadTrigger(onLevelMeter,this);
	while(!meters.empty())
	{
		delete meters[0];
		meters.erase(meters.begin());
	}
}

//??? include this to get colors.. but they should be in the settings registry actually
#include "drawPortion.h"
void CRecordDialog::setMeterValue(unsigned channel,float value)
{
	FXProgressBar *meter=meters[channel];
	if(value>=1.0)
	{
		meter->setBarColor(clippedWaveformColor);
		incClipCount();
	}
	else
		// need to make this more usefully be green for comfortable values, to yellow, to red at close to clipping
		meter->setBarColor(FXRGB((int)(value*255),(int)((1.0-value)*255),0));

	meter->setProgress((FXint)(value*1000));
}

void onLevelMeter(void *data)
{
	CRecordDialog *that=(CRecordDialog *)data;

	for(unsigned i=0;i<that->meters.size();i++)
		that->setMeterValue(i,that->recorder->getLastPeakValue(i));
	//that->getApp()->repaint();
}

bool CRecordDialog::show(ASoundRecorder *_recorder)
{
	recorder=_recorder;
	clearClipCount();

	for(unsigned i=0;i<recorder->getChannelCount();i++)
	{
		FXProgressBar *meter=new FXProgressBar(meterFrame,NULL,0,PROGRESSBAR_NORMAL|PROGRESSBAR_VERTICAL | LAYOUT_FILL_Y);
		meter->setBarBGColor(FXRGB(0,0,0));
		meter->setTotal(1000);
		meters.push_back(meter);
	}
	meterFrame->recalc();

	/* right now, fox barfs on the updates happening so fast, I will talk to Jereon about it later
	// may want to ignore this even if on a slow computer ??? let it be a setting
	recorder->setPeakReadTrigger(onLevelMeter,this);
	*/

	try
	{
		if(FXDialogBox::execute(PLACEMENT_CURSOR))
		{
			// if nothing was ever recorded, I should return false

			cleanupMeters();
			return(true);
		}
		cleanupMeters();
		return(false);
	}
	catch(...)
	{
		cleanupMeters();
		throw;
	}
}

long CRecordDialog::onStartButton(FXObject *sender,FXSelector sel,void *ptr)
{
	//reset the clip count here too ???
	return 1;
}

long CRecordDialog::onStopButton(FXObject *sender,FXSelector sel,void *ptr)
{
	return 1;
}

long CRecordDialog::onRedoButton(FXObject *sender,FXSelector sel,void *ptr)
{
	//reset the clip count here too ???
	return 1;
}

long CRecordDialog::onAddCueButton(FXObject *sender,FXSelector sel,void *ptr)
{
	return 1;
}

void CRecordDialog::incClipCount()
{
	clipCount++;
	clipCountLabel->setText(("Clip Count: "+istring(clipCount)).c_str());
}

void CRecordDialog::clearClipCount()
{
	clipCount=0;
	clipCountLabel->setText("Clip Count: 0");
}
