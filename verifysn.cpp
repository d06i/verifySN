#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>   
#include <unordered_set>
#include <future> 
#include <format>
#include <execution>

#include "xxh64.hpp"
#include "ui.hpp"
 
namespace fs = std::filesystem; 

// You can change file part numbers. But hash also changes.
constexpr uint64_t max_part_number = 16;
constexpr uint64_t part_size   = 4 * 1024; // per part size

std::atomic<uint64_t> invalidHashCount = 0, emptyFileCount = 0, fileCount = 0;  

UI ui;

// Measure elapsed time in function
template <typename F>
auto measure(const std::string& funcName, F&& func) {
  auto start = std::chrono::high_resolution_clock::now();
  func();
  auto end  = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); 
  return time;
}
 
//  Get hash of file.
std::string getHash(const fs::path& filename) {

  std::ifstream file(filename, std::ios::binary);
  std::vector<uint8_t> file_hash;  

  if (!file.is_open()) {
      ui.setElement(MessageTypes::warning, "ERROR", "File not found!");
      throw std::runtime_error("Cannot open file");
  }

  uintmax_t filesize = fs::file_size(filename); 

 // If file is empty just send zero
  if ( filesize == 0 )
    return "Empty file.";

  if (filesize <= 64 * 1024) {
      std::vector<unsigned char> temp(filesize, 0);
      file.read(reinterpret_cast<char*>(temp.data()) , filesize);
      file_hash.insert(file_hash.end(), temp.begin(), temp.end());
  } else {
 
    uint64_t interval = filesize / max_part_number;

    for (int i = 0; i < max_part_number; i++) {
        std::vector<unsigned char> temp(part_size, 0);
        uint64_t curr_offset = i * interval;

        file.seekg(curr_offset); // jump to offset

        int read_size = std::min( part_size, filesize - curr_offset ); // get last part 
        
        file.read(reinterpret_cast<char*>( temp.data() ), read_size );
        file_hash.insert( file_hash.end(), temp.begin(), temp.end() );
      }
  }
  file.close();
 
  // calculate the hash
  const uint64_t hash = xxh64::hash( reinterpret_cast<const char*>(file_hash.data()), file_hash.size(), filesize);
    
  // Convert hash to hex 
  return std::format("{:016X}", hash); 
} 

// Get hash and add to file
void hashFile(const fs::path& filename, bool save = false) {

  auto hash = getHash(filename);

  if (save) {
      static std::mutex file_mtx;
      std::lock_guard<std::mutex> lock(file_mtx);

      std::ofstream hashes("hash.txt", std::ios::app);

      if (!hashes.is_open()) {
          ui.setElement(MessageTypes::warning, "ERROR", "Failed to open hash.txt");  
          return;
      }

      hashes << hash << " " << filename.string() << "\n";
  } 
  else {
      if (hash == "Empty file.") {
          ui.setElement(MessageTypes::empty, hash, filename.string() );
          emptyFileCount++;
      }
      else 
          ui.setElement(MessageTypes::normal, hash, filename.string());
       
  }
      
}

// Get calculated hashes from hash.txt to memory.
std::unordered_map<std::string, std::string>  loadHash(){
  
    std::unordered_map<std::string, std::string> hashmap{};

    std::ifstream hashesFile("hash.txt");

    if (!hashesFile.is_open())  {
        ui.setElement(MessageTypes::warning, "ERROR", "Hash file not found!");
        return {};
     }
  
    std::string tempHash, tempFilename, line; 
   
   // Split the line into hash and filename.  
      while(std::getline(hashesFile, line))   {
        std::istringstream ss(line);
        ss >> tempHash;
        std::getline( ss >> std::ws, tempFilename);
        hashmap[tempHash] = tempFilename;
      }

      return hashmap;
}

void compare(const fs::path& filename, const std::unordered_map<std::string, std::string>& hashmap ) {

  const auto hash = getHash(filename); 
    
  if (hash == "Empty file.")
      return;

  auto it = hashmap.find(hash);
    
  if (it == hashmap.end()) {
      invalidHashCount++;
      ui.setElement(MessageTypes::invalid, hash, filename.string()); 
  }
   
} 

// Get hash of directory.
void hashDirectory(const fs::path& path, bool saveHash = false) {
    
    std::vector<fs::path> paths;

    for (const auto& files : fs::recursive_directory_iterator(path))
        if (files.is_regular_file()) {
            fileCount++;
            paths.push_back(files);
        }
            
    std::for_each( std::execution::par, paths.begin(), paths.end(),
        [saveHash](const auto& file) { hashFile(file, saveHash); }
        );

    ui.infos("Number of files: " + std::to_string(fileCount), "Empty files: " + std::to_string(emptyFileCount));

}
 
//  Compare hash of directory with parallel - experimental - 
void compDirectory(const fs::path& path) {
      
    const auto hashmap = loadHash();
    std::vector<fs::path> paths;

    for (const auto& files : fs::recursive_directory_iterator(path))
        if (files.is_regular_file())
            paths.push_back(files);
   
    std::for_each( std::execution::par, paths.begin(), paths.end(),
        [hashmap](const auto& file) { compare( file, hashmap ); }
    );

}

int main(int argc, const char *argv[]) {
    
  try {

      if (argc <= 1 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            ui.usage();
            return 0;
      }
        
   // List a hash file or directory
      if (argc == 2) {
          fs::path file = argv[1];
          if (fs::is_regular_file(file))
              hashFile(file);
          else
              hashDirectory(file, false);
      }

      // Save hash. 
      else if (argc == 3 && (std::string(argv[1]) == "--save" || std::string(argv[1]) == "-s")) {
          fs::path file = argv[2];

          if (fs::is_regular_file(file)) {
              hashFile(file, false);
              ui.setElement(MessageTypes::success, "Success", "Hashes saved to hash.txt.");
          }

          else if (fs::is_directory(file)) {
              auto time = measure("Elapsed time", [&file]() { hashDirectory(file, true); });
              ui.setElement(MessageTypes::success, "Success", "Hashes saved to hash.txt in " + std::to_string(time) + " ms." );
          }
      }

      // Compare hash of directory
      else if (argc == 3 && (std::string(argv[1]) == "--compare" || std::string(argv[1]) == "-c")) {
          fs::path file = argv[2];
          auto time = measure("Elapsed time", [&file]() { compDirectory(file); });
          if (invalidHashCount == 0)
              ui.setElement(MessageTypes::success, "Success", "All files compared in " + std::to_string(time) + " ms." );
      }

      else 
          std::cout << "Command not found! \n";
       
    
} catch (const std::exception &e) { 
    ui.setElement(MessageTypes::warning, "Warning!", e.what());
  }

    if (invalidHashCount >= 1) 
       ui.setElement(MessageTypes::warning, "Number of invalid hashes", std::to_string(invalidHashCount));

  return 0;
}
