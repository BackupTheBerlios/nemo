#########################################################################
# nemo makefile
# 
# Mahmoud Al Gammal
# 22/1/2004
#########################################################################

#########################################################################
# VARIABLES
#########################################################################

# output directories
BUILD_DIR	:= ./build
OBJ_DIR		:= $(BUILD_DIR)/obj
LIB_DIR		:= $(BUILD_DIR)/lib
BIN_DIR		:= $(BUILD_DIR)/bin

# include directories
INCLUDE_BASE		:= ./headers
INCLUDE_POSIX		:= $(INCLUDE_BASE)/posix


# Be includes
INCLUDE_BE		:= $(INCLUDE_BASE)/be
INCLUDE_ADDONS		:= $(INCLUDE_BE)/add-ons
INCLUDE_APP		:= $(INCLUDE_BE)/app
INCLUDE_DEVICE		:= $(INCLUDE_BE)/device
INCLUDE_DRIVERS		:= $(INCLUDE_BE)/drivers
INCLUDE_GAME		:= $(INCLUDE_BE)/game
INCLUDE_INTERFACE	:= $(INCLUDE_BE)/interface
INCLUDE_KERNEL		:= $(INCLUDE_BE)/kernel
INCLUDE_MAIL		:= $(INCLUDE_BE)/mail
INCLUDE_MEDIA		:= $(INCLUDE_BE)/media
INCLUDE_MIDI		:= $(INCLUDE_BE)/midi
INCLUDE_MIDI2		:= $(INCLUDE_BE)/midi2
INCLUDE_NET			:= $(INCLUDE_BE)/net
INCLUDE_STORAGE		:= $(INCLUDE_BE)/storage
INCLUDE_SUPPORT		:= $(INCLUDE_BE)/support
INCLUDE_TRANSLATION	:= $(INCLUDE_BE)/translation

INCLUDE_ALL_BE	:=	-I$(INCLUDE_BE) -I$(INCLUDE_ADDONS) \
					-I$(INCLUDE_APP) -I$(INCLUDE_DEVICE) \
					-I$(INCLUDE_DRIVERS) -I$(INCLUDE_GAME) \
					-I$(INCLUDE_INTERFACE) -I$(INCLUDE_KERNEL) \
					-I$(INCLUDE_MAIL) -I$(INCLUDE_MEDIA) \
					-I$(INCLUDE_MIDI) -I$(INCLUDE_MIDI2) \
					-I$(INCLUDE_NET) -I$(INCLUDE_STORAGE) \
					-I$(INCLUDE_SUPPORT) -I$(INCLUDE_TRANSLATION)

# private includes
INCLUDE_PRIVATE		:= $(INCLUDE_BASE)/private
INCLUDE_PRIVATE_APP	:= $(INCLUDE_PRIVATE)/app
# next 2 lines added by Fadi Raafat Rezk EDWARD
INCLUDE_PRIVATE_MEDIA	:= $(INCLUDE_PRIVATE)/media
INCLUDE_PRIVATE_SHARED	:= $(INCLUDE_PRIVATE)/shared


INCLUDE_ALL_PRIVATE	:= -I$(INCLUDE_PRIVATE) -I$(INCLUDE_PRIVATE_APP) \
						-I$(INCLUDE_PRIVATE_MEDIA) \
						-I$(INCLUDE_PRIVATE_SHARED) \
						-I$(SRC_BASE)/servers/media

# use this to include everything
INCLUDE_ALL		:= -I$(INCLUDE_POSIX) $(INCLUDE_ALL_BE) $(INCLUDE_ALL_PRIVATE)

# source directories
SRC_BASE	:= ./src
# By Fadi and Raymond on 22-2-2004
INCLUDE_SERVER		:=$(SRC_BASE)/servers
INCLUDE_SERVER_MEDIA	:=$(INCLUDE_SERVER)/media

#########################################################################
# COMPILER FLAGS
#########################################################################

CXXFLAGS	:= -g -D__NEMO__ -DDEBUG=1 -D_REENTRANT	-Wno-multichar

#########################################################################
# TARGET all
#########################################################################

all:
	make app_server
	make input_server
	make test_app
	
#########################################################################
# TARGET libroot.so
#########################################################################

NEMO_DEP	:= 	$(INCLUDE_PRIVATE)/nemo_debug.h \
				$(INCLUDE_PRIVATE)/nemo_debug.c

