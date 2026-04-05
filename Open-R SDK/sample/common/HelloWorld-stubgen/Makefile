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

RPCOMPONENTS=HelloWorld-stubgen
COMPONENTS=../PowerMonitor/PowerMonitor
INSTALLDIR=$(shell pwd)/MS
TARGETS=all install clean

.PHONY: $(TARGETS)

$(TARGETS):
	for c in $(RPCOMPONENTS); do \
		$(MAKE) -C $$c/robot INSTALLDIR=$(INSTALLDIR) $@ ; \
	done
	for c in $(COMPONENTS); do \
		$(MAKE) -C $$c INSTALLDIR=$(INSTALLDIR) $@ ; \
	done
