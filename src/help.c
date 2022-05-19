/*
 * @(#)help.c
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

#define MAX_STACK_SIZE 16
#define MAX_HELP_LINKS 128
#define MAX_MEMORY     32
#define HELP_WIDTH     142
#define HELP_HEIGHT    120
#define SCROLL_HEIGHT  (HELP_HEIGHT - 12)

typedef struct
{
  RectangleType rect;
  UInt16        resID;
} HelpLink;

// globals variable structure
typedef struct
{
  UInt32    romVersion;              // device ROM version
  Boolean   supportColor;            // does the device support color?

  UInt16    card;
  LocalID   dbID;                    // the card, dbID of the resource database

  UInt16    helpResId;
  UInt16    indexScreen;
  Char      *helpString;
  UInt16    helpLength;              // the help string resource

  Boolean   formLoaded;

  Int8      memoryPosition;
  Int8      memoryPositionLast;
  Coord     memoryOffset[MAX_MEMORY];
  UInt16    memory[MAX_MEMORY];      // navigational memory

  struct {

    Char      *helpTitle;
    WinHandle helpWindow;           
    UInt16    helpHeight;

    Int16     helpOffset;
    Int16     lastOffset;

    UInt16    linkCount;
    HelpLink  lstHelpLink[MAX_HELP_LINKS];  
                                     // the help system buffering resources

  } drawing;

} HelpGlobals;

// local routines
static void    FrmHelpOpenTopic(Boolean);
static void    FrmHelpAdjustNavigationButtons();
static Int16   FrmHelpInitialize();
static void    FrmHelpDraw();
static Int16   FrmHelpRenderPart(Coord, Coord, Boolean);
static void    FrmHelpTerminate();

static void    FrmHelpBeam();
static Err     FrmHelpBeamWriteProc(const void *, UInt32 *, void *);

/**
 * FrmHelp() replacement routine.
 *
 * This routine displays a modal help dialog to the user which can 
 * be invoked based on a message id (string resource) containing a 
 * mini text/image based formatting language.
 *
 * @param helpMsgId the string resource containing the help message
 */
