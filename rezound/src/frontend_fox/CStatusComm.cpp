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

#include "CStatusComm.h"

#include <stdio.h>

#include <stdexcept>

#include <fox/fx.h>

#include "CMainWindow.h"
#include "CProgressDialog.h"

CStatusComm::CStatusComm()
{
	for(size_t t=0;t<MAX_PROGRESS_DIALOGS;t++)
		progressDialogs[t]=NULL;
}

CStatusComm::~CStatusComm()
{
}

void CStatusComm::error(const string &message,VSeverity severity)
{
	switch(severity)
	{
	case none:
		FXMessageBox::error(gMainWindow,MBOX_OK,"Error",message.c_str());
		break;
	case light:
		FXMessageBox::error(gMainWindow,MBOX_OK,"Light Error",message.c_str());
		break;
	case medium:
		FXMessageBox::error(gMainWindow,MBOX_OK,"Medium Error",message.c_str());
		break;
	case hard:
		FXMessageBox::error(gMainWindow,MBOX_OK,"Hard Error",message.c_str());
		break;
	case fatal:
		FXMessageBox::error(gMainWindow,MBOX_OK,"Fatal Error!",message.c_str());
		break;
	default:
		FXMessageBox::error(gMainWindow,MBOX_OK,"Error -- unknown severity",message.c_str());
		break;
	}
}

void CStatusComm::warning(const string &message)
{
	FXMessageBox::warning(gMainWindow,MBOX_OK,"Warning",message.c_str());
}

void CStatusComm::message(const string &message)
{
	FXMessageBox::information(gMainWindow,MBOX_OK,"Note",message.c_str());
}

VAnswer CStatusComm::question(const string &message,VQuestion options)
{
	FXint flags=0;

	if((options&yesnoQues)==yesnoQues)
		flags|=MBOX_YES_NO;
	if((options&cancelQues)==cancelQues)
		flags|=MBOX_YES_NO_CANCEL;

	if(flags==0)
		flags=MBOX_OK;

	switch(FXMessageBox::question(gMainWindow,flags,"Question",message.c_str()))
	{
	case MBOX_CLICKED_YES:
		return(yesAns);
	case MBOX_CLICKED_NO:
		return(noAns);
	case MBOX_CLICKED_CANCEL:
		return(cancelAns);
	default:
		return(defaultAns);
	}
}

void CStatusComm::beep()
{
	gMainWindow->getApp()->beep();
}

int CStatusComm::beginProgressBar(const string &title)
{
	//printf("begin progress bar: %s\n",title.c_str());

	// find a place in progressDialogs to create a new one
	for(int handle=0;handle<MAX_PROGRESS_DIALOGS;handle++)
	{
		if(progressDialogs[handle]==NULL)
		{
			try
			{
				progressDialogs[handle]=new CProgressDialog(gMainWindow,title.c_str());
				progressDialogs[handle]->create();
				progressDialogs[handle]->show();

				/* ??? how can I get the stuff that needs to be repainted done so?
				progressDialogs[handle]->update();
				progressDialogs[handle]->repaint();
				progressDialogs[handle]->getApp()->flush();
				*/

				return(handle);
			}
			catch(exception &e)
			{
				progressDialogs[handle]=NULL;
				Error(e.what());
				return(-1);
			}
		}
	}

	return(-1);
}

void CStatusComm::updateProgressBar(int handle,int progress)
{
	////printf("update progress bar: %d\n",progress);

	if(handle>=0 && handle<MAX_PROGRESS_DIALOGS && progressDialogs[handle]!=NULL)
	{
		try
		{
			progressDialogs[handle]->setProgress(progress);
		}
		catch(exception &e)
		{ // oh well
			fprintf(stderr,"exception caught in %s -- %s\n",__func__,e.what());
		}
	}
}

void CStatusComm::endProgressBar(int handle)
{
	//printf("end progress bar\n");

	if(handle>=0 && handle<MAX_PROGRESS_DIALOGS && progressDialogs[handle]!=NULL)
	{
		try
		{
			progressDialogs[handle]->hide();
			delete progressDialogs[handle];
			progressDialogs[handle]=NULL;
		}
		catch(exception &e)
		{
			progressDialogs[handle]=NULL;
			Error(e.what());
		}
	}
}

void CStatusComm::endAllProgressBars()
{
	//printf("end all progress bars\n");

	for(int t=0;t<MAX_PROGRESS_DIALOGS;t++)
		endProgressBar(t);
}


