/*
    SDL_main.c, placed in the public domain by Sam Lantinga  4/13/98

    The WinMain function -- calls your program's main() function
*/

#include <stdio.h>
#include <stdlib.h>

/*#define WIN32_LEAN_AND_MEAN*/
#include <windows.h>

#ifdef _WIN32_WCE
# define DIR_SEPERATOR TEXT("\\")
# undef _getcwd
# define _getcwd(str,len)	wcscpy(str,TEXT(""))
# define setbuf(f,b)
# define setvbuf(w,x,y,z)
# define fopen		_wfopen
# define freopen	_wfreopen
# define remove(x)	DeleteFile(x)
#else
# define DIR_SEPERATOR TEXT("/")
# include <direct.h>
#endif

/* Include the SDL main definition header */
#include "SDL.h"
#include "SDL_main.h"

#ifdef main
# ifndef _WIN32_WCE_EMULATION
#  undef main
# endif /* _WIN32_WCE_EMULATION */
#endif /* main */

/* The standard output files */
#define STDOUT_FILE	TEXT("stdout.txt")
#define STDERR_FILE	TEXT("stderr.txt")

#ifndef NO_STDIO_REDIRECT
# ifdef _WIN32_WCE
  static wchar_t stdoutPath[MAX_PATH];
  static wchar_t stderrPath[MAX_PATH];
# else
  static char stdoutPath[MAX_PATH];
  static char stderrPath[MAX_PATH];
# endif
#endif

#if defined(_WIN32_WCE) && _WIN32_WCE < 300
/* seems to be undefined in Win CE although in online help */
#define isspace(a) (((CHAR)a == ' ') || ((CHAR)a == '\t'))
#endif /* _WIN32_WCE < 300 */

static void UnEscapeQuotes( char *arg )
{
	char *last = NULL;

	while( *arg ) {
		if( *arg == '"' && *last == '\\' ) {
			char *c_curr = arg;
			char *c_last = last;

			while( *c_curr ) {
				*c_last = *c_curr;
				c_last = c_curr;
				c_curr++;
			}
			*c_last = '\0';
		}
		last = arg;
		arg++;
	}
}

/* Parse a command line buffer into arguments */
static int ParseCommandLine(char *cmdline, char **argv)
{
	char *bufp;
	char *lastp = NULL;
	int argc, last_argc;

	argc = last_argc = 0;
	for ( bufp = cmdline; *bufp; ) {
		/* Skip leading whitespace */
		while ( isspace(*bufp) ) {
			++bufp;
		}
		/* Skip over argument */
		if ( *bufp == '"' ) {
			++bufp;
			if ( *bufp ) {
				if ( argv ) {
					argv[argc] = bufp;
				}
				++argc;
			}
			/* Skip over word */
			while ( *bufp && ( *bufp != '"' || *lastp == '\\' ) ) {
				lastp = bufp;
				++bufp;
			}
		} else {
			if ( *bufp ) {
				if ( argv ) {
					argv[argc] = bufp;
				}
				++argc;
			}
			/* Skip over word */
			while ( *bufp && ! isspace(*bufp) ) {
				++bufp;
			}
		}
		if ( *bufp ) {
			if ( argv ) {
				*bufp = '\0';
			}
			++bufp;
		}

		/* Strip out \ from \" sequences */
		if( argv && last_argc != argc ) {
			UnEscapeQuotes( argv[last_argc] );
		}
		last_argc = argc;
	}
	if ( argv ) {
		argv[argc] = NULL;
	}
	return(argc);
}

/* Show an error message */
static void ShowError(const char *title, const char *message)
{
/* If USE_MESSAGEBOX is defined, you need to link with user32.lib */
#ifdef USE_MESSAGEBOX
	MessageBox(NULL, message, title, MB_ICONEXCLAMATION|MB_OK);
#else
	fprintf(stderr, "%s: %s\n", title, message);
#endif
}

/* Pop up an out of memory message, returns to Windows */
static BOOL OutOfMemory(void)
{
	ShowError("Fatal Error", "Out of memory - aborting");
	return FALSE;
}

