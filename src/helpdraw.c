/*
 * @(#)helpdraw.c
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

typedef struct
{
  UInt8 data[MAX_STACK_SIZE];
  UInt8 index;
} RenderStack;

typedef struct
{
  RenderStack font;
  RenderStack color;
  RenderStack back;
  UInt8       align;      //0 left 1 rigth 2 center 3 justify
  UInt8       underline;  //0 none 1 dotted
  UInt8       vspaceing;
  Boolean     escape;
} DrawState;

typedef struct
{
  DrawState   draw;
  char        *StringPos;
  Int16       CurrentY;
  Boolean     LinkOpen;
  UInt8       pass;
} LineState;

static UInt8 CharToInt(char *h);
static UInt8 HexToInt(char *h);
static void  WinClipDrawBitmap(BitmapType *,Coord,Coord,int,int,UInt32);

/**
 * Initialize the help system display.
 *
 * @return the number of pixels required to draw the whole help, < 0 if error
 */
static Int16  
FrmHelpInitialize()
{
  HelpGlobals *gbls;
  Int16       result;

  // get a globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // lets assume the help is ok
  result = 0;

  // initialize
  if (gbls->drawing.helpTitle != NULL)
    MemPtrFree(gbls->drawing.helpTitle);
  gbls->drawing.helpTitle  = NULL;
  gbls->drawing.helpOffset = 0;
  gbls->drawing.lastOffset = -1;
  gbls->drawing.linkCount = 0;

  // lets determine the height of the help
  result = FrmHelpRenderPart(16000,16000,true);

  return result;
}

/**
 * Draw the help screen display.
 *
 * @param offset the y-coordinate value to offset the drawing.
 */
static void
FrmHelpDraw()
{
#ifdef DOUBLE_BUFFERED 
  const RectangleType helpArea = {{ 0, 0}, { HELP_WIDTH, HELP_HEIGHT}};

  RectangleType source = {{0,0},{HELP_WIDTH,HELP_HEIGHT}};
  RectangleType del = {{0,0},{HELP_WIDTH,HELP_HEIGHT}};
  HelpGlobals *gbls;
  WinHandle   currWindow;
  int           dest;
  int y1;
  int y2;
  int diff;

  // get a globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // draw the help
  currWindow = WinGetDrawWindow();
  WinSetDrawWindow(gbls->drawing.helpWindow);

  diff=gbls->drawing.lastOffset - gbls->drawing.helpOffset;

  if ((gbls->drawing.lastOffset >= 0) && (abs(diff) < HELP_HEIGHT)) {

    source.extent.y=HELP_HEIGHT-abs(diff);
    del.extent.y=abs(diff);
    if (diff>0) {

      //copy down
      source.topLeft.y=0;
      dest=abs(diff);
      del.topLeft.y=0;
      y1=0;
      y2=abs(diff);
    }
    else if (diff<0) {

      //copy up
      source.topLeft.y=abs(diff);
      dest=0;
      y1=HELP_HEIGHT-abs(diff);
      y2=HELP_HEIGHT;
      del.topLeft.y=y1;
    }

    else {
      y1=0;
      y2=0;
      dest=0;
    }

    if (y1!=y2)
      WinCopyRectangle(gbls->drawing.helpWindow,gbls->drawing.helpWindow,&source,0,dest,winPaint);
  }

  else {

    //render everything
    y1=0;
    y2=HELP_HEIGHT;
  }

  if (gbls->supportColor)
    WinSetBackColor(UIColorGetTableEntryIndex(UIDialogFill));
  WinEraseRectangle(&del,0);

  if (y2 > y1)
    FrmHelpRenderPart(y1,y2,false);

  WinSetDrawWindow(currWindow);

  // blit the changes to the screen
  WinCopyRectangle(gbls->drawing.helpWindow, currWindow,
                   &helpArea, 3, 15, winPaint);

  // keep track of it
  gbls->drawing.lastOffset = gbls->drawing.helpOffset;

#else

  const RectangleType helpArea = {{ 5, 17}, { HELP_WIDTH, HELP_HEIGHT}};
  RectangleType source = {{0+5,0+17},{HELP_WIDTH,HELP_HEIGHT}};
  RectangleType del = {{0+5,0+17},{HELP_WIDTH,HELP_HEIGHT}};
  HelpGlobals *gbls;
  WinHandle   currWindow;
  int           dest;
  int y1;
  int y2;
  int diff;

  // get a globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // draw the help
  currWindow = WinGetDrawWindow();
  WinSetDrawWindow(gbls->drawing.helpWindow);

  diff=gbls->drawing.lastOffset - gbls->drawing.helpOffset;

  if ((gbls->drawing.lastOffset >= 0) && (abs(diff) < HELP_HEIGHT)) {

    source.extent.y=HELP_HEIGHT-abs(diff);
    del.extent.y=abs(diff);
    if (diff>0) {

      //copy down
      source.topLeft.y=0+17;
      dest=abs(diff);
      del.topLeft.y=0+17;
      y1=0;
      y2=abs(diff);
    }
    else if (diff<0) {

      //copy up
      source.topLeft.y=abs(diff)+17;
      dest=0;
      y1=HELP_HEIGHT-abs(diff);
      y2=HELP_HEIGHT;
      del.topLeft.y=y1+17;
    }

    else {
      y1=0;
      y2=0;
      dest=0;
    }

    if (y1!=y2) {
      WinSetClip(&helpArea);
      WinCopyRectangle(gbls->drawing.helpWindow,gbls->drawing.helpWindow,&source,0+5,dest+17,winPaint);
      WinResetClip();
    }
  }

  else {

    //render everything
    y1=0;
    y2=HELP_HEIGHT;
  }

  if (gbls->supportColor)
    WinSetBackColor(UIColorGetTableEntryIndex(UIDialogFill));
  WinEraseRectangle(&del,0);

  WinSetClip(&helpArea);
  if (y2 > y1)
    FrmHelpRenderPart(y1,y2,false);
  WinResetClip();

  WinSetDrawWindow(currWindow);

  // keep track of it
  gbls->drawing.lastOffset = gbls->drawing.helpOffset;
#endif
}

