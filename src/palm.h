/*
 * @(#)palm.h
 *
 * Copyright 2000, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 *
 * This file was generated as part of the "palmsource" guide application
 * (for use at PalmSource 2000 [Santa Clara, CA]) specifically  designed 
 * for the Palm Computing Platform.
 *
 *   http://www.palm.com/
 *
 * The contents of this file are confidential and proprietary in nature
 * ("Confidential Information"). Redistribution or modification without 
 * prior consent of the original author(s) is prohibited.
 */

#ifndef _PALM_H
#define _PALM_H

// system includes
#include <PalmOS.h>

// application constants and structures
#define appCreator        'PS2k'
#define hlpString         'hSTR'
#define ftrHelpGlobals    1000

#define romVersion1   sysMakeROMVersion(1,0,0,sysROMStageRelease,0)
#define romVersion2   sysMakeROMVersion(2,0,0,sysROMStageRelease,0)
#define romVersion3   sysMakeROMVersion(3,0,0,sysROMStageRelease,0)
#define romVersion3_1 sysMakeROMVersion(3,1,0,sysROMStageRelease,0)
#define romVersion3_2 sysMakeROMVersion(3,2,0,sysROMStageRelease,0)
#define romVersion3_3 sysMakeROMVersion(3,3,0,sysROMStageRelease,0)
#define romVersion3_5 sysMakeROMVersion(3,5,0,sysROMStageRelease,0)

// local includes
#include "resource.h"
#include "help.h"
#include "gccfix.h"

// functions
extern UInt32  PilotMain(UInt16, MemPtr, UInt16);
extern void    InitApplication(void);
extern void    EventLoop(void);
extern void    EndApplication(void);

#endif 
