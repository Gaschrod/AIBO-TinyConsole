#
# Copyright 2003 Sony Corporation 
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
INSTALLDIR=../../../RP/host/MFC/MS
STRIP=strip
MKBIN=g++
STUBGEN=$(OPENRSDK_ROOT)/RP_OPEN_R/bin/rp-openr-stubgen2
MKBINFLAGS=
OPENRCONFIG=$(OPENRSDK_ROOT)/RP_OPEN_R/bin/rp-openr-config
LIBS=`$(OPENRCONFIG) --libs`
CXXFLAGS= \
	-DOPENR_WIN32API \
	-DOPENR_SHAREDMEM_FOR_MFCAPP \
	-mwindows \
	-O2 \
	-g \
	-I. \
	-I../.. \
	`$(OPENRCONFIG) --cflags`

OCF=

VPATH=../..
include ../../Makefile.common
