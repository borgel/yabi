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

struct yabi_State{
   bool                          initialized;
   bool                          started;
   struct yabi_Config            config;
   void*                         hwStateObject;

   struct yabi_ChannelRecord     channels[MAX_CONTROL_CHANNELS];
   uint32_t                      lastUpdateMS;

   yabi_FrameID               currentFrame;
};
static struct yabi_State state = {};


/*
 * Initialize this YABI instance.
 * `initialValues` are optional, but if provided will be used to set all initial
 * channel values on init. These initial values will be applied to all affected
 * channels on library start.
 * To not use them, pass in NULL and 0 for `initialValues` and `num` respectively.
 */
yabi_Error yabi_init(struct yabi_Config* const cfg, struct yabi_ChannelState* const initialValues, uint32_t num) {
   if(!cfg) {
      return YABI_BAD_PARAM;
   }

   //take a copy of the config info
   state.config = *cfg;

   state.currentFrame = 0;
   state.lastUpdateMS = 0;

   if(initialValues && num > 0 && num < MAX_CONTROL_CHANNELS) {
      struct yabi_ChannelState* iv;
      for(int i = 0; i < num; i++) {
         iv = &initialValues[i];

         //request instant transition to the initial value
         yabi_setChannel(iv->id, iv->value, 0);
      }
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
   issue channel change (or channel group change?) commands ith those values
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
   for(i = 0; i < MAX_CONTROL_CHANNELS; i++) {
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
   yabi_Error e;
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

   return YABI_OK;
}

yabi_Error yabi_setChannel(yabi_ChanID channelID, yabi_ChanValue newTarget, uint32_t transitionTimeMS) {
   if(channelID > MAX_CONTROL_CHANNELS) {
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