//help functions for rgb hex conversion
static UInt8
CharToInt(Char *h)
{
  UInt8 result = 0;

  if ((*h >= '0') && (*h<='9'))       result = (UInt8)(*h-'0');
  else  if ((*h >= 'a') && (*h<='f')) result = (UInt8)(*h-'a'+10);
  else  if ((*h >= 'A') && (*h<='F')) result = (UInt8)(*h-'A'+10);

  return result;
}

static void 
WinClipDrawBitmap(BitmapType *bitmapP, 
                  Coord x, Coord y,
                  int maxx, int maxy,
                  UInt32 romVersion)
{
  WinHandle off;
  UInt16 error;
  WinHandle oldwin;
  RectangleType area; // = {{0,0},{160-8,122}};

  if ( (romVersion < romVersion3_3)&& ((x<0) || (y<0) || (x+bitmapP->width >maxx) || (y+bitmapP->height >maxy)))  {
    //do clipping
    off=WinCreateOffscreenWindow(bitmapP->width,bitmapP->height,screenFormat,&error);
    if (off != NULL) {

      oldwin=WinGetDrawWindow();
      WinSetDrawWindow(off);
      WinDrawBitmap (bitmapP, 0, 0);

      WinSetDrawWindow(oldwin);

      //x clipping
      if (x<0) {
        area.topLeft.x=-x;
        area.extent.x=bitmapP->width + x;
        x=0;
      }
      else {
        area.topLeft.x=0;
        area.extent.x=bitmapP->width;
      }
      if ((area.topLeft.x + area.extent.x) > maxx)
        area.extent.x=maxx-area.topLeft.x;

      //y clipping
      if (y<0) {
        area.topLeft.y=-y;
        area.extent.y=bitmapP->height + y;
        y=0;
      }
      else {
        area.topLeft.y=0;
        area.extent.y=bitmapP->height;
      }
      if ((area.topLeft.y + area.extent.y) > maxy)
        area.extent.y=maxx-area.topLeft.y;

#ifdef DOUBLE_BUFFERED 
      WinCopyRectangle(off,oldwin,&area,x,y,winPaint);
#else
      WinCopyRectangle(off,oldwin,&area,x+5,y+17,winPaint);
#endif

      WinDeleteWindow(off, false);
    }
  }
  else
#ifdef DOUBLE_BUFFERED 
    WinDrawBitmap (bitmapP, x, y);
#else
    WinDrawBitmap (bitmapP, x+5, y+17);
#endif
}


