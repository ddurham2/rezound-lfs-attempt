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

#ifndef __CEditToolbar_H__
#define __CEditToolbar_H__

#include "../../config/common.h"


#include <fox/fx.h>

#include "CActionButton.h"

class CEditToolbar;
extern CEditToolbar *gEditToolbar;

class CEditToolbar : public FXTopWindow
{
	FXDECLARE(CEditToolbar);
public:

	CEditToolbar(FXWindow *mainWindow);
	~CEditToolbar();

	virtual void show();
	virtual void hide();

	FXButton *selectAllButton;

	FXButton *copyButton;
	FXButton *cutButton;
	FXButton *delButton;
	FXButton *cropButton;
	FXButton *pasteInsertButton;

	long onUndoButton(FXObject *sender,FXSelector sel,void *ptr);
	long onClearUndoHistoryButton(FXObject *sender,FXSelector sel,void *ptr);

	enum
	{
		ID_UNDO_BUTTON=FXTopWindow::ID_LAST,
		ID_CLEAR_UNDO_HISTORY_BUTTON,
		ID_LAST
	};

protected:
	CEditToolbar() {}

private:

	FXScrollWindow *scrollWindow;
	FXMatrix *contents;

};

#endif
