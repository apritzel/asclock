#include "asclock.h"
#include "version.h"

#include "default_theme/clock.xpm"
#include "default_theme/led.xpm"
#include "default_theme/month.xpm"
#include "default_theme/date.xpm"
#include "default_theme/weekday.xpm"
#include "default_theme/beats.xpm"

int ONLYSHAPE=0; /* default value is noshape */
int ICONIFIED=0; /* default is not iconified */
int itdocks=0;      /* default is not Dock mode */
/* X11 Variablen *************************************************************/
Display *dpy;	  /* welches DISPLAY */
Window Root;      /* Hintergrund-Drawable */
int screen;
int x_fd;
int d_depth;
XSizeHints mysizehints;
XWMHints mywmhints;
Pixel back_pix, fore_pix;
GC NormalGC;
Window iconwin, win;       /* My home is my window */
char *Geometry;
char *LedColor = "LightSeaGreen";
char exec_str[] = "echo no program has been specified >/dev/console";
char *ERR_colorcells = "not enough free color cells or xpm not found\n";
char *ampers = " &";
char month_xpm_fn[MAX_PATH_LEN]="";
char clock_xpm_fn[MAX_PATH_LEN]="";
char weekday_xpm_fn[MAX_PATH_LEN]="";
char date_xpm_fn[MAX_PATH_LEN]="";
char led_xpm_fn[MAX_PATH_LEN]="";
char hour_xpm_fn[MAX_PATH_LEN]="";
char min_xpm_fn[MAX_PATH_LEN]="";
char sec_xpm_fn[MAX_PATH_LEN]="";
char beats_xpm_fn[MAX_PATH_LEN]="";
char positions[MAX_PATH_LEN]="";
/* XPM Variables *************************************************************/
typedef struct _XpmIcon {
    Pixmap pixmap;
    Pixmap mask;
    XpmAttributes attributes;
}        XpmIcon;

XpmIcon asclock, led, month, date, weekday, beats;
XpmIcon visible;
time_t actualtime;
long actualmin;
int actbeats;

/* local functions *********************************************************/
#define MW_EVENTS   (ExposureMask | ButtonPressMask | StructureNotifyMask)

void GetXPM(void);
Pixel GetColor(char *name);
void RedrawWindow( XpmIcon *v);
void InsertTime();
void swatch_beats(int beats_cnt);

static Atom wm_delete_window;

