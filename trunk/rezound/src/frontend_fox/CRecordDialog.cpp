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

CRecordDialog *gRecordDialog;

FXDEFMAP(CRecordDialog) CRecordDialogMap[]=
{
//	Message_Type			ID					Message_Handler
	//FXMAPFUNC(SEL_COMMAND,		CRecordDialog::ID_OKAY_BUTTON,	CRecordDialog::onOkayButton),
};
		

FXIMPLEMENT(CRecordDialog,FXModalDialogBox,CRecordDialogMap,ARRAYNUMBER(CRecordDialogMap))



// ----------------------------------------

CRecordDialog::CRecordDialog(FXWindow *mainWindow) :
	FXModalDialogBox(mainWindow,"Record",310,290,FXModalDialogBox::ftVertical)//,

{
}

bool CRecordDialog::show(CActionSound *actionSound,CActionParameters *actionParameters)
{
	if(FXDialogBox::execute(PLACEMENT_CURSOR))
	{
		// if nothing was ever recorded, I should return false

		return(true);
	}
	return(false);

}

