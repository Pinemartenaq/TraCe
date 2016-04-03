#include <pebble.h>
#include <time.h>

Window *my_window;
static Layer *s_window_layer, *s_canvas_layer;
TextLayer *time_layer, *dist_layer, *star_layer;

static char s_current_time_buffer[8], s_current_dist_buffer[16], s_current_star_buffer[16];
static GPoint s_positions[71];

static int s_step_count = 0, s_direction = 0, s_star_count = 0, st_count = 0;

bool step_data_is_available() {
  return HealthServiceAccessibilityMaskAvailable &
    health_service_metric_accessible(HealthMetricStepCount,
      time_start_of_today(), time(NULL));
}

static void update_star_count(){
				s_star_count++;
				snprintf(s_current_star_buffer, sizeof(s_current_star_buffer),"* x %d", s_star_count);
				text_layer_set_text(star_layer, s_current_star_buffer);
        layer_mark_dirty(text_layer_get_layer(star_layer));
}

static void update_proc(Layer *layer, GContext *ctx) {  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  GPoint sP;  
  for(int i = 0; i < st_count; i++){
    sP.x = s_positions[i].x - 1;
    sP.y = s_positions[i].y - 1;
    
    graphics_fill_rect(ctx, GRect(sP.x, sP.y, 3, 3), 0, 0);
  }
  
  layer_mark_dirty(s_canvas_layer);
}

static void get_step_count() {
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
}

static void display_dist() {
  if (s_step_count <= 10000){
    int dis = s_step_count * 0.762;
    static char st[2] = "m";
    snprintf(s_current_dist_buffer, sizeof(s_current_dist_buffer),
            "%d %s", dis, st);
  }
  else{
    int d = s_step_count * 0.762;
    int dis = d / 1000;
    int dismod = d % 1000;
    dismod = dismod / 100;
    static char st[2] = "km";
    snprintf(s_current_dist_buffer, sizeof(s_current_dist_buffer),
            "%d.%d %s", dis, dismod, st);
  }
  
  text_layer_set_text(dist_layer, s_current_dist_buffer);
  layer_mark_dirty(text_layer_get_layer(dist_layer));
  layer_mark_dirty(s_canvas_layer);
}

static void health_handler(HealthEventType event, void *context) {
  if(event != HealthEventSleepUpdate) {
    get_step_count();
  
    snprintf(s_current_star_buffer, sizeof(s_current_star_buffer),
             "* x %d", s_star_count);
    
    GRect window_bounds = layer_get_bounds(s_window_layer);
    
    if (st_count == 71){
      s_positions[0] = GPoint(window_bounds.size.w / 2, window_bounds.size.h / 2);
      st_count = 0;
      update_star_count();
    } 
    else{
      GPoint p = s_positions[st_count - 1];
      GPoint np;
      
      if (s_direction == 0){
          np.x = p.x;
          np.y = p.y - 1;
      }
      else if(s_direction == 1){
        np.x = p.x + 1;
        np.y = p.y - 1;
      }
      else if(s_direction == 2){
        np.x = p.x + 1;
        np.y = p.y;
      }
      else if(s_direction == 3){
        np.x = p.x + 1;
        np.y = p.y + 1;
      }
      else if(s_direction == 4){
        np.x = p.x;
        np.y = p.y + 1;
      }
      else if(s_direction == 5){
        np.x = p.x - 1;
        np.y = p.y + 1;
      }
      else if(s_direction == 6){
        np.x = p.x - 1;
        np.y = p.y;
      }
      else if(s_direction == 7){
        np.x = p.x - 1;
        np.y = p.y - 1;
      }
      
      s_positions[st_count] = np;
    }
  }
  
  st_count++;
  display_dist();
  layer_mark_dirty(s_canvas_layer);
}

static void compass_heading_handler(CompassHeadingData heading_data) {
  int dir =  TRIGANGLE_TO_DEG(heading_data.magnetic_heading);

	if(dir >= 337.5 && dir < 22.5){
    s_direction = 0;
	}
	else if(dir >= 22.5 && dir < 67.5){
    s_direction = 1;
	}
  else if(dir >= 67.5 && dir < 112.5){
		s_direction = 2;
	}
	else if(dir >= 112.5 && dir < 157.5){
		s_direction = 3;
	}
	else if(dir >= 157.5 && dir < 202.5){
		s_direction = 4;
	}
	else if(dir >= 202.5 && dir < 247.5){
		s_direction = 5;
	}
	else if(dir >= 247.5 && dir < 292.5){
		s_direction = 6;
	}	
	else if(dir >= 292.5 && dir < 337.5){
		s_direction = 7;
	}	
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  strftime(s_current_time_buffer, sizeof(s_current_time_buffer),
           clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);

  text_layer_set_text(time_layer, s_current_time_buffer);
  
  time_t temp = time(NULL);
	struct tm *t_time= localtime(&temp);
  
	if(t_time->tm_hour == 0 && t_time->tm_min == 0 && t_time->tm_sec == 0){
    s_positions[0] = GPoint(72, 84);
    st_count = 1;
    s_star_count = 0;
    s_direction = 0;
    s_step_count = 0;
  }
}

static void window_load(Window *window) {
  GRect window_bounds = layer_get_bounds(s_window_layer);
  
 time_layer = text_layer_create(
      GRect(0, 140, window_bounds.size.w, 38));
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(time_layer));
  
  dist_layer = text_layer_create(
      GRect(0, 0, window_bounds.size.w, 38));
  text_layer_set_text_color(dist_layer, GColorWhite);
  text_layer_set_background_color(dist_layer, GColorClear);
  text_layer_set_font(dist_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(dist_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(dist_layer));
  
  star_layer = text_layer_create(
      GRect(0, 20, window_bounds.size.w, 38));
  text_layer_set_text_color(star_layer, GColorWhite);
  text_layer_set_background_color(star_layer, GColorClear);
  text_layer_set_font(star_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(star_layer, GTextAlignmentCenter);
  text_layer_set_text(star_layer, s_current_star_buffer);
  layer_add_child(s_window_layer, text_layer_get_layer(star_layer));
  
  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(s_window_layer, s_canvas_layer);
  
  s_positions[0] = GPoint(window_bounds.size.w / 2, window_bounds.size.h / 2);
  st_count ++;
  
  if(step_data_is_available()) {
    health_service_events_subscribe(health_handler, NULL);
  }
}

static void window_unload(Window *window) {
  layer_destroy(text_layer_get_layer(time_layer));
  layer_destroy(text_layer_get_layer(dist_layer));
  layer_destroy(text_layer_get_layer(star_layer));
  layer_destroy(s_canvas_layer);
}


void handle_init(void) {
  
  compass_service_set_heading_filter(DEG_TO_TRIGANGLE(2));
  compass_service_subscribe(&compass_heading_handler);
  
  my_window = window_create();
  s_window_layer = window_get_root_layer(my_window);
  window_set_background_color(my_window, GColorBlack);
  
  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  window_stack_push(my_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void handle_deinit(void) {
  compass_service_unsubscribe();
  health_service_events_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}