int main(int argc,char *argv[])
{
  int i;
  int ret;
  char themesdir[MAX_PATH_LEN] = "";
  unsigned int borderwidth ;
  char *display_name = NULL; 
  char *wname = "asclock";
  XGCValues gcv;
  unsigned long gcm;
  XEvent Event;
  XTextProperty name;
  XClassHint classHint;
  Geometry = "";
  
  /* Parse command line options */
  config();
  parseArgs(argc, argv);

  if(analog_visible || hour_visible || min_visible || sec_visible) {
    fprintf(stderr, "%s does not support analog clocks yet.\n", VERSION);
    fprintf(stderr, "You want the asclock-gtk versions\n");
  }

  /* Open the display */
  if (!(dpy = XOpenDisplay(display_name)))  
    { 
      fprintf(stderr,"asclock: can't open display %s\n", 
	      XDisplayName(display_name)); 
      exit (1); 
    } 
  screen= DefaultScreen(dpy);
  Root = RootWindow(dpy, screen);
  d_depth = DefaultDepth(dpy, screen);
  x_fd = XConnectionNumber(dpy);
  
  /* Icon Daten nach XImage konvertieren */
  GetXPM();
  postconfig();

  /* Create a window to hold the banner */
  mysizehints.flags= USSize|USPosition;
  mysizehints.x = 0;
  mysizehints.y = 0;

  back_pix = GetColor("white");
  fore_pix = GetColor("black");

  XWMGeometry(dpy, screen, Geometry, "64x64+0+0", (borderwidth =1), &mysizehints,
	      &mysizehints.x,&mysizehints.y,&mysizehints.width,&mysizehints.height, &i); 

  mysizehints.width = asclock.attributes.width;
  mysizehints.height= asclock.attributes.height;

  win = XCreateSimpleWindow(dpy,Root,mysizehints.x,mysizehints.y,
			    mysizehints.width,mysizehints.height,
			    borderwidth,fore_pix,back_pix);
  iconwin = XCreateSimpleWindow(dpy,win,mysizehints.x,mysizehints.y,
				mysizehints.width,mysizehints.height,
				borderwidth,fore_pix,back_pix);

  wm_delete_window = XInternAtom (dpy, 
                                  "WM_DELETE_WINDOW",
                                  False);
  (void) XSetWMProtocols (dpy, win,
                          &wm_delete_window, 1);


  /* Hints aktivieren */
  XSetWMNormalHints(dpy, win, &mysizehints);
  classHint.res_name =  "asclock";
  classHint.res_class = "ASClock";
  XSetClassHint(dpy, win, &classHint);

  XSelectInput(dpy,win,MW_EVENTS);
  XSelectInput(dpy,iconwin,MW_EVENTS);

  if (XStringListToTextProperty(&wname, 1, &name) ==0) {
    fprintf(stderr, "asclock: can't allocate window name\n");
    exit(-1);
  }
  XSetWMName(dpy, win, &name);
  
  /* Create a GC for drawing */
  gcm = GCForeground|GCBackground|GCGraphicsExposures;
  gcv.foreground = fore_pix;
  gcv.background = back_pix;
  gcv.graphics_exposures = FALSE;
  NormalGC = XCreateGC(dpy, Root, gcm, &gcv);  

  /* if (ONLYSHAPE) { try to make shaped window here */
    XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, asclock.mask, ShapeSet);
    XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, asclock.mask, ShapeSet);
  
  
  mywmhints.initial_state = (itdocks ? WithdrawnState
			     : (ICONIFIED ? IconicState : NormalState));
  mywmhints.icon_window = iconwin;
  mywmhints.icon_x = mysizehints.x;
  mywmhints.icon_y = mysizehints.y;
  mywmhints.flags = StateHint | IconWindowHint | IconPositionHint;
  if (itdocks) {
    mywmhints.window_group = win;
    mywmhints.flags |= WindowGroupHint;
  }
  XSetWMHints(dpy, win, &mywmhints);
    
  if (itdocks)
    XSetCommand(dpy, win, argv, argc);

  XMapWindow(dpy,win);

  InsertTime();
  RedrawWindow(&visible);
  while(1)
    {
      if (actualtime != mytime())
	  {
	  actualtime = mytime();
	  if(actualmin != actualtime / 60)
	    	{
	      		InsertTime();
	      		if (!itblinks) RedrawWindow(&visible);
	    	}
          if( beats_visible )
          {
            int beats = (int) (( ( actualtime + 60*60) % (24*60*60) ) / 86.4);
            swatch_beats(beats);
	    RedrawWindow(&visible);
          }

	  if (led_visible)
	    if( itblinks )
	      {  
		if (actualtime % 2)
		  XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
			    10*led_elem_width, 0,
			    (led_elem_width+1)/2, led_elem_height, 
			    ((showampm!=0) ? led_12h_colon_x : led_24h_colon_x), 
			    ((showampm!=0) ? led_12h_y : led_24h_y));
	    	else
		  /* Sekunden Doppelpunkt aus */
		  XCopyArea(dpy, asclock.pixmap, visible.pixmap, NormalGC,
			    ((showampm!=0) ? led_12h_colon_x : led_24h_colon_x), 
			    ((showampm!=0) ? led_12h_y : led_24h_y),
			    (led_elem_width+1)/2, led_elem_height, 
			    ((showampm!=0) ? led_12h_colon_x : led_24h_colon_x), 
			    ((showampm!=0) ? led_12h_y : led_24h_y));
	  	RedrawWindow(&visible);
	      }
	  }
      
      /* read a packet */
      while (XPending(dpy))
	{
	  XNextEvent(dpy,&Event);
	  switch(Event.type)
	    {
	    case Expose:
	      if(Event.xexpose.count == 0 )
		RedrawWindow(&visible);
	      break;
	    case ButtonPress:
	      system(exec_str);
	      break;
	    case DestroyNotify:
	      /*
		XFreeGC(dpy, NormalGC);
		XDestroyWindow(dpy, win);
		XDestroyWindow(dpy, iconwin);
	      */
	      XCloseDisplay(dpy);
	      exit(0); 
	    case ClientMessage:
	      {
		if( Event.xclient.data.l[0] == wm_delete_window)
		  {
		    XCloseDisplay(dpy);
		    exit(0);
		  }
	      }
	      break;
	    default:
	      break;      
	    }
	}
      XFlush(dpy);
