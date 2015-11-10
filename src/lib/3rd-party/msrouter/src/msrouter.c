
/***************************************************************************
 * msrouter.c
 *
 * Write input Mini-SEED records out into a user defined file and
 * directory layout.
 *
 * Written by Chad Trabant, IRIS Data Management Center
 *
 * modified 2007.030
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include <libmseed.h>

#include "dsarchive.h"

#define VERSION "2.1"
#define PACKAGE "msrouter"

static int parameter_proc (int argcount, char **argvec);
static char *getoptval (int argcount, char **argvec, int argopt);
static int  addarchive(const char *path, const char *layout);
static void addfile (char *filename);
static void usage (int level);
static void term_handler (int sig);

/* A chain of archive definitions */
typedef struct Archive_s {
  DataStream  datastream;
  struct Archive_s *next;
}
Archive;

typedef struct FileLink_s {
  char *filename;
  struct FileLink_s *next;
}
FileLink;

static flag verbose       = 0; 
static flag segments      = 0;    /* If continuous time segmented files */
static flag bindata       = 0;    /* Write binary data samples instead of records */
static int  reclen        = -1;   /* Forced record length for reading */
static double timetol     = -1.0; /* Time tolerance for continuous traces */
static double sampratetol = -1.0; /* Sample rate tolerance for continuous traces */
static char restampqind   = 0;    /* Re-stamp data record/quality indicator */
static char deleteinput   = 0;    /* Delete each input file after processing */

static FileLink *filelist    = 0;
static Archive  *archiveroot = 0;

int
main (int argc, char **argv)
{
  MSRecord *msr = 0;
  MSTraceGroup *mstg = 0;
  FileLink *flp;
  Archive *arch;
  long suffix;
  int retcode;
  
  /* Signal handling, use POSIX calls with standardized semantics */
  struct sigaction sa;
  
  sa.sa_flags = SA_RESTART;
  sigemptyset (&sa.sa_mask);
  
  sa.sa_handler = term_handler;
  sigaction (SIGINT, &sa, NULL);
  sigaction (SIGQUIT, &sa, NULL);
  sigaction (SIGTERM, &sa, NULL);
  
  sa.sa_handler = SIG_IGN;
  sigaction (SIGHUP, &sa, NULL);
  sigaction (SIGPIPE, &sa, NULL);
  
  /* Process given parameters (command line and parameter file) */
  if (parameter_proc (argc, argv) < 0)
    return -1;
  
  if ( segments )
    mstg = mst_initgroup (NULL);

  flp = filelist;
  
  /* Read each input file record by record */
  while ( flp )
    {
      if ( verbose )
	fprintf (stderr, "Reading %s\n", flp->filename);
      
      while ( (retcode = ms_readmsr (&msr, flp->filename, reclen, NULL, NULL,
				     1, bindata, verbose-1)) == MS_NOERROR )
	{
	  if ( verbose )
	    msr_print (msr, 0);
	  
	  /* Re-stamp quality indicator if specified */
	  if ( restampqind )
	    {
	      if ( verbose > 1 )
		fprintf (stderr, "Re-stamping data quality indicator to '%c'\n", restampqind);
	      
	      msr->dataquality = restampqind;
	      *(msr->record + 6) = restampqind;
	    }

          /* If continuous segment files */
          if ( segments )
            {
              MSTrace *mst = 0;
              MSTrace *mstmatch = 0;
              long nmatched = 0;

              mst = mst_addmsrtogroup (mstg, msr, 0, timetol, sampratetol);

              /* If the trace is new (num of samples match) count matching */
              if ( msr->samplecnt > 0 && msr->samplecnt == mst->samplecnt )
                {
                   mstmatch = mstg->traces;
                   while ( mstmatch )
                     {
		       mstmatch = mst_findmatch (mstmatch, 0, msr->network, msr->station, msr->location, msr->channel);
		       nmatched++;
		       mstmatch = mstmatch->next; 
                     }
		   
		   if ( nmatched > 1 )
		     {
		       mst->prvtptr = (void *) nmatched;
		     }
		}

	      suffix = (long) mst->prvtptr;
            }
	  else
	    {
	      suffix = 0;
	    }
	  
	  /* Process each record with each Archive defined */
	  arch = archiveroot;
	  while ( arch )
	    {
	      ds_streamproc (&arch->datastream, msr, suffix, verbose);
	      arch = arch->next;
	    }
	}
      
      if ( retcode != MS_ENDOFFILE )
	fprintf (stderr, "Error reading %s: %s\n", flp->filename, ms_errorstr(retcode));
      
      /* Make sure everything is cleaned up */
      ms_readmsr (&msr, NULL, 0, NULL, NULL, 0, 0, 0);
      
      /* Delete the input file if requested */
      if ( deleteinput )
	{
	  if ( verbose )
	    fprintf (stderr, "Deleting input file: %s\n", flp->filename);
	  
	  if ( unlink (flp->filename) )
	    fprintf (stderr, "Error deleting file: %s\n", flp->filename);
	}
      
      flp = flp->next;
    }

  /* Close each Archive */
  arch = archiveroot;
  while ( arch )
    {
      ds_streamproc (&arch->datastream, NULL, 0, verbose);
      arch = arch->next;
    }
  
  return 0;
}  /* End of main() */


