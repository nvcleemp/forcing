#
# Makefile for forcing
#
#
# Main developer: Nico Van Cleemput
# In collaboration with: Craig Larson
# 
# Copyright (C) 2013 Ghent University.
# Licensed under the GNU GPL, read the file LICENSE.txt for details.
#

SHELL = /bin/sh

CC32 = gcc 
CC64 = gcc 
CFLAGS = -O4 -Wall
COMPLETE = forcing forcing-64 forcing-profile forcing-debug
SOURCES = forcing.c freqtable.c Makefile COPYRIGHT.txt LICENSE.txt
FORCING_SOURCES = forcing.c cliquer.c graph.c reorder.c freqtable.c

all : 32bit

complete: $(COMPLETE)

32bit: forcing

64bit : forcing-64

profile : forcing-profile

debug : forcing-debug

forcing: $(FORCING_SOURCES)
	${CC32} $(CFLAGS) $(FORCING_SOURCES) -o forcing

forcing-64: $(FORCING_SOURCES)
	${CC64} $(CFLAGS) $(FORCING_SOURCES) -o forcing-64

forcing-profile: $(FORCING_SOURCES)
	${CC32} -Wall -pg -g $(FORCING_SOURCES) -o forcing-profile

forcing-debug: $(FORCING_SOURCES)
	${CC32} -Wall -rdynamic -g $(FORCING_SOURCES) -o forcing-debug

sources: forcing-sources.zip forcing-sources.tar.gz

forcing-sources.zip: $(SOURCES)
	zip forcing-sources $(SOURCES)

forcing-sources.tar.gz: $(SOURCES)
	tar czf forcing-sources.tar.gz $(SOURCES)

clean:
	rm -f $(COMPLETE)
