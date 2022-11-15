
include $(APPDIR)/Make.defs

CSRCS +=  RC.c RC_fs.c


.PHONY: RC_fs.c fsgen

RC_fs.c:fsgen
	@PATH=$(shell pwd) fsgen $(shell pwd)/fs $(shell pwd)/RC_fs.c

fsgen:
	@g++ -static -static-libgcc -static-libstdc++  $(shell pwd)/fsgen.cpp -o $(shell pwd)/fsgen


distclean::
	$(call DELFILE, RC_fs.c)
	$(call DELFILE, fsgen)

include $(APPDIR)/Application.mk
