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

#ifndef __CPasteEdit_H__
#define __CPasteEdit_H__

#include "../../../config/common.h"

#include "../AAction.h"

class CPasteEdit : public AAction
{
public:
	enum PasteTypes
	{
		ptInsert,
		ptReplace,
		ptMix, // is overwrite with mixMethod of mmOverwrite
		ptLimitedMix // is limited overwrite with mixMethod of mmOverwrite
	};

	CPasteEdit(const CActionSound actionSound,const bool pasteChannels[MAX_CHANNELS][MAX_CHANNELS],PasteTypes pasteType,MixMethods mixMethod=mmOverwrite);
	virtual ~CPasteEdit();


protected:
	bool doActionSizeSafe(CActionSound &actionSound,bool prepareForUndo);
	void undoActionSizeSafe(const CActionSound &actionSound);
	CanUndoResults canUndo(const CActionSound &actionSound) const;

private:
	PasteTypes pasteType;
	MixMethods mixMethod;
	bool pasteChannels[MAX_CHANNELS][MAX_CHANNELS];
	bool whichChannels[MAX_CHANNELS]; // or-ed together rows from pasteChannels which says which channels are affected


	void pasteData(const ASoundClipboard *clipboard,const bool pasteChannels[MAX_CHANNELS][MAX_CHANNELS],const CActionSound &actionSound,const sample_pos_t srcLength,bool invalidatePeakData,MixMethods initialMixMethod,MixMethods nonInitialMixMethod);

	// --- undo information --------
	sample_pos_t undoRemoveLength;
	sample_pos_t originalLength;

};

class CInsertPasteEditFactory : public AActionFactory
{
public:
	CInsertPasteEditFactory(AActionDialog *channelSelectDialog);

	CPasteEdit *manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const;

	bool doPreActionSetup();
};

class CReplacePasteEditFactory : public AActionFactory
{
public:
	CReplacePasteEditFactory(AActionDialog *channelSelectDialog);

	CPasteEdit *manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const;

	bool doPreActionSetup();
};

class COverwritePasteEditFactory : public AActionFactory
{
public:
	COverwritePasteEditFactory(AActionDialog *channelSelectDialog);

	CPasteEdit *manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const;

	bool doPreActionSetup();
};

class CLimitedOverwritePasteEditFactory : public AActionFactory
{
public:
	CLimitedOverwritePasteEditFactory(AActionDialog *channelSelectDialog);

	CPasteEdit *manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const;

	bool doPreActionSetup();
};

class CMixPasteEditFactory : public AActionFactory
{
public:
	CMixPasteEditFactory(AActionDialog *channelSelectDialog);

	CPasteEdit *manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const;

	bool doPreActionSetup();
};

class CLimitedMixPasteEditFactory : public AActionFactory
{
public:
	CLimitedMixPasteEditFactory(AActionDialog *channelSelectDialog);

	CPasteEdit *manufactureAction(const CActionSound &actionSound,const CActionParameters *actionParameters,bool advancedMode) const;

	bool doPreActionSetup();
};

#endif