void
FrmHelpReplacement(UInt16 helpMsgId)
{
  HelpGlobals *gbls;

  // create the globals object, and register it
  gbls = (HelpGlobals *)MemPtrNew(sizeof(HelpGlobals));
  FtrSet(appCreator, ftrHelpGlobals, (UInt32)gbls);

  // lets assume, old help only :)
  gbls->card               = 0;
  gbls->dbID               = NULL;
  gbls->helpString         = NULL;
  gbls->helpLength         = 0;
  gbls->helpResId          = -1;
  gbls->formLoaded         = false;
  gbls->memoryPosition     = 0;
  gbls->memoryPositionLast = 0;

  //
  // load the first topic
  //

  gbls->indexScreen        = helpMsgId;
  gbls->helpResId          = helpMsgId;
  FrmHelpOpenTopic(false);

  {
    Err err;

    // initialize the help system
    gbls->drawing.helpTitle  = NULL;
#ifdef DOUBLE_BUFFERED
    gbls->drawing.helpWindow =
      WinCreateOffscreenWindow(152,122,screenFormat,&err);
#else
    gbls->drawing.helpWindow = WinGetDisplayWindow();
    err = errNone;
#endif

    // initialize OK: bring up the help dialog
    if ((err == errNone) && (gbls->drawing.helpWindow != NULL)) {
      
      FormActiveStateType frmCurrState;
      FormType            *frmActive      = NULL;
      WinHandle           winDrawWindow   = NULL;
      WinHandle           winActiveWindow = NULL;

      // get the ROM version for the device
      FtrGet(sysFtrCreator, sysFtrNumROMVersion, &gbls->romVersion);

      // lets flag if the device supports color
      if (gbls->romVersion >= romVersion3_5) {
        WinScreenMode(winScreenModeGetSupportsColor,
                      NULL,NULL,NULL,&gbls->supportColor);
      }
      else
        gbls->supportColor = false;

      // save the active form/window
      if (gbls->romVersion >= romVersion3) {
        FrmSaveActiveState(&frmCurrState);
      }
      else {
        frmActive       = FrmGetActiveForm();
        winDrawWindow   = WinGetDrawWindow();
        winActiveWindow = WinGetActiveWindow();  // < palmos3.0, manual work
      }

      {
        EventType     event;
        UInt16        err;
        Boolean       keepFormOpen;
        FormType      *frm;
        ScrollBarType *sclBar;
        UInt16        val, min, max, pge;

        MemSet(&event, sizeof(EventType), 0);

        // load the system help dialog
        frm = FrmInitForm(helpForm);
        FrmSetActiveForm(frm);
        FrmDrawForm(frm);
        gbls->formLoaded = true;

        // get a reference to the scrollbar
        sclBar =
          (ScrollBarType *)FrmGetObjectPtr(frm,
              FrmGetObjectIndex(frm, helpFormScrollBar));
        SclGetScrollBar(sclBar, &val, &min, &max, &pge);

        // open the current topic!
        FrmHelpOpenTopic(true);
        gbls->memoryOffset[gbls->memoryPosition] = 0;
        gbls->memory[gbls->memoryPosition] = gbls->helpResId;
        gbls->memoryPositionLast = gbls->memoryPosition;

        // handle all events here (trap them before the OS does) :)
        keepFormOpen = (gbls->helpString != NULL);
        while (keepFormOpen) {

          EvtGetEvent(&event, evtWaitForever);

          // this is our exit condition
          keepFormOpen = (event.eType != frmCloseEvent);

          if (!SysHandleEvent(&event))
            if (!MenuHandleEvent(0, &event, &err))
              FrmHandleEvent(frm, &event);

          // we have some custom handling to do (for the form)
          switch (event.eType) 
          {
            case appStopEvent:
                 keepFormOpen = false;
                 EvtAddEventToQueue(&event);  
                 break;
                   
            case ctlSelectEvent:

                 switch (event.data.ctlSelect.controlID)
                 {
                    case helpFormBeamButton:
                         FrmHelpBeam();
                         break;

                    case helpFormPrevButton:

                         // can we navigate?
                         if (gbls->memoryPosition > 0) {

                           // save the current status :)
                           gbls->memoryOffset[gbls->memoryPosition] = 
			     gbls->drawing.helpOffset;

                           // switch to the topic
                           gbls->memoryPosition--;
                           gbls->helpResId = gbls->memory[gbls->memoryPosition];
                           FrmHelpOpenTopic(false);

                           // go back to right position
                           gbls->drawing.helpOffset = 
			     gbls->memoryOffset[gbls->memoryPosition];
                           SclGetScrollBar(sclBar, &val, &min, &max, &pge);
			   val = gbls->drawing.helpOffset;
                           SclSetScrollBar(sclBar, val, min, max, pge);
                           FrmHelpDraw();

                           // adjust the buttons
                           FrmHelpAdjustNavigationButtons();
                         }
                         break;

                    case helpFormHomeButton:

                         // switch to the index topic
                         gbls->helpResId = gbls->indexScreen;
                         FrmHelpOpenTopic(true);

                         // reset
                         gbls->memoryPosition     = 0;
                         gbls->memoryPositionLast = 0;

                         // adjust the buttons
                         FrmHelpAdjustNavigationButtons();
                         break;

                    case helpFormNextButton:

                         // can we navigate?
                         if (gbls->memoryPosition < gbls->memoryPositionLast) {

                           // save the current status :)
                           gbls->memoryOffset[gbls->memoryPosition] = 
			     gbls->drawing.helpOffset;

                           // switch to the topic
                           gbls->memoryPosition++;
                           gbls->helpResId = 
                             gbls->memory[gbls->memoryPosition];
                           FrmHelpOpenTopic(false);

                           // go back to right position
                           gbls->drawing.helpOffset = 
			     gbls->memoryOffset[gbls->memoryPosition];
                           SclGetScrollBar(sclBar, &val, &min, &max, &pge);
			   val = gbls->drawing.helpOffset;
                           SclSetScrollBar(sclBar, val, min, max, pge);
                           FrmHelpDraw();

                           // adjust the buttons
                           FrmHelpAdjustNavigationButtons();
                         }
                         break;

                   default:
                         break;
                 }
                 break;

            case sclRepeatEvent:

                 // get the scrollbar value
                 SclGetScrollBar(sclBar, &val, &min, &max, &pge);
                 gbls->drawing.helpOffset = val;

                 // draw the help!
                 FrmHelpDraw();
                 break;

            case penDownEvent:
                 {
		   const RectangleType helpArea = { {3,15}, {HELP_WIDTH, HELP_HEIGHT }};
                   Coord x, y;

                   // our x,y press :)
                   x = event.screenX;
                   y = event.screenY;

                   // user pressed inside area right ;)
                   if (RctPtInRectangle(x,y,&helpArea)) {

                     RectangleType penRect;
                     Int16         i, link;

                     // adjust to "help system" co-ordinates
                     x = event.screenX - 3;
                     y = event.screenY - 15 + gbls->drawing.helpOffset;

                     // lets see if they pressed inside a link
                     link = -1;
                     for (i=0; i<gbls->drawing.linkCount; i++) {
                       if (RctPtInRectangle(x,y,&gbls->drawing.lstHelpLink[i].rect)) {

                         // adjust to "screen"
                         penRect.topLeft.x = 
                           gbls->drawing.lstHelpLink[i].rect.topLeft.x + 3 - 1;
                         penRect.topLeft.y = 
                           gbls->drawing.lstHelpLink[i].rect.topLeft.y + 15 - gbls->drawing.helpOffset;
                         penRect.extent.x = 
                           gbls->drawing.lstHelpLink[i].rect.extent.x + 2; 
                         penRect.extent.y = 
                           gbls->drawing.lstHelpLink[i].rect.extent.y; 
  
                         link = i;
                       }
                     }
  
                     // did we find?
                     if (link != -1) {
 
		       RectangleType clip;
                       Boolean       penDown, inside;
                       Coord         penX, penY;

                       // clip to display
		       WinGetClip(&clip);
		       WinSetClip(&helpArea);

                       penDown = true;
                       inside  = true;

                       // wait until they lift the pen
                       WinInvertRectangle(&penRect,0);
                       while (penDown) {

                         EvtGetPen(&penX, &penY, &penDown);

                         // change of status
                         if (inside != RctPtInRectangle(penX,penY,&penRect)) {
                           inside = RctPtInRectangle(penX,penY,&penRect);
                           WinInvertRectangle(&penRect,0);
                         }
                       }

                       if (inside) WinInvertRectangle(&penRect,0);

                       // restore clipping
                       WinSetClip(&clip);

                       // if the pen is still inside, open link
                       if (inside) {

                         // save the current status :)
                         gbls->memoryOffset[gbls->memoryPosition] = 
                           gbls->drawing.helpOffset;

                         // switch to a new topic
                         gbls->helpResId = 
                           gbls->drawing.lstHelpLink[link].resID; 
                         FrmHelpOpenTopic(true);

                         // adjust the "memory"
                         {
                           // do we need to shift things?
                           if (gbls->memoryPosition == (MAX_MEMORY-1)) {
                             MemMove(&gbls->memory[0], 
                                     &gbls->memory[1], 
                                     (MAX_MEMORY-1) * sizeof(gbls->memory[0]));
                             MemMove(&gbls->memoryOffset[0], 
                                     &gbls->memoryOffset[1], 
                                     (MAX_MEMORY-1) * sizeof(gbls->memoryOffset[0]));
                           }
                           else
                             gbls->memoryPosition++;

                           gbls->memoryOffset[gbls->memoryPosition] = 0;
                           gbls->memory[gbls->memoryPosition] = gbls->helpResId;
                           gbls->memoryPositionLast = gbls->memoryPosition;
                         }

                         // adjust the buttons
                         FrmHelpAdjustNavigationButtons();
                       }
                     }
                   }
                 }
                 break;

            case keyDownEvent:

                 switch (event.data.keyDown.chr)
                 {
                   case pageUpChr:

                        // get the scrollbar value
                        SclGetScrollBar(sclBar, &val, &min, &max, &pge);
                        val = (SCROLL_HEIGHT > val) 
			      ? 0 : (val-SCROLL_HEIGHT);
                        SclSetScrollBar(sclBar, val, min, max, pge);
                        gbls->drawing.helpOffset = val;

                        // draw the help!
                        FrmHelpDraw();
                        break;

                   case pageDownChr:

                        // get the scrollbar value
                        SclGetScrollBar(sclBar, &val, &min, &max, &pge);
                        val = (max < (val+SCROLL_HEIGHT)) 
                              ? max : (val+SCROLL_HEIGHT);
                        SclSetScrollBar(sclBar, val, min, max, pge);
                        gbls->drawing.helpOffset = val;

                        // draw the help!
                        FrmHelpDraw();
                        break;

                   default:
                        break;
                 }
                 break;

            default:
                 break;
          }
        }

        // shut down the help system
        FrmHelpTerminate();
        FrmEraseForm(frm);
        FrmDeleteForm(frm);
      }

      // restore the active form/window
      if (gbls->romVersion >= romVersion3) {
        FrmRestoreActiveState(&frmCurrState);
      }
      else {
        FrmSetActiveForm(frmActive);
        WinSetDrawWindow(winDrawWindow);
        WinSetActiveWindow(winActiveWindow);     // < palmos3.0, manual work
      }

#ifdef DOUBLE_BUFFERED
      WinDeleteWindow(gbls->drawing.helpWindow, false);
#endif
    }

    // free memory used for help string
    MemPtrFree(gbls->helpString);
  }

  // free memory used
  MemPtrFree(gbls);
  FtrUnregister(appCreator, ftrHelpGlobals);
}