#ifdef SYSV
	poll((struct poll *) 0, (size_t) 0, 50);
#else
	usleep(50000L);			/* 5/100 sec */
#endif
    }
  return 0;
}
/****************************************************************************/
void nocolor(char *a, char *b)
{
 fprintf(stderr,"asclock: can't %s %s\n", a,b);
}
/****************************************************************************/
/* Konvertiere XPMIcons nach XImage */
void GetXPM(void)
{
  XColor col;
  XWindowAttributes attributes;
  int ret;

  /* for the colormap */
  XGetWindowAttributes(dpy,Root,&attributes);


  /* ---------------------------------------------------------------- */

  asclock.attributes.closeness = 40000; /* Allow for "similar" colors */
  asclock.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);

  if (clock_xpm_fn[0])
    ret = XpmReadFileToPixmap(dpy, Root, clock_xpm_fn,
			      &asclock.pixmap, &asclock.mask, &asclock.attributes);
  else 
    ret = XpmCreatePixmapFromData(dpy, Root, clock_xpm, &asclock.pixmap, 
				  &asclock.mask, &asclock.attributes);
 
  if(ret != XpmSuccess)
    {nocolor("create asclock xpm", ERR_colorcells);exit(1);}
  
  /* ---------------------------------------------------------------- */


  visible.attributes.closeness = 40000; /* Allow for "similar" colors */
  visible.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);

 if (clock_xpm_fn[0])
  ret = XpmReadFileToPixmap(dpy, Root, clock_xpm_fn, &visible.pixmap, 
				&visible.mask, &visible.attributes);
 else
  ret = XpmCreatePixmapFromData(dpy, Root, clock_xpm, &visible.pixmap, 
				&visible.mask, &visible.attributes);
 if(ret != XpmSuccess)
   {nocolor("create visible xpm", ERR_colorcells);exit(1);}


  /* ---------------------------------------------------------------- */

  if (led_visible) {
    led.attributes.closeness = 40000; /* Allow for "similar" colors */
    led.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);

    if(led_xpm_fn[0])
      ret = XpmReadFileToPixmap(dpy, Root, led_xpm_fn, &led.pixmap, 
			        &led.mask, &led.attributes);
    else
      ret = XpmCreatePixmapFromData(dpy, Root, led_xpm, &led.pixmap, 
  				    &led.mask, &led.attributes);
    if(ret != XpmSuccess)
      {nocolor("create led xpm", ERR_colorcells);exit(1);}

    if(led_elem_width==UNDEFINED) led_elem_width = led.attributes.width/15;
    if(led_elem_height==UNDEFINED) led_elem_height = led.attributes.height;
  }
  /* ---------------------------------------------------------------- */

  if (month_visible) 
  {
    month.attributes.closeness = 40000; /* Allow for "similar" colors */
    month.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);

    if( month_xpm_fn[0])
      ret = XpmReadFileToPixmap(dpy, Root, month_xpm_fn,
			        &month.pixmap, &month.mask, &month.attributes);
    else 
      ret = XpmCreatePixmapFromData(dpy, Root, month_xpm, &month.pixmap, 
				  &month.mask, &month.attributes);
    if(ret != XpmSuccess)
      {nocolor("create month xpm", ERR_colorcells);exit(1);}

    if(month_elem_width==UNDEFINED) month_elem_width = month.attributes.width;
    if(month_elem_height==UNDEFINED) month_elem_height = month.attributes.height/12;
  }
  /* ---------------------------------------------------------------- */

  if (day_visible)
  {
    date.attributes.closeness = 40000; /* Allow for "similar" colors */
    date.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);

    if (date_xpm_fn[0]) 
      ret = XpmReadFileToPixmap(dpy, Root, date_xpm_fn, &date.pixmap, 
			        &date.mask, &date.attributes);
    else
      ret = XpmCreatePixmapFromData(dpy, Root, date_xpm, &date.pixmap, 
 				    &date.mask, &date.attributes);
    if(ret != XpmSuccess)
      {nocolor("create date xpm", ERR_colorcells);exit(1);}

    if(day_elem_width==UNDEFINED) day_elem_width = date.attributes.width/10;
    if(day_elem_height==UNDEFINED) day_elem_height = date.attributes.height;
  }
  /* ---------------------------------------------------------------- */

  if (week_visible) 
  { 
    weekday.attributes.closeness = 40000; /* Allow for "similar" colors */
    weekday.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);

    if(weekday_xpm_fn[0]) 
      ret = XpmReadFileToPixmap(dpy, Root, weekday_xpm_fn,
			        &weekday.pixmap, &weekday.mask, &weekday.attributes);
    else
      ret = XpmCreatePixmapFromData(dpy, Root, weekday_xpm, &weekday.pixmap,
				    &weekday.mask, &weekday.attributes);
    if(ret != XpmSuccess)
      {nocolor("create weekday xpm", ERR_colorcells);exit(1);}

    if(week_elem_width==UNDEFINED) week_elem_width = weekday.attributes.width;
    if(week_elem_height==UNDEFINED) week_elem_height = weekday.attributes.height/7;
  }
  /* ---------------------------------------------------------------- */

  if (beats_visible) {

    beats.attributes.closeness = 40000; /* Allow for "similar" colors */
    beats.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);
   
    if(beats_xpm_fn[0])
      ret = XpmReadFileToPixmap(dpy, Root, beats_xpm_fn,
                              &beats.pixmap, &beats.mask, &beats.attributes);
    else
      ret = XpmCreatePixmapFromData(dpy, Root, beats_xpm, &beats.pixmap,
                                    &beats.mask, &beats.attributes);
    if(ret != XpmSuccess)
      {nocolor("create beats xpm", ERR_colorcells);exit(1);}
 
    if(beats_elem_width==UNDEFINED) beats_elem_width = beats.attributes.width/12;
    if(beats_elem_height==UNDEFINED) beats_elem_height = beats.attributes.height;

  }

}
/****************************************************************************/
/* Removes expose events for a specific window from the queue */
int flush_expose (Window w)
{
  XEvent dummy;
  int i=0;
  
  while (XCheckTypedWindowEvent (dpy, w, Expose, &dummy))i++;
  return i;
}

