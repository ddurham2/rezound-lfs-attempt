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

#ifndef __CvoxSoundTranslator_H__
#define __CvoxSoundTranslator_H__

#include "../../config/common.h"

#include "ApipedSoundTranslator.h"

class CvoxSoundTranslator : public ApipedSoundTranslator
{
public:
	CvoxSoundTranslator();
	virtual ~CvoxSoundTranslator();

	static bool checkForApp();

	bool handlesExtension(const string extension,const string filename) const;
	bool supportsFormat(const string filename) const;

	const vector<string> getFormatNames() const;
	const vector<vector<string> > getFormatFileMasks() const;

protected:

	bool onLoadSound(const string filename,CSound *sound) const;
	bool onSaveSound(const string filename,const CSound *sound,const sample_pos_t saveStart,const sample_pos_t saveLength) const;

};

#endif
