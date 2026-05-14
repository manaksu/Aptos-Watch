#include <pebble.h>

#define KEY_BG   0
#define KEY_CASE 1
#define KEY_SYM  2
#define KEY_TEX  3

#define PERSIST_BG   10
#define PERSIST_CASE 11
#define PERSIST_SYM  12
#define PERSIST_TEX  13

#define BG_BLACK    0
#define BG_DARKGREY 1
#define BG_NAVY     2
#define BG_EPAPER   3
#define BG_CREAM    4

#define SYM_WHITE 0
#define SYM_GREY  1
#define SYM_RED   2
#define SYM_AMBER 3

static Window    *s_window;
static TextLayer *s_text_layer;
static TextLayer *s_symbol_layer;
static Layer     *s_texture_layer;

static GFont s_font_main;
static GFont s_font_thin;
static GFont s_font_bold_48;

static char s_text_buf[128];

static int s_bg   = BG_BLACK;
static int s_case = 0;
static int s_sym  = SYM_WHITE;
static int s_tex  = 0;

// --- Rotating tech specs (changes every 4 hours) ---
static const char *SPECS[] = {
  "STM32F439 ARM CORTEXM4 64MHZ 256KB RAM 144X168 64COLOR",
  "Basalt Platform 64MHz Flash 128KB Stack 8KB Heap 24KB",
  "Pebble Time Steel IP67 7 Day Battery BLE 4.0 Vibration",
  "Cortex M4 FPU DSP Extension 2MB Flash 256KB SRAM",
  "144x168 Color Display 64 Colors eInk Transflective LCD",
  "accel gyro ambient light sensor microphone backlight",
};
#define NUM_SPECS 6

static const char *current_spec(struct tm *t) {
  int slot = (t->tm_hour / 4) % NUM_SPECS;
  return SPECS[slot];
}

static GColor bg_color(void) {
  switch (s_bg) {
    case BG_DARKGREY: return GColorDarkGray;
    case BG_NAVY:     return GColorOxfordBlue;
    case BG_EPAPER:   return GColorWhite;
    case BG_CREAM:    return GColorPastelYellow;
    default:          return GColorBlack;
  }
}

static GColor text_color(void) {
  return (s_bg == BG_EPAPER || s_bg == BG_CREAM) ? GColorBlack : GColorWhite;
}

static GColor sym_color(void) {
  bool light_bg = (s_bg == BG_EPAPER || s_bg == BG_CREAM);
  if (light_bg) {
    switch (s_sym) {
      case SYM_GREY:  return GColorDarkGray;
      case SYM_RED:   return GColorDarkCandyAppleRed;
      case SYM_AMBER: return GColorChromeYellow;
      default:        return GColorBlack;
    }
  }
  switch (s_sym) {
    case SYM_GREY:  return GColorLightGray;
    case SYM_RED:   return GColorRed;
    case SYM_AMBER: return GColorChromeYellow;
    default:        return GColorWhite;
  }
}

static bool is_cream(void) { return s_bg == BG_CREAM; }

static void texture_update_proc(Layer *layer, GContext *ctx) {
  if (!s_tex) return;
  GRect bounds = layer_get_bounds(layer);
  for (int y = 0; y < bounds.size.h; y += 3) {
    for (int x = (y % 6 == 0 ? 1 : 3); x < bounds.size.w; x += 6) {
      graphics_context_set_stroke_color(ctx,
        (s_bg == BG_EPAPER || s_bg == BG_CREAM) ? GColorLightGray : GColorDarkGray);
      graphics_draw_pixel(ctx, GPoint(x, y));
    }
  }
}

static void apply_settings(void) {
  window_set_background_color(s_window, bg_color());
  // Cream theme always uses thin font, others use bold
  text_layer_set_font(s_text_layer, is_cream() ? s_font_thin : s_font_main);
  text_layer_set_text_color(s_text_layer,   text_color());
  text_layer_set_text_color(s_symbol_layer, sym_color());
  layer_mark_dirty(s_texture_layer);
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void update_time(struct tm *t) {
  char raw[80];
  // Cream theme always mixed case like a terminal
  if (s_case == 1 || is_cream()) {
    strftime(raw, sizeof(raw), "%A, %B %d, %Y, %I/%M %p", t);
  } else {
    char tmp[80];
    strftime(tmp, sizeof(tmp), "%A, %B %d, %Y, %I/%M %p", t);
    for (int i = 0; tmp[i]; i++) {
      if (tmp[i] >= 'a' && tmp[i] <= 'z') tmp[i] -= 32;
    }
    snprintf(raw, sizeof(raw), "%s", tmp);
  }
  BatteryChargeState bat = battery_state_service_peek();
  snprintf(s_text_buf, sizeof(s_text_buf), "%s ~%d%% %s",
           raw, (int)bat.charge_percent, current_spec(t));
  text_layer_set_text(s_text_layer, s_text_buf);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *ft;

  ft = dict_find(iter, KEY_BG);
  if (ft && ft->type == TUPLE_INT) {
    s_bg = (int)ft->value->int32;
    persist_write_int(PERSIST_BG, s_bg);
  }
  ft = dict_find(iter, KEY_CASE);
  if (ft && ft->type == TUPLE_INT) {
    s_case = (int)ft->value->int32;
    persist_write_int(PERSIST_CASE, s_case);
  }
  ft = dict_find(iter, KEY_SYM);
  if (ft && ft->type == TUPLE_INT) {
    s_sym = (int)ft->value->int32;
    persist_write_int(PERSIST_SYM, s_sym);
  }
  ft = dict_find(iter, KEY_TEX);
  if (ft && ft->type == TUPLE_INT) {
    s_tex = (int)ft->value->int32;
    persist_write_int(PERSIST_TEX, s_tex);
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
  s_font_thin = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_MONO_16));
  s_font_bold_48 = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_BOLD_48));

  s_texture_layer = layer_create(bounds);
  layer_set_update_proc(s_texture_layer, texture_update_proc);
  layer_add_child(root, s_texture_layer);

  s_text_layer = text_layer_create(GRect(2, 2, bounds.size.w - 4, 120));
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, text_color());
  text_layer_set_font(s_text_layer, is_cream() ? s_font_thin : s_font_main);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_text_layer, "");
  layer_add_child(root, text_layer_get_layer(s_text_layer));

  s_symbol_layer = text_layer_create(
      GRect(bounds.size.w - 65, bounds.size.h - 62, 62, 62));
  text_layer_set_background_color(s_symbol_layer, GColorClear);
  text_layer_set_text_color(s_symbol_layer, sym_color());
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
  layer_destroy(s_texture_layer);
  fonts_unload_custom_font(s_font_main);
  fonts_unload_custom_font(s_font_thin);
  fonts_unload_custom_font(s_font_bold_48);
}

static void init(void) {
  s_bg   = persist_exists(PERSIST_BG)   ? persist_read_int(PERSIST_BG)   : BG_BLACK;
  s_case = persist_exists(PERSIST_CASE) ? persist_read_int(PERSIST_CASE) : 0;
  s_sym  = persist_exists(PERSIST_SYM)  ? persist_read_int(PERSIST_SYM)  : SYM_WHITE;
  s_tex  = persist_exists(PERSIST_TEX)  ? persist_read_int(PERSIST_TEX)  : 0;

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  app_message_open(128, 64);
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