/****************************************************************************/
/* Draws the icon window */
void RedrawWindow( XpmIcon *v)
{
  flush_expose (iconwin);
  XCopyArea(dpy,v->pixmap,iconwin,NormalGC,
	    0,0,v->attributes.width, v->attributes.height,0,0);
  flush_expose (win);
  XCopyArea(dpy,v->pixmap,win,NormalGC,
	    0,0,v->attributes.width, v->attributes.height,0,0);

}
/****************************************************************************/
Pixel GetColor(char *name)
{
  XColor color;
  XWindowAttributes attributes;

  XGetWindowAttributes(dpy,Root,&attributes);
  color.pixel = 0;
   if (!XParseColor (dpy, attributes.colormap, name, &color)) 
     {
       nocolor("parse",name);
     }
   else if(!XAllocColor (dpy, attributes.colormap, &color)) 
     {
       nocolor("alloc",name);
     }
  return color.pixel;
}

struct tm *clk;

void Twelve()
{
  int thishour;
  /* Stunde ohne am/pm */
  thishour = clk->tm_hour % 12;
  if (thishour == 0 )
    thishour = 12;

  if (clk->tm_hour >= 12)
    {
      /* PM */
      XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
		13*led_elem_width, 0, 
		led_ampm_width, led_elem_height, 
		led_ampm_x, led_ampm_y);
    }
  else
    /* AM */
    XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	      11*led_elem_width, 0, 
	      led_ampm_width, led_elem_height, 
	      led_ampm_x, led_ampm_y);
  
  /* the hour */
  if (thishour>9)
    XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	      led_elem_width, 0, 
	      led_elem_width, led_elem_height, 
	      led_12h_hour1_x, led_12h_y);

  XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	    led_elem_width * (thishour % 10), 0, 
	    led_elem_width, led_elem_height,
	    led_12h_hour2_x, led_12h_y);
  
  /* the minute */
  XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	    led_elem_width * (clk->tm_min / 10),  0,  
	    led_elem_width, led_elem_height, 
	    led_12h_min1_x, led_12h_y);

  XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	    led_elem_width * (clk->tm_min % 10), 0, 
	    led_elem_width, led_elem_height, 
	    led_12h_min2_x, led_12h_y);
}

