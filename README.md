# we-have-ott-at-home

A personal audio plugin that recreates the sound and controls of [Xfer Records OTT](https://xferrecords.com/freeware/). The project name is temporary and will change.

This is an independent project. It is not affiliated with Xfer Records.

## What it is

OTT is a three-band upward and downward compressor. It splits the signal into low, mid, and high bands, compresses each band hard in both directions, then mixes that result back with the dry signal. The result is the bright, dense, “over the top” character the plugin is known for.

The intended controls match the original:

| Control | Role |
| --- | --- |
| Depth | Wet/dry mix of the multiband compression |
| Time | Shared attack and release character |
| In Gain | Input level into the compressor |
| Out Gain | Output level after processing |
| Low / Mid / High | Per-band level |

## Status

Early scaffold. The processor, editor, and CMake files are in place as empty starting points. Signal processing and the user interface are not implemented yet.

```
src/
  OTTProcessor.h / .cpp    multiband compressor
  PluginProcessor.h / .cpp plugin wrapper
  PluginEditor.h / .cpp    user interface
CMakeLists.txt             build
```