LIBROOT_DEP	:=	$(INCLUDE_KERNEL)/OS.h \
				$(SRC_BASE)/kernel/libroot/os/port.cpp \
				$(SRC_BASE)/kernel/libroot/os/sem.cpp \
				$(SRC_BASE)/kernel/libroot/os/thread.cpp

LIBROOT_INCLUDES:=	-I$(INCLUDE_PRIVATE) -I$(INCLUDE_BE) -I$(INCLUDE_KERNEL) -I$(INCLUDE_SUPPORT) -I$(INCLUDE_STORAGE)

libroot: $(NEMO_DEP) $(LIBROOT_DEP)
####### compiling
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBROOT_INCLUDES) -o $(OBJ_DIR)/nemo_debug.o $(INCLUDE_PRIVATE)/nemo_debug.c
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBROOT_INCLUDES) -o $(OBJ_DIR)/port.o $(SRC_BASE)/kernel/libroot/os/port.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBROOT_INCLUDES) -o $(OBJ_DIR)/sem.o $(SRC_BASE)/kernel/libroot/os/sem.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBROOT_INCLUDES) -o $(OBJ_DIR)/thread.o $(SRC_BASE)/kernel/libroot/os/thread.cpp

####### linking
	$(CXX) -shared -fPIC -o $(LIB_DIR)/libroot.so \
	$(OBJ_DIR)/nemo_debug.o $(OBJ_DIR)/port.o $(OBJ_DIR)/sem.o $(OBJ_DIR)/thread.o \
	-lpthread

#######  install lib
#	cp -f $(LIB_DIR)/libroot.so /usr/lib

#########################################################################
# TARGET libbe.so
#########################################################################

