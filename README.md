# AVI Player

A simple, lightweight AVI video player for uncompressed video files, built with C++ and SDL2.

## Features

- Plays uncompressed AVI video files
- Supports multiple pixel formats:
  - 8-bit indexed color (with palette)
  - 16-bit RGB565
  - 24-bit RGB
  - 32-bit RGBA
- Maintains proper frame timing based on video FPS
- Cross-platform compatibility (Linux, macOS, Windows)
- Simple keyboard controls (ESC to quit)

## Requirements

### Dependencies
- **SDL2** development libraries
- **C++11** compatible compiler (GCC, Clang, MSVC)
- **Make** build system
- **Doxygen** (optional, for documentation generation)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev doxygen
```

**CentOS/RHEL/Fedora:**
```bash
sudo yum install gcc-c++ SDL2-devel doxygen
# or for newer versions:
sudo dnf install gcc-c++ SDL2-devel doxygen
```

**macOS (with Homebrew):**
```bash
brew install sdl2 doxygen
```

**Windows:**
- Install Visual Studio or MinGW
- Download SDL2 development libraries from [libsdl.org](https://www.libsdl.org/download-2.0.php)
- Extract and configure paths appropriately

## Building

### Quick Build
```bash
make
```

### Debug Build
```bash
make debug
```

### Check Dependencies
```bash
make check-deps
```

### Build Targets
- `make` or `make all` - Build the program
- `make debug` - Build with debug symbols
- `make clean` - Remove build artifacts
- `make docs` - Generate documentation
- `make install` - Install to system (requires sudo)
- `make help` - Show all available targets

## Usage

### Basic Usage
```bash
./avi_player your_video.avi
```

### Converting Compressed Videos
If you have a compressed AVI file, convert it to uncompressed format first:

```bash
# Convert to 24-bit RGB
ffmpeg -i input.avi -c:v rawvideo -pix_fmt rgb24 -f avi output.avi

# Convert to 24-bit BGR (recommended)
ffmpeg -i input.avi -c:v rawvideo -pix_fmt bgr24 -f avi output_bgr.avi

# Convert to 8-bit indexed color
ffmpeg -i input.avi -c:v rawvideo -pix_fmt pal8 -f avi output_indexed.avi
```

### Controls
- **ESC** - Exit the player
- **Close Window** - Exit the player

## Project Structure

```
avi_player/
├── avi_player.h     # Main class header with documentation
├── avi_player.cpp   # Implementation
├── main.cpp         # Main program entry point
├── Makefile         # Build configuration
├── Doxyfile         # Doxygen configuration
├── README.md        # This file
└── build/           # Build artifacts (created during compilation)
    └── *.o          # Object files
```

## Documentation

Generate HTML documentation using Doxygen:

```bash
make docs
```

Documentation will be generated in the `docs/html/` directory. Open `docs/html/index.html` in your web browser.

## Technical Details

### Supported AVI Formats
- **Container:** RIFF AVI format
- **Video:** Uncompressed video streams only
- **Compression:** BI_RGB (compression = 0)
- **Pixel Formats:**
  - 8-bit indexed (with palette)
  - 16-bit RGB565
  - 24-bit BGR (AVI standard)
  - 32-bit BGRA (AVI standard)

### Frame Timing
The player respects the original video frame rate by reading the `microSecPerFrame` value from the AVI main header and maintaining precise timing during playback.

### Memory Usage
The player uses streaming architecture, loading only one frame at a time to minimize memory usage, making it suitable for large video files.

## Examples

### Example 1: Playing a converted video
```bash
# Convert a compressed video
ffmpeg -i sample.mp4 -c:v rawvideo -pix_fmt bgr24 -f avi sample_uncompressed.avi

# Play it
./avi_player sample_uncompressed.avi
```

### Example 2: Creating a test video
```bash
# Create a test pattern video
ffmpeg -f lavfi -i testsrc=duration=10:size=640x480:rate=30 -c:v rawvideo -pix_fmt bgr24 -f avi test.avi

# Play the test video
./avi_player test.avi
```

## Troubleshooting

### Common Issues

1. **"SDL Init Error"**
   - Ensure SDL2 development libraries are installed
   - Check that your system has proper graphics drivers

2. **"Error: Compressed formats not supported"**
   - Your AVI file uses a compressed codec
   - Convert to uncompressed format using FFmpeg

3. **"Error: Cannot open file"**
   - Check file path and permissions
   - Ensure the file exists and is readable

4. **Black screen during playback**
   - Usually indicates pixel format mismatch
   - Try converting with different pixel formats

### Debug Mode
Build with debug symbols for troubleshooting:
```bash
make debug
gdb ./avi_player
```

## Limitations

- **Compressed formats:** Only uncompressed AVI files are supported
- **Audio:** No audio playback (video only)
- **Seeking:** No seeking/scrubbing support
- **Playlist:** Plays one file at a time

## Contributing

This is a simple educational project demonstrating AVI file parsing and SDL2 usage. Feel free to extend it with additional features like:

- Audio playback support
- Compressed codec support (requires FFmpeg integration)
- Seeking/scrubbing functionality
- Playlist support
- Video filters

## License

This project is provided as-is for educational purposes. Feel free to use and modify as needed.

## Version History

- **v1.0** - Initial release with basic uncompressed AVI playback support