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

#ifndef __CActionMenuCommand_H__
#define __CActionMenuCommand_H__

#include "../../config/common.h"

#include <fox/fx.h>

/*
 * This class is a normal FXMenuCommand except that the window it's
 * on doesn't need to setup an event handler for it.  The event
 * is always performing the action (given by the first parameter
 * to the constructor) on the active sound window
 *
 * So it can be used to conventiently add many buttons to a toolbar
 * and only specify the AActionFactorys on construction, and that's 
 * it
 * 
 */

class AActionFactory;

class CActionMenuCommand : public FXMenuCommand
{
	FXDECLARE(CActionMenuCommand)
public:

	CActionMenuCommand(AActionFactory *_actionFactory,FXComposite* p, const FXString& text, FXIcon* ic=NULL, FXuint opts=0);

	long onMouseClick(FXObject *sender,FXSelector sel,void *ptr);
	long onKeyClick(FXObject *sender,FXSelector sel,void *ptr);

	long onQueryTip(FXObject* sender,FXSelector sel,void *ptr);

protected:
	CActionMenuCommand() {}

private:
	AActionFactory * const actionFactory;
	FXString tip;

};

#endif
