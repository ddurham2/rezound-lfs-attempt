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

#ifndef __CSoundListWindow_H__
#define __CSoundListWindow_H__

#include "../../config/common.h"
#include "fox_compat.h"

#include <fox/fx.h>

class CSoundListWindow;
extern CSoundListWindow *gSoundListWindow;

class CSoundWindow;

class CSoundListWindow : public FXTopWindow
{
	FXDECLARE(CSoundListWindow);
public:

	CSoundListWindow(FXWindow *mainWindow);
	virtual ~CSoundListWindow();

	enum 
	{
		ID_SOUND_LIST=FXTopWindow::ID_LAST,
		ID_LAST
	};

	virtual void show();
	virtual void hide();

	long onSoundListChange(FXObject *sender,FXSelector sel,void *ptr);

	long onCloseWindow(FXObject *sender,FXSelector sel,void *ptr);

	void addSoundWindow(CSoundWindow *window);
	void removeSoundWindow(CSoundWindow *window);
	void updateWindowName(CSoundWindow *window);

protected:
	CSoundListWindow() {}

private:

	FXPacker *contents;
		FXComposite *soundListFrame;
			FXList *soundList;

	void hideOrShow();
};

#endif