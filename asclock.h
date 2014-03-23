#ifndef ASCLOCK_H
#define ASCLOCK_H
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>

extern char VERSION[];

#define TRUE 1
#define FALSE 0
#define VISIBLE 1
#define UNDEFINED 99999

typedef struct _symbol symbol;

struct _symbol
{
  char *name;
  int *addr;
};
extern symbol config_symbols[];

#define MAX_PATH_LEN 512
#define INT_TYPE 0;

/* the xpm data */
static char *clock_xpm[];
static char *month_xpm[];
static char *weekday_xpm[];
static char *led_xpm[];
static char *date_xpm[];
static char *hour_xpm[];
static char *minute_xpm[];
static char *second_xpm[];
static char *beats_xpm[];

/* the filenames */
extern char clock_xpm_fn[MAX_PATH_LEN];
extern char month_xpm_fn[MAX_PATH_LEN];
extern char weekday_xpm_fn[MAX_PATH_LEN];
extern char led_xpm_fn[MAX_PATH_LEN];
extern char date_xpm_fn[MAX_PATH_LEN];
extern char hour_xpm_fn[MAX_PATH_LEN];
extern char min_xpm_fn[MAX_PATH_LEN];
extern char sec_xpm_fn[MAX_PATH_LEN];
extern char beats_xpm_fn[MAX_PATH_LEN];

extern char exec_str[MAX_PATH_LEN];

extern int itblinks;
extern int showampm;
extern int itdocks;

extern int led_visible;
extern int led_elem_width;
extern int led_elem_height;
extern int led_12h_hour1_x;
extern int led_12h_hour2_x;
extern int led_12h_colon_x;
extern int led_12h_min1_x;
extern int led_12h_min2_x;
extern int led_ampm_x;
extern int led_ampm_y;
extern int led_ampm_width;
extern int led_12h_y;
extern int led_24h_hour1_x;
extern int led_24h_hour2_x;
extern int led_24h_colon_x;
extern int led_24h_min1_x;
extern int led_24h_min2_x;
extern int led_24h_y;

extern int week_visible;
extern int week_elem_width;
extern int week_elem_height;
extern int week_x;
extern int week_y;

extern int day_visible;
extern int day_elem_width;
extern int day_elem_height;
extern int day_x;
extern int day1_x;
extern int day2_x;
extern int day_y;

extern int month_visible;
extern int month_elem_width;
extern int month_elem_height;
extern int month_x;
extern int month_y;

extern int analog_visible;
extern int hour_visible ;
extern int hour_center_x ;
extern int hour_center_y ;
extern int hour_rot_x ;
extern int hour_rot_y ;

extern int min_visible ;
extern int min_center_x ;
extern int min_center_y ;
extern int min_rot_x ;
extern int min_rot_y ;

extern int sec_visible ;
extern int sec_center_x ;
extern int sec_center_y ;
extern int sec_rot_x ;
extern int sec_rot_y ;

extern int beats_visible;
extern int beats_at_x;
extern int beats_at_width;
extern int beats1_x;
extern int beats2_x;
extern int beats3_x;
extern int beats_y;
extern int beats_elem_width;
extern int beats_elem_height;

extern char *hour_map;
extern char *min_map;
extern char *sec_map;

/********* symbols.c ***************/
void init_symbols();
void postconfig();

/********* config.c ****************/

void config();
void parseArgs(int argc, char **argv);

/********* parser.c ****************/

int read_init(FILE *f);
int read_type(int *type);
int read_token(char *str, int max);
int read_assign();
int read_int(int *ret);
int read_semicolon();

#endif /* ASCLOCK_H */