/***************************************************************************
 * parameter_proc:
 * Process the command line parameters.
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
parameter_proc (int argcount, char **argvec)
{
  int optind;
  int idletimeout   = 300; /* idle archive timeout */
  char *tptr;
  
  /* Process all command line arguments */
  for (optind = 1; optind < argcount; optind++)
    {
      if (strcmp (argvec[optind], "-V") == 0)
	{
	  fprintf (stderr, "%s version: %s\n", PACKAGE, VERSION);
	  exit (0);
	}
      else if (strcmp (argvec[optind], "-h") == 0)
	{
	  usage (0);
	  exit (0);
	}
      else if (strcmp (argvec[optind], "-H") == 0)
	{
	  usage (1);
	  exit (0);
	}
      else if (strncmp (argvec[optind], "-v", 2) == 0)
	{
	  verbose += strspn (&argvec[optind][1], "v");
	}
      else if (strcmp (argvec[optind], "-Q") == 0)
        {
	  tptr = getoptval(argcount, argvec, optind++);
          restampqind = *tptr;
	  
	  if ( ! MS_ISDATAINDICATOR (restampqind) )
	    {
	      fprintf(stderr, "Invalid data indicator: '%c'\n", restampqind);
	      exit (1);
	    }
        }
      else if (strcmp (argvec[optind], "-D") == 0)
        {
          deleteinput = 1;
        }
      else if (strcmp (argvec[optind], "-c") == 0)
        {
          segments = 1;
        }
      else if (strcmp (argvec[optind], "-B") == 0)
        {
          bindata = 1;
        }
      else if (strcmp (argvec[optind], "-tt") == 0)
        {
          timetol = strtod (getoptval(argcount, argvec, optind++), NULL);
        }
      else if (strcmp (argvec[optind], "-rt") == 0)
        {
          sampratetol = strtod (getoptval(argcount, argvec, optind++), NULL);
        }
      else if (strcmp (argvec[optind], "-r") == 0)
	{
	  reclen = strtol (getoptval(argcount, argvec, optind++), NULL, 10);
	}
      else if (strcmp (argvec[optind], "-i") == 0)
	{
	  idletimeout = strtol (getoptval(argcount, argvec, optind++), NULL, 10);
	}
      else if (strcmp (argvec[optind], "-A") == 0)
	{
	  if ( addarchive(getoptval(argcount, argvec, optind++), NULL) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-CHAN") == 0)
	{
	  if ( addarchive(getoptval(argcount, argvec, optind++), CHANLAYOUT) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-CDAY") == 0)
	{
	  if ( addarchive(getoptval(argcount, argvec, optind++), CDAYLAYOUT) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-BUD") == 0)
	{
	  if ( addarchive(getoptval(argcount, argvec, optind++), BUDLAYOUT) == -1 )
	    return -1;
	}
      else if (strcmp (argvec[optind], "-CSS") == 0)
	{
	  if ( addarchive(getoptval(argcount, argvec, optind++), CSSLAYOUT) == -1 )
	    return -1;
	}
      else if (strncmp (argvec[optind], "-", 1) == 0 &&
	       strlen (argvec[optind]) > 1 )
	{
	  fprintf(stderr, "Unknown option: %s\n", argvec[optind]);
	  exit (1);
	}
      else
	{
	  addfile (argvec[optind]);
	}
    }
  
  /* Report the program version */
  if ( verbose )
    fprintf (stderr, "%s version: %s\n", PACKAGE, VERSION);
  
  /* Make sure input file(s) specified */
  if ( filelist == 0 )
    {
      if ( verbose )
	fprintf (stderr, "Reading from standard input\n");
      
      addfile ("-");
    }
    
  /* If no archiving is specified print a warning */
  if ( ! archiveroot )
    {
      fprintf (stderr, "WARNING: no target archives were specified, try '-h' for usage\n");
    }
  /* Otherwise fill in the global parameters for each entry */
  else
    {
      Archive *curarch = archiveroot;
      
      while ( curarch != NULL )
	{
	  curarch->datastream.idletimeout = idletimeout;
	  curarch = curarch->next;
	}
    }
  
  return 0;
}  /* End of parameter_proc() */


/***************************************************************************
 * getoptval:
 * Return the value to a command line option; checking that the value is 
 * itself not an option (starting with '-') and is not past the end of
 * the argument list.
 *
 * argcount: total arguments in argvec
 * argvec: argument list
 * argopt: index of option to process, value is expected to be at argopt+1
 *
 * Returns value on success and exits with error message on failure
 ***************************************************************************/
static char *
getoptval (int argcount, char **argvec, int argopt)
{
  if ( argvec == NULL || argvec[argopt] == NULL ) {
    fprintf (stderr, "getoptval(): NULL option requested\n");
    exit (1);
  }
  
  if ( (argopt+1) < argcount && *argvec[argopt+1] != '-' )
    return argvec[argopt+1];
  
  fprintf (stderr, "Option %s requires a value\n", argvec[argopt]);
  exit (1);
}  /* End of getoptval() */


/***************************************************************************
 * addarchive:
 * Add entry to the data stream archive chain.  'layout' if defined
 * will be appended to 'path'.
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
addarchive( const char *path, const char *layout )
{
  Archive *newarch;
  int pathlayout;
  
  if ( ! path )
    {
      fprintf (stderr, "addarchive: cannot add archive with empty path\n");
      return -1;
    }

  newarch = (Archive *) malloc (sizeof (Archive));
  
  if ( newarch == NULL )
    {
      fprintf (stderr, "addarchive: cannot allocate memory for new archive definition\n");
      return -1;
    }
  
  /* Setup new entry and add it to the front of the chain */
  pathlayout = strlen (path) + 2;
  if ( layout )
    pathlayout += strlen (layout);
  
  newarch->datastream.path = (char *) malloc (pathlayout);
  
  if ( layout )
    snprintf (newarch->datastream.path, pathlayout, "%s/%s", path, layout);
  else
    snprintf (newarch->datastream.path, pathlayout, "%s", path);
  
  newarch->datastream.grouproot = NULL;
  
  if ( newarch->datastream.path == NULL )
    {
      fprintf (stderr, "addarchive: cannot allocate memory for new archive path\n");
      if ( newarch )
	free (newarch);
      return -1;
    }
  
  newarch->next = archiveroot;
  archiveroot = newarch;
  
  return 0;
}  /* End of addarchive() */


/***************************************************************************
 * addfile:
 *
 * Add file to end of the global file list (filelist).
 ***************************************************************************/
static void
addfile (char *filename)
{
  FileLink *lastlp, *newlp;
  
  if ( filename == NULL )
    {
      fprintf (stderr, "addfile(): No file name specified\n");
      return;
    }
  
  lastlp = filelist;
  while ( lastlp != 0 )
    {
      if ( lastlp->next == 0 )
        break;
      
      lastlp = lastlp->next;
    }
  
  newlp = (FileLink *) malloc (sizeof (FileLink));
  newlp->filename = strdup(filename);
  newlp->next = 0;
  
  if ( lastlp == 0 )
    filelist = newlp;
  else
    lastlp->next = newlp;
  
}  /* End of addfile() */


/***************************************************************************
 * usage:
 * Print the usage message and exit.
 ***************************************************************************/
static void
usage (int level)
{
  fprintf (stderr, "%s version: %s\n\n", PACKAGE, VERSION);
  fprintf (stderr, "Write input Mini-SEED records to user defined file and directory layouts.\n\n");
  fprintf (stderr, "Usage: %s [options] [file1 file2 file3 ...]\n\n", PACKAGE);
  fprintf (stderr,
	   " ## Options ##\n"
	   " -V             Report program version\n"
	   " -h             Show this usage message\n"
	   " -H             Show usage message with 'format' details\n"
	   " -v             Be more verbose, multiple flags can be used\n"
	   " -Q DRQ         Re-stamp input data records with specified quality: D, R, or Q\n"
	   " -D             Delete each input file after processing\n"
           " -c             Split files on continuous segments, suffix will be added\n"
	   " -B             Write binary data samples instead of Mini-SEED records\n"
           " -tt secs       Specify a time tolerance for continuous segments\n"
           " -rt diff       Specify a sample rate tolerance for continuous segments\n"
	   " -r reclen      Specify input record length in bytes, default is autodetection\n"
	   " -i timeout     Idle stream entries might be closed (seconds), default 300\n"
	   " -A format      Write all records is a custom directory/file layout (try -H)\n"
	   "\n"
	   " # Preset format layouts #\n"
	   " -CHAN dir      Write all records into separate Net.Sta.Loc.Chan files\n"
	   " -CDAY dir      Write all records into separate Net.Sta.Loc.Chan-day files\n"
	   " -BUD BUDdir    Write all records in a BUD file layout\n"

	   " -CSS CSSdir    Write all records in a CSS-like file layout\n"
	   "\n"
	   " file(s)        File(s) of input Mini-SEED, default is stdin\n"
	   "\n");

  if  ( level )
    {
      fprintf (stderr,
	       "\n"
	       "The archive 'format' argument is expanded for each record using the\n"
	       "following flags:\n"
	       "\n"
	       "  n : network code, white space removed\n"
	       "  s : station code, white space removed\n"
	       "  l : location code, white space removed\n"
	       "  c : channel code, white space removed\n"
	       "  Y : year, 4 digits\n"
	       "  y : year, 2 digits zero padded\n"
	       "  j : day of year, 3 digits zero padded\n"
	       "  H : hour, 2 digits zero padded\n"
	       "  M : minute, 2 digits zero padded\n"
	       "  S : second, 2 digits zero padded\n"
	       "  F : fractional seconds, 4 digits zero padded\n"
	       "  q : single character record quality indicator (D, R, Q)\n"
	       "  L : data record length in bytes\n"
	       "  r : Sample rate (Hz) as a rounded integer\n"
	       "  R : Sample rate (Hz) as a float with 6 digit precision\n"
	       "  %% : the percent (%%) character\n"
	       "  # : the number (#) character\n"
	       "\n"
	       "The flags are prefaced with either the %% or # modifier.  The %% modifier\n"
	       "indicates a defining flag while the # indicates a non-defining flag.\n"
	       "All records with the same set of defining flags will be written to the\n"
	       "same file. Non-defining flags will be expanded using the values in the\n"
	       "first record for the resulting file name.\n"
	       "\n");
    }
}  /* End of usage() */


/***************************************************************************
 * term_handler:
 * Signal handler routine.
 ***************************************************************************/
static void
term_handler (int sig)
{
  exit (0);
}
