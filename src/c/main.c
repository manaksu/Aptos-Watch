#include <pebble.h>

static Window    *s_window;
static TextLayer *s_text_layer;
static TextLayer *s_symbol_layer;

static GFont s_font_main;
static GFont s_font_bold_48;

static char s_text_buf[48];

static void update_time(struct tm *t) {
  char raw[48];
  strftime(raw, sizeof(raw), "%A, %B %d, %Y, %I/%M %p", t);
  // uppercase
  for (int i = 0; raw[i]; i++) {
    if (raw[i] >= 'a' && raw[i] <= 'z') raw[i] -= 32;
  }
  snprintf(s_text_buf, sizeof(s_text_buf), "%s", raw);
  text_layer_set_text(s_text_layer, s_text_buf);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void window_load(Window *window) {
  Layer *root   = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);

  window_set_background_color(window, GColorBlack);

  s_font_main = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_MAIN_14));
  s_font_bold_48 = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_BOLD_48));

  // One flowing text block, top-left, wraps naturally
  s_text_layer = text_layer_create(GRect(2, 2, bounds.size.w - 4, 120));
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, GColorWhite);
  text_layer_set_font(s_text_layer, s_font_main);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_text_layer, "");
  layer_add_child(root, text_layer_get_layer(s_text_layer));

  // "<" bottom-right
  s_symbol_layer = text_layer_create(
      GRect(bounds.size.w - 62, bounds.size.h - 62, 62, 62));
  text_layer_set_background_color(s_symbol_layer, GColorClear);
  text_layer_set_text_color(s_symbol_layer, GColorWhite);
  text_layer_set_font(s_symbol_layer, s_font_bold_48);
  text_layer_set_text_alignment(s_symbol_layer, GTextAlignmentRight);
  text_layer_set_text(s_symbol_layer, "<");
  layer_add_child(root, text_layer_get_layer(s_symbol_layer));

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  text_layer_destroy(s_symbol_layer);
  fonts_unload_custom_font(s_font_main);
  fonts_unload_custom_font(s_font_bold_48);
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
