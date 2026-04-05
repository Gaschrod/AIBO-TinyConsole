#
# Copyright 2002,2004 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

HOST_COMPONENTS=../../SampleSubject1 ../../SampleSubject2
INSTALLROOT=$(shell pwd)
TARGETS=all install clean

.PHONY: $(TARGETS)

$(TARGETS):
	for c in $(HOST_COMPONENTS); do \
		$(MAKE) -C $$c/host INSTALLDIR=$(INSTALLROOT)/MS $@ ; \
	done
