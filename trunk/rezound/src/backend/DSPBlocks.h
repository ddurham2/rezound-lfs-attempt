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

#ifndef __DSPBlocks_H__
#define __DSPBlocks_H__

#include "../../config/common.h"

#include <stdexcept>
#include <algorithm>

#include <istring>

#include "CSound_defs.h"
#include "unit_conv.h"

#include "ALFO.h"


// ??? make these that could be a template be a template rather than fixed types

/* --- TDSPDelay --------------------------------
 *	- This class is uses a circular buffer to delay the input values by a certain delay time
 *	- There are several ways of retrieving samples out of the delay line.
 *		- Some to pull from the end of the delay, and some to pull from the middle of the delay
 *
 *	- all delay time parameters are in samples not seconds
 *
 *      - the template parameter is the type of data to be delayed
 */
template<class sample_t> class TDSPDelay
{
public:
	TDSPDelay(const unsigned _maxDelayTime=1) :
		buffer(NULL),
		maxDelayTime(0),

		putPos(0),
		getPos(0)
	{
		setDelayTime(_maxDelayTime);
	}

	void setDelayTime(unsigned _maxDelayTime)
	{
		_maxDelayTime++; // fudging? or necessary because delayTime could be passed equal to the maxDelayTime? 
		/*
		if(_maxDelayTime<1)
			throw(runtime_error(string(__func__)+" -- maxDelayTime parameter too small: "+istring(_maxDelayTime)));
		*/

		maxDelayTime=_maxDelayTime;

		if(buffer!=NULL)
			delete [] buffer;

		buffer=new sample_t[maxDelayTime]; // expect to get an exception on error
		clear();
	}

	virtual ~TDSPDelay()
	{
		if(buffer!=NULL)
			delete [] buffer;
	}

	void clear()
	{
		memset(buffer,0,maxDelayTime*sizeof(*buffer));
		putPos=getPos=maxDelayTime;
	}

	// give an input sample, returns the sample delayed by the constructed delay time
	sample_t processSample(const sample_t input)
	{
		sample_t output=getSample();
		putSample(input);
		return(output);
	}

	void putSample(const sample_t s)
	{
		buffer[(putPos++)%maxDelayTime]=s;
	}

	const sample_t getSample()
	{
		const sample_t s=buffer[(getPos-maxDelayTime)%maxDelayTime];
		getPos++;
		return(s);
	}

	const sample_t getSample(const unsigned delayTime)
	{
		const sample_t s=buffer[(putPos-delayTime)%maxDelayTime];
		return(s);
	}

	const sample_t getSample(const float delayTime)
	{
		const float fReadPos=putPos-delayTime;
		const unsigned iReadPos=(unsigned)fReadPos; // floored

		const float p2=fReadPos-iReadPos;
		const float p1=1.0-p2;

		const sample_t s=(sample_t)(p1*buffer[(iReadPos)%maxDelayTime] + p2*buffer[(iReadPos+1)%maxDelayTime]);
		return(s);
	}


private:
	sample_t *buffer;
	unsigned maxDelayTime;
	unsigned putPos,getPos;
};


/* --- CDSPLevelDetector --------------------------------------
 *
 *	- This class can be used to determine the level or volume of an audio source.
 *	- It uses a progressing window of past samples to calculate an RMS (root-mean-square which is sqrt((s1^2 + s2^2 + ... Sn^2)/N) ) versus using a normal average or 'mean'
 *	- The windowTime given at construction is in samples not seconds
 *	- It is often necessary to initialize the object with 'windowTime' samples before even using the value from readLevel()
 *		- To do this, simply call readLevel() for the first 'windowTime' samples in the audio stream
 */
class CDSPLevelDetector
{
public:
	CDSPLevelDetector(const unsigned _windowTime) :
		window(_windowTime),
		sumOfSquaredSamples(0.0),
		windowTime(_windowTime),
		iWindowTime(_windowTime)
	{
		// ??? based on windowTotals type and sample_t's type there is a maximum time I should impose on windowTime
	}

	virtual ~CDSPLevelDetector()
	{
	}

