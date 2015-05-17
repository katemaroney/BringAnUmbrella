#include <pebble.h>
#define KEY_TEMP 0
#define KEY_CONDITIONS 1

/*base view */
static Window *base;
static TextLayer *weatherTemp;
static TextLayer *weatherCat;
static TextLayer *placeholder;
static GColor background;
static GColor text;

static void mainWindowLoad(Window *window){
  weatherTemp = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(weatherTemp, background);
  text_layer_set_text_color(weatherTemp, text);
  text_layer_set_text(weatherTemp, "70 F");
  text_layer_set_font(weatherTemp, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(weatherTemp, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(base), text_layer_get_layer(weatherTemp));
  
  weatherCat = text_layer_create(GRect(0, 75, 144, 50));
  text_layer_set_background_color(weatherCat, background);
  text_layer_set_text_color(weatherTemp, text);
  text_layer_set_text(weatherCat, "Cloudy");
  text_layer_set_font(weatherCat, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(weatherCat, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(base), text_layer_get_layer(weatherCat));
  
  placeholder = text_layer_create(GRect(0, 95, 144, 50));
  text_layer_set_background_color(placeholder, background);
  text_layer_set_text_color(placeholder, text);
  text_layer_set_font(placeholder, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(placeholder, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(base), text_layer_get_layer(placeholder));
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char buffer[] = "00:00";
  
  if (clock_is_24h_style() == true){
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  text_layer_set_text(placeholder, buffer);
}

static void mainWindowUnload(Window *window){
  text_layer_destroy(weatherTemp);
  text_layer_destroy(weatherCat);
  text_layer_destroy(placeholder);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
   update_time(); 
   if (tick_time->tm_min % 30 == 0) {
     DictionaryIterator *iter;
     app_message_outbox_begin(&iter);
     dict_write_uint8(iter, 0, 0);
     app_message_outbox_send();
   }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Message recieved");
  static char temp_buffer[8];
  static char conditions_buffer[32];
  Tuple *t = dict_read_first(iterator);
  while (t != NULL){
    switch(t->key){
      case KEY_TEMP:
        snprintf(temp_buffer, sizeof(temp_buffer),"%d F", (int)t->value->int32);
        text_layer_set_text(weatherTemp, temp_buffer);
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        text_layer_set_text(weatherCat, conditions_buffer);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
    }
    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failure");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success");
}

static void init(){
  base = window_create();
  window_set_window_handlers(base, (WindowHandlers) {
    .load = mainWindowLoad,
    .unload = mainWindowUnload
  });
  
  window_stack_push(base, true);
  background = GColorClear;
  text = GColorBlack;

  
  /* subscribe to TickTimerService
     This allows us to access the current time.
     Necessary to run functions based on time change.
  */
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(){
  window_destroy(base);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}