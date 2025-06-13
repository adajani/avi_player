/**
 * @file avi_player.h
 * @brief Simple AVI Player for Uncompressed Video Files
 * @author Your Name
 * @date 2025
 * @version 1.0
 * 
 * This header defines a simple AVI video player that can load and play
 * uncompressed AVI files using SDL2 for rendering. The player supports
 * various uncompressed pixel formats including 8-bit indexed, 16-bit RGB565,
 * 24-bit RGB, and 32-bit RGBA.
 */

#ifndef AVI_PLAYER_H
#define AVI_PLAYER_H

#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <chrono>
#include <thread>

/**
 * @brief RIFF file header structure
 * 
 * Contains the basic RIFF container information for AVI files.
 */
#pragma pack(push, 1)
struct RIFFHeader {
    char signature[4];      ///< "RIFF" signature
    uint32_t fileSize;      ///< Total file size minus 8 bytes
    char format[4];         ///< "AVI " format identifier
};

/**
 * @brief Main AVI header structure (avih chunk)
 * 
 * Contains global information about the AVI file including
 * frame rate, dimensions, and total frame count.
 */
struct AVIMainHeader {
    uint32_t microSecPerFrame;      ///< Frame duration in microseconds
    uint32_t maxBytesPerSec;        ///< Maximum data rate
    uint32_t paddingGranularity;    ///< Padding granularity
    uint32_t flags;                 ///< AVI file flags
    uint32_t totalFrames;           ///< Total number of frames
    uint32_t initialFrames;         ///< Initial frames for interleaved files
    uint32_t streams;               ///< Number of streams
    uint32_t suggestedBufferSize;   ///< Suggested buffer size
    uint32_t width;                 ///< Video width in pixels
    uint32_t height;                ///< Video height in pixels
    uint32_t reserved[4];           ///< Reserved fields
};

/**
 * @brief Stream header structure (strh chunk)
 * 
 * Contains information about individual streams (video/audio).
 */
struct AVIStreamHeader {
    char fccType[4];                ///< Stream type ('vids', 'auds', etc.)
    char fccHandler[4];             ///< Codec handler
    uint32_t flags;                 ///< Stream flags
    uint16_t priority;              ///< Stream priority
    uint16_t language;              ///< Language code
    uint32_t initialFrames;         ///< Initial frames
    uint32_t scale;                 ///< Time scale
    uint32_t rate;                  ///< Rate (rate/scale = samples/second)
    uint32_t start;                 ///< Start time
    uint32_t length;                ///< Stream length
    uint32_t suggestedBufferSize;   ///< Suggested buffer size
    uint32_t quality;               ///< Quality indicator
    uint32_t sampleSize;            ///< Sample size
    struct {
        int16_t left;               ///< Left coordinate
        int16_t top;                ///< Top coordinate
        int16_t right;              ///< Right coordinate
        int16_t bottom;             ///< Bottom coordinate
    } frame;                        ///< Frame rectangle
};

/**
 * @brief Bitmap info header structure (strf chunk for video)
 * 
 * Contains detailed information about the video format.
 */
struct BitmapInfoHeader {
    uint32_t size;                  ///< Header size
    int32_t width;                  ///< Image width
    int32_t height;                 ///< Image height (negative = top-down)
    uint16_t planes;                ///< Number of color planes
    uint16_t bitCount;              ///< Bits per pixel
    uint32_t compression;           ///< Compression type
    uint32_t sizeImage;             ///< Image size in bytes
    int32_t xPelsPerMeter;          ///< Horizontal resolution
    int32_t yPelsPerMeter;          ///< Vertical resolution
    uint32_t clrUsed;               ///< Colors used
    uint32_t clrImportant;          ///< Important colors
};

/**
 * @brief RGB color quad for palette entries
 * 
 * Used for 8-bit indexed color palettes.
 */
struct RGBQuad {
    uint8_t blue;                   ///< Blue component
    uint8_t green;                  ///< Green component
    uint8_t red;                    ///< Red component
    uint8_t reserved;               ///< Reserved (usually 0)
};
#pragma pack(pop)

/**
 * @brief Chunk header structure
 * 
 * Generic chunk header used throughout AVI files.
 */
struct ChunkHeader {
    char fourCC[4];                 ///< Four-character code
    uint32_t size;                  ///< Chunk data size
};

/**
 * @brief Simple AVI Player Class
 * 
 * This class provides functionality to load and play uncompressed AVI files
 * using SDL2 for rendering. It supports multiple pixel formats and maintains
 * proper frame timing based on the video's native frame rate.
 * 
 * Supported formats:
 * - 8-bit indexed color (with palette)
 * - 16-bit RGB565
 * - 24-bit RGB (BGR in AVI)
 * - 32-bit RGBA (BGRA in AVI)
 * 
 * Usage example:
 * @code
 * AVIPlayer player;
 * if (player.loadAVI("video.avi") && player.initSDL()) {
 *     player.play();
 * }
 * @endcode
 */
class AVIPlayer {
private:
    SDL_Window* window;             ///< SDL window handle
    SDL_Renderer* renderer;         ///< SDL renderer handle
    SDL_Texture* texture;           ///< SDL texture for frame display
    
    std::ifstream file;             ///< Input file stream
    AVIMainHeader mainHeader;       ///< Main AVI header
    AVIStreamHeader streamHeader;   ///< Video stream header
    BitmapInfoHeader bitmapHeader;  ///< Bitmap format header
    
