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

#include "CSinglePoleFilter.h"

#include <stdexcept>

#include "../CActionSound.h"
#include "../CActionParameters.h"

#include "../DSP/SinglePoleFilters.h"
#include "../unit_conv.h"

CSinglePoleFilter::CSinglePoleFilter(const CActionSound &actionSound,FilterTypes _filterType,float _gain,float _frequency,float _bandwidth) :
	AAction(actionSound),
	filterType(_filterType),
	gain(_gain),
	frequency(_frequency),
	bandwidth(_bandwidth)
{
}

CSinglePoleFilter::~CSinglePoleFilter()
{
}

bool CSinglePoleFilter::doActionSizeSafe(CActionSound &actionSound,bool prepareForUndo)
{
	const sample_pos_t start=actionSound.start;
	const sample_pos_t stop=actionSound.stop;
	const sample_pos_t selectionLength=actionSound.selectionLength();

	if(prepareForUndo)
		moveSelectionToTempPools(actionSound,mmSelection,actionSound.selectionLength());

	for(unsigned i=0;i<actionSound.sound->getChannelCount();i++)
	{
		if(actionSound.doChannel[i])
		{
			CRezPoolAccesser dest=actionSound.sound->getAudio(i);
			const CRezPoolAccesser src=prepareForUndo ? actionSound.sound->getTempAudio(tempAudioPoolKey,i) : actionSound.sound->getAudio(i);
			sample_pos_t srcOffset=prepareForUndo ? start : 0;
			const register float gain=this->gain;

			#define STATUS_BAR_UPDATE(t) \
				if(statusBar.update(t)) \
				{ /* cancelled */ \
					if(prepareForUndo) \
						undoActionSizeSafe(actionSound); \
					else \
						actionSound.sound->invalidatePeakData(i,actionSound.start,t); \
					return false; \
				}

			switch(filterType)
			{
			case ftLowpass:
			{
				CStatusBar statusBar("Lowpass Filter -- Channel "+istring(i),start,stop,true); 

				TDSPSinglePoleLowpassFilter<mix_sample_t> filter(freq_to_fraction(frequency,actionSound.sound->getSampleRate()));
				for(sample_pos_t t=start;t<=stop;t++)
				{
					dest[t]=ClipSample(filter.processSample((mix_sample_t)(gain*src[t-srcOffset])));
					STATUS_BAR_UPDATE(t)
				}
			break;
			}

			case ftHighpass:
			{
				CStatusBar statusBar("Highpass Filter -- Channel "+istring(i),start,stop,true); 

				TDSPSinglePoleHighpassFilter<mix_sample_t> filter(freq_to_fraction(frequency,actionSound.sound->getSampleRate()));
				for(sample_pos_t t=start;t<=stop;t++)
				{
					dest[t]=ClipSample(filter.processSample((mix_sample_t)(gain*src[t-srcOffset])));
					STATUS_BAR_UPDATE(t)
				}
			break;
			}

			case ftBandpass:
			{
				CStatusBar statusBar("Bandpass Filter -- Channel "+istring(i),start,stop,true); 

				TDSPBandpassFilter<mix_sample_t> filter(freq_to_fraction(frequency,actionSound.sound->getSampleRate()),freq_to_fraction(bandwidth,actionSound.sound->getSampleRate()));
				for(sample_pos_t t=start;t<=stop;t++)
				{
					dest[t]=ClipSample(filter.processSample((mix_sample_t)(gain*src[t-srcOffset])));
					STATUS_BAR_UPDATE(t)
				}
			break;
			}

			case ftNotch:
			{
				CStatusBar statusBar("Notch Filter -- Channel "+istring(i),start,stop,true); 
				TDSPNotchFilter<mix_sample_t> filter(freq_to_fraction(frequency,actionSound.sound->getSampleRate()),freq_to_fraction(bandwidth,actionSound.sound->getSampleRate()));
				for(sample_pos_t t=start;t<=stop;t++)
				{
					dest[t]=ClipSample(filter.processSample((mix_sample_t)(gain*src[t-srcOffset])));
					STATUS_BAR_UPDATE(t)
				}
			break;
			}

			default:
				throw(runtime_error(string(__func__)+" -- invalid filterType: "+istring(filterType)));
			}


			if(!prepareForUndo)
				actionSound.sound->invalidatePeakData(i,actionSound.start,actionSound.stop);
		}
	}

	return(true);
}

