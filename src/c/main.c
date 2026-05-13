#include <pebble.h>

// --- Layers ---
static Window    *s_window;
static TextLayer *s_symbol_layer;  // "<"  — Aptos Display Bold 48
static TextLayer *s_time_layer;    // HH:MM — Aptos Display 36
static TextLayer *s_date_layer;    // Day, DD Mon — Aptos Display 22

// --- Fonts ---
static GFont s_font_bold_48;
static GFont s_font_36;
static GFont s_font_22;

// --- Buffers ---
static char s_time_buf[6];   // "HH:MM\0"
static char s_date_buf[16];  // "Mon, 13 May\0"

// ---------------------------------------------------------------------------
static void update_time(struct tm *tick_time) {
  strftime(s_time_buf, sizeof(s_time_buf),
           clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buf);

  strftime(s_date_buf, sizeof(s_date_buf), "%a, %d %b", tick_time);
  text_layer_set_text(s_date_layer, s_date_buf);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

// ---------------------------------------------------------------------------
static void window_load(Window *window) {
  Layer *root   = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);  // 144 × 168 basalt

  window_set_background_color(window, GColorBlack);

  // Load fonts
  s_font_bold_48 = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_BOLD_48));
  s_font_36 = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_36));
  s_font_22 = fonts_load_custom_font(
      resource_get_handle(RESOURCE_ID_APTOS_22));

  // ── Symbol "<" ──────────────────────────────────────────────────────────
  // Pebble font renderer adds top padding; start y a bit high and tune
  // empirically after first build if needed.
  s_symbol_layer = text_layer_create(GRect(0, 22, bounds.size.w, 62));
  text_layer_set_background_color(s_symbol_layer, GColorClear);
  text_layer_set_text_color(s_symbol_layer, GColorWhite);
  text_layer_set_font(s_symbol_layer, s_font_bold_48);
  text_layer_set_text_alignment(s_symbol_layer, GTextAlignmentCenter);
  text_layer_set_text(s_symbol_layer, "<");
  layer_add_child(root, text_layer_get_layer(s_symbol_layer));

  // ── Time "HH:MM" ────────────────────────────────────────────────────────
  // y = 22 + 62 + 10 = 94
  s_time_layer = text_layer_create(GRect(0, 94, bounds.size.w, 46));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_font_36);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_layer, "00:00");
  layer_add_child(root, text_layer_get_layer(s_time_layer));

  // ── Date "Wed, 13 May" ──────────────────────────────────────────────────
  // y = 94 + 46 + 4 = 144
  s_date_layer = text_layer_create(GRect(0, 144, bounds.size.w, 24));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorLightGray);
  text_layer_set_font(s_date_layer, s_font_22);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "");
  layer_add_child(root, text_layer_get_layer(s_date_layer));

  // Seed display immediately
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_symbol_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);

  fonts_unload_custom_font(s_font_bold_48);
  fonts_unload_custom_font(s_font_36);
  fonts_unload_custom_font(s_font_22);
}

// ---------------------------------------------------------------------------
static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(MINUTE_UNITS, tick_handler);
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
