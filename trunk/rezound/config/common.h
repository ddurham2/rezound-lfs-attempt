/* $Id$
 * 
 * Copyright (C) 2002 - Anthony Ventimiglia
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
#ifndef COMMON_H
#define COMMON_H

/* common.h -- This file will deal with low-level portability problems. It
 * should be includede at the top of every package file. */

//This is added for g++-3.0 and later, which requires it
using namespace std;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Redefine PACKAGE TO REZOUND_PACKAGE to clear up possible conflicts with
 * other autoconfed packages 
 */
#ifdef PACKAGE
static const char *REZOUND_PACKAGE=PACKAGE;
# undef PACKAGE
#else
# define REZOUND_PACKAGE "rezound"	
/* Just in case it wasn't defined, the only time this should happen if for some reason someone would decide to compile without an automake built Makefile*/
#endif

/* do the same thing for VERSION */
#ifdef VERSION
static const char *REZOUND_VERSION=VERSION;
# undef VERSION
#else
# define REZOUND_VERSION	"vx.xxx" /* just in case it wasn't defined. */
#endif


/* 
 * According to the gcc info pages this should take care of detecting gcc and
 * define __func__ to __PRETTY_FUNCTION__ if g++ is being used to compile. No
 * configure trickery is needed. Defining here will affect all code since
 * common.h is included all around. 
 */
#ifdef __GNUC__ //Using gcc
# define __func__ __PRETTY_FUNCTION__
#else // not gcc
/*
 * I'm not sure if I should define this to __func__ or undefine it, this is
 * redundant, but I put it here to make it obvious, so if we'd rather undefine
 * it, it stands out here. 
 */
# define __func__ __func__
#endif //__GNUC__


/*
 * This should really be a configure test because 'nearbyint() and round()' might 
 * exist one day on BSD I could also just use rint, but nearbyint is supposed to 
 * be slightly faster because it doesn't raise the inexact math exception.
 */
#ifdef __FreeBSD__
#define nearbyint rint
#define round rintf
#endif

/*
 * It is not good to simply use bool==bool because anything non-zero is seen as true
 * which wouldn't equate.  This returns true iff both are non-zero or both are zero
 */
static bool compareBool(int a,int b) { return (a && b) || (!a && !b); }

#endif /* COMMON_H */