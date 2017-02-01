#ifndef YABI_H__
#define YABI_H__

#include <stdint.h>
#include <stdbool.h>

//User configurable max channel. Also sets the max 'Channel ID',
//which goes from 0-> MAX_CONTROL_CHANNELS.
#define  MAX_CONTROL_CHANNELS          32

typedef enum {
   YABI_OK,
   YABI_UNIMPLIMENTED,
   YABI_NOT_INITIALIZED,
   YABI_BAD_PARAM,
   YABI_ERR,
} yabi_Error;

//TODO support other types here?
typedef uint32_t     yabi_ChanID;
typedef uint32_t     yabi_ChanValue;
typedef uint32_t     yabi_FrameID;

static const yabi_ChanID   CHANNEL_INACTIVE;

struct yabi_ChannelGroup {
   yabi_ChanID       id;
   yabi_ChanValue    newValue;
   uint32_t          transitionTime;
};

struct yabi_ChannelState {
   yabi_ChanID       id;
   yabi_ChanValue    value;
};


//Callbacks
typedef void (*yabi_FrameStartCallback)(yabi_FrameID frame);
typedef void (*yabi_FrameEndCallback)(yabi_FrameID frame);
typedef void (*yabi_ChannelChange)(yabi_ChanID chan, yabi_ChanValue value);
typedef void (*yabi_ChannelGroupChange)(struct yabi_ChannelState channelVals[], uint32_t num);

typedef void* const (*yabi_HardwareSetup)(void);
typedef void (*yabi_HardwareTeardown)(void* const hwConfig);

struct yabi_HardwareConfig {
   yabi_HardwareSetup      setup;
   yabi_HardwareTeardown   teardown;
   void*                   hwConfig;
};

struct yabi_Config {
   yabi_FrameStartCallback    frameStartCB;
   yabi_FrameEndCallback      frameEndCB;
   yabi_ChannelChange         channelChangeCB;
   yabi_ChannelGroupChange    channelChangeGroupCB;
   struct yabi_HardwareConfig hwConfig;
};

yabi_Error yabi_init(struct yabi_Config* const cfg);
yabi_Error yabi_giveTime(uint32_t systimeMS);
yabi_Error yabi_setStarted(bool start);
yabi_Error yabi_setChannel(yabi_ChanID channelID, yabi_ChanValue newTarget, uint32_t transitionTimeMS);
yabi_Error yabi_setChannelGroup(struct yabi_ChannelGroup channels[], uint32_t num);


#endif//YABI_H__

