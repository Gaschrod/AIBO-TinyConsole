#
# Copyright 2002,2003,2004 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

COMPONENTS=../../../../common/PowerMonitor/PowerMonitor
INSTALLROOT=$(shell pwd)
TARGETS=all install clean

.PHONY: $(TARGETS)

$(TARGETS):
	for c in $(COMPONENTS); do \
		$(MAKE) -C $$c INSTALLDIR=$(INSTALLROOT)/MS $@ ; \
	done
