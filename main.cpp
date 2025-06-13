/**
 * @file main.cpp
 * @brief Main entry point for the AVI Player application
 * @author Ahmed Dajani <adajani@iastate.edu>
 * @date 2025
 * @version 1.0
 * 
 * This is the main entry point for the AVI Player. It handles command line
 * arguments, creates an AVIPlayer instance, and manages the playback lifecycle.
 */

#include "avi_player.h"
#include <iostream>

/**
 * @brief Print usage information
 * 
 * Displays help text showing how to use the program.
 * 
 * @param programName Name of the program executable
 */
void printUsage(const char* programName) {
    std::cout << "AVI Player v1.0 - Simple Uncompressed AVI Video Player" << std::endl;
    std::cout << "Usage: " << programName << " <avi_file_path>" << std::endl;
    std::cout << std::endl;
    std::cout << "Supported formats:" << std::endl;
    std::cout << "  - Uncompressed AVI files only" << std::endl;
    std::cout << "  - 8-bit indexed color (with palette)" << std::endl;
    std::cout << "  - 16-bit RGB565" << std::endl;
    std::cout << "  - 24-bit RGB" << std::endl;
    std::cout << "  - 32-bit RGBA" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC key or close window to exit" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: For compressed AVI files, convert to uncompressed format first:" << std::endl;
    std::cout << "  ffmpeg -i input.avi -c:v rawvideo -pix_fmt bgr24 -f avi output.avi" << std::endl;
}

/**
 * @brief Main program entry point
 * 
 * Processes command line arguments, validates the AVI file, initializes
 * the player, and starts playback.
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return 0 on success, 1 on error
 */
int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 2) {
        if (argc > 2) {
            std::cerr << "Error: Too many arguments" << std::endl;
        } else {
            std::cerr << "Error: Missing AVI file path" << std::endl;
        }
        std::cerr << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    std::string filepath = argv[1];
    
    // Check if file exists by trying to open it
    std::ifstream testFile(filepath);
    if (!testFile.is_open()) {
        std::cerr << "Error: Cannot access file '" << filepath << "'" << std::endl;
        std::cerr << "Please check that the file exists and you have read permissions." << std::endl;
        return 1;
    }
    testFile.close();
    
    std::cout << "Loading AVI file: " << filepath << std::endl;
    
    // Create player instance
    AVIPlayer player;
    
    // Load the AVI file
    if (!player.loadAVI(filepath)) {
        std::cerr << "Failed to load AVI file: " << filepath << std::endl;
        std::cerr << std::endl;
        std::cerr << "Common issues:" << std::endl;
        std::cerr << "  - File may be compressed (use FFmpeg to convert)" << std::endl;
        std::cerr << "  - File may be corrupted or invalid" << std::endl;
        std::cerr << "  - File may not be an AVI format" << std::endl;
        return 1;
    }
    
    // Initialize SDL
    if (!player.initSDL()) {
        std::cerr << "Failed to initialize SDL graphics" << std::endl;
        std::cerr << "Please ensure you have proper graphics drivers installed." << std::endl;
        return 1;
    }
    
    // Start playback
    std::cout << std::endl;
    player.play();
    
    std::cout << "Playback finished. Goodbye!" << std::endl;
    return 0;
}