NAME 		= polymarker
DIR_SRC = $(DIR_SERVICES_ROOT_SRC)/polymarker/src
DIR_INCLUDE = $(DIR_SERVICES_ROOT_SRC)/polymarker/include
CC		= g++
CXX		= g++
MAKEDEPEND 	= gcc -MM -MF



ifeq ($(DIR_ROOT),)
export DIR_ROOT = $(realpath $(../..))
endif

ifneq ($(PROPERTIES_SET),1)
include $(DIR_BUILD_ROOT)/project.properties
endif

export DIR_INSTALL=$(DIR_GRASSROOTS_INSTALL)/services

BUILD		= debug


VPATH	= \
	$(DIR_SRC) \
	

INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_HANDLER_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_GRASSROOTS_PARAMS_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_HTSLIB_INC)
	
SRCS 	= \
	polymarker_service.c \
	polymarker_service_job.c \
	polymarker_tool.cpp \
	system_polymarker_tool.cpp

CPPFLAGS += -DPOLYMARKER_LIBRARY_EXPORTS 

LDFLAGS += -L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_HTSLIB_LIB) -lhts \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_SERVICE_LIB) -l$(GRASSROOTS_SERVICE_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_PARAMS_LIB) -l$(GRASSROOTS_PARAMS_LIB_NAME) \
	-lpthread


include $(DIR_BUILD_ROOT)/generic_makefiles/shared_library.makefile