/* SDL_Quit() shouldn't be used with atexit() directly because
   calling conventions may differ... */
static void cleanup(void)
{
	SDL_Quit();
}

/* Remove the output files if there was no output written */
static void cleanup_output(void)
{
#ifndef NO_STDIO_REDIRECT
	FILE *file;
	int empty;
#endif

	/* Flush the output in case anything is queued */
	fclose(stdout);
	fclose(stderr);

#ifndef NO_STDIO_REDIRECT
	/* See if the files have any output in them */
	if ( stdoutPath[0] ) {
		file = fopen(stdoutPath, TEXT("rb"));
		if ( file ) {
			empty = (fgetc(file) == EOF) ? 1 : 0;
			fclose(file);
			if ( empty ) {
				remove(stdoutPath);
			}
		}
	}
	if ( stderrPath[0] ) {
		file = fopen(stderrPath, TEXT("rb"));
		if ( file ) {
			empty = (fgetc(file) == EOF) ? 1 : 0;
			fclose(file);
			if ( empty ) {
				remove(stderrPath);
			}
		}
	}
#endif
}

#if defined(_MSC_VER) && !defined(_WIN32_WCE)
/* The VC++ compiler needs main defined */
#define console_main main
#endif

/* Opens a file. */
char* openFile()
{
  char* outPathName = (char*)malloc(MAX_PATH * 10);
  char* title = (char*)malloc(MAX_PATH * 10);
  int error;
  char* msg;
  OPENFILENAME ofn;
  int pathNameLen;

  outPathName[0] = '\0';
  title[0] = '\0';

  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = NULL;
  ofn.hInstance = NULL;
  ofn.lpstrFilter = "Gameexe Control File\0Gameexe.ini\0\0";
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = outPathName;
  ofn.nMaxFile = MAX_PATH * 10;
  ofn.lpstrFileTitle = title;
  ofn.nMaxFileTitle = MAX_PATH * 10;
  ofn.lpstrInitialDir = NULL; /* Eventually, we should be smart about this. */
  ofn.lpstrTitle = "Open RealLive Game...";
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lpstrDefExt = NULL;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;

  if(GetOpenFileName(&ofn)) {
    /* Chop off the worthless Gameexe.ini file */
    pathNameLen = strlen(ofn.lpstrFile);
    if(pathNameLen > 11)
      ofn.lpstrFile[pathNameLen - 11] = '\0';
    return ofn.lpstrFile;
  } else {
    error = CommDlgExtendedError();

	/* No error; user canceled. */
	if(error == 0)
	  return NULL;

    msg = "Unknown";
    switch (error)
    {
        case CDERR_FINDRESFAILURE: msg = "CDERR_FINDRESFAILURE"; break;
        case CDERR_INITIALIZATION: msg = "CDERR_INITIALIZATION"; break;
        case CDERR_LOADRESFAILURE: msg = "CDERR_LOADRESFAILURE"; break;
        case CDERR_LOADSTRFAILURE: msg = "CDERR_LOADSTRFAILURE"; break;
        case CDERR_LOCKRESFAILURE: msg = "CDERR_LOCKRESFAILURE"; break;
        case CDERR_MEMALLOCFAILURE: msg = "CDERR_MEMALLOCFAILURE"; break;
        case CDERR_MEMLOCKFAILURE: msg = "CDERR_MEMLOCKFAILURE"; break;
        case CDERR_NOHINSTANCE: msg = "CDERR_NOHINSTANCE"; break;
        case CDERR_NOHOOK: msg = "CDERR_NOHOOK"; break;
        case CDERR_NOTEMPLATE: msg = "CDERR_NOTEMPLATE"; break;
        case CDERR_STRUCTSIZE: msg = "CDERR_STRUCTSIZE"; break;
        case  PDERR_RETDEFFAILURE: msg = "PDERR_RETDEFFAILURE"; break;
        case  PDERR_PRINTERNOTFOUND: msg = "PDERR_PRINTERNOTFOUND"; break;
        case  PDERR_PARSEFAILURE: msg = "PDERR_PARSEFAILURE"; break;
        case  PDERR_NODEVICES: msg = "PDERR_NODEVICES"; break;
        case  PDERR_NODEFAULTPRN: msg = "PDERR_NODEFAULTPRN"; break;
        case  PDERR_LOADDRVFAILURE: msg = "PDERR_LOADDRVFAILURE"; break;
        case  PDERR_INITFAILURE: msg = "PDERR_INITFAILURE"; break;
        case  PDERR_GETDEVMODEFAIL: msg = "PDERR_GETDEVMODEFAIL"; break;
        case  PDERR_DNDMMISMATCH: msg = "PDERR_DNDMMISMATCH"; break;
        case  PDERR_DEFAULTDIFFERENT: msg = "PDERR_DEFAULTDIFFERENT"; break;
        case  PDERR_CREATEICFAILURE: msg = "PDERR_CREATEICFAILURE"; break;
        default: break;
    }

	fprintf(stderr, "Couldn't bring up open box: %s (%d)\n", msg, error);

    return NULL;
  }
}

