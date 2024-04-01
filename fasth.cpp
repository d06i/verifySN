#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <future>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace fs = std::filesystem;

constexpr int part_number = 100;
std::mutex mtx;

inline uint64_t mix(uint64_t h) {
  h ^= h >> 23;
  h *= 0x2127599bf4325c37ULL;
  h ^= h >> 47;
  return h;
}

inline uint64_t fasthash64(const void *buf, size_t len, uint64_t seed) {
  const uint64_t m = 0x880355f21e6d1965ULL;
  const uint64_t *pos = (const uint64_t *)buf;
  const uint64_t *end = pos + (len / 8);
  const unsigned char *pos2;
  uint64_t h = seed ^ (len * m);
  uint64_t v;

  while (pos != end) {
    v = *pos++;
    h ^= mix(v);
    h *= m;
  }

  pos2 = (const unsigned char *)pos;
  v = 0;

  switch (len & 7) {
    case 7:
      v ^= (uint64_t)pos2[6] << 48;
    case 6:
      v ^= (uint64_t)pos2[5] << 40;
    case 5:
      v ^= (uint64_t)pos2[4] << 32;
    case 4:
      v ^= (uint64_t)pos2[3] << 24;
    case 3:
      v ^= (uint64_t)pos2[2] << 16;
    case 2:
      v ^= (uint64_t)pos2[1] << 8;
    case 1:
      v ^= (uint64_t)pos2[0];
      h ^= mix(v);
      h *= m;
  }

  return mix(h);
}


std::vector<uint64_t> calculatHashes(fs::path filename) {
  
  std::ifstream file(filename, std::ios::binary);
  std::vector<uint8_t> f; 
  f.reserve(part_number); 
  std::vector<uint64_t> hashes;

  if (!file.is_open()) {
    std::cout << "File is not found!\n";
    return;
  }

  uintmax_t filesize = fs::file_size(filename);
  uintmax_t parts = filesize / part_number;
 
 for (int i{0}; i <= part_number; i++) {
    file.seekg(i * parts); 
    f.emplace_back(file.get());
  }

  uint64_t hash = fasthash64(f.data(), f.size(), filesize);
  hashes.push_back(hash);
  return hashes;
}

std::string getHashV2(){
  std::vector<uint64_t> hashVector;
}

std::string getHash(fs::path filename) {

  std::ifstream file(filename, std::ios::binary);
  std::vector<uint8_t> f; 
  f.reserve(part_number);
  char character;

  if (!file.is_open()) {
    std::cout << "File is not found!\n";
    return 0;
  }

  uintmax_t filesize = fs::file_size(filename);
  uintmax_t parts = filesize / part_number;
 
 for (int i{0}; i <  part_number; i++) {
    file.seekg(i * parts); 
    f.push_back(file.get());
  }

  uint64_t hash = fasthash64(f.data(), f.size(), filesize);
  file.close();

  std::stringstream stream;
  stream << std::setw(16) << std::setfill('0') << std::hex << std::uppercase
         << hash;
  return stream.str();
}

void hashFile(fs::path filename, bool save = false) {
  auto hash = getHash(filename);
  
  if (save) {
   static std::ofstream hashes("hash.txt", std::ios::app);

   if (!hashes.is_open()){
    std::cout << "hash.txt create is failed!";
    return;  } 
    hashes << hash << " " << filename.string() << "\n";
  } else {
    std::cout << hash << " " << filename.string() << "\n";
  } 
} 

void loadHash(std::vector<std::string>& hashes, std::vector<std::string>& filenames){ 
   std::ifstream hashesFile("hash.txt");
    if (!hashesFile.is_open()) {
        std::cerr << "Hash file not found!";
    } 
    std::string tempHash, tempFilename, line; 
      while(std::getline(hashesFile, line)){
        std::istringstream ss(line);
        ss >> tempHash;
        std::getline( ss >> std::ws, tempFilename);
        hashes.push_back(tempHash);
        filenames.push_back(tempFilename);   
    }
}

void compareV4(fs::path filename, std::vector<std::string>& hashes) { 
    auto t = getHash(filename);
    auto found = std::find( hashes.begin(), hashes.end(), t);
    if (!(found != hashes.end())) 
       std::cout << "[-] Invalid\tHASH: " << t << "\tFilename: " << filename.string() << "\n";
}
 
int iteration = 0;
int linesCount = 0;

void allFiles(fs::path path, bool saveHashes = false) {
  for (auto files : fs::recursive_directory_iterator(path)) {
    if (files.is_regular_file())
      hashFile(files, saveHashes);
    }
  }

void allFilesV2(fs::path path){

  std::ofstream hashes("hash.txt");

  if(!hashes.is_open()) throw std::runtime_error("File not created!\n");

   for (auto files : fs::recursive_directory_iterator(path)) {
    if (files.is_regular_file()){
      auto hash = getHash(files);
      hashes << hash <<  " " << files.path().string() << "\n";
      }
    }
}

void compDir(fs::path path){
  std::vector<std::string> hash, files;
  loadHash(hash, files);
  for (auto& files : fs::recursive_directory_iterator(path)){
    if(files.is_regular_file())
    compareV4(files, hash);
    iteration++;
  } }

void compDirV2(fs::path path) {
    std::vector<std::string> hash, files;
    loadHash(hash, files);

    std::vector<std::future<void>> futures;
    
    for (auto& files : fs::recursive_directory_iterator(path)) {
        if (files.is_regular_file())
         futures.emplace_back(std::async(std::launch::async, compareV4, files, std::ref(hash))); 
    }
    for (auto& future : futures)
        future.wait();
}  

void usage() { 
  std::cout << "Usage: program.exe [options] file.\n"
                "\t-save : save hashes to hash.txt.\n"
                "\t-compare: compare hash. \n";
  }

int main(int argc, const char *argv[]) {
  const std::string x =
      "################################################\n"
      "#   Fast File Verification Tool by D06i        #\n"
      "#                                              #\n"
      "#  It's your life, live it however you wanna   #\n" 
      "################################################\n";

  std::cout << x << std::endl;

  try {
      auto start = std::chrono::high_resolution_clock::now();

    if (argc == 2) {
      fs::path file = argv[1];
      if (fs::is_regular_file(file)) {
        hashFile(file);
      } else {
        allFiles(file);
      }
    }

    else if (argc == 3 && std::string(argv[1]) == "-save") {
      fs::path file = argv[2];
      if (fs::is_regular_file(file)){
        hashFile(file, true); 
        std::cout << "[+] Hashes saved to hash.txt.";}
      else if (fs::is_directory(file)) {
        allFilesV2(file);
        std::cout << "[+] Hashes saved to hash.txt.";
      }
    }

    else if (argc == 3 && std::string(argv[1]) == "-compare") {
      fs::path file = argv[2];
        compDirV2(file);
    }

    else if (argc <= 1) {
      usage();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << "\nElapsed time: " << std::dec << time << " ms.";
} catch (const std::exception &e) {
    std::cerr << e.what() << "\n[!] Check file or directory name!";
  }

  std::cout << "\n\nCompared times: " << iteration << "\nLines Count: " << linesCount;
    
  return 0;
}