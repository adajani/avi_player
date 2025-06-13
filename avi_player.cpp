/**
 * @file avi_player.cpp
 * @brief Implementation of the AVI Player class
 * @author Ahmed Dajani <adajani@iastate.edu>
 * @date 2025
 * @version 1.0
 */

#include "avi_player.h"

AVIPlayer::AVIPlayer() 
    : window(nullptr), renderer(nullptr), texture(nullptr), 
      frameWidth(0), frameHeight(0), fps(0), totalFrames(0), 
      currentFrame(0), bitsPerPixel(0), bytesPerPixel(0), isTopDown(false),
      sdlPixelFormat(SDL_PIXELFORMAT_UNKNOWN), isValid(false) {
}

AVIPlayer::~AVIPlayer() {
    cleanup();
}

bool AVIPlayer::loadAVI(const std::string& filepath) {
    file.open(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filepath << std::endl;
        return false;
    }
    
    // Read RIFF header
    RIFFHeader riffHeader;
    file.read(reinterpret_cast<char*>(&riffHeader), sizeof(RIFFHeader));
    
    if (strncmp(riffHeader.signature, "RIFF", 4) != 0 || 
        strncmp(riffHeader.format, "AVI ", 4) != 0) {
        std::cerr << "Error: Not a valid AVI file" << std::endl;
        return false;
    }
    
    // Parse AVI chunks
    if (!parseAVIChunks()) {
        std::cerr << "Error: Failed to parse AVI structure" << std::endl;
        return false;
    }
    
    // Calculate FPS
    if (mainHeader.microSecPerFrame > 0) {
        fps = 1000000 / mainHeader.microSecPerFrame;
    } else {
        fps = 30; // Default fallback
    }
    
    frameWidth = mainHeader.width;
    frameHeight = mainHeader.height;
    totalFrames = mainHeader.totalFrames;
    
    // Handle negative height (indicates top-down bitmap)
    if (bitmapHeader.height < 0) {
        isTopDown = true;
        bitmapHeader.height = -bitmapHeader.height;
        frameHeight = bitmapHeader.height;
        std::cout << "  Image orientation: Top-down" << std::endl;
    } else {
        isTopDown = false;
        std::cout << "  Image orientation: Bottom-up" << std::endl;
    }
    
    // Determine pixel format from bitmap header
    if (!determinePixelFormat()) {
        std::cerr << "Error: Unsupported pixel format" << std::endl;
        return false;
    }
    
    std::cout << "AVI Info:" << std::endl;
    std::cout << "  Resolution: " << frameWidth << "x" << frameHeight << std::endl;
    std::cout << "  FPS: " << fps << std::endl;
    std::cout << "  Total Frames: " << totalFrames << std::endl;
    std::cout << "  Bits Per Pixel: " << bitsPerPixel << std::endl;
    std::cout << "  Compression: " << bitmapHeader.compression << std::endl;
    std::cout << "  Duration: " << (totalFrames / (float)fps) << " seconds" << std::endl;
    
    isValid = true;
    return true;
}