/* This is where execution begins [console apps] */
int console_main(int argc, char *argv[])
{
	size_t n;
	char *bufp, *appname;
	int status;
	char* new_argv[3];

	/* Get the class name from argv[0] */
	appname = argv[0];
	if ( (bufp=SDL_strrchr(argv[0], '\\')) != NULL ) {
		appname = bufp+1;
	} else
	if ( (bufp=SDL_strrchr(argv[0], '/')) != NULL ) {
		appname = bufp+1;
	}

	if ( (bufp=SDL_strrchr(appname, '.')) == NULL )
		n = SDL_strlen(appname);
	else
		n = (bufp-appname);

	bufp = SDL_stack_alloc(char, n+1);
	if ( bufp == NULL ) {
		return OutOfMemory();
	}
	SDL_strlcpy(bufp, appname, n+1);
	appname = bufp;

	/* Load SDL dynamic link library */
	if ( SDL_Init(SDL_INIT_NOPARACHUTE) < 0 ) {
		ShowError("WinMain() error", SDL_GetError());
		return(FALSE);
	}
	atexit(cleanup_output);
	atexit(cleanup);

	/* Sam:
	   We still need to pass in the application handle so that
	   DirectInput will initialize properly when SDL_RegisterApp()
	   is called later in the video initialization.
	 */
	SDL_SetModuleHandle(GetModuleHandle(NULL));

    /* erg:04/02/08: If we weren't passed any arguments, show a dialog
	   to the user to select the game folder. */
	if (argc == 1)
	{
      /* We weren't given a file. Select one. */
	  char* locationName = openFile();

	  if(locationName == NULL)
		  exit(-1);

      new_argv[0] = argv[0];
	  new_argv[1] = locationName;
	  new_argv[2] = NULL;

	  argc = 2;
	  argv = new_argv;
	}

	/* Run the application main() code */
	status = SDL_main(argc, argv);

	/* Exit cleanly, calling atexit() functions */
	exit(status);

	/* Hush little compiler, don't you cry... */
	return 0;
}

