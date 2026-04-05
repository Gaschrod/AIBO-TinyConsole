#
# Copyright 2004 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

OPENRSDK_ROOT?=/usr/local/OPEN_R_SDK
CP=cp
INSTALLDIR=../../RP/host/MS
STRIP=strip
MKBIN=g++
STUBGEN=$(OPENRSDK_ROOT)/RP_OPEN_R/bin/rp-openr-stubgen2
MKBINFLAGS=
OPENRCONFIG=$(OPENRSDK_ROOT)/RP_OPEN_R/bin/rp-openr-config
LIBS=`$(OPENRCONFIG) --libs`
CXXFLAGS= \
	-O2 \
	-g \
	-I. \
	-I.. \
	`$(OPENRCONFIG) --cflags`
OCF=

VPATH=..
include ../Makefile.common