	// every time you read the level, you have to supply the next sample in the audio stream
		// ??? I could avoid using sqrt by saying that I return the 'power' and not the 'level' and I would then have to square the value I compare this return value with, instead
	const mix_sample_t readLevel(mix_sample_t newSample)
	{
		const mix_sample_t currentAmplitude=(mix_sample_t)sqrt(sumOfSquaredSamples/windowTime);

		// remove oldest sample from running statistic
		sumOfSquaredSamples-=window.getSample();

		// add new sample to running statistic
		const mix_sample_t temp=newSample*newSample;
		sumOfSquaredSamples+=(temp);
		window.putSample(temp);
		
		return(currentAmplitude);
	}

	const unsigned getWindowTime() const
	{
		return(iWindowTime); // the value this was constructed with
	}

private:

	TDSPDelay<mix_sample_t> window; // used to know a past set of samples
	double sumOfSquaredSamples;
	const double windowTime;
	const unsigned iWindowTime;
};


/* --- CDSPNoiseGate -----------------------------------------
 *
 * - This DSP block begins the mute the sound when the level of the audio drops
 *   below the given 'threshold'.  The rate at which it begins to mute the data is
 *   given by the 'gainAttackTime' and the rate at which it begins to come out of the
 *   mute once the level is again above the threshold is given by 'gainReleaseTime'.
 *   The level is detected with a moving average (performed by CDSPLevelDetector) 
 *   the width of the moving window is given by 'windowTime'
 *
 * - To use simply construct the block with the desired parameters and repeatedly call 
 *   processSample() which returns the sample that was given but adjusted by the 
 *   calculated gain.
 *
 * - Note: it may be desirable to initialize the level detector within the algorithm
 *   by calling initSample() for 'windowTime' samples before calling processSample() 
 *   and using its return value.  This is because we don't really know the level before 
 *   enough sample have been observed.
 *
 * Info reference: http://www.harmony-central.com/Effects/Articles/Expansion/
 *
 */
class CDSPNoiseGate
{
public:
	// all times are in samples
	CDSPNoiseGate(unsigned _windowTime,sample_t _threshold,unsigned _gainAttackTime,unsigned _gainReleaseTime) :
		windowTime(_windowTime),
		threshold(_threshold),
		gainAttackVelocity(1.0/(float)_gainAttackTime),
		gainReleaseVelocity(1.0/(float)_gainReleaseTime),
	
		gain(1.0),
		levelDetector(windowTime)
	{
		// ??? verify parameters?
	}

	virtual ~CDSPNoiseGate()
	{
	}

	// using this is optional -- it is used to initialize the level detector -- see Note above
	void initSample(const mix_sample_t s)
	{
		levelDetector.readLevel(s);
	}

	const mix_sample_t processSample(const mix_sample_t s)
	{
		const mix_sample_t level=levelDetector.readLevel(s);
		
		if(level<=threshold && gain>0.0)
			gain= max(gain-gainAttackVelocity,(float)0.0);
		else if(level>threshold && gain<1.0)
			gain= min(gain+gainReleaseVelocity,(float)1.0);

			// ??? should be able to make this 3 return statements in the 2 cases+else above
		return(gain!=1.0 ? (mix_sample_t)(s*gain) : s);
	}


	// can be used to reset the internal gain if desired
	void resetGain(const float _gain=1.0)
	{
		gain=_gain;
	}

	const unsigned getWindowTime() const
	{
		return(windowTime); // the value this was constructed with
	}


private:
	const unsigned windowTime;
	const mix_sample_t threshold;
	const float gainAttackVelocity;
	const float gainReleaseVelocity;

	float gain;
	CDSPLevelDetector levelDetector;
};


/* --- CDSPCompressor ----------------------------------------
 *
 * - This DSP block begins to change the gain from 1.0 to a different value based 
 *   on the ratio on the sound when the level of the level signal rises above the given 
 *   'threshold'.  The rate at which it begins to change this gain is given by the 
 *   'attackTime' and the rate at which it begins to return to a gain of 1.0 once 
 *   the level is again below the threshold is given by 'releaseTime'.  The level 
 *   is detected with a moving average (performed by CDSPLevelDetector) the width
 *   of the moving window is given by 'windowTime'
 *
 * - To use simply construct the block with the desired parameters and repeatedly call 
 *   processSample() which returns the 'inputSample' that was given but adjusted by the 
 *   calculated gain which acts according to the changes in level for successive 
 *   'levelSample' values.
 *
 * - Note: it may be desirable to initialize the level detector within the algorithm
 *   by calling initSample() for 'windowTime' samples before calling processSample() 
 *   and using its return value.  This is because we don't really know the level before 
 *   enough sample have been observed.
 *
 * Info reference: http://www.harmony-central.com/Effects/Articles/Compression/
 * 		   and newsgroup: comp.dsp  my thread, 'compressor question'
 *
 */
