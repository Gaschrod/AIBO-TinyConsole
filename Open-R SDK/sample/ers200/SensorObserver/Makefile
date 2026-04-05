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

COMPONENTS=SensorObserver ../../common/PowerMonitor/PowerMonitor
INSTALLDIR=$(shell pwd)/MS
TARGETS=all install clean

.PHONY: $(TARGETS)

$(TARGETS):
	for dir in $(COMPONENTS); do \
		$(MAKE) -C $$dir INSTALLDIR=$(INSTALLDIR) $@ ; \
	done