/**
 * Load the current topic.
 *
 * @param draw should we draw the offscreen buffer to the screen?
 */
static void    
FrmHelpOpenTopic(Boolean draw)
{
  HelpGlobals *gbls;
  UInt16      appCard;
  LocalID     appdbID;

  UInt16      helpLength;
  Char        *helpString;

  // get a globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // lets get the current application details
  SysCurAppDatabase(&appCard, &appdbID);

  // assume, we didn't find it
  helpString = NULL;
  helpLength = 0;

  if (helpString == NULL) {

    DmOpenRef dbRef;

    // try and open the application resource
    dbRef = DmOpenDatabase(appCard, appdbID, dmModeReadOnly);
    if (dbRef != NULL) {

      // can we locate the help resource?
      if (DmFindResource(dbRef, hlpString, gbls->helpResId, NULL) != -1) {

        gbls->card = appCard;
        gbls->dbID = appdbID;

        // make a copy of the help resource string
        {
          MemHandle hlpH;

          hlpH = DmGet1Resource(hlpString, gbls->helpResId);
          helpLength = MemHandleSize(hlpH);
          helpString = 
            (UInt8 *)MemPtrNew(sizeof(UInt8) * helpLength+1);
          if (helpString != NULL) {
            MemMove(helpString, MemHandleLock(hlpH), helpLength);
            MemHandleUnlock(hlpH);
            DmReleaseResource(hlpH);
            helpString[helpLength] = 0; 
          }
        }

      }

      // close database
      DmCloseDatabase(dbRef);
    } 
  }

  // did we find the topic?
  if (helpString != NULL) {

    // clean up previous help topic (if needed)
    if (gbls->helpString != NULL) MemPtrFree(gbls->helpString);
    gbls->helpString = helpString;
    gbls->helpLength = helpLength;

    // ok, lets try and load this baby :)
    if (gbls->formLoaded) {

      FormType      *frm;
      ScrollBarType *sclBar;
      UInt16        val, min, max, pge;

      // get the form references
      frm = FrmGetActiveForm();

      // initialize the help system
      gbls->drawing.helpHeight = FrmHelpInitialize();

      // adjust the frame title
      if (gbls->drawing.helpTitle != NULL) {
        FrmSetTitle(frm, gbls->drawing.helpTitle);
      }
      else
        FrmSetTitle(frm, "Tips");

      // adjust the scroll bar
      sclBar =
        (ScrollBarType *)FrmGetObjectPtr(frm,
           FrmGetObjectIndex(frm, helpFormScrollBar));

      SclGetScrollBar(sclBar, &val, &min, &max, &pge);
      val = gbls->drawing.helpHeight;
      max = (val > HELP_HEIGHT) ? (val-(HELP_HEIGHT)) : 0;
      SclSetScrollBar(sclBar, 0, min, max, pge);

      // draw the help!
      if (draw) FrmHelpDraw();
    }
  }
}