class CDSPCompressor
{
public:
	// all times are in samples
	CDSPCompressor(unsigned _windowTime,sample_t _threshold,float _compressionRatio,unsigned _attackTime,unsigned _releaseTime) :
		windowTime(_windowTime),
		threshold(_threshold),
		compressionRatio(_compressionRatio),

		// have to take into account the ratio to be reached for the calculation 
		// of the envelope velocity (value to add/sub to bouncingRatio each sample frame)
		attackVelocity((_compressionRatio-1.0)/_attackTime),
		releaseVelocity((_compressionRatio-1.0)/_releaseTime),

		bouncingRatio(1.0),
		levelDetector(windowTime)
	{
		// ??? verify parameters?
	}

	virtual ~CDSPCompressor()
	{
	}

	// using this is optional -- it is used to initialize the level detector -- see Note above
	void initSample(const mix_sample_t s)
	{
		levelDetector.readLevel(s);
	}

	const mix_sample_t processSample(const mix_sample_t inputSample,const mix_sample_t levelSample)
	{
		const mix_sample_t level=levelDetector.readLevel(levelSample);
		
		// ??? I need to allow compression ratio to also be less than 1 to act as an exciter
		if(level>=threshold && bouncingRatio!=compressionRatio)
			bouncingRatio= min(bouncingRatio+attackVelocity,compressionRatio);
		else if(level<threshold && bouncingRatio!=1.0)
			bouncingRatio= max(bouncingRatio-releaseVelocity,(float)1.0);

		// attempt to smooth out the way the attack sounds
		//const float _bouncingRatio=unitRange_to_otherRange_linear(unitRange_to_unitRange_squared(otherRange_to_unitRange_linear(bouncingRatio,1.0,compressionRatio)),1.0,compressionRatio);
		#define _bouncingRatio bouncingRatio

		if(_bouncingRatio>1.0)
			return (mix_sample_t)(inputSample * pow((double)threshold/(double)level,(_bouncingRatio-1.0)/_bouncingRatio) );
		else
			return inputSample;
		
	}

	// this works like processSample except that it uses all the channels in the frame to determine the signal level and adjust all the samples in the frame according to that singular calculated level
	// its output is a modification of the inputFrame parameter
	void processSampleFrame(mix_sample_t *inputFrame,const unsigned frameSize)
	{
		// determine the maximum sample value in the frame to use as the level detector's input
		mix_sample_t levelSample=inputFrame[0];
		if(levelSample<0) levelSample=-levelSample;
		for(unsigned t=1;t<frameSize;t++)
		{
			mix_sample_t l=inputFrame[t];
			if(l<0) l=-l;
			levelSample=max(levelSample,l);
		}
			
		const mix_sample_t level=levelDetector.readLevel(levelSample);
		
		// ??? I need to allow compression ratio to also be less than 1 to act as an exciter
		if(level>=threshold && bouncingRatio!=compressionRatio)
			bouncingRatio= min(bouncingRatio+attackVelocity,compressionRatio);
		else if(level<threshold && bouncingRatio!=1.0)
			bouncingRatio= max(bouncingRatio-releaseVelocity,(float)1.0);

		if(bouncingRatio>1.0)
		{
			const double g=pow((double)threshold/(double)level,(bouncingRatio-1.0)/bouncingRatio);
			for(unsigned t=0;t<frameSize;t++)
				inputFrame[t]=(mix_sample_t)(inputFrame[t]*g);
		}
	}


	const unsigned getWindowTime() const
	{
		return(windowTime); // the value this was constructed with
	}


private:
	const unsigned windowTime;
	const mix_sample_t threshold;
	const float compressionRatio;
	const float attackVelocity;
	const float releaseVelocity;

