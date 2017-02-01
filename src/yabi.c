#include "yabi/yabi.h"

#include <stdbool.h>

struct yabi_State{
   bool                       initialized;
   bool                       started;
   struct yabi_Config         config;
   void*                      hwStateObject;

   struct yabi_ChannelState   channels[MAX_CONTROL_CHANNELS];
   uint32_t                   lastUpdateMS;

   //FIXME better way to do this?
   yabi_FrameID               currentFrame;
};
static struct yabi_State state = {};


yabi_Error yabi_init(struct yabi_Config* const cfg) {
   if(!cfg) {
      return YABI_BAD_PARAM;
   }

   //take a copy of the config info
   state.config = *cfg;

   state.currentFrame = 0;
   state.lastUpdateMS = 0;

   state.initialized = true;
   state.started = false;

   return YABI_OK;
}

yabi_Error yabi_giveTime(uint32_t systimeMS) {
   if(!state.initialized || !state.started) {
      return YABI_NOT_INITIALIZED;
   }

   struct yabi_ChannelState* c;

   //TODO write this

   //calculate ∆ time step from last update
   //iterate through all channels
   //calculate position change for each channel
   //issue channel change (or channel group change?) commands ith those values
   //update last update timestamp

   if(state.config.frameStartCB) {
      state.config.frameStartCB(state.currentFrame);
   }

   //TODO use calculation that prevents wraps
   uint32_t timeChange = systimeMS - state.lastUpdateMS;

   int i;
   for(i = 0; i < MAX_CONTROL_CHANNELS; i++) {
      c = &state.channels[i];
      if(c->id == CHANNEL_INACTIVE) {
         continue;
      }

      //TODO do apply channel change
   }

   if(state.config.frameStartCB) {
      state.config.frameStartCB(state.currentFrame);
   }

   state.lastUpdateMS = systimeMS;
   state.currentFrame++;

   return YABI_UNIMPLIMENTED;
}

yabi_Error yabi_setStarted(bool start) {
   if(start) {
      if(state.config.hwConfig.setup) {
         state.hwStateObject = state.config.hwConfig.setup();
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

   //TODO write this
   return YABI_UNIMPLIMENTED;
}

yabi_Error yabi_setChannelGroup(struct yabi_ChannelGroup channels[], uint32_t num) {
   //TODO write this
   return YABI_UNIMPLIMENTED;
}

