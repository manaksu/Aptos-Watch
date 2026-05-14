#include <pebble.h>

#define KEY_BG   0
#define KEY_CASE 1

#define PERSIST_BG   10
#define PERSIST_CASE 11

// BG options
#define BG_BLACK     0
#define BG_DARKGREY  1
#define BG_NAVY      2
#define BG_EPAPER    3

static Window    *s_window;
static TextLayer *s_text_layer;
static TextLayer *s_symbol_layer;

static GFont s_font_main;
static GFont s_font_bold_48;

static char s_text_buf[128];

static int s_bg   = BG_BLACK;
static int s_case = 0;  // 0 = allcaps, 1 = mixed

static GColor bg_color(void) {
  switch (s_bg) {
    case BG_DARKGREY: return GColorDarkGray;
    case BG_NAVY:     return GColorOxfordBlue;
    case BG_EPAPER:   return GColorWhite;
    default:          return GColorBlack;
  }
}

static GColor text_color(void) {
  return (s_bg == BG_EPAPER) ? GColorBlack : GColorWhite;
}

static GColor symbol_color(void) {
  return (s_bg == BG_EPAPER) ? GColorBlack : GColorWhite;
}

static void apply_settings(void) {
  window_set_background_color(s_window, bg_color());
  text_layer_set_text_color(s_text_layer,   text_color());
  text_layer_set_text_color(s_symbol_layer, symbol_color());
}

static void update_time(struct tm *t) {
  char raw[80];
  if (s_case == 0) {
    // All caps
    char tmp[80];
    strftime(tmp, sizeof(tmp), "%A, %B %d, %Y, %I/%M %p", t);
    for (int i = 0; tmp[i]; i++) {
      if (tmp[i] >= 'a' && tmp[i] <= 'z') tmp[i] -= 32;
    }
    snprintf(raw, sizeof(raw), "%s", tmp);
  } else {
    // Mixed case
    strftime(raw, sizeof(raw), "%A, %B %d, %Y, %I/%M %p", t);
  }
  BatteryChargeState bat = battery_state_service_peek();
  snprintf(s_text_buf, sizeof(s_text_buf), "%s ~%d%% STM32F439 ARM CORTEX-M4 64MHZ 256KB RAM 144X168 64COLOR", raw, (int)bat.charge_percent);
  text_layer_set_text(s_text_layer, s_text_buf);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *t;

  t = dict_find(iter, KEY_BG);
  if (t) {
    s_bg = (int)t->value->int32;
    persist_write_int(PERSIST_BG, s_bg);
  }

  t = dict_find(iter, KEY_CASE);
  if (t) {
    s_case = (int)t->value->int32;
    persist_write_int(PERSIST_CASE, s_case);
  }

  apply_settings();
  time_t now = time(NULL);
  struct tm *tm_now = localtime(&now);
  update_time(tm_now);
}

static void window_load(Window *window) {
  Layer *root   = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);

  window_set_background_color(window, bg_color());

  s_font_main = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_MAIN_14));
  s_font_bold_48 = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_BOLD_48));

  s_text_layer = text_layer_create(GRect(2, 2, bounds.size.w - 4, 120));
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, text_color());
  text_layer_set_font(s_text_layer, s_font_main);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_text_layer, "");
  layer_add_child(root, text_layer_get_layer(s_text_layer));

  s_symbol_layer = text_layer_create(
      GRect(bounds.size.w - 62, bounds.size.h - 62, 62, 62));
  text_layer_set_background_color(s_symbol_layer, GColorClear);
  text_layer_set_text_color(s_symbol_layer, symbol_color());
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
  s_bg   = persist_exists(PERSIST_BG)   ? persist_read_int(PERSIST_BG)   : BG_BLACK;
  s_case = persist_exists(PERSIST_CASE) ? persist_read_int(PERSIST_CASE) : 0;

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  app_message_open(64, 64);
  app_message_register_inbox_received(inbox_received);
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