	// this is the compressionRatio affected by the attack and 
	// release times as the level goes above and below the threshold
	float bouncingRatio;

	CDSPLevelDetector levelDetector;
};



/* --- CDSPDelayEffect ----------------------------
 *
 * - This DSP block is used to create a delay effect or echoes.  It is different
 *   than the CDSPDelay block in that this one is a full fledged effect more than
 *   just a DSP component block.
 *
 * - The constructor is given the number of taps, tap times, tap gains, and tap feed backs
 * - To use this DSP block, simply construct it with the parameters and call processSample()
 *   to given and get each successive sample.
 * - The following is a diagram showing how this block works
	Based somewhat from http://www.harmony-central.com/Effects/Articles/Delay/
 
                             ---             ---             ---                
              --------------| + |<----------| + |<----------| + |<-- ... <------------
             |               ---             ---             ---                      |
             |                ^               ^               ^                       | 
             |         tap 1  |        tap 2  |        tap 3  |                tap N  | 
             |       feedback/_\     feedback/_\     feedback/_\             feedback/_\
             v                |               |               |                       |
  input     ---     -------   |     -------   |     -------   |             -------   |
    ------>| + |-->| Delay |--+--->| Delay |--+--->| Delay |--+----> ... ->| Delay |--+
       |    ---     -------   |     -------   |     -------   |             -------   |
       |                     _|_             _|_             _|_                     _|_
       |                tap 1\_/        tap 2\_/        tap 3\_/     ...        tap N\_/
       |                 gain |          gain |          gain |                  gain |
       |                      v               v               v                       v
       |                     ---             ---             ---                     ---     output
        ------------------->| + |---------->| + |---------->| + |--> ... ---------->| + |------>
                             ---             ---             ---                     ---  

 *
 */
class CDSPDelayEffect
{
public:
		// ??? convert these to vectors of floats and also change the way action parameters are passed
		// actually convert tapTimes to a vector of unsigned
	CDSPDelayEffect(size_t _tapCount,float *_tapTimes,float *_tapGains,float *_tapFeedbacks) :
		tapCount(_tapCount),

		tapTimes(_tapTimes),
		tapGains(_tapGains),
		tapFeedbacks(_tapFeedbacks),

		delays(new TDSPDelay<mix_sample_t>[_tapCount])
	{
		// perhaps fill with data from the past not zero for initial delay time number of samples
		for(size_t t=0;t<tapCount;t++)
			delays[t].setDelayTime((unsigned)tapTimes[t]);
	}

	virtual ~CDSPDelayEffect()
	{
		delete [] delays;
	}

	const mix_sample_t processSample(const mix_sample_t srcSample)
	{
		mix_sample_t fedbackSample=srcSample;
		mix_sample_t outputSample=srcSample;
		
		int k=tapCount-1;

		// start at the last tap and propogate backwards (in the diagram)
		const mix_sample_t s=delays[k].getSample();
		outputSample+=(mix_sample_t)(s*tapGains[k]);
		fedbackSample+=(mix_sample_t)(s*tapFeedbacks[k]);
		k--;
		
		for(;k>=0;k--)
		{
			const mix_sample_t s=delays[k].getSample();
			outputSample+=(mix_sample_t)(s*tapGains[k]);
			fedbackSample+=(mix_sample_t)(s*tapFeedbacks[k]);
			delays[k+1].putSample(s);
		}

		// write to delay 
		delays[0].putSample(fedbackSample);

		return(outputSample);
	}

private:
	const size_t tapCount;

	const float *tapTimes; // in samples
	const float *tapGains; // ??? change to unsigned
	const float *tapFeedbacks;

	TDSPDelay<mix_sample_t> *delays;
};


/* --- CFlangeEffect ------------------------------------
 *
 * - This DSP block produces a flange effect based on the parameters given to 
 *   the constructor.  
 *
 * - To use this DSP block, simply construct it with the parameters and call processSample()
 *   to given and get each successive sample.
 *
 * - Here is a diagram that shows how this block works:
	From http://www.harmony-central.com/Effects/Articles/Flanging/ (slightly modified)

                   feedback gain
                         /|
                --------< |<------- 
               |         \|        |
               |                   |
               |        -----      |
               |       | LFO |     |
               |        -----      |
               V          |        |    wet gain
  input       ---      -------     |       |\         ---    output
    -------->| + |--->| Delay |----------->| >------>| + |------>
         |    ---      -------             |/         ---
	 |                                             ^
         |                              dry gain       |
         |                                 |\          |
          -------------------------------->| >---------
                                           |/   
 *
 */
