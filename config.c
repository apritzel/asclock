#include "asclock.h"
#include "config.h"
#include "default.h"

extern char *Geometry;

/* load the variables from a predefined theme ****************************************/

void config ()
{
  init_symbols();
#include "default_theme/config"

}

/*************************************************************************************/

static char *help_message[] = {
"where options include:",
"    -h, --help              this help message",
"    -v, --version           asclock version",
"    -12                     12 hour format",
"    -24                     24 hour format",
"    -d                      WindowMaker docking",
"    -p                      position on screen (e.g., -100-100)",
"    -exe <program>          program to start on click",
"    -theme <theme-dir>      directory containing a set of xpms",
"    -noblink                don't blink",
NULL
};
 
char *themes_directories[] = {
  "./themes/",
  "/usr/local/share/asclock/",
  "/usr/share/asclock/",
  "",
  NULL
};

void usage(char *ProgName)
{
  char **cpp;
  DIR *dfd;
  struct dirent *dp;

  fprintf(stderr,"usage:  %s [-options ...] \n", ProgName);
  for (cpp = help_message; *cpp; cpp++) {
    fprintf(stderr, "%s\n", *cpp);
  }
  fprintf(stderr,"\n");

  /* list themes in all known themes directories */
  fprintf(stderr, "----------------------------------------------------------------\n");
  fprintf(stderr, "List of installed themes directories\n\n");

  for (cpp= themes_directories; *cpp; cpp++)
    {
      fprintf(stderr, "dir : %s\n", *cpp);

      if((dfd = opendir(*cpp)) == NULL)
	  printf(" not found\n\n"); 
      else {
	while((dp = readdir(dfd)) != NULL)
	  if ( dp->d_name[0]!='.' )
	    printf("%s\n", dp->d_name);
        closedir(dfd);
      }
      printf("\n\n");

    }

  exit(1);
}

int findTheme(char*input, char *ret)
{
  char **dir;
  DIR *d;
  int n;

  n = MAX_PATH_LEN - strlen(input);

  for(dir = themes_directories; *dir; dir++)
    {

      if( strlen(*dir) < n )
	{
	  strncpy(ret, *dir, MAX_PATH_LEN);
	  strncat(ret, input, MAX_PATH_LEN);      

	  if( (d = opendir(ret)) != NULL )
	  {
	    closedir(d);
	    return TRUE;
	  }
	}
    }
  return FALSE;
}

char* getLanguageExtension()
{
	char* lang;

	if((lang = getenv("LC_ALL")) == NULL)
	{
		if((lang = getenv("LC_MESSAGES")) == NULL)
               {
			if((lang = getenv("LANG")) == NULL)
			{
				lang="C";
			}
		}
	}

	char** local = supported_locales;
	while(*local)
	{
		if(strncasecmp(*local, lang, 2) == 0)
			return *local;
		local++;
	}

	fprintf(stderr, "Locale %s not supported\n", lang);
	return NULL;
}

