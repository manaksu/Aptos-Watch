#include <pebble.h>

// --- Layers ---
static Window    *s_window;
static TextLayer *s_date_layer;    // top: "MONDAY, JANUARY 10, 2026"
static TextLayer *s_time_layer;    // "IT IS : HH/MM/SS AM"
static TextLayer *s_symbol_layer;  // "<" bottom-right

// --- Fonts ---
static GFont s_font_bold_48;
static GFont s_font_date;
static GFont s_font_time;

// --- Buffers ---
static char s_date_buf[32];
static char s_time_buf[24];

// ---------------------------------------------------------------------------
static void update_time(struct tm *t) {
  // Full date, uppercased: "MONDAY, JANUARY 10, 2026"
  char raw_date[32];
  strftime(raw_date, sizeof(raw_date), "%A, %B %d, %Y", t);
  for (int i = 0; raw_date[i]; i++) {
    if (raw_date[i] >= 'a' && raw_date[i] <= 'z')
      raw_date[i] -= 32;
  }
  snprintf(s_date_buf, sizeof(s_date_buf), "%s", raw_date);
  text_layer_set_text(s_date_layer, s_date_buf);

  // Time: "IT IS : HH/MM/SS AM"
  char hms[12];
  strftime(hms, sizeof(hms), "%I:%M %p", t);
  snprintf(s_time_buf, sizeof(s_time_buf), "%s", hms);
  text_layer_set_text(s_time_layer, s_time_buf);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

// ---------------------------------------------------------------------------
static void window_load(Window *window) {
  Layer *root   = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);  // 144 x 168 basalt

  window_set_background_color(window, GColorBlack);

  s_font_bold_48 = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_BOLD_48));
  s_font_date = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_DATE_18));
  s_font_time = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_TIME_28));

  // -- Date: top-left, wraps full width, 2 lines at ~18pt
  s_date_layer = text_layer_create(GRect(0, 2, bounds.size.w, 52));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_font_date);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(s_date_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_date_layer, "");
  layer_add_child(root, text_layer_get_layer(s_date_layer));

  // -- Time: below date, wraps full width
  s_time_layer = text_layer_create(GRect(0, 58, bounds.size.w, 52));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_font_time);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(s_time_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_time_layer, "");
  layer_add_child(root, text_layer_get_layer(s_time_layer));

  // -- Symbol "<": bottom-right corner
  int sym_w = 62;
  int sym_h = 62;
  s_symbol_layer = text_layer_create(
      GRect(bounds.size.w - sym_w, bounds.size.h - sym_h, sym_w, sym_h));
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
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_symbol_layer);

  fonts_unload_custom_font(s_font_bold_48);
  fonts_unload_custom_font(s_font_date);
  fonts_unload_custom_font(s_font_time);
}

// ---------------------------------------------------------------------------
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
