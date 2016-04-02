#include <pebble.h>

Window *my_window;
static Layer *s_window_layer;
TextLayer *time_layer, *dist_layer;

static char s_current_time_buffer[8];

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  strftime(s_current_time_buffer, sizeof(s_current_time_buffer),
           clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);

  text_layer_set_text(time_layer, s_current_time_buffer);
}

static void window_load(Window *window) {
  GRect window_bounds = layer_get_bounds(s_window_layer);
  
 time_layer = text_layer_create(
      GRect(0, 140, window_bounds.size.w, 38));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(time_layer));
  
  dist_layer = text_layer_create(
      GRect(0, 0, window_bounds.size.w, 38));
  text_layer_set_text_color(dist_layer, GColorWhite);
  text_layer_set_background_color(dist_layer, GColorClear);
  text_layer_set_font(dist_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(dist_layer, GTextAlignmentCenter);
  text_layer_set_text(dist_layer, "Hello!");
  layer_add_child(s_window_layer, text_layer_get_layer(dist_layer));
}

static void window_unload(Window *window) {
  layer_destroy(text_layer_get_layer(time_layer));
}


void handle_init(void) {
  
  my_window = window_create();
  s_window_layer = window_get_root_layer(my_window);
  window_set_background_color(my_window, GColorCobaltBlue);
  
  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  window_stack_push(my_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void handle_deinit(void) {
  text_layer_destroy(time_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
