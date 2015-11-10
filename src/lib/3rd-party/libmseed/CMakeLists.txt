SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

SET(MSEED_HEADERS
    libmseed.h
    lmplatform.h
    packdata.h
    steimdata.h
    unpackdata.h
)
SET(MSEED_SOURCES
    fileutils.c
    genutils.c
    gswap.c
    lmplatform.c
    logging.c
    lookup.c
    msrutils.c
    pack.c
    packdata.c
    parseutils.c
    selection.c
    tracelist.c
    traceutils.c
    unpack.c
    unpackdata.c
)

ADD_DEFINITIONS("-fPIC")
ADD_LIBRARY(mseed STATIC ${MSEED_HEADERS} ${MSEED_SOURCES})