bool AVIPlayer::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    window = SDL_CreateWindow("AVI Player",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            frameWidth, frameHeight,
                            SDL_WINDOW_SHOWN);
    
    if (!window) {
        std::cerr << "Window Creation Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer Creation Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create texture with the determined pixel format
    texture = SDL_CreateTexture(renderer,
                              sdlPixelFormat,
                              SDL_TEXTUREACCESS_STREAMING,
                              frameWidth, frameHeight);
    
    if (!texture) {
        std::cerr << "Texture Creation Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    return true;
}

void AVIPlayer::play() {
    if (!isValid) {
        std::cerr << "Error: AVI file not loaded or invalid" << std::endl;
        return;
    }
    
    bool quit = false;
    SDL_Event e;
    
    auto frameTime = std::chrono::milliseconds(1000 / fps);
    auto lastFrameTime = std::chrono::steady_clock::now();
    
    std::cout << "Playing AVI... Press ESC or close window to exit." << std::endl;
    
    while (!quit && currentFrame < totalFrames) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || 
                (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
        }
        
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - lastFrameTime >= frameTime) {
            renderFrame(currentFrame);
            currentFrame++;
            lastFrameTime = currentTime;
        }
        
        // Small delay to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    if (currentFrame >= totalFrames) {
        std::cout << "Playback completed!" << std::endl;
        // Wait for user to close window
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT || 
                    (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                    quit = true;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool AVIPlayer::determinePixelFormat() {
    bitsPerPixel = bitmapHeader.bitCount;
    bytesPerPixel = (bitsPerPixel + 7) / 8;
    
    // Only support uncompressed formats
    if (bitmapHeader.compression != 0) {
        std::cerr << "Error: Compressed formats not supported (compression = " 
                  << bitmapHeader.compression << ")" << std::endl;
        return false;
    }
    
    switch (bitsPerPixel) {
        case 8:
            // 8-bit indexed color
            sdlPixelFormat = SDL_PIXELFORMAT_RGB24; // We'll convert to RGB24
            std::cout << "  Format: 8-bit indexed color" << std::endl;
            break;
        case 16:
            // 16-bit RGB (usually RGB565)
            sdlPixelFormat = SDL_PIXELFORMAT_RGB565;
            std::cout << "  Format: 16-bit RGB565" << std::endl;
            break;
        case 24:
            // 24-bit RGB (stored as BGR in AVI)
            sdlPixelFormat = SDL_PIXELFORMAT_RGB24;
            std::cout << "  Format: 24-bit RGB" << std::endl;
            break;
        case 32:
            // 32-bit RGBA (stored as BGRA in AVI)
            sdlPixelFormat = SDL_PIXELFORMAT_RGBA32;
            std::cout << "  Format: 32-bit RGBA" << std::endl;
            break;
        default:
            std::cerr << "Error: Unsupported bit depth: " << bitsPerPixel << std::endl;
            return false;
    }
    
    return true;
}

bool AVIPlayer::parseAVIChunks() {
    ChunkHeader chunk;
    bool foundMainHeader = false;
    
    while (file.read(reinterpret_cast<char*>(&chunk), sizeof(ChunkHeader))) {
        if (strncmp(chunk.fourCC, "LIST", 4) == 0) {
            char listType[4];
            file.read(listType, 4);
            
            if (strncmp(listType, "hdrl", 4) == 0) {
                // Header list - parse headers
                parseHeaderList(chunk.size - 4);
                foundMainHeader = true;
            } else if (strncmp(listType, "movi", 4) == 0) {
                // Movie data - index frame positions
                indexFrames(chunk.size - 4);
                break;
            } else {
                // Skip other lists
                file.seekg(chunk.size - 4, std::ios::cur);
            }
        } else {
            // Skip other chunks
            file.seekg(chunk.size, std::ios::cur);
        }
    }
    
    return foundMainHeader && !frameOffsets.empty();
}

void AVIPlayer::parseHeaderList(uint32_t size) {
    uint32_t bytesRead = 0;
    ChunkHeader chunk;
    
    while (bytesRead < size && file.read(reinterpret_cast<char*>(&chunk), sizeof(ChunkHeader))) {
        bytesRead += sizeof(ChunkHeader);
        
        if (strncmp(chunk.fourCC, "avih", 4) == 0) {
            // Main AVI header
            file.read(reinterpret_cast<char*>(&mainHeader), sizeof(AVIMainHeader));
        } else if (strncmp(chunk.fourCC, "LIST", 4) == 0) {
            char listType[4];
            file.read(listType, 4);
            bytesRead += 4;
            
            if (strncmp(listType, "strl", 4) == 0) {
                // Stream list
                parseStreamList(chunk.size - 4);
            } else {
                file.seekg(chunk.size - 4, std::ios::cur);
            }
        } else {
            // Skip other chunks
            file.seekg(chunk.size, std::ios::cur);
        }
        
        bytesRead += chunk.size;
    }
}

void AVIPlayer::parseStreamList(uint32_t size) {
    uint32_t bytesRead = 0;
    ChunkHeader chunk;
    
    while (bytesRead < size && file.read(reinterpret_cast<char*>(&chunk), sizeof(ChunkHeader))) {
        bytesRead += sizeof(ChunkHeader);
        
        if (strncmp(chunk.fourCC, "strh", 4) == 0) {
            // Stream header
            file.read(reinterpret_cast<char*>(&streamHeader), sizeof(AVIStreamHeader));
        } else if (strncmp(chunk.fourCC, "strf", 4) == 0) {
            // Stream format (bitmap info for video)
            if (strncmp(streamHeader.fccType, "vids", 4) == 0) {
                file.read(reinterpret_cast<char*>(&bitmapHeader), sizeof(BitmapInfoHeader));
                
                // Read palette if present (for 8-bit indexed color)
                uint32_t remainingBytes = chunk.size - sizeof(BitmapInfoHeader);
                if (remainingBytes > 0 && bitmapHeader.bitCount == 8) {
                    uint32_t paletteEntries = remainingBytes / sizeof(RGBQuad);
                    palette.resize(paletteEntries);
                    file.read(reinterpret_cast<char*>(palette.data()), remainingBytes);
                    std::cout << "  Read palette with " << paletteEntries << " entries" << std::endl;
                } else if (remainingBytes > 0) {
                    // Skip remaining bytes if not palette data
                    file.seekg(remainingBytes, std::ios::cur);
                }
            } else {
                file.seekg(chunk.size, std::ios::cur);
            }
        } else {
            file.seekg(chunk.size, std::ios::cur);
        }
        
        bytesRead += chunk.size;
    }
}

void AVIPlayer::indexFrames(uint32_t movieSize) {
    auto movieStart = file.tellg();
    ChunkHeader chunk;
    
    while (file.tellg() < movieStart + static_cast<std::streampos>(movieSize) && 
           file.read(reinterpret_cast<char*>(&chunk), sizeof(ChunkHeader))) {
        
        if (strncmp(chunk.fourCC, "00dc", 4) == 0 || // Uncompressed video
            strncmp(chunk.fourCC, "00db", 4) == 0) { // DIB format
            
            frameOffsets.push_back(static_cast<uint32_t>(file.tellg()));
            frameSizes.push_back(chunk.size);
        }
        
        // Skip chunk data (pad to even boundary)
        uint32_t skipSize = chunk.size;
        if (skipSize % 2 == 1) skipSize++; // AVI chunks are padded to even boundaries
        file.seekg(skipSize, std::ios::cur);
    }
    
    std::cout << "Indexed " << frameOffsets.size() << " frames" << std::endl;
}

void AVIPlayer::renderFrame(uint32_t frameIndex) {
    if (frameIndex >= frameOffsets.size()) return;
    
    file.seekg(frameOffsets[frameIndex]);
    
    // Read frame data
    std::vector<uint8_t> frameData(frameSizes[frameIndex]);
    file.read(reinterpret_cast<char*>(frameData.data()), frameSizes[frameIndex]);
    
    // Update texture
    void* pixels;
    int pitch;
    SDL_LockTexture(texture, nullptr, &pixels, &pitch);
    
    convertAndCopyFrame(frameData, static_cast<uint8_t*>(pixels), pitch);
    
    SDL_UnlockTexture(texture);
    
    // Render
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void AVIPlayer::convertAndCopyFrame(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch) {
    switch (bitsPerPixel) {
        case 8:
            convert8BitToRGB24(frameData, pixels, pitch);
            break;
        case 16:
            convert16BitToRGB565(frameData, pixels, pitch);
            break;
        case 24:
            convert24BitBGRToRGB(frameData, pixels, pitch);
            break;
        case 32:
            convert32BitBGRAToRGBA(frameData, pixels, pitch);
            break;
    }
}

void AVIPlayer::convert8BitToRGB24(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch) {
    for (uint32_t y = 0; y < frameHeight; ++y) {
        uint32_t srcY = isTopDown ? y : (frameHeight - 1 - y);
        uint8_t* dst = pixels + y * pitch;
        const uint8_t* src = frameData.data() + srcY * frameWidth;
        
        for (uint32_t x = 0; x < frameWidth; ++x) {
            uint8_t paletteIndex = src[x];
            if (paletteIndex < palette.size()) {
                const RGBQuad& color = palette[paletteIndex];
                dst[x * 3 + 0] = color.red;
                dst[x * 3 + 1] = color.green;
                dst[x * 3 + 2] = color.blue;
            } else {
                // Fallback for invalid palette index
                dst[x * 3 + 0] = dst[x * 3 + 1] = dst[x * 3 + 2] = 0;
            }
        }
    }
}

void AVIPlayer::convert16BitToRGB565(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch) {
    for (uint32_t y = 0; y < frameHeight; ++y) {
        uint32_t srcY = isTopDown ? y : (frameHeight - 1 - y);
        uint16_t* dst = reinterpret_cast<uint16_t*>(pixels + y * pitch);
        const uint16_t* src = reinterpret_cast<const uint16_t*>(frameData.data() + srcY * frameWidth * 2);
        
        for (uint32_t x = 0; x < frameWidth; ++x) {
            // AVI stores as little-endian, so no conversion needed for RGB565
            dst[x] = src[x];
        }
    }
}

void AVIPlayer::convert24BitBGRToRGB(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch) {
    for (uint32_t y = 0; y < frameHeight; ++y) {
        uint32_t srcY = isTopDown ? y : (frameHeight - 1 - y);
        uint8_t* dst = pixels + y * pitch;
        const uint8_t* src = frameData.data() + srcY * frameWidth * 3;
        
        for (uint32_t x = 0; x < frameWidth; ++x) {
            // Convert BGR to RGB
            dst[x * 3 + 0] = src[x * 3 + 2]; // R
            dst[x * 3 + 1] = src[x * 3 + 1]; // G
            dst[x * 3 + 2] = src[x * 3 + 0]; // B
        }
    }
}

void AVIPlayer::convert32BitBGRAToRGBA(const std::vector<uint8_t>& frameData, uint8_t* pixels, int pitch) {
    for (uint32_t y = 0; y < frameHeight; ++y) {
        uint32_t srcY = isTopDown ? y : (frameHeight - 1 - y);
        uint8_t* dst = pixels + y * pitch;
        const uint8_t* src = frameData.data() + srcY * frameWidth * 4;
        
        for (uint32_t x = 0; x < frameWidth; ++x) {
            // Convert BGRA to RGBA
            dst[x * 4 + 0] = src[x * 4 + 2]; // R
            dst[x * 4 + 1] = src[x * 4 + 1]; // G
            dst[x * 4 + 2] = src[x * 4 + 0]; // B
            dst[x * 4 + 3] = src[x * 4 + 3]; // A
        }
    }
}

void AVIPlayer::cleanup() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (file.is_open()) {
        file.close();
    }
    SDL_Quit();
}