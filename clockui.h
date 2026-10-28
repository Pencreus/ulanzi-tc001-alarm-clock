#pragma once
// Tiny pixel-font helpers for the Ulanzi TC001 32x8 matrix.
// 3x5 text font (A-Z 0-9 and a few symbols) and 4x7 big digits for the time.
#include <string>
#include "esphome/core/color.h"
#include "esphome/components/display/display.h"

namespace clockui {

static const char CHARSET[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-.:%/!*";

// 5 rows per glyph, 3 bits per row (MSB = left pixel)
static const uint8_t FONT35[][5] = {
    {0, 0, 0, 0, 0},  // space
    {7, 5, 5, 5, 7},  // 0
    {2, 6, 2, 2, 7},  // 1
    {7, 1, 7, 4, 7},  // 2
    {7, 1, 7, 1, 7},  // 3
    {5, 5, 7, 1, 1},  // 4
    {7, 4, 7, 1, 7},  // 5
    {7, 4, 7, 5, 7},  // 6
    {7, 1, 1, 2, 2},  // 7
    {7, 5, 7, 5, 7},  // 8
    {7, 5, 7, 1, 7},  // 9
    {2, 5, 7, 5, 5},  // A
    {6, 5, 6, 5, 6},  // B
    {3, 4, 4, 4, 3},  // C
    {6, 5, 5, 5, 6},  // D
    {7, 4, 6, 4, 7},  // E
    {7, 4, 6, 4, 4},  // F
    {3, 4, 5, 5, 3},  // G
    {5, 5, 7, 5, 5},  // H
    {7, 2, 2, 2, 7},  // I
    {1, 1, 1, 5, 2},  // J
    {5, 5, 6, 5, 5},  // K
    {4, 4, 4, 4, 7},  // L
    {5, 7, 7, 5, 5},  // M
    {6, 5, 5, 5, 5},  // N
    {2, 5, 5, 5, 2},  // O
    {6, 5, 6, 4, 4},  // P
    {2, 5, 5, 7, 3},  // Q
    {6, 5, 6, 5, 5},  // R
    {3, 4, 2, 1, 6},  // S
    {7, 2, 2, 2, 2},  // T
    {5, 5, 5, 5, 7},  // U
    {5, 5, 5, 5, 2},  // V
    {5, 5, 7, 7, 5},  // W
    {5, 5, 2, 5, 5},  // X
    {5, 5, 2, 2, 2},  // Y
    {7, 1, 2, 4, 7},  // Z
    {0, 0, 7, 0, 0},  // -
    {0, 0, 0, 0, 2},  // .
    {0, 2, 0, 2, 0},  // :
    {5, 1, 2, 4, 5},  // %
    {1, 1, 2, 4, 4},  // /
    {2, 2, 2, 0, 2},  // !
    {7, 5, 7, 0, 0},  // * (degree)
};

// 4x7 digits, 4 bits per row (MSB = left pixel)
static const uint8_t BIG[10][7] = {
    {6, 9, 9, 9, 9, 9, 6},        // 0
    {2, 6, 2, 2, 2, 2, 7},        // 1
    {6, 9, 1, 2, 4, 8, 15},       // 2
    {14, 1, 1, 6, 1, 1, 14},      // 3
    {9, 9, 9, 15, 1, 1, 1},       // 4
    {15, 8, 8, 14, 1, 1, 14},     // 5
    {6, 8, 8, 14, 9, 9, 6},       // 6
    {15, 1, 1, 2, 4, 4, 4},       // 7
    {6, 9, 9, 6, 9, 9, 6},        // 8
    {6, 9, 9, 7, 1, 1, 6},        // 9
};

inline int text_width(const std::string &s) { return s.empty() ? 0 : (int) s.size() * 4 - 1; }

inline void draw_char(esphome::display::Display &it, int x, int y, char c, esphome::Color col) {
  if (c >= 'a' && c <= 'z') c -= 32;
  int idx = 0;
  for (int i = 0; CHARSET[i]; i++) {
    if (CHARSET[i] == c) {
      idx = i;
      break;
    }
  }
  for (int r = 0; r < 5; r++)
    for (int b = 0; b < 3; b++)
      if (FONT35[idx][r] & (4 >> b)) it.draw_pixel_at(x + b, y + r, col);
}

inline void draw_text(esphome::display::Display &it, int x, int y, const std::string &s, esphome::Color col) {
  for (size_t i = 0; i < s.size(); i++) draw_char(it, x + (int) i * 4, y, s[i], col);
}

inline void draw_text_centered(esphome::display::Display &it, int y, const std::string &s, esphome::Color col) {
  draw_text(it, (32 - text_width(s)) / 2, y, s, col);
}

inline void draw_big_digit(esphome::display::Display &it, int x, int y, int d, esphome::Color col) {
  if (d < 0 || d > 9) return;
  for (int r = 0; r < 7; r++)
    for (int b = 0; b < 4; b++)
      if (BIG[d][r] & (8 >> b)) it.draw_pixel_at(x + b, y + r, col);
}

// ---- animations (each draw_* is called every display frame, ~100 ms) -------------------
inline esphome::Color hsv(float h, float s, float v) {  // h in degrees
  float c = v * s;
  float hp = fmodf(h, 360.0f) / 60.0f;
  float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
  float r = 0, g = 0, b = 0;
  if (hp < 1) { r = c; g = x; }
  else if (hp < 2) { r = x; g = c; }
  else if (hp < 3) { g = c; b = x; }
  else if (hp < 4) { g = x; b = c; }
  else if (hp < 5) { r = x; b = c; }
  else { r = c; b = x; }
  float m = v - c;
  return esphome::Color((uint8_t) ((r + m) * 255), (uint8_t) ((g + m) * 255), (uint8_t) ((b + m) * 255));
}

// falling streaks: matrix = green trails with a bright head, rain = short blue drops
inline void draw_streaks(esphome::display::Display &it, bool matrix) {
  static float head[32], spd[32];
  static int len[32];
  static bool inited = false;
  if (!inited) {
    for (int x = 0; x < 32; x++) {
      head[x] = -esphome::random_float() * 14.0f;
      spd[x] = 0.15f;
      len[x] = 3;
    }
    inited = true;
  }
  for (int x = 0; x < 32; x++) {
    head[x] += spd[x];
    if (head[x] - len[x] > 8.0f) {
      head[x] = -esphome::random_float() * 10.0f;
      spd[x] = matrix ? 0.12f + esphome::random_float() * 0.35f : 0.35f + esphome::random_float() * 0.5f;
      len[x] = matrix ? 3 + (int) (esphome::random_float() * 5) : 2;
    }
    for (int k = 0; k < len[x]; k++) {
      int y = (int) head[x] - k;
      if (y < 0 || y > 7) continue;
      float f = 1.0f - (float) k / (float) len[x];
      if (matrix)
        it.draw_pixel_at(x, y, k == 0 ? esphome::Color(190, 255, 190) : esphome::Color(0, (uint8_t) (255 * f * f), 0));
      else
        it.draw_pixel_at(x, y, k == 0 ? esphome::Color(150, 200, 255) : esphome::Color(0, (uint8_t) (60 * f), (uint8_t) (200 * f)));
    }
  }
}

inline void draw_rainbow(esphome::display::Display &it) {
  float t = esphome::millis() * 0.05f;
  for (int x = 0; x < 32; x++)
    for (int y = 0; y < 8; y++) it.draw_pixel_at(x, y, hsv(x * 11.25f + t + y * 6.0f, 1.0f, 0.9f));
}

inline void draw_plasma(esphome::display::Display &it) {
  float t = esphome::millis() * 0.002f;
  for (int x = 0; x < 32; x++)
    for (int y = 0; y < 8; y++) {
      float v = sinf(x * 0.35f + t) + sinf(y * 0.8f + t * 1.3f) + sinf((x + y) * 0.25f + t * 0.7f);
      it.draw_pixel_at(x, y, hsv((v + 3.0f) / 6.0f * 300.0f + t * 20.0f, 1.0f, 0.9f));
    }
}

inline void draw_fire(esphome::display::Display &it) {
  static float heat[32][8];
  for (int x = 0; x < 32; x++) heat[x][7] = 0.55f + esphome::random_float() * 0.45f;
  for (int y = 0; y < 7; y++)
    for (int x = 0; x < 32; x++) {
      float a = heat[x][y + 1];
      float l = heat[(x + 31) % 32][y + 1];
      float r = heat[(x + 1) % 32][y + 1];
      float v = (a * 2.0f + l + r) / 4.0f - 0.09f - esphome::random_float() * 0.05f;
      heat[x][y] = v < 0 ? 0 : v;
    }
  for (int y = 0; y < 8; y++)
    for (int x = 0; x < 32; x++) {
      float h = heat[x][y];
      it.draw_pixel_at(x, y, esphome::Color((uint8_t) (255 * std::min(1.0f, h * 2.0f)),
                                            (uint8_t) (255 * std::max(0.0f, std::min(1.0f, h * 2.0f - 0.7f))),
                                            (uint8_t) (255 * std::max(0.0f, h * 2.0f - 1.6f))));
    }
}

// rainbow-coloured text scrolling right to left
inline void draw_scroller(esphome::display::Display &it, const std::string &s) {
  int total = (int) s.size() * 4 + 32;
  int off = (int) (esphome::millis() / 70) % total;
  for (size_t i = 0; i < s.size(); i++) {
    int x = 32 - off + (int) i * 4;
    if (x < -3 || x > 31) continue;
    draw_char(it, x, 1, s[i], hsv(i * 28.0f + esphome::millis() * 0.05f, 1.0f, 1.0f));
  }
}

// 5x7 alarm bell icon
inline void draw_bell(esphome::display::Display &it, int x, int y, esphome::Color col) {
  static const uint8_t BELL[7] = {4, 14, 14, 14, 31, 0, 4};
  for (int r = 0; r < 7; r++)
    for (int b = 0; b < 5; b++)
      if (BELL[r] & (16 >> b)) it.draw_pixel_at(x + b, y + r, col);
}

// ---- alarm schedule helpers --------------------------------------------
// ESPTime::day_of_week is 1=Sunday .. 7=Saturday. Mon-Fri = 2..6.
static const char *const DOW3[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

inline bool is_weekday(int dow) { return dow >= 2 && dow <= 6; }

inline bool alarm_due(esphome::ESPTime now, bool wk_en, int wk_h, int wk_m, bool we_en, int we_h, int we_m) {
  if (is_weekday(now.day_of_week)) return wk_en && now.hour == wk_h && now.minute == wk_m;
  return we_en && now.hour == we_h && now.minute == we_m;
}

// Text for the next armed alarm, e.g. "WED 7:10". Returns false if none armed.
inline bool next_alarm_text(esphome::ESPTime now, bool wk_en, int wk_h, int wk_m, bool we_en, int we_h, int we_m,
                            std::string &out) {
  int now_min = now.hour * 60 + now.minute;
  for (int off = 0; off < 8; off++) {
    int dow = (now.day_of_week - 1 + off) % 7 + 1;
    bool wk = is_weekday(dow);
    bool en = wk ? wk_en : we_en;
    if (!en) continue;
    int h = wk ? wk_h : we_h;
    int m = wk ? wk_m : we_m;
    if (off == 0 && h * 60 + m <= now_min) continue;
    char buf[16];
    snprintf(buf, sizeof(buf), "%s %d:%02d", DOW3[dow - 1], h % 12 == 0 ? 12 : h % 12, m);
    out = buf;
    return true;
  }
  return false;
}

// Next page in direction dir (+1/-1) among pages whose bit is set in mask.
inline int next_page(int cur, int dir, int mask, int npages) {
  for (int i = 1; i <= npages; i++) {
    int p = ((cur + dir * i) % npages + npages) % npages;
    if (mask & (1 << p)) return p;
  }
  return cur;
}

}  // namespace clockui
