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

#include "AStatusComm.h"
#include "initialize.h"
#include "settings.h"

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <stdexcept>
#include <string>

#include <CNestedDataFile/CNestedDataFile.h>

#include "ASoundPlayer.h"
static ASoundPlayer *gSoundPlayer=NULL; // saved value for deinitialize


#include "AAction.h"
#include "CNativeSoundClipboard.h"
#include "CRecordSoundClipboard.h"

#include "CLoadedSound.h"
#include "CSoundPlayerChannel.h"

// for mkdir  --- possibly wouldn't port???
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>	// for getenv

#include <CPath.h>

static bool checkForHelpFlag(int argc,char *argv[]);
static bool checkForVersionFlag(int argc,char *argv[]);
static void printUsage(const string app);

static void checkForAudioMethodFlag(int argc,char *argv[]);


/* 
 * These are expected to be provided by whatever frontend is enabled,
 * if ever more than one frontend can be enabled, then I would address
 * issues raised then.
 */
extern void readFrontendSettings();
extern void writeFrontendSettings();


bool initializeBackend(ASoundPlayer *&soundPlayer,int argc,char *argv[])
{
	try
	{
		// mask out SIGFPE (Floating-Point exception) signals
		{
			sigset_t sig_set;
			sigemptyset(&sig_set);
			sigaddset(&sig_set,SIGFPE);
			sigprocmask(SIG_BLOCK,&sig_set,NULL);
		}


		if(checkForHelpFlag(argc,argv))
			return false;
		if(checkForVersionFlag(argc,argv))
			return false;

		checkForAudioMethodFlag(argc,argv);


		// make sure that ~/.rezound exists
		gUserDataDirectory=string(getenv("HOME"))+istring(CPath::dirDelim)+".rezound";
		const int mkdirResult=mkdir(gUserDataDirectory.c_str(),0700);
		const int mkdirErrno=errno;
		if(mkdirResult!=0 && mkdirErrno!=EEXIST)
			throw(runtime_error(string(__func__)+" -- error creating "+gUserDataDirectory+" -- "+strerror(mkdirErrno)));



		// -- 1
			// if there is an error opening the registry file then
			// the system probably crashed... delete the registry file and
			// warn user that the previous run history is lost.. but that
			// they may beable to recover the last edits if they go load
			// the files that were being edited (since the pool files will
			// still exist for all previously open files)
		const string registryFilename=gUserDataDirectory+istring(CPath::dirDelim)+"registry.dat";
		try
		{
			CPath(registryFilename).touch();
			gSettingsRegistry=new CNestedDataFile(registryFilename,true);
		}
		catch(exception &e)
		{
			// well, then start with an empty one
					// ??? call a function to setup the initial registry, we could either insert values manually, or copy a file from the share dir and maybe have to change a couple of things specific to this user
					// 	because later I expect there to be many necesary default settings
			gSettingsRegistry=new CNestedDataFile("",true);
			gSettingsRegistry->setFilename(gUserDataDirectory+"/registry.dat");

			Error(string("Error reading registry -- ")+e.what());
		}

		// read backend setting variables from registry
		readBackendSettings();
		readFrontendSettings();


		// -- 2
		soundPlayer=gSoundPlayer=ASoundPlayer::createInitializedSoundPlayer();


		// -- 3
		for(unsigned t=1;t<=3;t++)
		{
			const string filename=gClipboardDir+CPath::dirDelim+gClipboardFilenamePrefix+".clipboard"+istring(t)+"."+istring(getuid());
			try
			{
				AAction::clipboards.push_back(new CNativeSoundClipboard(_("Native Clipboard ")+istring(t),filename));
			}
			catch(exception &e)
			{
				Warning(e.what());
				remove(filename.c_str());
			}
		}
		for(unsigned t=1;t<=3;t++)
		{
			const string filename=gClipboardDir+CPath::dirDelim+gClipboardFilenamePrefix+".record"+istring(t)+"."+istring(getuid());
			try
			{
				AAction::clipboards.push_back(new CRecordSoundClipboard(_("Record Clipboard ")+istring(t),filename,soundPlayer));
			}
			catch(exception &e)
			{
				Warning(e.what());
				remove(filename.c_str());
			}
		}


			// make sure the global clipboard selector index is in range
		if(gWhichClipboard>=AAction::clipboards.size())
			gWhichClipboard=0; 
	}
	catch(exception &e)
	{
		Error(e.what());
		try { deinitializeBackend(); } catch(...) { /* oh well */ }
		exit(0);
	}

	return true;
}