void TwentyFour()
{
  /* the hour */
  XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	    led_elem_width * (clk->tm_hour / 10), 0, 
	    led_elem_width, led_elem_height, 
	    led_24h_hour1_x, led_24h_y);

  XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	    led_elem_width * (clk->tm_hour % 10), 0, 
	    led_elem_width, led_elem_height, 
	    led_24h_hour2_x, led_24h_y);

  /* Minute */
  XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	    led_elem_width * (clk->tm_min / 10), 0, 
	    led_elem_width, led_elem_height, 
	    led_24h_min1_x, led_24h_y);

  XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	    led_elem_width * (clk->tm_min % 10), 0, 
	    led_elem_width, led_elem_height, 
	    led_24h_min2_x, led_24h_y);
  
}
/****************************************************************************/

int mytime()
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  return tv.tv_sec;
}
/****************************************************************************/
void swatch_beats(int beats_cnt)
{
    int pos1 = (beats_cnt/100) % 10;
    int pos2 = (beats_cnt/10 ) % 10;
    int pos3 = beats_cnt % 10;

  if( (!itblinks) || (actualtime % 2==1) )
    XCopyArea(dpy, beats.pixmap, visible.pixmap, NormalGC,
              10*beats_elem_width, 0,
              beats_at_width, beats_elem_height,
              beats_at_x, beats_y);
  else
    XCopyArea(dpy, asclock.pixmap, visible.pixmap, NormalGC,
              beats_at_x, beats_y,
              beats_at_width, beats_elem_height,
              beats_at_x, beats_y);

  XCopyArea(dpy, beats.pixmap, visible.pixmap, NormalGC,
            pos1*beats_elem_width, 0,
            beats_elem_width, beats_elem_height,
            beats1_x, beats_y);

  XCopyArea(dpy, beats.pixmap, visible.pixmap, NormalGC,
            pos2*beats_elem_width, 0,
            beats_elem_width, beats_elem_height,
            beats2_x, beats_y);

  XCopyArea(dpy, beats.pixmap, visible.pixmap, NormalGC,
 	    pos3*beats_elem_width, 0,
            beats_elem_width, beats_elem_height,
            beats3_x, beats_y);
}
/****************************************************************************/
void InsertTime()
{
  int thismonth, thisweekday, thisdate;

  /* Zeit auslesen */
  actualtime = mytime();
  actualmin = actualtime / 60;

  clk = localtime(&actualtime);

  /* leere asclock holen */
  XCopyArea(dpy, asclock.pixmap, visible.pixmap, NormalGC,
	    0,0,mysizehints.width,mysizehints.height,0,0);


  if( led_visible )
    {
      if (showampm)
	  Twelve();
      else
	TwentyFour();
    }
  
  if( beats_visible )
  {
    int beats = (int) (( ( actualtime + 60*60) % (24*60*60) ) / 86.4);
    swatch_beats(beats);
  }

  /* Month */ 
  if( month_visible )
    XCopyArea(dpy, month.pixmap, visible.pixmap, NormalGC,
	      0, month_elem_height*(clk->tm_mon ), 
	      month_elem_width, month_elem_height, 
	      month_x, month_y);

  /* Date */
  if( day_visible ) {
    if (clk->tm_mday>9)
      {
	XCopyArea(dpy, date.pixmap, visible.pixmap, NormalGC,
		  day_elem_width * ( clk->tm_mday / 10 ), 0, 
		  day_elem_width, day_elem_height, 
		  day1_x, day_y);
	
	XCopyArea(dpy, date.pixmap, visible.pixmap, NormalGC,
		  day_elem_width * ( clk->tm_mday % 10), 0, 
		  day_elem_width, day_elem_height, 
		  day2_x, day_y);
      }
    else
      XCopyArea(dpy, date.pixmap, visible.pixmap, NormalGC,
		day_elem_width * clk->tm_mday , 0, 
		day_elem_width, day_elem_height, 
		day_x, day_y);
  }

  /* Wochentag */
  if (week_visible)
    XCopyArea(dpy, weekday.pixmap, visible.pixmap, NormalGC,
	      0, week_elem_height * ((clk->tm_wday +6) % 7), 
	      week_elem_width, week_elem_height, 
	      week_x, week_y); 
  
  if (led_visible && !itblinks ) 
    /* Sekunden Doppelpunkt ein */
    XCopyArea(dpy, led.pixmap, visible.pixmap, NormalGC,
	      10*led_elem_width, 0,
	      (led_elem_width+1)/2, led_elem_height, 
	      (showampm!=0 ? led_12h_colon_x : led_24h_colon_x), 
	      (showampm!=0 ? led_12h_y    : led_24h_y));
}












