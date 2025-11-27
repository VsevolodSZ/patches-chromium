# Chromium Anti-Detection Patches

## Overview
Complete set of patches for Chromium to implement GPU fingerprinting spoofing, hardware configuration randomization, and canvas fingerprinting resistance.

## Files Modified

### 1. `src/third_party/blink/renderer/core/frame/webgl_gpu_params.h` (NEW FILE)
**Purpose:** Define WebGL GPU parameters for 7 different GPUs

**Changes:**
- Created new header file with `WebGLGPUParams` structure
- Structure contains:
  - `unmasked_vendor` and `unmasked_renderer` strings for WebGL API
  - Standard WebGL version strings (vendor, renderer, version, shading_language_version)
  - 45+ WebGL integer parameters (texture sizes, buffer limits, etc.)
  - Shader precision parameters (fragment and vertex shaders)

**GPU Profiles Added:**
1. **RTX 3060** - `kWebGLRTX3060Params`
2. **AMD Radeon RX 5700 XT** - `kWebGLAMDRX5700XTParams`
3. **GTX 1060** - `kWebGLGTX1060Params`
4. **RTX 2060** - `kWebGLRTX2060Params`
5. **GTX 1650 Ti** - `kWebGLGTX1650TiParams`
6. **RTX 4070** - `kWebGLRTX4070Params`
7. **GTX 1050 Ti** - `kWebGLGTX1050TiParams` (replaced GTX 980 Ti due to detection issues)

**Key Details:**
- All GPU profiles share identical int/float parameters (intentional for consistency)
- Only `unmasked_renderer` differs between profiles to show GPU name
- Parameters include realistic WebGL API limits matching actual hardware

### 2. `src/third_party/blink/renderer/core/frame/hardware_config.h` (NEW FILE)
**Purpose:** Define hardware configuration profiles and selection logic

**Changes:**
- Created `HardwareConfig` struct linking CPU cores, RAM, GPU vendor/name, and WebGL parameters
- Defined 7 realistic hardware combinations:
  - Profile 1: 4 cores, 8GB RAM + GTX 1060
  - Profile 2: 4 cores, 16GB RAM + GTX 1050 Ti (was GTX 980 Ti)
  - Profile 3: 6 cores, 16GB RAM + GTX 1650 Ti
  - Profile 4: 8 cores, 16GB RAM + RTX 2060
  - Profile 5: 8 cores, 32GB RAM + RTX 3060
  - Profile 6: 12 cores, 32GB RAM + AMD RX 5700 XT
  - Profile 7: 16 cores, 32GB RAM + RTX 4070

- Implemented `GetSelectedHardwareConfig()` function:
  - Selects ONE random profile at browser startup
  - Uses `static bool initialized` flag to ensure selection happens exactly once per process
  - Uses high-resolution clock (nanoseconds) as RNG seed
  - Selected config remains constant for entire browser session

### 3. `src/third_party/blink/renderer/modules/webgl/canvas_noise_injector.h` (NEW FILE)
**Purpose:** Header for canvas fingerprinting noise injection

**Contains:**
- `CanvasNoiseInjector` class with noise injection logic
- Methods for injecting deterministic pixel noise
- Seed generation and management

### 4. `src/third_party/blink/renderer/modules/webgl/canvas_noise_injector.cc` (NEW FILE)
**Purpose:** Implementation of canvas noise injection

**Changes:**
- Generates deterministic seed at browser startup (from high-resolution clock)
- Injects noise into canvas `readPixels()` results
- Noise characteristics:
  - Affects 0.1% of pixels (1 in 1000)
  - Each affected pixel modified by ±1-2 in value
  - Uses MT19937 PRNG for deterministic generation
  - Noise is stable within same browser session (same seed throughout)

### 5. `src/third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.cc` (MODIFIED)
**Purpose:** Integrate canvas noise injection into WebGL readPixels

**Changes:**
- Modified `ReadPixelsHelper()` method to call noise injection after reading pixels
- Calculates pixel data size based on GL format (RGBA, RGB, etc.)
- Injects noise before returning pixel data to JavaScript
- Ensures fingerprinting resistance without breaking WebGL functionality

## How It Works

### Hardware Configuration Selection
1. Browser starts → `GetSelectedHardwareConfig()` called
2. First call generates random seed from nanosecond-precision clock
3. Random profile selected from 7 combinations (uniform distribution)
4. Profile cached in static variable for entire session
5. All subsequent calls return same profile
6. Session-to-session: different profile selected (different seed)

### WebGL Spoofing
1. When JavaScript queries WebGL parameters (via canvas context)
2. Returns parameters from selected hardware profile
3. GPU name in `UNMASKED_RENDERER_WEBGL` matches selected profile
4. All parameters consistent with each other (no mismatches)

### Canvas Fingerprinting Resistance
1. JavaScript calls canvas `readPixels()`
2. After GPU reads pixels, noise injector modifies result
3. Minimal modification (0.1% of pixels, ±1-2 values)
4. Prevents perfect canvas fingerprint match
5. Noise deterministic (same seed = same noise pattern)

## Anti-Detection Goals

✓ Prevents simple fingerprinting (identical parameter values)
✓ Randomizes between realistic hardware configurations
✓ Adds canvas noise to break fingerprinting scripts
✓ Maintains stability within browser session
✓ GPU parameters match selected hardware profile
✓ Prevents WebGL mismatch detection (parameters are consistent)

## Known Issues

- **WebGL Parameter Mismatch:** All 7 GPU profiles use identical int/float parameters. Real GPUs have different limits. This may cause detection by sophisticated anti-bot systems (marked as `hasBadWebGL: true` in some detection tools).

- **Missing Spoofing:** Other fingerprinting vectors not covered:
  - `navigator.hardwareConcurrency` (shows real CPU cores, not spoofed)
  - WebRTC fingerprinting (IP leak)
  - AudioContext fingerprinting
  - Font enumeration
  - Plugin list
  - Screen resolution fingerprinting

## Future Improvements

1. Use realistic WebGL parameters per GPU (different limits for GTX 1060 vs RTX 4070)
2. Spoof `navigator.hardwareConcurrency` to match selected CPU cores
3. Spoof `navigator.deviceMemory` to match selected RAM
4. Implement WebRTC candidate filtering
5. Spoof AudioContext frequency responses
6. Implement timezone/locale spoofing (integrate with GeoIP patches)

## Testing

Build with: `ninja -C out\Release chrome`

Test detection at: BrowserLeaks.com, CanvasBlocker, or Stealth detection tools