#include "ASoundFileManager.h"
bool handleMoreBackendArgs(ASoundFileManager *fileManager,int argc,char *argv[])
{
	bool forceFilenames=false;
	bool playLoadedWhenDone=false;
	for(int t=1;t<argc;t++)
	{
		if(strcmp(argv[t],"--")==0)
		{ // anything after a '--' flag is assumed as a filename to load
			forceFilenames=true;
			continue;
		}

		if(!forceFilenames && (strcmp(argv[t],"-p")==0 || strcmp(argv[t],"--play")==0))
		{
			playLoadedWhenDone=true;
		}
		else
#ifdef HAVE_LIBAUDIOFILE
		// also handle a --raw flag to force the loading of the following argument as a file
		if(!forceFilenames && strcmp(argv[t],"--raw")==0)
		{
			if(t>=argc-1)
			{
				printUsage(argv[0]);
				return(false);
			}

			t++; // next arg is filename
			const string filename=argv[t];

			try
			{
				fileManager->open(filename,true);
			}
			catch(exception &e)
			{
				Error(e.what());
			}
		}
		else
#endif
		// load as a filename if the first char of the arg is not a '-'
		if(forceFilenames || argv[t][0]!='-')
		{ // not a flag
			const string filename=argv[t];
			try
			{
				fileManager->open(filename);
			}
			catch(exception &e)
			{
				Error(e.what());
			}
		}
	}

	if(playLoadedWhenDone)
	{
		for(size_t t=0;t<fileManager->getOpenedCount();t++)
			fileManager->getSound(t)->channel->play();
	}

	return true;
}

void deinitializeBackend()
{
	// reverse order of creation


	// -- 3
	while(!AAction::clipboards.empty())
	{
		delete AAction::clipboards[0];
		AAction::clipboards.erase(AAction::clipboards.begin());
	}


	// -- 2
	if(gSoundPlayer!=NULL)
	{
		gSoundPlayer->deinitialize();
		delete gSoundPlayer;
	}


	// -- 1
	writeFrontendSettings();
	writeBackendSettings();

	gSettingsRegistry->save();
	delete gSettingsRegistry;

}


static bool checkForHelpFlag(int argc,char *argv[])
{
	for(int t=1;t<argc;t++)
	{
		if(strcmp(argv[t],"--")==0)
			break; // stop at '--'

		if(strcmp(argv[t],"--help")==0)
		{
			printUsage(argv[0]);
			return true;
		}
	}
	return false;
}

static bool checkForVersionFlag(int argc,char *argv[])
{
	for(int t=1;t<argc;t++)
	{
		if(strcmp(argv[t],"--")==0)
			break; // stop at '--'

		if(strcmp(argv[t],"--version")==0)
		{
			printf("%s %s\n",REZOUND_PACKAGE,REZOUND_VERSION);
			printf("Written primarily by David W. Durham\nSee the AUTHORS document for more details\n\n");
			printf("This is free software; see the source for copying conditions.  There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
			printf("\n");
			printf("Project homepage\n\thttp://rezound.sourceforge.net\n");
			printf("Please report bugs to\n\thttp://sourceforge.net/tracker/?atid=105056&group_id=5056\n");
			printf("\n");
			return true;
		}
	}
	return false;
}

static void checkForAudioMethodFlag(int argc,char *argv[])
{
	for(int t=1;t<argc;t++)
	{
		if(strcmp(argv[t],"--")==0)
			break; // stop at '--'

		if(strncmp(argv[t],"--audio-method=",15)==0)
			gDefaultAudioMethod=argv[t]+15;
	}
}

static void printUsage(const string app)
{
	printf("Usage: %s [option | filename]... [-- [filename]...]\n",app.c_str());
	printf("Options:\n");
#ifdef HAVE_LIBAUDIOFILE
	printf("\t--raw filename   load filename interpreted as raw data\n");
#endif                             
	printf("\n");              
	printf("\t--help                   show this help message and exit\n");
	printf("\t--version                show version information and exit\n");
	printf("\t-p OR --play             simultaneously plays the sound files given at the command line after all are loaded\n");
	printf("\t--audio-method=<method>  method to try first for audio I/O\n");
	printf("\t  Audio Method(s) Are:\n");
#ifdef ENABLE_OSS
	printf("\t\toss\n");
#endif
#ifdef ENABLE_JACK
	printf("\t\tjack\n");
#endif
#ifdef ENABLE_PORTAUDIO
	printf("\t\tportaudio\n");
#endif
	printf("\t\tnull\n");

	printf("\n");
	printf("Notes:\n");
	printf("\t- Anything after a '--' flag will be assumed as a filename to load\n");
	printf("\t- The file ~/.rezound/registry.dat does contain some settings that\n\t  can only be changed by editing the file (right now)\n");

	printf("\n");
	printf("Project homepage:\n\thttp://rezound.sourceforge.net\n");
	printf("Please report bugs to:\n\thttp://sourceforge.net/tracker/?atid=105056&group_id=5056\n");

	printf("Please consider joining the ReZound-users mailing list:\n\thttp://lists.sourceforge.net/lists/listinfo/rezound-users\n");

	
	printf("\n");
}

