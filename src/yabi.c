#include "yabi/yabi.h"

#include <stdbool.h>

/*
 * A note on channel accounting:
 *    This is designed to only support up to MAX_CONTROL_CHANNELS channels of
 *    control. If that's the case, and NO duplicate control channels are allowed,
 *    than storing channel info progressively from slot 0 up
 *    (AKA ChanID != bucket) only gives us faster channel change calculation
 *    time. For now, that is a future design goal, so we won't do it.
 */
struct yabi_State{
   bool                                initialized;
   bool                                started;
   struct yabi_Config                  config;
   void*                               hwStateObject;

   // user alloated pointer to channel state data
   struct yabi_ChannelRecord*          channels;
   uint32_t                            numChannels;
   uint32_t                            lastUpdateMS;

   yabi_FrameID                        currentFrame;
};
static struct yabi_State state = {0};


/*
 * Initialize this YABI instance.
 * `initialValues` are optional, but if provided will be used to set all initial
 * channel values on init. These initial values will be applied to all affected
 * channels on library start.
 * To not use them, pass in NULL and 0 for `initialValues` and `num` respectively.
 */
yabi_Error yabi_init(struct yabi_Config* const cfg, struct yabi_ChannelStateConfiguration const * const chanConfig) {
   if(!cfg || !chanConfig) {
      return YABI_BAD_PARAM;
   }
   else if(chanConfig->numChannels == 0 || !chanConfig->channelStorage){
      return YABI_BAD_PARAM;
   }

   //take a copy of the config info
   state.config = *cfg;

   state.currentFrame = 0;
   state.lastUpdateMS = 0;

   //wire up user provided channel data storage
   state.channels = chanConfig->channelStorage;
   state.numChannels = chanConfig->numChannels;

   for(int i = 0; i < state.numChannels; i++) {
      //request instant transition to 0
      yabi_setChannel(i, 0, 0);
   }

   state.started = false;
   state.initialized = true;

   return YABI_OK;
}

/*
   Workflow:
   calculate ∆ time step from last update
   iterate through all channels
   calculate position change for each channel
   issue channel change (or channel group change?) commands with those values
       in the future, if it's a group change, batch all changes together and issue them in one call
   update last update timestamp
*/
yabi_Error yabi_giveTime(uint32_t systimeMS) {
   if(!state.initialized || !state.started) {
      return YABI_NOT_INITIALIZED;
   }


   if(state.config.frameStartCB) {
      state.config.frameStartCB(state.currentFrame);
   }

   //FIXME use calculation that prevents wraps
   uint32_t timeChange = systimeMS - state.lastUpdateMS;

   float timeFraction;
   struct yabi_ChannelRecord *r;
   int i;
   for(i = 0; i < state.numChannels; i++) {
      r = &state.channels[i];
      if(r->id == CHANNEL_INACTIVE) {
         continue;
      }

      if(r->value != r->valueTarget) {
         if(systimeMS >= r->transitionEndMS) {
            r->value = r->valueTarget;
         }
         else {
            //TODO make all this fixed point
            timeFraction = r->transitionEndMS - r->transitionStartMS;
            timeFraction = timeChange / timeFraction;

            //LERP it (start + percent * (end - start));
            r->value += timeFraction * (r->valueTarget - r->valuePrevious);
         }

         if(state.config.channelChangeCB) {
            state.config.channelChangeCB(r->id, r->value);
         }
      }
   }

   if(state.config.frameEndCB) {
      state.config.frameEndCB(state.currentFrame);
   }

   state.lastUpdateMS = systimeMS;
   state.currentFrame++;

   return YABI_OK;
}

yabi_Error yabi_setStarted(bool start) {
   yabi_Error e = YABI_OK;
   if(start) {
      if(state.config.hwConfig.setup) {
         state.hwStateObject = state.config.hwConfig.setup();

         //apply all values to all live channels
         e = yabi_giveTime(0);
      }
   }
   else {
      if(state.config.hwConfig.teardown) {
         state.config.hwConfig.teardown(state.hwStateObject);
      }
   }

   state.started = start;

   return e;
}

yabi_Error yabi_setChannel(yabi_ChanID channelID, yabi_ChanValue newTarget, uint32_t transitionTimeMS) {
   if(channelID > state.numChannels) {
      return YABI_BAD_PARAM;
   }

   struct yabi_ChannelRecord *r = &state.channels[channelID];

   r->valuePrevious = r->value;
   r->valueTarget = newTarget;
   // this will be imprecise, as we don't know the exact systime.
   r->transitionStartMS = state.lastUpdateMS;
   //FIXME check for rollover
   r->transitionEndMS = state.lastUpdateMS + transitionTimeMS;
   //This is redundant, but about as efficient as using a bool to indicate 'active'. This facilitates later changes though.
   r->id = channelID;

   return YABI_OK;
}

yabi_Error yabi_setChannelGroup(struct yabi_ChannelGroup channels[], uint32_t num) {
   //TODO write this
   return YABI_UNIMPLIMENTED;
}

