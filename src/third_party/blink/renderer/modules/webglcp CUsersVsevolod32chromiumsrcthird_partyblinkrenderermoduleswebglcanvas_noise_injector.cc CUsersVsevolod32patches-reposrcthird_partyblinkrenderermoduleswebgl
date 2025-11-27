// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_CANVAS_NOISE_INJECTOR_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_CANVAS_NOISE_INJECTOR_H_

#include <cstdint>
#include <cstddef>

namespace blink {

class CanvasNoiseInjector {
 public:
  // Инициализирует шум при старте браузера
  static void Initialize();

  // Инжектирует шум в буфер пикселей (для Canvas и WebGL)
  // pixels - буфер данных (RGBA формат)
  // length - размер буфера в байтах
  static void InjectNoise(uint8_t* pixels, size_t length);

 private:
  // Генерирует случайный seed при первом вызове
  static uint64_t GetSeed();

  // Хранит seed на время сессии браузера
  static uint64_t canvas_noise_seed_;
  static bool initialized_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_CANVAS_NOISE_INJECTOR_H_
