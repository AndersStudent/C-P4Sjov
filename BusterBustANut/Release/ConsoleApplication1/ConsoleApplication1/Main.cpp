#include <iostream>
#include <filesystem> 
#include <string>
#include <vector>
#include <sndfile.h>

namespace fs = std::filesystem;

int main() {

    // Define the directory path
    std::string directoryPath = "C:\\Users\\Ander\\Documents\\C-P4Sjov\\Audios";

    // Collect file paths
    std::vector<std::string> audioFilePaths;

    // Iterate over the files in the directory
    for (const auto& entry : fs::directory_iterator(directoryPath))
    {
        // Extract the filename without extension
        std::string filename = entry.path().stem().string();

        // Check if the filename matches "Source1" or "Target1"
        if (filename == "Source1" || filename == "Target1")
        {
            audioFilePaths.push_back(entry.path().string());
        }
    }

    // Check if we have both files
    if (audioFilePaths.size() != 2) {
        std::cerr << "Could not find both audio files.\n";
        return 1;
    }

    // Define an SF_INFO structure
    SF_INFO fileInfo;

    // Open both audio files
    SNDFILE* sourceFile = sf_open(audioFilePaths[0].c_str(), SFM_READ, &fileInfo);
    if (!sourceFile) {
        std::cerr << "Failed to open source audio file: " << sf_strerror(NULL) << std::endl;
        return 1;
    }

    SNDFILE* targetFile = sf_open(audioFilePaths[1].c_str(), SFM_READ, &fileInfo);
    if (!targetFile) {
        std::cerr << "Failed to open target audio file.\n";
        sf_close(sourceFile);
        return 1;
    }

    // Get audio file info
    SF_INFO sourceInfo, targetInfo;
    sf_command(sourceFile, SFC_GET_CURRENT_SF_INFO, &sourceInfo, sizeof(sourceInfo));
    sf_command(targetFile, SFC_GET_CURRENT_SF_INFO, &targetInfo, sizeof(targetInfo));

    // Check if both files have the same format
    if (sourceInfo.samplerate != targetInfo.samplerate || sourceInfo.channels != targetInfo.channels) {
        std::cerr << "Audio files have different formats.\n";
        sf_close(sourceFile);
        sf_close(targetFile);
        return 1;
    }

    // Create the output audio file
    std::string outputFilePath = directoryPath + "\\Results\\newaudio.wav";
    SF_INFO outputInfo = sourceInfo;
    SNDFILE* outputFile = sf_open(outputFilePath.c_str(), SFM_WRITE, &outputInfo);
    if (!outputFile) {
        std::cerr << "Failed to create output audio file.\n";
        sf_close(sourceFile);
        sf_close(targetFile);
        return 1;
    }

    // Read and add audio data
    const int bufferSize = 1024;
    std::vector<float> buffer(bufferSize * sourceInfo.channels);
    sf_count_t sourceReadCount, targetReadCount;
    while ((sourceReadCount = sf_readf_float(sourceFile, buffer.data(), bufferSize)) &&
        (targetReadCount = sf_readf_float(targetFile, buffer.data(), bufferSize))) {
        for (int i = 0; i < bufferSize * sourceInfo.channels; ++i) {
            buffer[i] += buffer[i + bufferSize * sourceInfo.channels]; // Add the samples
        }
        sf_writef_float(outputFile, buffer.data(), sourceReadCount);
    }

    // Close files
    sf_close(sourceFile);
    sf_close(targetFile);
    sf_close(outputFile);

    std::cout << "Audio files successfully added. Output file: " << outputFilePath << std::endl;

    return 0;
}
