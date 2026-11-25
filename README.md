# We Have OTT At Home

A personal JUCE plugin that recreates the sound and controls of [Xfer Records OTT](https://xferrecords.com/freeware/). The name is temporary.

OTT is a three-band upward and downward compressor. It splits the signal into low, mid, and high, compresses each band hard in both directions, and mixes the result with the dry signal. The result is the bright, dense character the original is known for.

## Signal path

`OTTProcessor` runs the effect. Each block is processed as follows:

1. **Input gain**, smoothed over 15 ms.
2. A dry copy is kept for the depth mix.
3. A Linkwitz–Riley crossover splits the signal at **88.3 Hz** and **2.5 kHz**. The low band passes through an allpass at the upper crossover so the bands stay phase-aligned.
4. Each band has its own peak envelope, shared across channels. Attack and release are one-pole coefficients scaled by the Time control.
5. Envelope level is turned into gain by a soft-knee downward compressor and a soft-knee upward compressor. Upward gain is capped at +30 dB and fades out below a −72 dB noise floor.
6. Dynamic gain is combined with a fixed per-band makeup gain and the band’s level control.
7. The three bands are summed, mixed with the dry signal by Depth, and scaled by output gain.

The last dynamic gain of each band is stored for metering.

### Band defaults

| Band | Crossover | Down threshold | Down ratio | Up threshold | Up ratio | Attack | Release | Makeup |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Low | below 88.3 Hz | −33.8 dB | 66.7:1 | −40.8 dB | 4.17:1 | 47.8 ms | 282 ms | +5.2 dB |
| Mid | 88.3 Hz – 2.5 kHz | −30.2 dB | 66.7:1 | −41.8 dB | 4.17:1 | 22.4 ms | 282 ms | +5.7 dB |
| High | above 2.5 kHz | −35.5 dB | 100:1 | −40.8 dB | 4.17:1 | 13.5 ms | 132 ms | +10.3 dB |

Every band uses a 6 dB knee. Time multiplies attack and release. Upward and Downward scale the boost and the reduction.

## Parameters

The plugin exposes these controls through `AudioProcessorValueTreeState`:

| Parameter | Role |
| --- | --- |
| Depth | Wet/dry mix of the compressed signal (0–100%) |
| Time | Attack and release multiplier (0.1×–10×) |
| Upward | Amount of upward compression (0–200%) |
| Downward | Amount of downward compression (0–200%) |
| In Gain | Level into the compressor |
| Out Gain | Level after the depth mix |
| Low / Mid / High | Per-band level, added to the makeup gain |

## Build

C++20, [JUCE](https://github.com/juce-framework/JUCE), CMake 3.22 or newer. JUCE is expected at `libs/JUCE`. The target builds VST3, AU, and Standalone under the product name **We Have OTT At Home** (Haramonics).

```
cmake -B build -S .
cmake --build build --config Release
```

## Layout

```
src/OTTProcessor.h / .cpp      crossover, envelopes, compression
src/PluginProcessor.h / .cpp   JUCE wrapper and parameters
src/PluginEditor.h / .cpp      user interface
CMakeLists.txt                 VST3, AU, and Standalone target
```

## Status

The multiband compressor is implemented in `OTTProcessor`. The plugin wrapper declares the parameter layout and the engine; `processBlock`, state save/load, and the editor are still empty, so the plugin does not process audio end to end yet.
