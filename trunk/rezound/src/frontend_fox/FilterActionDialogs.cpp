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

#include "FilterActionDialogs.h"
#include "../backend/unit_conv.h"




static const double interpretValue_filter(const double x,const int s) { return(x*s); }
static const double uninterpretValue_filter(const double x,const int s) { return(x/s); }


// --- single pole lowpass ---------------

CSinglePoleLowpassFilterDialog::CSinglePoleLowpassFilterDialog(FXWindow *mainWindow) :
	CActionParamDialog(mainWindow,"Single Pole Lowpass Filter",300,400)
{
	addSlider("Cutoff Frequency","Hz",interpretValue_filter,uninterpretValue_filter,NULL,500.0,5,100000,5000,false);
}

// --- single pole highpass --------------

CSinglePoleHighpassFilterDialog::CSinglePoleHighpassFilterDialog(FXWindow *mainWindow) :
	CActionParamDialog(mainWindow,"Single Pole Highpass Filter",300,400)
{
	addSlider("Cutoff Frequency","Hz",interpretValue_filter,uninterpretValue_filter,NULL,1000.0,5,100000,10000,false);
}

// --- bandpass --------------------------

CBandpassFilterDialog::CBandpassFilterDialog(FXWindow *mainWindow) :
	CActionParamDialog(mainWindow,"Bandpass Filter",350,400)
{
	addSlider("Cutoff Frequency","Hz",interpretValue_filter,uninterpretValue_filter,NULL,1000.0,5,100000,10000,false);
	addSlider("Band Width","Hz",interpretValue_filter,uninterpretValue_filter,NULL,500.0,5,100000,1000,false);
}

// --- notch -----------------------------

CNotchFilterDialog::CNotchFilterDialog(FXWindow *mainWindow) :
	CActionParamDialog(mainWindow,"Notch Filter",350,400)
{
	addSlider("Cutoff Frequency","Hz",interpretValue_filter,uninterpretValue_filter,NULL,1000.0,5,100000,10000,false);
	addSlider("Band Width","Hz",interpretValue_filter,uninterpretValue_filter,NULL,500.0,5,100000,1000,false);
}



