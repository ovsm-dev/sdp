
#ifndef DSARCHIVE_H
#define DSARCHIVE_H

#include <time.h>

/* Define pre-formatted archive layouts */
#define CHANLAYOUT  "%n.%s.%l.%c"
#define CDAYLAYOUT  "%n.%s.%l.%c.%Y:%j:#H:#M:#S"
#define BUDLAYOUT   "%n/%s/%s.%n.%l.%c.%Y.%j"
#define CSSLAYOUT   "%Y/%j/%s.%c.%Y:%j:#H:#M:#S"

typedef struct DataStreamGroup_s
{
  char   *defkey;
  int     filed;
  time_t  modtime;
  struct  DataStreamGroup_s *next;
}
DataStreamGroup;

typedef struct DataStream_s
{
  char   *path;
  int     idletimeout;
  struct  DataStreamGroup_s *grouproot;
}
DataStream;

extern int ds_streamproc (DataStream *datastream, MSRecord *msr,
                          long suffix, int verbose);

#endif /* DSARCHIVE_H */