static UInt8
HexToInt(Char *h)
{
  return (CharToInt(h)*16)+CharToInt(h+1);
}

/**
 * Render a portion of the help screen.
 *
 * @param screenY1 the starting y-value (line)
 * @param screenY2 the ending y-value (line)
 * @param init initialize links and calculate height
 * @return the number of pixels required draw, < 0 if error
 */
static Int16
FrmHelpRenderPart(Coord screenY1, Coord screenY2, Boolean init)
{
  HelpGlobals *gbls;
  DmOpenRef   dbRef;
  Int16       result;

  //state information
  DrawState   state;          //current drawing state
  LineState   line;           //save of line state for second pass
  char       *StringPos;      //current position in string
  char       *StringEnd;      //end of region in string
  Coord       CurrentX;       //current x pos
  Coord       CurrentY;       //current y pos
  Boolean     LinkOpen=false; //link tag currenty open?

  //line drawing
  Coord      LineHeight=0;  //Height of line (maximum of all atoms in the line
  Coord      AtomWidth=0;   //Width of current Atom
  Coord      AtomHeight=0;  //Height of current Atom
  Boolean    Indent=false;  //is currently a indent set
  Coord      IndentX1=0;    //indent left
  Coord      IndentX2=0;    //indent right
  Coord      IndentY=0;     //position to stop indent (after image)
  Boolean    ChangeLine;    //Change to next line
  Boolean    drawn=false;   //second pass required

  //alignment data
  Coord      xadd;              //for alignment
  Coord      xadd2;             //for alignment
  Coord      space=0;           //correct alignment for space at end  //changed
  UInt16     spacecount=0;      //number of spaces in line  //changed
  UInt16     andcount=0;        //number of & in line
  float      spacejustify=0;    //for text justify
  Boolean    fit;               //dose atom fit in line
  Boolean    dontjustify=false; //dont justify this line (last line of justifed paragraph)

  //image handling
  MemHandle  imageH;        //handle
  BitmapType *imageP;       //pointer
  char       mode;          //image alignment

  //help vars
  UInt16        num;           //for parameter conversion
  RGBColorType  rgb;           //for rgb conversion
  RectangleType rc;
  char          command;       //command letter
  UInt8         BackIndex;     //Index of background color (to save space drawing in color)
  Boolean       dofirst;       //init first pass?
  Boolean       wordescape;    //save escape state
  Int16         ystart;
  HelpLink      *link;

  // get a globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // lets assume rendering is ok
  result = 0;

  // open the help resource database (images etc to be found here)
  dbRef = DmOpenDatabase(gbls->card, gbls->dbID, dmModeReadOnly);

  // code here Jens :)
  //
  // - do your parsing and drawing
  // - just read the resources (the correct DB is open now)

  //init draw state
  state.font.data[0]=0;
  state.font.index=0;
  state.align=0;
  state.underline=0;
  state.vspaceing=0;
  state.escape=false;
  if (gbls->supportColor)  {

    state.color.data[0]=UIColorGetTableEntryIndex(UIFieldText);
    state.color.index=0;
    state.back.data[0]=UIColorGetTableEntryIndex(UIDialogFill);
    state.back.index=0;
  }

  //init other vars
  CurrentX=0;      //start drawing left

  if (init)
    ystart=0;
  else
    ystart=gbls->drawing.helpOffset;

  CurrentY=-ystart; //start at negative value, drawing starts if helpy>=screenY1
  xadd=0;
  IndentX1=0;
  IndentX2=HELP_WIDTH;
  Indent=false;
  line.pass=0;
  LinkOpen=false;
  if (gbls->supportColor) 
    BackIndex=UIColorGetTableEntryIndex(UIDialogFill);
  else
    BackIndex=0;
  LineHeight=state.vspaceing;

  //set palmos drawing state
  FntSetFont(state.font.data[0]);
  WinSetUnderlineMode(noUnderline);
  if (gbls->supportColor) {

    WinSetTextColor(state.color.data[0]);
    WinSetForeColor(state.color.data[0]);
    WinSetBackColor(state.back.data[0]);
  }

  StringPos=gbls->helpString;
  do {

    ChangeLine=false;

    //check for  command
    if ((*StringPos == '<') && (!state.escape)) {

      AtomHeight=0;

      //get command
      command=*(StringPos+1);
      //Set current pos to start of command params
      StringPos+=2;
      //Find end of tag
      StringEnd=StringPos;
      while ((*StringEnd != '>') && (*StringEnd != 0)) StringEnd++;

      switch (command)
      {
        case 't' :
             // title
             // free if existing
             if (gbls->drawing.helpTitle != NULL) {

               MemPtrFree(gbls->drawing.helpTitle);
               gbls->drawing.helpTitle=NULL;
             }

             // max length of title 32 chars
             if (StringEnd-StringPos <= 32) {
               gbls->drawing.helpTitle=MemPtrNew(StringEnd-StringPos+1);
               MemSet(gbls->drawing.helpTitle,StringEnd-StringPos+1,0);
               MemMove(gbls->drawing.helpTitle,StringPos,StringEnd-StringPos);
             }
             break;
        case 'l' :
             //link
             if ( gbls->drawing.linkCount < MAX_HELP_LINKS ) {

               LinkOpen=true;
               if (init) {

                 drawn=true;
                 if (line.pass !=1) {

                   num=StrAToI(StringPos);

                   //adjust for justified text?
                   if ( (state.align == 3) && (line.pass==2) )
                     xadd=spacecount*spacejustify;
                     //xadd=fxtoi(Mulfx(itofx(spacecount),spacejustify));

                   link=&gbls->drawing.lstHelpLink[gbls->drawing.linkCount];
                   link->rect.topLeft.x=CurrentX+xadd;
                   link->rect.topLeft.y=CurrentY+ystart;
                   link->resID=num;
                 }
               }
             }
             break;

        case 'f':  //font
             if (state.font.index<MAX_STACK_SIZE-1) {

               num=StrAToI(StringPos);
               state.font.index++;
               state.font.data[state.font.index]=num;
               FntSetFont(num);
               AtomHeight=FntCharHeight();
             }
             break;
        case 'u':
             //underline
             state.underline=1;
             WinSetUnderlineMode(grayUnderline);
             break;
        case 'c' :
             //font color
             if  ((gbls->supportColor) && (state.color.index<MAX_STACK_SIZE-1) ) {
               //check correct format
               if (StringEnd-StringPos == 6) {
                 rgb.r=HexToInt(StringPos);
                 rgb.g=HexToInt(StringPos+2);
                 rgb.b=HexToInt(StringPos+4);
                 num=WinRGBToIndex(&rgb);

                 state.color.index++;
                 state.color.data[state.color.index]=num;
                 WinSetTextColor(num);
                 WinSetForeColor(num);
               }
             }
             break;
        case 'b' :
             //back color
             if  (gbls->supportColor && (state.back.index<MAX_STACK_SIZE-1)) {

               //check correct size
               if (StringEnd-StringPos == 6) {
                 rgb.r=HexToInt(StringPos);
                 rgb.g=HexToInt(StringPos+2);
                 rgb.b=HexToInt(StringPos+4);
                 num=WinRGBToIndex(&rgb);

                 state.back.index++;
                 state.back.data[state.back.index]=num;
                 WinSetBackColor(num);
               }
             }
             break;
        case 'i' :
             //image
             //aligned image?
             if (StringEnd-StringPos <= 8) {
               if ((*StringPos == 'l')  || (*StringPos == 'r')) {
                 mode=*StringPos;
                 StringPos++;
               }
               else
                 mode='0';

               // open image
               num=StrAToI(StringPos);
               imageH=DmGetResource('Tbmp', num);
               if (imageH !=NULL) {
                 imageP = MemHandleLock(imageH);

                 //aligned image
                 if ((mode == 'l') || (mode == 'r')) {
                   if (CurrentX != IndentX1) {
                     //we are not at beginning of a new line
                     ChangeLine=true;
                     dontjustify=true;
                     StringEnd=StringPos-4; //try again next line
                   }
                   else {
                     if (Indent)
                        CurrentY=IndentY;
                     if (mode=='l') {
                       IndentX1=imageP->width+4;
                       IndentX2=HELP_WIDTH;
                     }
                     else  {
                       IndentX1=0;
                       IndentX2=HELP_WIDTH-imageP->width-4;
                     }
                     IndentY=CurrentY+imageP->height;
                     Indent=true;
                     CurrentX=IndentX1;

                     //part of image visible?
                     if (CurrentY+imageP->height>=screenY1) {
                       if (line.pass != 1) {
                         if (mode=='l')
                           WinClipDrawBitmap(imageP, 0, CurrentY+LineHeight,HELP_WIDTH,HELP_HEIGHT,gbls->romVersion);
                         else
                           WinClipDrawBitmap(imageP,HELP_WIDTH-imageP->width, CurrentY+LineHeight,HELP_WIDTH,HELP_HEIGHT,gbls->romVersion);
                       }
                       else
                         drawn=true;
                     }
                   }
                 }
                 else {
                   //image in text
                   //does it fit?
                   if ((CurrentX+imageP->width<IndentX2) || (CurrentX == 0 )) {
                     AtomHeight=imageP->height;
                     AtomWidth=imageP->width;
                     if (CurrentY+AtomHeight>=screenY1) {
                       if ( (state.align == 3) && (line.pass==2) )
                         xadd=spacecount*spacejustify;
                         //xadd=fxtoi(Mulfx(itofx(spacecount),spacejustify));

                       if (line.pass != 1)
                         WinClipDrawBitmap(imageP, CurrentX+xadd, CurrentY,HELP_WIDTH,HELP_HEIGHT,gbls->romVersion);
                       else
                         drawn=true;
                     }
                     CurrentX+=AtomWidth;
                   }
                   else {
                     ChangeLine=true;
                     //image did not fit, assume current font height for return
                     AtomHeight=FntCharHeight();
                     StringEnd=StringPos-3; //try again next line
                   }
                 }
                 MemHandleUnlock(imageH);
                 DmReleaseResource(imageH);
               }
             }
             break;

        case 'a':
             //align
             if ((*StringPos) == 'l')
               num=0;
             else if ((*StringPos) == 'r')
               num=1;
             else if ((*StringPos) == 'c')
               num=2;
             else if ((*StringPos) == 'j')
               num=3;
             else
               num=0;

             if (num != 3)
               dontjustify=true;

             state.align=num;
             ChangeLine=true;
             break;
        case 's':
             // separator
             AtomHeight=LineHeight+8;
             if (AtomHeight>LineHeight)
                LineHeight=AtomHeight;

             if (CurrentY+AtomHeight>=screenY1)  {
               if (line.pass != 1)
#ifdef DOUBLE_BUFFERED 
                 WinDrawLine(4,CurrentY+LineHeight-4,HELP_WIDTH-4,CurrentY+LineHeight-4);
#else
                 WinDrawLine(4+5,CurrentY+LineHeight-4+17,HELP_WIDTH-4+5,CurrentY+LineHeight-4+17);
#endif
               else
                 drawn=true;
             //drawn=true;    
             //ChangeLine=true;

             }
             break;
        case 'x':
             // absolut x
             if (StringEnd-StringPos <= 3) {
               num=StrAToI(StringPos);
               if ((num>=0) && (num<=159))
                 CurrentX=num;
             }
             break;
        case 'h':
             // horizontal space
             if (StringEnd-StringPos <= 3) {
               num=StrAToI(StringPos);
               if ((num>=0) && (num<=159))
                 CurrentX+=num;
             }
             break;
        case 'v':
             // vertical space/line height
             if (StringEnd-StringPos <= 3) {
               num=StrAToI(StringPos);
               if ((num>=0) && (num<=159))
               state.vspaceing=num;
             }
             break;
        case '/':
             // end tag
             switch (*StringPos)
             {
                case 'l':
                     // link
                     if (init)
                       if ((LinkOpen) && (line.pass !=1)) {
                         if ( (state.align == 3) &&  (line.pass==2) )
                            xadd=spacecount*spacejustify;
                            //xadd=fxtoi(Mulfx(itofx(spacecount),spacejustify));

                         link=&gbls->drawing.lstHelpLink[gbls->drawing.linkCount];
                         link->rect.extent.x=(CurrentX+xadd)-link->rect.topLeft.x;
                         link->rect.extent.y=(CurrentY+LineHeight+ystart)-link->rect.topLeft.y;
                         gbls->drawing.linkCount++;
                       }
                     LinkOpen=false;
                     break;
                case 'f':
                     // font
                     if (state.font.index>0) {
                       state.font.index--;
                       FntSetFont(state.font.data[state.font.index]);
                       AtomHeight=FntCharHeight();
                     }
                     break;
                case 'c' :
                     //color
                     if ((gbls->supportColor) && (state.color.index>0)){
                       state.color.index--;
                       WinSetTextColor(state.color.data[state.color.index]);
                       WinSetForeColor(state.color.data[state.color.index]);
                     }
                     break;
                case 'b' :
                     //back color
                     if ((gbls->supportColor) && (state.back.index>0)) {
                       state.back.index--;
                       WinSetBackColor(state.back.data[state.back.index]);
                     }
                     break;
                case 'a':
                     //align
                     state.align=0;
                     dontjustify=true;
                     ChangeLine=true;
                     break;
                case 'i' :
                     //image
                     ChangeLine=true;
                     dontjustify=true;
                     drawn=true;
                     if (line.pass !=1) {
                       //already last line?
                       if (IndentY>=CurrentY+LineHeight) {
                         //no, pretend being at end of picture
                         CurrentY=IndentY;
                         LineHeight=0;
                       }
                     }
                     break;
                case 'u' :
                     //underline
                     state.underline=0;
                     WinSetUnderlineMode(noUnderline);
                     break;
                case 'v' :
                     //line height
                     state.vspaceing=0;
                     break;
             }
             break; //   end tag
      }
      if (AtomHeight>LineHeight)
        LineHeight=AtomHeight;
      StringPos=StringEnd;
      if (*StringPos!=0)
        StringPos++;
    }
    else if (*StringPos == '\n') {
      //return
      if (LineHeight==0)
        LineHeight=FntCharHeight();

      dontjustify=true;
      ChangeLine=true;
      StringPos++;
    }
    else if ((*StringPos == '&') && (!state.escape)) {
      //escape
      andcount++;
      state.escape=true;
      StringPos++;
    }
    else if (*StringPos == ' ') {
      //space
      state.escape=false;
      spacecount++;
      space=FntCharWidth(' ');
      AtomHeight=FntCharHeight();

      if ((state.align == 3) &&  (line.pass==2) ) {
        xadd=spacecount*spacejustify;
        //xadd=fxtoi(Mulfx(itofx(spacecount),spacejustify));
        xadd2=(spacecount-1)*spacejustify;
        //xadd2=fxtoi(Mulfx(itofx(spacecount-1),spacejustify));
      }
      else
        xadd2=xadd;

      if ((CurrentY+AtomHeight>=screenY1) && (CurrentX + space + xadd<= IndentX2) ) {
        if (line.pass != 1) {
          //draw space?
          if (gbls->supportColor && (BackIndex != state.back.data[state.back.index])) {
            rc.topLeft.x = CurrentX+xadd2;
            rc.topLeft.y = CurrentY;
            rc.extent.x = space+xadd-xadd2;
            rc.extent.y = AtomHeight;
            WinEraseRectangle(&rc,0);
          }

          //underline
          if (LinkOpen)
#ifdef DOUBLE_BUFFERED 
            WinDrawLine(CurrentX+xadd2,CurrentY+AtomHeight-1,CurrentX+space+xadd,CurrentY+AtomHeight-1);
#else
            WinDrawLine(CurrentX+xadd2+5,CurrentY+AtomHeight-1+17,CurrentX+space+xadd+5,CurrentY+AtomHeight-1+17);
#endif
          else if (state.underline==1)
#ifdef DOUBLE_BUFFERED 
            WinDrawGrayLine(CurrentX+xadd2,CurrentY+AtomHeight-1,CurrentX+space+xadd,CurrentY+AtomHeight-1);
#else
            WinDrawGrayLine(CurrentX+xadd2+5,CurrentY+AtomHeight-1+17,CurrentX+space+xadd+5,CurrentY+AtomHeight-1+17);
#endif
        }
        else
          drawn=true;
      }

      CurrentX+=space;
      if (AtomHeight>LineHeight)
        LineHeight=AtomHeight;

      StringPos++;
    }
    else {
      // normal text

      //find word
      StringEnd=StringPos;
      AtomWidth=0;
      wordescape=state.escape;
      while ((*StringEnd != '\n') && ((*StringEnd != '<') || (wordescape))  && ((*StringEnd != '&') || (wordescape))  && (*StringEnd != ' ') && (*StringEnd !=0) && (CurrentX+AtomWidth<IndentX2 )) {

        wordescape=false;
        AtomWidth+=FntCharWidth(*StringEnd);
        StringEnd++;
      }

      fit=(CurrentX+AtomWidth) < IndentX2;
      //if it does not fit and there are no spaces draw anyway and split
      if ((!fit) && (spacecount==0) && (andcount==0)) {

        ChangeLine=true;
        fit=true;
        StringEnd--;
        AtomWidth-=FntCharWidth(*StringEnd);
      }

      //does the word fit
      if (fit) {
        // yes,
        state.escape=false;
        AtomHeight=FntCharHeight();
        if (AtomHeight>LineHeight)
          LineHeight=AtomHeight;

        if ((state.align == 3) && (line.pass==2))
          xadd=spacecount*spacejustify;
          //xadd=fxtoi(Mulfx(itofx(spacecount),spacejustify));

        //draw the word?
        if (CurrentY+AtomHeight>=screenY1) {
          if (line.pass != 1) {
#ifdef DOUBLE_BUFFERED 
            WinDrawChars(StringPos,StringEnd-StringPos,CurrentX+xadd,CurrentY);
#else
            WinDrawChars(StringPos,StringEnd-StringPos,CurrentX+xadd+5,CurrentY+17);
#endif
            //underline link
            if (LinkOpen)
#ifdef DOUBLE_BUFFERED 
              WinDrawLine(CurrentX+xadd,CurrentY+AtomHeight-1,CurrentX+xadd+AtomWidth-1,CurrentY+AtomHeight-1);
#else
              WinDrawLine(CurrentX+xadd+5,CurrentY+AtomHeight-1+17,CurrentX+xadd+AtomWidth-1+5,CurrentY+AtomHeight-1+17);
#endif
          }
          else
            drawn=true;
        }
        CurrentX+=AtomWidth;
        StringPos=StringEnd;
        space=0;
      }
      else {
        //if word does not fit, try next line
        ChangeLine=true;
      }
    }

    if ((ChangeLine) || (*StringPos==0))
    {
      //split multiline links
      //close open link
      if (init)
        if ((LinkOpen) && (line.pass !=1)) {
          if ((state.align == 3) &&  (line.pass==2))
            xadd=spacecount*spacejustify;
            //xadd=fxtoi(Mulfx(itofx(spacecount),spacejustify));

          link=&gbls->drawing.lstHelpLink[gbls->drawing.linkCount];

          link->rect.extent.x=(CurrentX+xadd-space)-link->rect.topLeft.x;
          if ((link->rect.topLeft.x+link->rect.extent.x) > HELP_WIDTH)
            link->rect.extent.x=HELP_WIDTH-link->rect.topLeft.x;
          link->rect.extent.y=(CurrentY+LineHeight+ystart)-link->rect.topLeft.y;
          gbls->drawing.linkCount++;
        }

      if ( (state.align != 0) || (line.pass==1) ) {
        dofirst=true;
        if (line.pass==1) {

          dofirst=false;
          //first pass done
          if (drawn) {
            line.pass=2; // do second pass

            //restore state at begin of line
            StringPos=line.StringPos;
//          state=line.draw;
            MemMove(&state,&line.draw,sizeof(DrawState));
            LinkOpen=line.LinkOpen;

            FntSetFont(state.font.data[state.font.index]);
            if (gbls->supportColor) {
              WinSetTextColor(state.color.data[state.color.index]);
              WinSetForeColor(state.color.data[state.color.index]);
              WinSetBackColor(state.back.data[state.back.index]);
            }

            if (state.underline==0)
              WinSetUnderlineMode(noUnderline);
            else if (state.underline==1)
              WinSetUnderlineMode(grayUnderline);

              //calculate align
            if (state.align == 1)
              xadd=IndentX2-(CurrentX-space);
            else if (state.align == 2)
              xadd=(IndentX2-(CurrentX-space))/2;
            else if (state.align == 3) {
              if (!dontjustify) {
                if (space>0)
                  spacecount--;
                spacejustify=((float)IndentX2-((float)CurrentX-(float)space))/spacecount;
                //spacejustify=itofx(IndentX2) - Divfx(itofx(CurrentX-space),itofx(spacecount));
              }
              else
                spacejustify=0;
              xadd=0;
            }
            CurrentX=IndentX1;  //start second pass left
            LineHeight=state.vspaceing;
          }
          else {
            line.pass=2; //no drawing needed, pretend pass 2 is done
            dofirst=true;
          }
        }

        if (dofirst) {
          dontjustify=false;
          if (state.align !=0)
             line.pass=1; //do first pass
          else
             line.pass=0; //normal text
          //remember state
          line.StringPos=StringPos;
//        line.draw=state;
          MemMove(&line.draw,&state,sizeof(DrawState));
          line.LinkOpen=LinkOpen;
          drawn=false;

          //do a return
          xadd=0;
          CurrentY+=LineHeight;
          CurrentX=IndentX1;
          LineHeight=state.vspaceing;
        }
      }
      else {
        //normal return
        CurrentY+=LineHeight;
        CurrentX=IndentX1;
        LineHeight=state.vspaceing;
        line.pass=0;
        xadd=0;
      }
      //FrmCustomAlert(alertID_text2,"y:",help,NULL);
      spacecount=0;
      andcount=0;
      ChangeLine=false;

      //check indent
      if (Indent) {
        if (CurrentY>=IndentY) {
          Indent=false;
          IndentX1=0;
          IndentX2=HELP_WIDTH;
          CurrentX=0;
        }
      }

      //start line with open link?
      if (init)
        if (LinkOpen) {
          if (gbls->drawing.linkCount < MAX_HELP_LINKS) {
            drawn=true;
            if (line.pass !=1) {
              //init next link
              link=&gbls->drawing.lstHelpLink[gbls->drawing.linkCount];
              link->resID=gbls->drawing.lstHelpLink[gbls->drawing.linkCount-1].resID;
              link->rect=gbls->drawing.lstHelpLink[gbls->drawing.linkCount-1].rect;
              drawn=true; //second pass required for link
            }
          }
          else {
            LinkOpen=false;
          }
        }
    }
  } while ((CurrentY < screenY2) && (*StringPos !=0));

  if ((Indent) && (IndentY>CurrentY))
    result=IndentY;
  else
    result=CurrentY+LineHeight;

  // close the help resource database
  DmCloseDatabase(dbRef);

  return result;
}

/**
 * Terminate the help system display.
 */
static void  
FrmHelpTerminate()
{
  HelpGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&gbls);

  // clean up
  if (gbls->drawing.helpTitle != NULL)
    MemPtrFree(gbls->drawing.helpTitle);
}