/**
 * Adjust the navigation buttons based on the current memory status.
 */
static void    
FrmHelpAdjustNavigationButtons()
{
  HelpGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // the previous button
  {
    MemHandle bitmapHandle = DmGet1Resource('Tbmp', 
      (gbls->memoryPosition == 0) 
         ? bitmapPrevOff : bitmapPrev);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 112, 140);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);
  }

  // the next button
  {
    MemHandle bitmapHandle = DmGet1Resource('Tbmp', 
      (gbls->memoryPosition == gbls->memoryPositionLast) 
        ? bitmapNextOff : bitmapNext);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 140, 140);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);
  }
}

#include "helpdraw.c"

/**
 * Beam the application via the Exchange API.
 */
void
FrmHelpBeam()
{
  UInt16        card;
  LocalID       dbID;
  ExgSocketType socket;
  Err           err, e;

  // lets get the current application details
  SysCurAppDatabase(&card, &dbID);

  // configure
  MemSet(&socket, sizeof(ExgSocketType), 0);
  socket.target      = sysFileCLauncher;
  socket.description = "PalmSource 2000 Conference Guide";
  socket.name        = "palmsource.prc";

  err = errNone;
  e = ExgPut(&socket); err |= e;
  if (err == errNone) {
    e = ExgDBWrite(FrmHelpBeamWriteProc, &socket, NULL, dbID, card); err |= e;
    e = ExgDisconnect(&socket, err); err |= e;
  }

  // did everything go ok?
  if (err != errNone)
    SndPlaySystemSound(sndError);
}

/**
 * ExgDBWrite callback function.
 *
 * @param data the data to send.
 * @param size the size of the packet.
 * @param userData the protocol.
 * @return errNone if successful, other otherwise.
 */
static Err
FrmHelpBeamWriteProc(const void *data,
                     UInt32     *size,
                     void       *userData)
{
  Err err = errNone;

  // send it over!
  *size = ExgSend((ExgSocketType *)userData, (void *)data, *size, &err);

  return err;
}
