ADD_SUBDIRECTORY(configfile)

# Add libmseed at top level
ADD_SUBDIRECTORY(libmseed)

# Include libmseed directory when building those...
ADD_SUBDIRECTORY(msi)
ADD_SUBDIRECTORY(msmod)
ADD_SUBDIRECTORY(msrouter)