AAction::CanUndoResults CSinglePoleFilter::canUndo(const CActionSound &actionSound) const
{
	return(curYes);
}

void CSinglePoleFilter::undoActionSizeSafe(const CActionSound &actionSound)
{
	restoreSelectionFromTempPools(actionSound,actionSound.start,actionSound.selectionLength());
}


// --------------------------------------------------

CSinglePoleLowpassFilterFactory::CSinglePoleLowpassFilterFactory(AActionDialog *channelSelectDialog,AActionDialog *normalDialog) :
	AActionFactory("Single Pole Lowpass Filter","Single Pole Lowpass Filter",false,channelSelectDialog,normalDialog,NULL)
{
}

CSinglePoleLowpassFilterFactory::~CSinglePoleLowpassFilterFactory()
{
}

CSinglePoleFilter *CSinglePoleLowpassFilterFactory::manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const
{
	return(new CSinglePoleFilter(
		actionSound,
		CSinglePoleFilter::ftLowpass,
		(float)actionParameters->getDoubleParameter("Gain"),
		(float)actionParameters->getDoubleParameter("Cutoff Frequency")
	));
}

// --------------------------------------------------

CSinglePoleHighpassFilterFactory::CSinglePoleHighpassFilterFactory(AActionDialog *channelSelectDialog,AActionDialog *normalDialog) :
	AActionFactory("Single Pole Highpass Filter","Single Pole Highpass Filter",false,channelSelectDialog,normalDialog,NULL)
{
}

CSinglePoleHighpassFilterFactory::~CSinglePoleHighpassFilterFactory()
{
}

CSinglePoleFilter *CSinglePoleHighpassFilterFactory::manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const
{
	return(new CSinglePoleFilter(
		actionSound,
		CSinglePoleFilter::ftHighpass,
		(float)actionParameters->getDoubleParameter("Gain"),
		(float)actionParameters->getDoubleParameter("Cutoff Frequency")
	));
}

// --------------------------------------------------

CBandpassFilterFactory::CBandpassFilterFactory(AActionDialog *channelSelectDialog,AActionDialog *normalDialog) :
	AActionFactory("Bandpass Filter","Bandpass Filter",false,channelSelectDialog,normalDialog,NULL)
{
}

CBandpassFilterFactory::~CBandpassFilterFactory()
{
}

CSinglePoleFilter *CBandpassFilterFactory::manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const
{
	return(new CSinglePoleFilter(
		actionSound,
		CSinglePoleFilter::ftBandpass,
		(float)actionParameters->getDoubleParameter("Gain"),
		(float)actionParameters->getDoubleParameter("Center Frequency"),
		(float)actionParameters->getDoubleParameter("Band Width")
	));
}

// --------------------------------------------------

CNotchFilterFactory::CNotchFilterFactory(AActionDialog *channelSelectDialog,AActionDialog *normalDialog) :
	AActionFactory("Notch Filter","Notch Filter",false,channelSelectDialog,normalDialog,NULL)
{
}

CNotchFilterFactory::~CNotchFilterFactory()
{
}

CSinglePoleFilter *CNotchFilterFactory::manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const
{
	return(new CSinglePoleFilter(
		actionSound,
		CSinglePoleFilter::ftNotch,
		(float)actionParameters->getDoubleParameter("Gain"),
		(float)actionParameters->getDoubleParameter("Center Frequency"),
		(float)actionParameters->getDoubleParameter("Band Width")
	));
}

