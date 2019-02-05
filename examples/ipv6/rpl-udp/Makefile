all: udp-client udp-server
CONTIKI=../../..

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

ifdef WITH_COMPOWER
APPS+=powertrace
CFLAGS+= -DCONTIKIMAC_CONF_COMPOWER=1 -DWITH_COMPOWER=1 -DQUEUEBUF_CONF_NUM=4
endif

ifdef SERVER_REPLY
CFLAGS+=-DSERVER_REPLY=$(SERVER_REPLY)
endif
ifdef PERIOD
CFLAGS+=-DPERIOD=$(PERIOD)
endif

ifeq ($(MAKE_WITH_NON_STORING),1)
CFLAGS += -DWITH_NON_STORING=1
endif
# For the use of the new archtecture, this declaration is necessary
#CONTIKI_COMM_NEW_STACK = IoTUS
CONTIKI_WITH_RIME = 1
CONTIKI_WITH_IPV6 = 1

include $(CONTIKI)/Makefile.include