class CDSPFlangeEffect
{
public:
	// delayTime is in samples
	// LFODepth is in samples (the amount of samples delayed beyond delayTime when the LFO is at 1)
	// The LFO object must have a range of [0,1] and nothing greater
	CDSPFlangeEffect(unsigned _delayTime,float _wetGain,float _dryGain,ALFO *_LFO,unsigned _LFODepth,float _feedback) :
		delay((unsigned)(_delayTime+_LFODepth)),
		delayTime(_delayTime),

		wetGain(_wetGain),
		dryGain(_dryGain),

		LFO(_LFO),

		LFODepth(_LFODepth),

		feedback(_feedback)
	{
		// ??? I could validate the parameters
	}

	virtual ~CDSPFlangeEffect()
	{
	}

	const mix_sample_t processSample(const mix_sample_t srcSample)
	{
		// calculate the delay time in samples from the LFO
		const float _delayTime=(delayTime+(LFODepth*LFO->nextValue()));

		// read a sample from the delay
		const mix_sample_t delayedSample=delay.getSample(_delayTime);

		// write to delay
		delay.putSample((mix_sample_t)(srcSample+(delayedSample*feedback)));

		return((mix_sample_t)((srcSample*dryGain)+(delayedSample*wetGain)));
	}

private:
	TDSPDelay<mix_sample_t> delay;
	const float delayTime;
	const float wetGain;
	const float dryGain;
	ALFO * const LFO;
	const float LFODepth;
	const float feedback;
};




/* --- TDSPConvolver ------------------------------------
 * 
	This class is a DSP block to do a sample by sample convolution of the given 
	array of coefficients with the input given by the repeated calls to processSample()

	The frist template parameter specifies the type of the input samples (and is thus 
	the type also of the output, the return value of processSample() ).  And The second
	template parameter specifies the type of the coefficients.
 */
template<class sample_t,class coefficient_t> class TDSPConvolver
{
public:
	TDSPConvolver(const coefficient_t _coefficients[],size_t _coefficientCount) :
		coefficients(_coefficients),
		coefficientCount(_coefficientCount),
		coefficientCountSub1(_coefficientCount-1),
		delay(_coefficientCount)
	{
	}

	virtual ~TDSPConvolver()
	{
	}

	sample_t processSample(const sample_t input)
	{
		coefficient_t output=0;
		delay.putSample((coefficient_t)input);

		register unsigned i=0;
		for(int t=coefficientCountSub1;t>=0;t--)
			output+=delay.getSample(i++)*coefficients[t];
		
		return((sample_t)output);
	}

private:
	const coefficient_t *coefficients; // aka, the impluse response
	const int coefficientCount;
	const int coefficientCountSub1;

	TDSPDelay<coefficient_t> delay;
};

#if 0
#error these need conversion to mix_sample_t and what not
class CDSPCombFilter
{
public:
	CDSPCombFilter(const int delayTime,const float _g) :
		g(_g),
		delay(delayTime)
	{
	}

	const mix_sample_t getput(const mix_sample_t s)
	{
		const mix_sample_t ts=delay.getSample();
		delay.putSample(mix(s,gain(ts,g)));
		return(ts);
	}

private:
	const float g;
	TDSPDelay<mix_sample_t> delay;
};

class CDSPAllPassFilter
{
public:
	CDSPAllPassFilter(const int delayTime,const float _g) :
		g(_g),
		delay(delayTime)
	{
	}

	const mix_sample_t getput(sample_t s)
	{
		mix_sample_t ts=delay.getSample();

		mix_sample_t r=mix(ts,gain(s,-g));

		delay.putSample(mix(s,gain(r,g)));

		return(r);

	}

private:
	const float g;
	TDSPDelay<mix_sample_t> delay;
};

#endif

#endif
