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

#include "COSSSoundPlayer.h"

#include <errno.h>
#include <string.h>

#include <stdio.h> // for fprintf

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <sys/soundcard.h>

#include <stdexcept>
#include <string>
#include <iostream>

#include <istring>

// ??? edit this to be able to detect necessary parameters from the typeof sample_t and CHANNELS defines from some global h file

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define BITS 16
#define OSS_PCM_FORMAT AFMT_S16_LE // needs to match for what is above and type of sample_t ???

#define BUFFER_COUNT 4
#define BUFFER_SIZE_BYTES 4096						// buffer size in bytes
#define BUFFER_SIZE_BYTES_LOG2 12					// log2(BUFFER_SIZE_BYTES) -- that is 2^this is BUFFER_SIZE_BYTES
#define BUFFER_SIZE_SAMPLES (BUFFER_SIZE_BYTES/(BITS/8)/CHANNELS) 	// in samples


COSSSoundPlayer::COSSSoundPlayer() :
	ASoundPlayer(),

	audio_fd(-1),
	initialized(false),

	playThread(this)
{
}

COSSSoundPlayer::~COSSSoundPlayer()
{
	deinitialize();
}

void COSSSoundPlayer::initialize()
{
	if(!initialized)
	{
		ASoundPlayer::initialize();

		// open OSS device
		if((audio_fd=open("/dev/dsp",O_WRONLY,0)) == -1) 
			throw(runtime_error(string(__func__)+" -- error opening OSS device -- "+strerror(errno)));
		//printf("OSS: device: %s\n","/dev/dsp");

		// set the bit rate and endianness
		int format=OSS_PCM_FORMAT; // signed 16-bit little endian
		if (ioctl(audio_fd, SNDCTL_DSP_SETFMT,&format)==-1)
		{
			close(audio_fd);
			throw(runtime_error(string(__func__)+" -- error setting the bit rate -- "+strerror(errno)));
		}
		else if(format!=OSS_PCM_FORMAT)
		{
			close(audio_fd);
			throw(runtime_error(string(__func__)+" -- error setting the bit rate -- the device does not support "+istring(BITS)+" bit little endian data"));
		}
		//printf("OSS: format: %d\n",format);


		// set number of channels 
		int stereo=CHANNELS-1;     /* 0=mono, 1=stereo */
		if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &stereo)==-1)
		{
			close(audio_fd);
			throw(runtime_error(string(__func__)+" -- error setting the number of channels -- "+strerror(errno)));
		}
		else if (stereo!=1)
		{
			close(audio_fd);
			throw(runtime_error(string(__func__)+" -- error setting the number of channels -- the device does not support stereo"));
		}
		//printf("OSS: channel count: %d\n",CHANNELS);

		devices[0].channelCount=CHANNELS; // make note of the number of channels for this device (??? which is only device zero for now)

		// set the sample rate
		int speed=SAMPLE_RATE;
		if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed)==-1) 
		{
			close(audio_fd);
			throw(runtime_error(string(__func__)+" -- error setting the sample rate -- "+strerror(errno)));
		} 
		if (speed!=SAMPLE_RATE)
		{ 
			fprintf(stderr,("warning: OSS used a different sample rate ("+istring(speed)+") than what was asked for ("+istring(SAMPLE_RATE)+")\n").c_str());
			//close(audio_fd);
			//throw(runtime_error(string(__func__)+" -- error setting the sample rate -- the sample rate is not supported"));
		} 
		//printf("OSS: sample rate: %d\n",speed);

		devices[0].sampleRate=speed; // make note of the sample rate for this device (??? which is only device zero for now)


		// set the buffering parameters
				// ??? I'm no quite sure why I have to -1 here... when I would print the info below, it just always had 1 more than I asked for
		int arg=((BUFFER_COUNT-1)<<16)+BUFFER_SIZE_BYTES_LOG2;  // 0xMMMMSSSS; where 0xMMMM is the number of buffers and 2^0xSSSS is the buffer size
		int parm=arg;
		if (ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &parm)==-1) 
		{
			close(audio_fd);
			throw(runtime_error(string(__func__)+" -- error setting the buffering parameters -- "+strerror(errno)));
		}
		else if(arg!=parm)
		{
			close(audio_fd);
			throw(runtime_error(string(__func__)+" -- error setting the buffering parameters -- "+istring(BUFFER_COUNT)+" buffers, each "+istring(BUFFER_SIZE_BYTES)+" bytes long, not supported"));
		}


		// get the fragment info
		audio_buf_info info;
		if(ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info)==-1)
		{
			close(audio_fd);
			throw(runtime_error(string(__func__)+" -- error getting the buffering parameters -- "+strerror(errno)));
		}
		/*
		printf("OSS: info.fragments: %d\n",info.fragments);
		printf("OSS: info.fragstotal: %d\n",info.fragstotal);
		printf("OSS: info.fragsize: %d\n",info.fragsize);
		printf("OSS: info.bytes: %d\n",info.bytes);
		*/


		// start play thread
		playThread.Start();

		initialized=true;
	}
	else
		throw(runtime_error(string(__func__)+" -- already initialized"));
}

void COSSSoundPlayer::deinitialize()
{
	if(initialized)
	{
		ASoundPlayer::deinitialize();

		// stop play thread
		playThread.kill=true;
		playThread.wait();

		// close OSS audio device
		close(audio_fd);

		initialized=false;
	}
}


COSSSoundPlayer::CPlayThread::CPlayThread(COSSSoundPlayer *_parent) :
	Thread(),

	kill(false),
	parent(_parent)
{
}

COSSSoundPlayer::CPlayThread::~CPlayThread()
{
}

void COSSSoundPlayer::CPlayThread::Run()
{
	try
	{
		/*
		 * NOTE: in linux, during testing when this thread was overruning this
		 * 	mix buffer, the thread would die, but the process would hang.
		 * 	I think it was sending a signal that perhaps I should catch.
		 *      So, I'm just making it bigger than necessary
		 */
		sample_t buffer[BUFFER_SIZE_SAMPLES*CHANNELS*2]; 

		while(!kill)
		{
			// can mixChannels throw any exception?
			parent->mixSoundPlayerChannels(CHANNELS,buffer,BUFFER_SIZE_SAMPLES*2);

			int len;
			if((len=write(parent->audio_fd,buffer,BUFFER_SIZE_BYTES))!=BUFFER_SIZE_BYTES)
				fprintf(stderr,"warning: didn't write whole buffer -- only wrote %d of %d bytes\n",len,BUFFER_SIZE_BYTES);

		}
	}
	catch(exception &e)
	{
		cerr << "exception caught in play thread: " << e.what() << endl;
		abort();
	}
	catch(...)
	{
		cerr << "unknown exception caught in play thread" << endl;
		abort();
	}
}