/* This is where execution begins [windowed apps] */
#ifdef _WIN32_WCE
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw)
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
#endif
{
	HINSTANCE handle;
	char **argv;
	int argc;
	char *cmdline;
#ifdef _WIN32_WCE
	wchar_t *bufp;
	int nLen;
#else
	char *bufp;
	size_t nLen;
#endif
#ifndef NO_STDIO_REDIRECT
	DWORD pathlen;
#ifdef _WIN32_WCE
	wchar_t path[MAX_PATH];
#else
	char path[MAX_PATH];
#endif
	FILE *newfp;
#endif

	/* Start up DDHELP.EXE before opening any files, so DDHELP doesn't
	   keep them open.  This is a hack.. hopefully it will be fixed
	   someday.  DDHELP.EXE starts up the first time DDRAW.DLL is loaded.
	 */
	handle = LoadLibrary(TEXT("DDRAW.DLL"));
	if ( handle != NULL ) {
		FreeLibrary(handle);
	}

#ifndef NO_STDIO_REDIRECT
	pathlen = GetModuleFileName(NULL, path, SDL_arraysize(path));
	while ( pathlen > 0 && path[pathlen] != '\\' ) {
		--pathlen;
	}
	path[pathlen] = '\0';

#ifdef _WIN32_WCE
	wcsncpy( stdoutPath, path, SDL_arraysize(stdoutPath) );
	wcsncat( stdoutPath, DIR_SEPERATOR STDOUT_FILE, SDL_arraysize(stdoutPath) );
#else
	SDL_strlcpy( stdoutPath, path, SDL_arraysize(stdoutPath) );
	SDL_strlcat( stdoutPath, DIR_SEPERATOR STDOUT_FILE, SDL_arraysize(stdoutPath) );
#endif

	/* Redirect standard input and standard output */
	newfp = freopen(stdoutPath, TEXT("w"), stdout);

#ifndef _WIN32_WCE
	if ( newfp == NULL ) {	/* This happens on NT */
#if !defined(stdout)
		stdout = fopen(stdoutPath, TEXT("w"));
#else
		newfp = fopen(stdoutPath, TEXT("w"));
		if ( newfp ) {
			*stdout = *newfp;
		}
#endif
	}
#endif /* _WIN32_WCE */

#ifdef _WIN32_WCE
	wcsncpy( stderrPath, path, SDL_arraysize(stdoutPath) );
	wcsncat( stderrPath, DIR_SEPERATOR STDOUT_FILE, SDL_arraysize(stdoutPath) );
#else
	SDL_strlcpy( stderrPath, path, SDL_arraysize(stderrPath) );
	SDL_strlcat( stderrPath, DIR_SEPERATOR STDERR_FILE, SDL_arraysize(stderrPath) );
#endif

	newfp = freopen(stderrPath, TEXT("w"), stderr);
#ifndef _WIN32_WCE
	if ( newfp == NULL ) {	/* This happens on NT */
#if !defined(stderr)
		stderr = fopen(stderrPath, TEXT("w"));
#else
		newfp = fopen(stderrPath, TEXT("w"));
		if ( newfp ) {
			*stderr = *newfp;
		}
#endif
	}
#endif /* _WIN32_WCE */

	setvbuf(stdout, NULL, _IOLBF, BUFSIZ);	/* Line buffered */
	setbuf(stderr, NULL);			/* No buffering */
#endif /* !NO_STDIO_REDIRECT */

#ifdef _WIN32_WCE
	nLen = wcslen(szCmdLine)+128+1;
	bufp = SDL_stack_alloc(wchar_t, nLen*2);
	wcscpy (bufp, TEXT("\""));
	GetModuleFileName(NULL, bufp+1, 128-3);
	wcscpy (bufp+wcslen(bufp), TEXT("\" "));
	wcsncpy(bufp+wcslen(bufp), szCmdLine,nLen-wcslen(bufp));
	nLen = wcslen(bufp)+1;
	cmdline = SDL_stack_alloc(char, nLen);
	if ( cmdline == NULL ) {
		return OutOfMemory();
	}
	WideCharToMultiByte(CP_ACP, 0, bufp, -1, cmdline, nLen, NULL, NULL);
#else
	/* Grab the command line */
	bufp = GetCommandLine();
	nLen = SDL_strlen(bufp)+1;
	cmdline = SDL_stack_alloc(char, nLen);
	if ( cmdline == NULL ) {
		return OutOfMemory();
	}
	SDL_strlcpy(cmdline, bufp, nLen);
#endif

	/* Parse it into argv and argc */
	argc = ParseCommandLine(cmdline, NULL);
	argv = SDL_stack_alloc(char*, argc+1);
	if ( argv == NULL ) {
		return OutOfMemory();
	}
	ParseCommandLine(cmdline, argv);

	/* Run the main program (after a little SDL initialization) */
	console_main(argc, argv);

	/* Hush little compiler, don't you cry... */
	return 0;
}
