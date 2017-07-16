#ifndef YABI_H__
#define YABI_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
   YABI_OK,
   YABI_UNIMPLIMENTED,
   YABI_NOT_INITIALIZED,
   YABI_BAD_PARAM,
   YABI_ERR,
} yabi_Error;

//TODO support other types?
typedef uint32_t     yabi_ChanID;
typedef uint32_t     yabi_ChanValue;
typedef uint32_t     yabi_FrameID;

static const yabi_ChanID   CHANNEL_INACTIVE  = 0xFFFFFFFF;

struct yabi_ChannelGroup {
   yabi_ChanID       id;
   yabi_ChanValue    newValue;
   uint32_t          transitionTime;
};

struct yabi_ChannelState {
   yabi_ChanID       id;
   yabi_ChanValue    value;
};

// opaque struct used internally allocate channel info
struct yabi_ChannelRecord;

//Callbacks
typedef void (*yabi_FrameStartCallback)(yabi_FrameID frame);
typedef void (*yabi_FrameEndCallback)(yabi_FrameID frame);
typedef void (*yabi_ChannelChange)(yabi_ChanID chan, yabi_ChanValue value);
typedef void (*yabi_ChannelGroupChange)(struct yabi_ChannelState channelVals[], uint32_t num);
typedef yabi_ChanValue (*yabi_Interpolator)(yabi_ChanValue current, yabi_ChanValue start, yabi_ChanValue end, float fraction, float absoluteFraction);

typedef void* const (*yabi_HardwareSetup)(void);
typedef void (*yabi_HardwareTeardown)(void* const hwConfig);

struct yabi_HardwareConfig {
   yabi_HardwareSetup      setup;
   yabi_HardwareTeardown   teardown;
   void*                   hwConfig;
};

struct yabi_ChannelStateConfiguration {
   struct yabi_ChannelRecord * const   channelStorage;
   uint32_t                            numChannels;
};

struct yabi_Config {
   yabi_FrameStartCallback    frameStartCB;
   yabi_FrameEndCallback      frameEndCB;
   yabi_ChannelChange         channelChangeCB;
   yabi_ChannelGroupChange    channelChangeGroupCB;
   yabi_Interpolator          interpolator;
   struct yabi_HardwareConfig hwConfig;
};

yabi_Error yabi_init(struct yabi_Config* const cfg, struct yabi_ChannelStateConfiguration const * const chanConfig);
yabi_Error yabi_giveTime(uint32_t systimeMS);
yabi_Error yabi_setStarted(bool start);
yabi_Error yabi_setChannel(yabi_ChanID channelID, yabi_ChanValue newTarget, uint32_t transitionTimeMS);
yabi_Error yabi_setChannelGroup(struct yabi_ChannelGroup channels[], uint32_t num);

// ssshhh, don't look at this. It's meant to be private! But it needs to be exposed to allow
// static allocation.
struct yabi_ChannelRecord {
   yabi_ChanID       id;

   //past data
   yabi_ChanValue    valuePrevious;
   //ABSOLUTE systime start
   uint32_t          transitionStartMS;

   //current data
   yabi_ChanValue    value;

   //future data
   yabi_ChanValue    valueTarget;
   //ABSOLUTE systime end
   uint32_t          transitionEndMS;
};


#endif//YABI_H__

