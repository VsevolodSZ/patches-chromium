// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/webgl/canvas_noise_injector.h"

#include <random>
#include <chrono>
#include <algorithm>

namespace blink {

// Статические переменные для хранения seed на время сессии
uint64_t CanvasNoiseInjector::canvas_noise_seed_ = 0;
bool CanvasNoiseInjector::initialized_ = false;

void CanvasNoiseInjector::Initialize() {
  if (initialized_) {
    return;
  }

  // Генерируем seed на основе текущего времени
  auto now = std::chrono::high_resolution_clock::now();
  auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch()).count();

  canvas_noise_seed_ = static_cast<uint64_t>(nanos);
  initialized_ = true;
}

uint64_t CanvasNoiseInjector::GetSeed() {
  if (!initialized_) {
    Initialize();
  }
  return canvas_noise_seed_;
}

void CanvasNoiseInjector::InjectNoise(uint8_t* pixels, size_t length) {
  if (!pixels || length == 0) {
    return;
  }

  uint64_t seed = GetSeed();

  // Создаём детерминированный генератор на основе seed
  std::mt19937_64 rng(seed);

  // Инжектируем шум в ~0.1% пикселей (каждый 1000-й пиксель)
  // Изменяем каждый RGB канал на ±1 или ±2
  for (size_t i = 0; i < length; i += 1000) {
    if (i < length) {
      // Генерируем случайное смещение: -2, -1, 0, 1, 2
      int offset = (rng() % 5) - 2;

      // Убеждаемся что значение остаётся в диапазоне [0, 255]
      int new_value = static_cast<int>(pixels[i]) + offset;
      pixels[i] = static_cast<uint8_t>(
          std::clamp(new_value, 0, 255));
    }
  }
}

}  // namespace blink