LIBBE_DEP	:=	libroot \
				$(INCLUDE_BE)/*.h \
				$(INCLUDE_APP)/*.h $(INCLUDE_PRIVATE_APP)/*.h $(SRC_BASE)/kits/app/*.cpp \
				$(INCLUDE_DEVICE)/*.h \
				$(INCLUDE_DRIVERS)/*.h \
				$(INCLUDE_GAME)/*.h \
				$(INCLUDE_INTERFACE)/*.h \
				$(INCLUDE_KERNEL)/*.h \
				$(INCLUDE_MAIL)/*.h \
				$(INCLUDE_MEDIA)/*.h \
				$(INCLUDE_MIDI)/*.h \
				$(INCLUDE_MIDI2)/*.h \
				$(INCLUDE_NET)/*.h \
				$(INCLUDE_STORAGE)/*.h \
				$(INCLUDE_SUPPORT)/*.h $(SRC_BASE)/kits/support/*.cpp \
				$(INCLUDE_TRANSLATION)/*.h \

LIBBE_INCLUDES:=	$(INCLUDE_ALL_BE) $(INCLUDE_ALL_PRIVATE)

libbe: $(LIBBE_DEP)
####### compiling
#	Support Kit
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBBE_INCLUDES) -o $(OBJ_DIR)/Archivable.o $(SRC_BASE)/kits/support/Archivable.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBBE_INCLUDES) -o $(OBJ_DIR)/List.o $(SRC_BASE)/kits/support/List.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBBE_INCLUDES) -o $(OBJ_DIR)/Locker.o $(SRC_BASE)/kits/support/Locker.cpp
#	Application Kit
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBBE_INCLUDES) -o $(OBJ_DIR)/Handler.o $(SRC_BASE)/kits/app/Handler.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBBE_INCLUDES) -o $(OBJ_DIR)/Looper.o $(SRC_BASE)/kits/app/Looper.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBBE_INCLUDES) -o $(OBJ_DIR)/ObserverList.o $(SRC_BASE)/kits/app/ObserverList.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBBE_INCLUDES) -o $(OBJ_DIR)/TokenSpace.o $(SRC_BASE)/kits/app/TokenSpace.cpp

####### linking
	$(CXX) -shared -fPIC -o $(LIB_DIR)/libbe.so \
	$(OBJ_DIR)/Archivable.o \
	$(OBJ_DIR)/List.o \
	$(OBJ_DIR)/Locker.o \
	-lroot

#######  install lib
#	cp -f $(LIB_DIR)/libbe.so /usr/lib

###########################################
## Code added by Fadi Raafat Rezk EDWARD ##
###########################################


#########################################################################
# TARGET libmedia.so
#########################################################################

LIBMEDIA_DEP	:=	libroot \
			libbe	\
				$(INCLUDE_BE)/*.h \
				$(INCLUDE_APP)/*.h \
				$(INCLUDE_DEVICE)/*.h \
				$(INCLUDE_DRIVERS)/*.h \
				$(INCLUDE_GAME)/*.h \
				$(INCLUDE_INTERFACE)/*.h \
				$(INCLUDE_KERNEL)/*.h \
				$(INCLUDE_MAIL)/*.h \
				$(INCLUDE_MEDIA)/*.h $(INCLUDE_PRIVATE_MEDIA)/*.h $(SRC_BASE)/servers/media/*.h $(INCLUDE_SERVER_MEDIA)/*.h $(SRC_BASE)/kits/media/*.cpp $(INCLUDE_PRIVATE_SHARED)/*.h \
				$(INCLUDE_MIDI)/*.h \
				$(INCLUDE_MIDI2)/*.h \
				$(INCLUDE_NET)/*.h \
				$(INCLUDE_STORAGE)/*.h \
				$(INCLUDE_SUPPORT)/*.h \
				$(INCLUDE_TRANSLATION)/*.h \

LIBMEDIA_INCLUDES:=	$(INCLUDE_ALL_BE) $(INCLUDE_ALL_PRIVATE) 

libmedia: $(LIBMEDIA_DEP)
####### compiling
#	Media Kit
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaNode.o $(SRC_BASE)/kits/media/MediaNode.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/BufferConsumer.o $(SRC_BASE)/kits/media/BufferConsumer.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/BufferProducer.o $(SRC_BASE)/kits/media/BufferProducer.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/Buffer.o $(SRC_BASE)/kits/media/Buffer.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/BufferGroup.o $(SRC_BASE)/kits/media/BufferGroup.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/Controllable.o $(SRC_BASE)/kits/media/Controllable.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/FileInterface.o $(SRC_BASE)/kits/media/FileInterface.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaEventLooper.o $(SRC_BASE)/kits/media/MediaEventLooper.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimeSource.o $(SRC_BASE)/kits/media/TimeSource.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimedEventQueue.o $(SRC_BASE)/kits/media/TimedEventQueue.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimedEventQueuePrivate.o $(SRC_BASE)/kits/media/TimedEventQueuePrivate.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimeCode.o $(SRC_BASE)/kits/media/TimeCode.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaAddOn.o $(SRC_BASE)/kits/media/MediaAddOn.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/BufferIdCache.o $(SRC_BASE)/kits/media/BufferIdCache.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/ChannelMixer.o $(SRC_BASE)/kits/media/ChannelMixer.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/DataExchange.o $(SRC_BASE)/kits/media/DataExchange.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/DefaultMediaTheme.o $(SRC_BASE)/kits/media/DefaultMediaTheme.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaFiles.o $(SRC_BASE)/kits/media/MediaFiles.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/Sound.o $(SRC_BASE)/kits/media/Sound.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaEncoder.o $(SRC_BASE)/kits/media/MediaEncoder.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TrackReader.o $(SRC_BASE)/kits/media/TrackReader.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimeSourceObjectManager.o $(SRC_BASE)/kits/media/TimeSourceObjectManager.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimeSourceObject.o $(SRC_BASE)/kits/media/TimeSourceObject.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/SharedBufferList.o $(SRC_BASE)/kits/media/SharedBufferList.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/SamplingrateConverter.o $(SRC_BASE)/kits/media/SamplingrateConverter.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/SampleConverter.o $(SRC_BASE)/kits/media/SampleConverter.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/RealtimeAlloc.o $(SRC_BASE)/kits/media/RealtimeAlloc.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/ReaderPlugin.o $(SRC_BASE)/kits/media/ReaderPlugin.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/PortPool.o $(SRC_BASE)/kits/media/PortPool.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/PlaySound.o $(SRC_BASE)/kits/media/PlaySound.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/Notifications.o $(SRC_BASE)/kits/media/Notifications.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/!missing_symbols.o $(SRC_BASE)/kits/media/!missing_symbols.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaTheme.o $(SRC_BASE)/kits/media/MediaTheme.cpp
#	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaPlugin.o $(SRC_BASE)/kits/media/MediaPlugin.cpp


####### linking
	$(CXX) -shared -fPIC -o $(LIB_DIR)/libmedia.so \
	$(OBJ_DIR)/MediaNode.o \
	$(OBJ_DIR)/BufferConsumer.o \
	$(OBJ_DIR)/BufferProducer.o \
	$(OBJ_DIR)/Buffer.o \
	$(OBJ_DIR)/BufferGroup.o \
	$(OBJ_DIR)/Controllable.o \
	$(OBJ_DIR)/FileInterface.o \
	$(OBJ_DIR)/MediaEventLooper.o \
	$(OBJ_DIR)/TimeSource.o \
	$(OBJ_DIR)/TimedEventQueue.o \
	$(OBJ_DIR)/TimedEventQueuePrivate.o \
	$(OBJ_DIR)/TimeCode.o \
	$(OBJ_DIR)/MediaAddOn.o \
	$(OBJ_DIR)/BufferIdCache.o \
	$(OBJ_DIR)/ChannelMixer.o \
	$(OBJ_DIR)/DataExchange.o \
	$(OBJ_DIR)/DefaultMediaTheme.o \
	$(OBJ_DIR)/MediaFiles.o \
	$(OBJ_DIR)/Sound.o \
	$(OBJ_DIR)/MediaEncoder.o \
	$(OBJ_DIR)/TrackReader.o \
	$(OBJ_DIR)/TimeSourceObjectManager.o \
	$(OBJ_DIR)/TimeSourceObject.o \
	$(OBJ_DIR)/SharedBufferList.o \
	$(OBJ_DIR)/SamplingrateConverter.o \
	$(OBJ_DIR)/SampleConverter.o \
	$(OBJ_DIR)/RealtimeAlloc.o \
	$(OBJ_DIR)/ReaderPlugin.o \
	$(OBJ_DIR)/PortPool.o \
	$(OBJ_DIR)/PlaySound.o \
	$(OBJ_DIR)/Notifications.o \
	$(OBJ_DIR)/!missing_symbols.o \
	$(OBJ_DIR)/MediaTheme.o \
	$(OBJ_DIR)/MediaPlugin.o \
	-lbe \
	-lroot

	
	
libmedia2: $(LIBMEDIA_DEP)
####### compiling
#	Media Kit
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaNode.o $(SRC_BASE)/kits/media/MediaNode.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/BufferConsumer.o $(SRC_BASE)/kits/media/BufferConsumer.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/BufferProducer.o $(SRC_BASE)/kits/media/BufferProducer.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/Buffer.o $(SRC_BASE)/kits/media/Buffer.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/BufferGroup.o $(SRC_BASE)/kits/media/BufferGroup.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/Controllable.o $(SRC_BASE)/kits/media/Controllable.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/FileInterface.o $(SRC_BASE)/kits/media/FileInterface.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaEventLooper.o $(SRC_BASE)/kits/media/MediaEventLooper.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimeSource.o $(SRC_BASE)/kits/media/TimeSource.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimedEventQueue.o $(SRC_BASE)/kits/media/TimedEventQueue.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimedEventQueuePrivate.o $(SRC_BASE)/kits/media/TimedEventQueuePrivate.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimeCode.o $(SRC_BASE)/kits/media/TimeCode.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaAddOn.o $(SRC_BASE)/kits/media/MediaAddOn.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/BufferIdCache.o $(SRC_BASE)/kits/media/BufferIdCache.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/ChannelMixer.o $(SRC_BASE)/kits/media/ChannelMixer.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/DataExchange.o $(SRC_BASE)/kits/media/DataExchange.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/DefaultMediaTheme.o $(SRC_BASE)/kits/media/DefaultMediaTheme.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaFiles.o $(SRC_BASE)/kits/media/MediaFiles.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/Sound.o $(SRC_BASE)/kits/media/Sound.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaEncoder.o $(SRC_BASE)/kits/media/MediaEncoder.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TrackReader.o $(SRC_BASE)/kits/media/TrackReader.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimeSourceObjectManager.o $(SRC_BASE)/kits/media/TimeSourceObjectManager.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/TimeSourceObject.o $(SRC_BASE)/kits/media/TimeSourceObject.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/SharedBufferList.o $(SRC_BASE)/kits/media/SharedBufferList.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/SamplingrateConverter.o $(SRC_BASE)/kits/media/SamplingrateConverter.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/SampleConverter.o $(SRC_BASE)/kits/media/SampleConverter.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/RealtimeAlloc.o $(SRC_BASE)/kits/media/RealtimeAlloc.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/ReaderPlugin.o $(SRC_BASE)/kits/media/ReaderPlugin.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/PortPool.o $(SRC_BASE)/kits/media/PortPool.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/PlaySound.o $(SRC_BASE)/kits/media/PlaySound.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/Notifications.o $(SRC_BASE)/kits/media/Notifications.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/!missing_symbols.o $(SRC_BASE)/kits/media/!missing_symbols.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaTheme.o $(SRC_BASE)/kits/media/MediaTheme.cpp
	$(CXX) $(CXXFLAGS) -c -fPIC $(LIBMEDIA_INCLUDES) -o $(OBJ_DIR)/MediaPlugin.o $(SRC_BASE)/kits/media/MediaPlugin.cpp

####### linking
	$(CXX) -shared -fPIC -o $(LIB_DIR)/libmedia.so \
	$(OBJ_DIR)/MediaNode.o \
	$(OBJ_DIR)/BufferConsumer.o \
	$(OBJ_DIR)/BufferProducer.o \
	$(OBJ_DIR)/Buffer.o \
	$(OBJ_DIR)/BufferGroup.o \
	$(OBJ_DIR)/Controllable.o \
	$(OBJ_DIR)/FileInterface.o \
	$(OBJ_DIR)/MediaEventLooper.o \
	$(OBJ_DIR)/TimeSource.o \
	$(OBJ_DIR)/TimedEventQueue.o \
	$(OBJ_DIR)/TimedEventQueuePrivate.o \
	$(OBJ_DIR)/TimeCode.o \
	$(OBJ_DIR)/MediaAddOn.o \
	$(OBJ_DIR)/BufferIdCache.o \
	$(OBJ_DIR)/ChannelMixer.o \
	$(OBJ_DIR)/DataExchange.o \
	$(OBJ_DIR)/DefaultMediaTheme.o \
	$(OBJ_DIR)/MediaFiles.o \
	$(OBJ_DIR)/Sound.o \
	$(OBJ_DIR)/MediaEncoder.o \
	$(OBJ_DIR)/TrackReader.o \
	$(OBJ_DIR)/TimeSourceObjectManager.o \
	$(OBJ_DIR)/TimeSourceObject.o \
	$(OBJ_DIR)/SharedBufferList.o \
	$(OBJ_DIR)/SamplingrateConverter.o \
	$(OBJ_DIR)/SampleConverter.o \
	$(OBJ_DIR)/RealtimeAlloc.o \
	$(OBJ_DIR)/ReaderPlugin.o \
	$(OBJ_DIR)/PortPool.o \
	$(OBJ_DIR)/PlaySound.o \
	$(OBJ_DIR)/Notifications.o \
	$(OBJ_DIR)/!missing_symbols.o \
	$(OBJ_DIR)/MediaTheme.o \
	$(OBJ_DIR)/MediaPlugin.o \
	-lbe \
	-lroot	


#######  install lib
#	cp -f $(LIB_DIR)/libmedia.so /usr/lib


###########################
## Fadi's code ends here ##
###########################


#########################################################################
# TARGET app_server
#########################################################################

APP_SERVER_DEP	:=	libbe \
			$(SRC_BASE)/servers/app/*.*

APP_SERVER_INCLUDES:=	$(INCLUDE_ALL_BE) $(INCLUDE_ALL_PRIVATE) -I$(SRC_BASE)/servers/app

app_server: $(APP_SERVER_DEP)
####### compiling
	$(CXX) $(CXXFLAGS) -c $(APP_SERVER_INCLUDES) -o $(OBJ_DIR)/AppServer.o $(SRC_BASE)/servers/app/AppServer.cpp
	$(CXX) $(CXXFLAGS) -c $(APP_SERVER_INCLUDES) -o $(OBJ_DIR)/ServerApp.o $(SRC_BASE)/servers/app/ServerApp.cpp

####### linking
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/app_server \
	$(OBJ_DIR)/AppServer.o \
	$(OBJ_DIR)/ServerApp.o \
	-lbe

#########################################################################
# TARGET input_server
#########################################################################

INPUT_SERVER_DEP:=	app_server

INPUT_SERVER_INCLUDES:=	$(INCLUDE_ALL_BE) -I$(SRC_BASE)/servers/app

input_server: $(INPUT_SERVER_DEP)
####### compiling
	$(CXX) $(CXXFLAGS) -c $(INPUT_SERVER_INCLUDES) -o $(OBJ_DIR)/input_server.o $(SRC_BASE)/test/input_server.cpp

####### linking
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/input_server \
	$(OBJ_DIR)/input_server.o \
	-lroot

#########################################################################
# TARGET test_app
#########################################################################

TEST_APP_DEP	:= 	app_server
TEST_APP_INCLUDES:=	$(INCLUDE_ALL_BE) -I$(SRC_BASE)/servers/app

test_app: $(TEST_APP_DEP)
####### compiling
	$(CXX) $(CXXFLAGS) -c $(TEST_APP_INCLUDES) -o $(OBJ_DIR)/test_app.o $(SRC_BASE)/test/test_app.cpp
####### linking
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/test_app \
	$(OBJ_DIR)/test_app.o \
	-lroot

#########################################################################
# Code by fadi and Raymond on the 25/2/2004
#########################################################################

#########################################################################
# TARGET node_test
#########################################################################

MEDIA_TEST_DEP	:= 	libmedia \
			$(INCLUDE_MEDIA)/*.h \
			$(SRC_BASE)/test/media/node_test/*.*
MEDIA_TEST_INCLUDES:=	$(INCLUDE_ALL_BE) \
			-I$(SRC_BASE)/test/media/node_test
			#$(INCLUDE_POSIX) 
#-I$(SRC_BASE)/servers/app

node_test: $(MEDIA_TEST_DEP)
####### compiling
	$(CXX) $(CXXFLAGS) -c $(MEDIA_TEST_INCLUDES) -o $(OBJ_DIR)/nodetest.o $(SRC_BASE)/test/media/node_test/main.cpp
	$(CXX) $(CXXFLAGS) -c $(MEDIA_TEST_INCLUDES) -o $(OBJ_DIR)/ProducerNode.o $(SRC_BASE)/test/media/node_test/ProducerNode.cpp
	$(CXX) $(CXXFLAGS) -c $(MEDIA_TEST_INCLUDES) -o $(OBJ_DIR)/ConsumerNode.o $(SRC_BASE)/test/media/node_test/ConsumerNode.cpp
	$(CXX) $(CXXFLAGS) -c $(MEDIA_TEST_INCLUDES) -o $(OBJ_DIR)/misc.o $(SRC_BASE)/test/media/node_test/misc.cpp
####### linking
#	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/node_test \
#	$(OBJ_DIR)/nodetest.o \
#	$(OBJ_DIR)/ProducerNode.o \
#	$(OBJ_DIR)/ConsumerNode.o \
#	-lmedia \
#	-lbe \
#	-lroot


#########################################################################
# Their code ends here
#########################################################################	


# sem_test
#=========
sem_test:	headers/private/nemo_debug.o \
			src/kernel/libroot/os/sem.o \
			src/kernel/libroot/os/thread.o \
			src/kits/support/Locker.o \
			test/sem_test.o

	${CXX} ${CXXFLAGS} -o test/bin/sem_test \
	headers/private/nemo_debug.o \
	src/kernel/libroot/os/sem.o \
	src/kernel/libroot/os/thread.o \
	src/kits/support/Locker.o \
	test/sem_test.o \
	-lpthread
	make clean_obj

# locker_test
#=========
locker_test:	headers/private/nemo_debug.o \
				src/kernel/libroot/os/sem.o \
				src/kernel/libroot/os/thread.o \
				src/kits/support/Locker.o \
				test/locker_test.o

	${CXX} ${CXXFLAGS} -o test/bin/locker_test \
	headers/private/nemo_debug.o \
	src/kernel/libroot/os/sem.o \
	src/kernel/libroot/os/thread.o \
	src/kits/support/Locker.o \
	test/locker_test.o \
	-lpthread
	make clean_obj

# handler_test
#=========
handler_test:	headers/private/nemo_debug.o \
				src/kernel/libroot/os/sem.o \
				src/kernel/libroot/os/thread.o \
				src/kits/support/Locker.o \
				src/kits/support/List.o \
				src/kits/app/TokenSpace.o \
				src/kits/app/ObserverList.o \
				src/kits/app/Handler.o \
				test/handler_test.o

	${CXX} ${CXXFLAGS} -o test/bin/handler_test \
	headers/private/nemo_debug.o \
	src/kernel/libroot/os/sem.o \
	src/kernel/libroot/os/thread.o \
	src/kits/support/Locker.o \
	src/kits/support/List.o \
	src/kits/app/TokenSpace.o \
	src/kits/app/ObserverList.o \
	src/kits/app/Handler.o \
	test/handler_test.o \
	-lpthread
	make clean_obj

#########################################################################
# TARGET clean
#########################################################################

.PHONEY: clean
clean:
	rm -f $(OBJ_DIR)/* $(LIB_DIR)/* $(BIN_DIR)/*
