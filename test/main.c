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
         res = yabi_setChannel(5, 100, 400);
         res = yabi_setChannel(2, 100, 452);
         res = yabi_setChannel(7, 250, 10);
         if(res != YABI_OK) {
            printf("SetChannel Err: Code %d\n", res);
         }
      }
      else if(i == 3) {
         res = yabi_setChannel(7, 10, 550);
      }
      else if(i == 10) {
         res = yabi_setChannel(2, 20, 452);
      }

      yabi_giveTime(time);

      usleep(US_TO_MS * delayMS);
      time += delayMS;
   }
}

static yabi_ChanValue t1_Interpolate(yabi_ChanValue current, yabi_ChanValue start, yabi_ChanValue end, float fraction) {
   //FIXME rm
   if(end == 10) {
      printf("inter: c:%d s:%d e:%d f:%f\n", current, start, end, fraction);
      //printf("cur - 
      //TODO how to we determine which way to approach? -- or ++ around the circle?
      // mod math?
      // 250->10 is faster ++ than --
      //compare to target and target + 255
      // 250->255+10 (265) is faster ++ than 250->10

      // rollover is the new less than
      if(0xFF + end > start) {
         printf("a");
      }
      else {
         printf("b");
      }
      /*
      if(end > start) {
         printf("e-s = %u vs (e+F) - s = %u\n", end-start, (end+0xFF)-start);
         if(end - start > (end + 0xFF) - start) {
            printf("down");
         }
         else {
            printf("rollover");
         }
      }
      else {
         printf("s-e = %u vs MMM = %u\n", start - end, (start+0xFF)-end);
         if(start - end> (start + 0xFF) - end) {
            printf("down");
         }
         else {
            printf("rollover");
         }
      }
      */
      /*
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
      */

      return end;
   }
   else {
      return end;
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
      .interpolator           = t1_Interpolate,
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

