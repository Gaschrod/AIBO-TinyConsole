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

COMPONENTS=HelloWorld HelloWorld-stubgen ObjectComm ObjectComm-multi SoundPlay SoundRec PowerMonitor ImageObserver MoNet Crash RobotDesign EchoServer EchoClient TinyFTPD ImageCapture ERA201D1Info DNSLookUp UDPEchoServer BattChecker W3AIBO NTP
TARGETS=all install clean

.PHONY: $(TARGETS)

$(TARGETS):
	for dir in $(COMPONENTS); do \
		$(MAKE) -C $$dir $@ ; \
	done
