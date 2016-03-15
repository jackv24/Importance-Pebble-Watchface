#include <pebble.h>
#include "watchface.h"

void init() {
   show_watchface();
}

void deinit() {
   hide_watchface();
}

int main()
{
   init();
   app_event_loop();
   deinit();
   
   return 0;
}