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

#ifndef __AFrontendHooks_H__
#define __AFrontendHooks_H__

#include "../../config/common.h"

class AFrontendHooks;

#include <string>

#include "CSound_defs.h"

class ASoundRecorder;

extern AFrontendHooks *gFrontendHooks;

class AFrontendHooks
{
public:

	AFrontendHooks() { }
	virtual ~AFrontendHooks() { }

	// prompt with an open file dialog (return false if the prompt was cancelled)
	virtual bool promptForOpenSoundFilename(string &filename,bool &readOnly)=0;

	// prompt with a save file dialog (return false if the prompt was cancelled)
	virtual bool promptForSaveSoundFilename(string &filename)=0;

	// prompt for a new sound to be created asking for the given parameters (return false if the prompt was cancelled)
	virtual bool promptForNewSoundParameters(string &filename,unsigned &channelCount,unsigned &sampleRate,sample_pos_t &length)=0;
	virtual bool promptForNewSoundParameters(string &filename,unsigned &channelCount,unsigned &sampleRate)=0;
	virtual bool promptForNewSoundParameters(unsigned &channelCount,unsigned &sampleRate)=0;

	// should prompt for the user to choose a directory
	virtual bool promptForDirectory(string &dirname,const string title)=0;

	// prompt for recording, this function will have to be more than just an interface and do work 
	// since it should probably show level meters and be able to insert cues while recording etc.
	virtual bool promptForRecord(ASoundRecorder *recorder)=0;


	// called when the user is saving an ogg file and compression parameters are needed
	struct OggCompressionParameters
	{
		enum 
		{
			brVBR, 
			brQuality 
		} method;

		// method==brVBR
		int minBitRate;
		int normBitRate;
		int maxBitRate;

		// method==brQuality
		float quality;
	};
	virtual bool promptForOggCompressionParameters(OggCompressionParameters &parameters)=0;

	// called when the user is saving an mp3 file and compression parameters are needed
	struct Mp3CompressionParameters
	{
		enum 
		{
			brCBR, 
			brABR, 
			brQuality 
		} method;

		// method==brCBR
		int constantBitRate;

		// method==brABR
		int minBitRate;
		int normBitRate;
		int maxBitRate;

		// method==brQuality
		int quality; // 0(highest quality) -> 9(lowest quality)

		bool useFlagsOnly;
		string additionalFlags;
	};
	virtual bool promptForMp3CompressionParameters(Mp3CompressionParameters &parameters)=0;

};

#endif