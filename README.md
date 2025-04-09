# ? ApplicationLoopback — Windows Audio Capture Tool

A command-line utility to capture/block audio from specific Windows processes. Perfect for streaming, podcasting, and debugging!

---

## ? Key Features

- ? **Process tree control** - `includetree`/`excludetree` modes
- ? **PID targeting** - Capture audio from specific processes
- ? **WAV output** or **raw PCM stream** (`-stream` flag)
- ? **Smart silence skipping** (16-bit threshold detection)
- ? **Low-latency capture** (<50ms buffer)

---

## ? Installation

### Requirements
- Windows 10/11 x64
- [Visual Studio 2022](https://visualstudio.microsoft.com/) with C++ tools
- Windows SDK (10.0.19041.0+)

```bash
# Clone repo
git clone https://github.com/yourname/ApplicationLoopback.git
cd ApplicationLoopback

# Build Release x64
MSBuild ApplicationLoopback.sln /p:Configuration=Release /p:Platform=x64
```

---

## ? Usage Syntax

```bash
ApplicationLoopback.exe [PID] [MODE] [OUTPUT] [OPTIONS]
```

### Arguments
| Parameter  | Description                                                                                                                                                                    |
|------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `[PID]`    | Target process ID                                                                                                                                                              |
| `[MODE]`   | `includetree` to capture process + children or `excludetree` to capture all exclude this process                                                                               |
| `[OUTPUT]` | `output.wav` file or `-stream` to use stdout (for another programs like https://github.com/BiosNod/NetAudioStream if you want to stream your device/process audio via network) |

### `[Options]`
| Flag            | Effect                                     |
|-----------------|--------------------------------------------|
| `-silence`      | Skip silent audio chunks                   |
| `-skipheaders`  | Omit WAV header (stream mode only)         |

---

## ? Examples

### Basic Capture
```bash
# Capture Chrome audio to file
ApplicationLoopback.exe 1234 includetree chrome_audio.wav
```

### Stream to FFmpeg
```bash
# Pipe raw PCM to FFmpeg for live encoding
ApplicationLoopback.exe 5678 excludetree -stream -skipheaders | ^
ffmpeg -f s16le -ar 44100 -ac 2 -i pipe:0 output.mp3
```

### Skip silent audio data
```bash
# Capture Teams meeting without pauses
ApplicationLoopback.exe 9012 includetree meeting.wav -silence
```

### Real-time raw audio data to stdout
```bash
# Capture Teams meeting without pauses
ApplicationLoopback.exe 9012 includetree -stream -silence -skipheaders
```