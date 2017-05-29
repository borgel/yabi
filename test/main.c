#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

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
         if(res != YABI_OK) {
            printf("SetChannel Err: Code %d\n", res);
         }
      }
      else if(i == 10) {
         res = yabi_setChannel(2, 20, 452);
      }

      yabi_giveTime(time);

      usleep(US_TO_MS * delayMS);
      time += delayMS;
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