int loadTheme(char *themesdir)
{
  FILE *f;
  char filename[MAX_PATH_LEN];
  char* langExt = "";

  char token[64];
  int type;

  strcpy(filename, themesdir);
  strcat(filename, "/config");

  f = fopen(filename, "r");

  if(!f)
    {
      fprintf(stderr, "%s not found\n", filename);
      fprintf(stderr,"There is a problem with this theme.\n");
      return FALSE;
    }

  init_symbols();

  /* make sure nothing is accidentally visible */
  led_visible = 0;
  day_visible = 0;
  week_visible = 0;
  month_visible = 0;
  analog_visible = 0;
  hour_visible = 0;
  min_visible = 0;
  sec_visible = 0;
  beats_visible = 0;
  read_init(f);

  /* parse the config file for int's */
  while(read_type(&type))
    {
      symbol *s;

      if(!read_token(token, 64)) 
	printf("read_token failed\n");

      s = config_symbols;
      while((s->name) && (strcmp(s->name, token)!=0))
        s++;
     
      if(!(s->name)) {
	fprintf(stderr, "There is no such thing as a variable called %s\n", token);
	exit(-1);
      } 

      if(!read_assign(f))
	printf("read_assign failed\n");
      
      if(!read_int(s->addr))
	printf("read_int failed\n");
      
      if(!read_semicolon(f))
	printf("read_semicolon failed\n");
    }

  day_x = (day1_x + day2_x)/2;
  analog_visible = hour_visible || min_visible || sec_visible;

  fclose(f);

  langExt = getLanguageExtension();

  strcpy(clock_xpm_fn, themesdir);
  strcat(clock_xpm_fn, "/clock.xpm");

  if (langExt==NULL)
  {
	strcpy(month_xpm_fn, themesdir);
	strcat(month_xpm_fn, "/month.xpm");

	strcpy(weekday_xpm_fn, themesdir);
	strcat(weekday_xpm_fn, "/weekday.xpm");
  }
  else
  {
	sprintf(month_xpm_fn,   "%s%s%s%s", themesdir, "/month.",   langExt, ".xpm");
	//printf("Opening:%s\n",month_xpm_fn);
	sprintf(weekday_xpm_fn, "%s%s%s%s", themesdir, "/weekday.", langExt, ".xpm");
	//printf("Opening:%s\n",weekday_xpm_fn);
  }

  strcpy(date_xpm_fn, themesdir);
  strcat(date_xpm_fn, "/date.xpm");

  strcpy(led_xpm_fn, themesdir);
  strcat(led_xpm_fn, "/led.xpm");

  strcpy(hour_xpm_fn, themesdir);
  strcat(hour_xpm_fn, "/hour.xpm");

  strcpy(min_xpm_fn, themesdir);
  strcat(min_xpm_fn, "/minute.xpm");

  strcpy(sec_xpm_fn, themesdir);
  strcat(sec_xpm_fn, "/second.xpm");

  strcpy(beats_xpm_fn, themesdir);
  strcat(beats_xpm_fn, "/beats.xpm");

  return TRUE;
}

void parseArgs(int argc, char **argv)
{
  int i;
  char themesdir[MAX_PATH_LEN];
  char *ProgName = argv[0];
  int themeloaded;

  itblinks=TRUE;
  itdocks=FALSE;
  themeloaded=FALSE;
  for(i=1;i<argc;i++) {
    char *arg= argv[i];

    if (arg[0] == '-') {
      switch(arg[1]) {

      case 'v':
	printf("%s: Version %s\n", argv[0], VERSION);
        exit(0);
      case '-':
        if(strncmp(arg+2, "version", 7)==0)
        {
          printf("%s: Version %s\n", argv[0], VERSION);
          exit(0);
	}
	/* FALLTHROUGH */
      case 'h':
	usage(ProgName);
	exit(0);

      case 'n':
	itblinks = FALSE;
	continue;
      case 'i':
      case 'd':
	itdocks = TRUE;
	continue;
      case '1':
        showampm=1;
        continue;
      case '2':
        showampm=0;
        continue;
      case 's':
        if(! findTheme("shaped", themesdir) )
          {
            fprintf(stderr, "Could not find theme shaped\n\n");
            exit(-1);
          }

        if(!loadTheme(themesdir))
          exit(-1);

	itdocks = TRUE;
	themeloaded = TRUE;
        continue;
			case 'p':
				if(++i >=argc) usage(ProgName);
				Geometry = argv[i];
				continue;
      case 't':
        if(++i >=argc) usage(ProgName);

        if(! findTheme(argv[i], themesdir) )
          {
            fprintf(stderr, "Could not find theme %s\n\n", argv[i]);
            exit(-1);
          }

        if(!loadTheme(themesdir))
          exit(-1);

	themeloaded = TRUE;
        continue;
      case 'e':
        if(++i >=argc) usage(ProgName);
        strcpy(exec_str, argv[i]);
        strcat(exec_str, " &");
        continue;

      }
    }
  }

  if (! themeloaded)
  {
      // if the default theme in the file system (i.e. asclock-themes is
      // present) then that theme is loaded (probably in an l10n version)

      //fprintf(stderr, "No theme loaded");

      if(! findTheme(deftheme, themesdir) )
      {
	fprintf(stderr, "Could not find theme %s in path\n\n", deftheme);
	fprintf(stderr, "using built in (and dropping i18n)\n\n");
      }
      else
      {
       if(!loadTheme(themesdir))
         exit(-1);
      }

      // else use built in theme (only availabe in English)
  }
}
