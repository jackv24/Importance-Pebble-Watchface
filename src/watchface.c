#include <pebble.h>
#include "watchface.h"

//COnfig key values
#define KEY_BATTERY_BACKGROUND_COLOR 0
#define KEY_BATTERY_FOREGROUND_COLOR 1

#define KEY_TIME_COLOR 2
#define KEY_DATE_COLOR 3
#define KEY_BACKGROUND_COLOR 4

static Window *s_window;
static GFont s_res_font_naftalene_64;
static GFont s_res_roboto_condensed_21;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static GColor b_bg_color;
static GColor b_fg_color;
static GColor bg_color;
static GColor time_color;
static GColor date_color;

static BitmapLayer *s_connection_layer;
static GBitmap *s_connection_bitmap;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, bg_color);
  #ifndef PBL_SDK_3
      window_set_fullscreen(s_window, true);
  #endif
  
   int offset = PBL_IF_RECT_ELSE(0, 18);
   
  s_res_font_naftalene_64 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NAFTALENE_64));
  s_res_roboto_condensed_21 = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  // s_time_layer
  s_time_layer = text_layer_create(GRect(0 + offset, 40, 144, 64));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, time_color);
  text_layer_set_text(s_time_layer, "04:33");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, s_res_font_naftalene_64);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_time_layer);
  
  // s_date_layer
  s_date_layer = text_layer_create(GRect(0 + offset, 25, 144, 49));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, date_color);
  text_layer_set_text(s_date_layer, "24/2");
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_date_layer);
  
  // s_battery_layer
  s_battery_layer = layer_create(GRect(14 + offset, 115, 115, 10));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_battery_layer);
   
   //s_connection_layer
   s_connection_bitmap = gbitmap_create_with_resource(PBL_IF_COLOR_ELSE(RESOURCE_ID_IMAGE_DISCONNECTED, RESOURCE_ID_IMAGE_DISCONNECTED_BW));
   
   s_connection_layer = bitmap_layer_create(GRect(64 + offset, 130, 19, 19));
   bitmap_layer_set_bitmap(s_connection_layer, s_connection_bitmap);
   layer_add_child(window_get_root_layer(s_window), bitmap_layer_get_layer(s_connection_layer));
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
  fonts_unload_custom_font(s_res_font_naftalene_64);
   
   gbitmap_destroy(s_connection_bitmap);
   bitmap_layer_destroy(s_connection_layer);
}

static int s_battery_level;
static bool s_isBatteryCharging;

static void battery_update_proc(Layer *layer, GContext *ctx) {
   GRect bounds = layer_get_bounds(layer);
   
   int width = (int)(float)(((float)s_battery_level / 100.0F) * 114.0F);
   
   graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(b_bg_color, GColorBlack));
   graphics_fill_rect(ctx, bounds, 0, GCornerNone);
   
   if(s_isBatteryCharging) {
      graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite));
      
      width = 114.0F;
   }
   else {
      graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(b_fg_color, GColorWhite));
   }
   
   graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
   Tuple *b_bg_color_t = dict_find(iter, KEY_BATTERY_BACKGROUND_COLOR);
   if(b_bg_color_t) {
      int bg_color = b_bg_color_t->value->int32;
      
      persist_write_int(KEY_BATTERY_BACKGROUND_COLOR, bg_color);
      
      b_bg_color = GColorFromHEX(bg_color);
   }
   
   Tuple *b_fg_color_t = dict_find(iter, KEY_BATTERY_FOREGROUND_COLOR);
   if(b_fg_color_t) {
      int fg_color = b_fg_color_t->value->int32;
      
      persist_write_int(KEY_BATTERY_FOREGROUND_COLOR, fg_color);
      
      b_fg_color = GColorFromHEX(fg_color);
   }
   
    //reload config
   layer_mark_dirty(s_battery_layer);
   
   //background, time and date
   Tuple *bg_color_t = dict_find(iter, KEY_BACKGROUND_COLOR);
   if(bg_color_t) {
      int color = bg_color_t->value->int32;
      
      persist_write_int(KEY_BACKGROUND_COLOR, color);
      
      bg_color = GColorFromHEX(color);
   }
   
   Tuple *time_color_t = dict_find(iter, KEY_TIME_COLOR);
   if(time_color_t) {
      int color = time_color_t->value->int32;
      
      persist_write_int(KEY_TIME_COLOR, color);
      
      time_color = GColorFromHEX(color);
   }
   
   Tuple *date_color_t = dict_find(iter, KEY_DATE_COLOR);
   if(date_color_t) {
      int color = date_color_t->value->int32;
      
      persist_write_int(KEY_DATE_COLOR, color);
      
      date_color = GColorFromHEX(color);
   }
   
   //refresh ui
   window_set_background_color(s_window, bg_color);
   text_layer_set_text_color(s_time_layer, time_color);
   text_layer_set_text_color(s_date_layer, date_color);
}

static void battery_callback(BatteryChargeState state) {
   s_battery_level = state.charge_percent;
   s_isBatteryCharging = state.is_charging;
   
   layer_mark_dirty(s_battery_layer);
}

static void bluetooth_callback(bool connected) {
   layer_set_hidden(bitmap_layer_get_layer(s_connection_layer), connected);
   
   if(!connected) {
      vibes_double_pulse();
   }
}

static void update_time() {
   time_t temp = time(NULL);
   struct tm *tick_time = localtime(&temp);
   
   static char s_buffer[8];
   strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
   
   text_layer_set_text(s_time_layer, s_buffer);
   
   static char date_buffer[16];
   strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
   
   text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
   update_time();
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_watchface(void) {
   if(persist_exists(KEY_BACKGROUND_COLOR))
      bg_color = GColorFromHEX(persist_read_int(KEY_BACKGROUND_COLOR));
   if(persist_exists(KEY_TIME_COLOR))
      time_color = GColorFromHEX(persist_read_int(KEY_TIME_COLOR));
   if(persist_exists(KEY_DATE_COLOR))
      date_color = GColorFromHEX(persist_read_int(KEY_DATE_COLOR));
   
   initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
   
   update_time();
   bluetooth_callback(connection_service_peek_pebble_app_connection());
   
   layer_set_update_proc(s_battery_layer, battery_update_proc);
   battery_callback(battery_state_service_peek());
   
   tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
   battery_state_service_subscribe(battery_callback);
   
   connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bluetooth_callback
   });
   
   layer_mark_dirty(s_battery_layer);
   
   if(persist_exists(KEY_BATTERY_BACKGROUND_COLOR))
      b_bg_color = GColorFromHEX(persist_read_int(KEY_BATTERY_BACKGROUND_COLOR));
   if(persist_exists(KEY_BATTERY_FOREGROUND_COLOR))
      b_fg_color = GColorFromHEX(persist_read_int(KEY_BATTERY_FOREGROUND_COLOR));
   
   app_message_register_inbox_received(inbox_received_handler);
   app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void hide_watchface(void) {
  window_stack_remove(s_window, true);
}
