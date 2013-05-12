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
COMPLETE = build/forcing build/forcing-64 build/forcing-profile build/forcing-debug
SOURCES = forcing.c freqtable.c Makefile COPYRIGHT.txt LICENSE.txt
FORCING_SOURCES = forcing.c cliquer.c graph.c reorder.c freqtable.c

all : 32bit

complete: $(COMPLETE)

32bit: build/forcing

64bit : build/forcing-64

profile : build/forcing-profile

debug : build/forcing-debug

build/forcing: $(FORCING_SOURCES)
	mkdir -p build
	${CC32} $(CFLAGS) $(FORCING_SOURCES) -o build/forcing

build/forcing-64: $(FORCING_SOURCES)
	mkdir -p build
	${CC64} $(CFLAGS) $(FORCING_SOURCES) -o build/forcing-64

build/forcing-profile: $(FORCING_SOURCES)
	mkdir -p build
	${CC32} -Wall -pg -g $(FORCING_SOURCES) -o build/forcing-profile

build/forcing-debug: $(FORCING_SOURCES)
	mkdir -p build
	${CC32} -Wall -rdynamic -g $(FORCING_SOURCES) -o build/forcing-debug

sources: dist/forcing-sources.zip dist/forcing-sources.tar.gz

dist/forcing-sources.zip: $(SOURCES)
	mkdir -p dist
	zip dist/forcing-sources $(SOURCES)

dist/forcing-sources.tar.gz: $(SOURCES)
	mkdir -p dist
	tar czf dist/forcing-sources.tar.gz $(SOURCES)

clean:
	rm -rf build
	rm -rf dist
