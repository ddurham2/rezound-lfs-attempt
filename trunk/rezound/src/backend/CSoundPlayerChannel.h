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

#ifndef __CSoundPlayerChannel_H__
#define __CSoundPlayerChannel_H__


#include "../../config/common.h"

class CSoundPlayerChannel;

#include <CMutex.h>
#include <CRWLock.h>
#include <CConditionVariable.h>
#include <AThread.h>
#include <TMemoryPipe.h>

#include "CTrigger.h"
#include "CSound_defs.h"
class ASoundPlayer;

// ??? perhaps these should change to float if sample_t is floating point .. perhap define them where sample_t is defined
#define MAX_VOLUME_SCALAR 32767
#define MAX_VOLUME_SCALAR_DIV 32768

class CSoundPlayerChannel
{
public:
	virtual ~CSoundPlayerChannel();

	CSound *getSound() const;
	CSound * const sound;

	enum LoopTypes
	{
		ltLoopNone,		// don't loop
		ltLoopNormal,		// loop either the whole sound or the selection (depending on playSelectionOnly passed to play())
		ltLoopSkipMost,		// loop normally except skip most of the middle and put a short gap there instead
		ltLoopGapBeforeRepeat	// loop normally except put a short gap after the end point so it's clear where the loop point is
	};

	// the position arg is only used iff playLooped and playSelectionOnly are both false
	void play(sample_pos_t position=0,LoopTypes _playLooped=ltLoopNone,bool _playSelectionOnly=false);
	void pause();
	void stop();

	bool isPlaying() const;
	bool isPaused() const;
	bool isPlayingSelectionOnly() const;
	bool isPlayingLooped() const;

	void setSeekSpeed(float _seekSpeed);
	float getSeekSpeed() const;

	// really play position
	sample_pos_t getPosition() const { return playPosition; }
	void setPosition(sample_pos_t newPosition);

	sample_pos_t getStartPosition() const { return startPosition; }
	void setStartPosition(sample_pos_t newPosition);

	sample_pos_t getStopPosition() const { return stopPosition; }
	void setStopPosition(sample_pos_t newPosition);

	void setMute(unsigned channel,bool mute);
	bool getMute(unsigned channel) const;

	void addOnPlayTrigger(TriggerFunc triggerFunc,void *data);
	void removeOnPlayTrigger(TriggerFunc triggerFunc,void *data);
	void addOnPauseTrigger(TriggerFunc triggerFunc,void *data);
	void removeOnPauseTrigger(TriggerFunc triggerFunc,void *data);

	const vector<int16_t> getOutputRoutes() const;

	// pass and empty vector if this is not to restore, but possibly recreate the output routes
	void updateAfterEdit(const vector<int16_t> &restoreOutputRoutes);

private:

	friend class ASoundPlayer;

	// - called by ASoundPlayer
	// - nChannels is the number of channels buffer represents (i.e 1 mono, 2 stereo, etc)
	// - bufferSize is in sample frames
	void mixOntoBuffer(const unsigned nChannels,sample_t * const buffer,const size_t bufferSize);

	mutable CMutex prebufferPositionMutex;
	sample_pos_t prebufferPosition;
	bool prebufferChunk();

	void playingHasEnded();

	class CPrebufferThread : public AThread
	{
	public:
		CPrebufferThread(CSoundPlayerChannel *parent);
		void main();

		CSoundPlayerChannel *parent;
		bool kill;
		CConditionVariable waitForPlay;
		mutable CMutex waitForPlayMutex;
	} prebufferThread;

	friend class CPrebufferThread;

	struct RPrebufferedChunk
	{
		RPrebufferedChunk(const unsigned channelCount,const unsigned sampleRate);
		virtual ~RPrebufferedChunk();

		const unsigned channelCount;
		const unsigned sampleRate;
		sample_t *data;
		sample_fpos_t offset; // how many sample frames have already been used from data (should always be less than size)
		unsigned size; // how many sample frames are in data (does not depend on offset)

		bool isGap; // if this is true, then disregard 'data' and use the signal in the gap buffer (defined in the .cpp)
		sample_pos_t gapSignalBufferOffset; // the offset into the gapSignalBuffer to use if isGap is true

		sample_pos_t playPosition; // this is the play position of the first sample in this chunk

		vector<bool> outputRouting[MAX_CHANNELS];
	};

	CRWLock chunkObjectsMutex;
	size_t prebufferedChunksIndex; // this is the index of the next chunk to use from prebufferedChunks;
	vector<RPrebufferedChunk *> prebufferedChunks;
	TMemoryPipe<RPrebufferedChunk *> prebufferedChunkPipe;

	sample_t prevFrame[MAX_CHANNELS];

	sample_pos_t gapSignalBufferLength; // in frames
	sample_t *gapSignalBuffer;
	void createPrebufferedChunks();
	void destroyPrebufferedChunks();

	ASoundPlayer *player;
	CSoundPlayerChannel(ASoundPlayer *_player,CSound *_sound);

	// Playing Status and Play Positions
	volatile bool prebuffering,playing,paused,playSelectionOnly;
	volatile LoopTypes loopType;
	bool lastBufferWasGapSignal; // true if the last buffer that was processed in mixOntoBuffer had its isGap flag turned on (if this is the case, then I have to handle setting the play position in the setSeekSpeed() method a l;ttle different)
	sample_pos_t playPosition;
	float seekSpeed;
		float playSpeedForMixer; int playSpeedForChunker;
	volatile sample_pos_t startPosition,stopPosition;

	// Mute
	bool muted[MAX_CHANNELS];

	// Which output route to use
	unsigned outputRoute;

	CTrigger playTrigger,pauseTrigger;

	void deinit();
	void init();

	void createInitialOutputRoute();
	const vector<bool> getOutputRoute(unsigned deviceIndex,unsigned audioChannel) const;

	RPrebufferedChunk *getPrebufferChunk();
};

#endif