    uint32_t frameWidth;            ///< Video frame width
    uint32_t frameHeight;           ///< Video frame height
    uint32_t fps;                   ///< Frames per second
    uint32_t totalFrames;           ///< Total number of frames
    uint32_t currentFrame;          ///< Current frame index
    uint32_t bitsPerPixel;          ///< Bits per pixel
    uint32_t bytesPerPixel;         ///< Bytes per pixel
    bool isTopDown;                 ///< True if bitmap is top-down
    
    std::vector<uint32_t> frameOffsets;  ///< File offsets for each frame
    std::vector<uint32_t> frameSizes;    ///< Size of each frame in bytes
    std::vector<RGBQuad> palette;        ///< Color palette for 8-bit mode
    
    SDL_PixelFormatEnum sdlPixelFormat;  ///< SDL pixel format
    bool isValid;                        ///< True if file loaded successfully

public:
    /**
     * @brief Constructor
     * 
     * Initializes all member variables to default values.
     */
    AVIPlayer();
    
    /**
     * @brief Destructor
     * 
     * Cleans up SDL resources and closes files.
     */
    ~AVIPlayer();
    
    /**
     * @brief Load an AVI file
     * 
     * Parses the AVI file structure, extracts headers, and indexes frames.
     * Only supports uncompressed AVI files.
     * 
     * @param filepath Path to the AVI file
     * @return true if file loaded successfully, false otherwise
     */
    bool loadAVI(const std::string& filepath);
    
    /**
     * @brief Initialize SDL subsystem
     * 
     * Creates SDL window, renderer, and texture based on video dimensions.
     * Must be called after loadAVI() and before play().
     * 
     * @return true if SDL initialized successfully, false otherwise
     */
    bool initSDL();
    
    /**
     * @brief Play the loaded video
     * 
     * Starts video playback with proper frame timing. Handles SDL events
     * for user input (ESC to quit). Blocks until playback completes or
     * user quits.
     */
    void play();

private:
    /**
     * @brief Determine pixel format from bitmap header
     * 
     * Analyzes the bitmap header to determine the pixel format and
     * sets up appropriate SDL pixel format and conversion parameters.
     * 
     * @return true if format is supported, false otherwise
     */
    bool determinePixelFormat();
    
    /**
     * @brief Parse AVI file chunks
     * 
     * Reads through the AVI file structure, parsing headers and
     * finding the movie data section.
     * 
     * @return true if parsing successful, false otherwise
     */
    bool parseAVIChunks();
    
    /**
     * @brief Parse header list chunk
     * 
     * Processes the header list containing main header and stream headers.
     * 
     * @param size Size of the header list
     */
    void parseHeaderList(uint32_t size);
    
    /**
     * @brief Parse stream list chunk
     * 
     * Processes individual stream information including format details.
     * 
     * @param size Size of the stream list
     */
    void parseStreamList(uint32_t size);
    
    /**
     * @brief Index video frames
     * 
     * Scans the movie data section and records the file offset and size
     * of each video frame for efficient seeking during playback.
     * 
     * @param movieSize Size of the movie data section
     */
    void indexFrames(uint32_t movieSize);
    
    /**
     * @brief Render a specific frame
     * 
     * Reads frame data from file, converts pixel format if necessary,
     * and renders to the SDL texture.
     * 
     * @param frameIndex Index of the frame to render
     */
    void renderFrame(uint32_t frameIndex);
    
    /**
     * @brief Convert and copy frame data
     * 
     * Dispatches to appropriate conversion function based on pixel format.
     * 
     * @param frameData Raw frame data from AVI file
     * @param pixels Destination pixel buffer
     * @param pitch Row stride in bytes
     */
    void convertAndCopyFrame(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch);
    
    /**
     * @brief Convert 8-bit indexed to RGB24
     * 
     * Converts 8-bit palette indices to 24-bit RGB using color palette.
     * 
     * @param frameData Source indexed pixel data
     * @param pixels Destination RGB pixel buffer
     * @param pitch Row stride in bytes
     */
    void convert8BitToRGB24(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch);
    
    /**
     * @brief Convert 16-bit to RGB565
     * 
     * Copies 16-bit RGB565 data with proper byte order handling.
     * 
     * @param frameData Source 16-bit pixel data
     * @param pixels Destination pixel buffer
     * @param pitch Row stride in bytes
     */
    void convert16BitToRGB565(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch);
    
    /**
     * @brief Convert 24-bit BGR to RGB
     * 
     * Converts AVI's BGR byte order to SDL's RGB byte order.
     * 
     * @param frameData Source BGR pixel data
     * @param pixels Destination RGB pixel buffer
     * @param pitch Row stride in bytes
     */
    void convert24BitBGRToRGB(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch);
    
    /**
     * @brief Convert 32-bit BGRA to RGBA
     * 
     * Converts AVI's BGRA byte order to SDL's RGBA byte order.
     * 
     * @param frameData Source BGRA pixel data
     * @param pixels Destination RGBA pixel buffer
     * @param pitch Row stride in bytes
     */
    void convert32BitBGRAToRGBA(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch);
    
    /**
     * @brief Clean up resources
     * 
     * Destroys SDL objects, closes files, and resets state.
     */
    void cleanup();
};

#endif // AVI_PLAYER_H