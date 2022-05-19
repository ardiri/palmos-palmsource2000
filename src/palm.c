/*
 * @(#)palm.c
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

#include "palm.h"

/**
 * The Palm Computing Platform entry routine (mainline).
 *
 * @param cmd         a word value specifying the launch code.
 * @param cmdPBP      pointer to a structure associated with the launch code.
 * @param launchFlags additional launch flags.
 * @return zero if launch successful, non zero otherwise.
 */
UInt32  
PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
  UInt32 result = 0;

  // what type of launch was this?
  switch (cmd) 
  {
    case sysAppLaunchCmdNormalLaunch:
         {
           UInt32 version = 0;

           // get the ROM version for the device
           FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);

           // palmhelp requires palmos 3.0 (grayscale MINIMUM)
           if (version >= romVersion3) {

             // change the the "highest" possible mode :)
	     {
	       UInt32 depthsToTry[] = { 8, 4, 2 };
	       UInt32 *depthPtr = &depthsToTry[0];

	       // loop until a valid mode is found
	       while (WinScreenMode(winScreenModeSet,NULL,NULL,depthPtr,NULL)) {
	         // try next depth
		 depthPtr++;
	       }
	     }

             // display the help :)
             FrmHelpReplacement(5000);
           }
	   else {

             // if launching app, display dialog (nice one)
	     FormType *form = FrmInitForm(deviForm);
	     FrmDoDialog(form);
	     FrmDeleteForm(form);

             // exit the application "gracefully" :>
             if (version < romVersion2) {
               AppLaunchWithCommand(sysFileCDefaultApp,sysAppLaunchCmdNormalLaunch,NULL);
	     }
	   }
         }
         break;

    default:
         break;
  }

  return result;
}
