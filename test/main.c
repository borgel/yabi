#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#include "yabi/yabi.h"

#define US_TO_MS     (1000)

void test1(void);

int main(void) {
   printf("Starting YABI Tests...\n");

   test1();

   return 0;
}

static void t1_FrameStart(yabi_FrameID fm) {
   printf("Starting Frame %d\n", fm);
}
static void t1_FrameEnd(yabi_FrameID fm) {
   printf("Ending Frame %d\n", fm);
}
static void t1_ChanCH(yabi_ChanID chan, yabi_ChanValue v) {
   printf("\tChan #%d -> %d\n", chan, v);
}
static void t1_ChanGroupCH(struct yabi_ChannelState channelVals[], uint32_t num) {
   printf("\tChanging %d Chans:\n", num);
   for(int i = 0; i < num; i++) {
      printf("\t\t%d -> %d\n", channelVals[i].id, channelVals[i].value);
   }
}

int HwConfigFake = 5;
void* const t1_HwSetup() {
   printf("Setting Up Hardware...\n");
   //return something to check later
   return &HwConfigFake;
}
void t1_HwTeardown(void* const hwConfig) {
   printf("Tearing Down Hardware...\n");
   int *configBack = (int*)hwConfig;
   if(*configBack != HwConfigFake) {
      printf("\tGot back a different HW config pointer!\n");
   }
}

static void t1_doLoop(int iterations, int delayMS) {
   yabi_Error res;

   printf("Running loop...\n");

   uint32_t time = 0;
   for(int i = 0; i < iterations; i++) {
      if(i == 2) {
         //chan id, target, time (ms)
         //res = yabi_setChannel(5, 100, 400);
         //res = yabi_setChannel(2, 100, 452);
         res = yabi_setChannel(0, 10, 10);   //rollover bottom
         //res = yabi_setChannel(7, 250, 10);   //rollover top
         if(res != YABI_OK) {
            printf("SetChannel Err: Code %d\n", res);
         }
      }
      else if(i == 3) {
         res = yabi_setChannel(0, 250, 550); //rollober bottom
         //res = yabi_setChannel(7, 10, 550);   //rollover top
      }
      else if(i == 10) {
         //res = yabi_setChannel(2, 20, 452);
      }

      yabi_giveTime(time);

      usleep(US_TO_MS * delayMS);
      time += delayMS;
   }
}

static yabi_ChanValue t1_RolloverInterpolate(yabi_ChanValue current, yabi_ChanValue start, yabi_ChanValue end, float fraction, float absoluteFraction) {
   printf("inter: c:%d s:%d e:%d f:%f af:%f\n", current, start, end, fraction, absoluteFraction);

   bool increasing;
   uint32_t change;
   uint8_t mod = 0;

   //printf("cur - 
   //TODO how to we determine which way to approach? -- or ++ around the circle?
   // mod math?
   // 250->10 is faster ++ than --
   //compare to target and target + 255
   // 250->255+10 (265) is faster ++ than 250->10

   if(end > start)   // XXX increasing
   {
      increasing = true;

      printf("inc (%d -> %d) ", start, end);
      if( end - start > (start + 0xFF) - end) {
         printf(" ROLL TOP   \n");
         mod = 0xFF;
         increasing = false;
      }
   }
   else     // XXX decreasing
   {
      increasing = false;

      printf("dec (%d -> %d) ", start, end);
      if( start - end > (end + 0xFF) - start) {
         printf(" ROLL BOTTOM   \n");
         mod = 0xFF;
         increasing = true;
      }
   }

   if(increasing) {
      change = fraction * (float)((float)(end + mod) - (float)start);
      // make sure any change < 0 is rounded up (we only deal in integers)
      change = (change == 0) ? 1 : change;
      return (uint8_t)(current + change);
   }
   else {
      change = fraction * (float)((float)(start + mod) - (float)end);
      change = (change == 0) ? 1 : change;
      return (uint8_t)(current - change);
   }

   /*
   // what fork is this? inc and inc/distance?
   //
   //TODO need abs?
   if(((0xFF + end) - start) < (end - start)) {
      printf("sub");
      mod = 0xFF;
   }
   else {
      printf("normal");
      mod = 0;
   }

   change = fraction * (float)((float)(end + mod) - (float)start);
   return (uint8_t)(current + change);
   */

   if(end > start) {
      change = fraction * (float)((float)end - (float)start);
      // make sure any change < 0 is rounded up (we only deal in integers)
      change = (change == 0) ? 1 : change;
      return current + change;
   }
   else {
      change = fraction * (float)(start - end);
      change = (change == 0) ? 1 : change;
      return current - change;
   }
}

// the backing datastore of channel state
static struct yabi_ChannelRecord chanStore[10];
static struct yabi_ChannelStateConfiguration chanConfig = {
   .channelStorage   = chanStore,
   .numChannels      = 10,
};

//init and teardown
void test1(void) {
   yabi_Error res;

   struct yabi_Config cfg = {
      .frameStartCB           = t1_FrameStart,
      .frameEndCB             = t1_FrameEnd,
      .channelChangeCB        = t1_ChanCH,
      .channelChangeGroupCB   = t1_ChanGroupCH,
      .interpolator           = t1_RolloverInterpolate,
      .hwConfig = {
         .setup               = t1_HwSetup,
         .teardown            = t1_HwTeardown,
         .hwConfig            = NULL,
      },
   };

   res = yabi_init(&cfg, &chanConfig);
   printf("Done with init\n");
   yabi_setStarted(true);

   t1_doLoop(20, 100);

   yabi_setStarted(false);
}

