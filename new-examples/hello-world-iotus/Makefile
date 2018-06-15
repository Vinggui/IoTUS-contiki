CONTIKI_PROJECT = hello-world-iotus
APPS+=powertrace

# For the use of the new archtecture, this declaration is necessary
CONTIKI_COMM_NEW_STACK = IoTUS

# These 3 declarations are mandatory to explicity define IoTUS
IOTUS_CONF_USING_TRANSPORT = 0
IOTUS_CONF_USING_ROUTING = 1
IOTUS_CONF_USING_DATA_LINK = 1

# User can use dynamic mode by declaring this parameter to "DYNAMIC" or "Static"
#IOTUS_CONF_COMPILE_MODE = STATIC
# If Static is used, then specific single protocols per layer must be selected
#IOTUS_CONF_STATIC_TRANSPORT_LAYER =   null_transport_v1.0.0
#IOTUS_CONF_STATIC_ROUTING_LAYER =     edytee_routing_v1.0.0
#IOTUS_CONF_STATIC_DATA_LINK_LAYER =   edytee_MAC_v1.0.0

IOTUS_CONF_COMPILE_MODE = DYNAMIC
# If dynamic mode is selected, then profiles must be selected or explicite protocols
IOTUS_CONF_DYNAMIC_MODE_PROFILES = TEST
#IOTUS_CONF_TRANSPORT_PROTOCOLS_LIST = null_v1.0.0
#IOTUS_CONF_ROUTING_PROTOCOLS_LIST = null_v1.0.0 edytee_routing_v1.0.0
#IOTUS_CONF_DATA_LINK_PROTOCOLS_LIST = null_v1.0.0 edytee_MAC_v1.0.0

all: $(CONTIKI_PROJECT)

CONTIKI = ../..
include $(CONTIKI)/Makefile.include
