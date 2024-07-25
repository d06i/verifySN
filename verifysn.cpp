#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm> 
#include <thread> 
#include <functional>

namespace fs = std::filesystem;

// You can change file part numbers. But hash also changes.
constexpr int part_number = 32;
uint64_t invalidHash = 0;

// Measure elapsed time in function
void measure(std::string funcName, std::function<void()> func) {
  auto start = std::chrono::high_resolution_clock::now();
  func();
  auto end  = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  std::cout << funcName << " -> " << time << " ms.\n";
}

///////////////////// Fast-Hash algorithm -> https://github.com/ztanml/fast-hash /////////////////////////////
uint64_t mix(uint64_t h) {
  h ^= h >> 23;
  h *= 0x2127599bf4325c37ULL;
  h ^= h >> 47;
  return h;
}

uint64_t fasthash64(const void *buf, size_t len, uint64_t seed) {
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
/////////////////////////////////////////////////////////////////////////

//  Get hash of file.
std::string getHash(fs::path filename) {

  std::ifstream file(filename, std::ios::binary | std::ios::in);
  std::vector<uint8_t> file_hash; 
  file_hash.reserve(part_number); 

  if (!file.is_open()) {
    std::cout << "File is not found!\n";
    return 0;
  }

  uintmax_t filesize = fs::file_size(filename);
  uintmax_t parts = filesize / part_number;

 // If file is empty just send zero
  if ( filesize == 0 )
    return "0000000000000000";

 // Get one char on file parts.
  char buff[1024];
  for (int i=0; i < part_number; i++){
    file.seekg( i * parts );
    file.read(buff, 1024);
    file_hash.emplace_back( buff[0] );
  }

  uint64_t hash = fasthash64(file_hash.data(), file_hash.size(), filesize);
  file.close();

  // Convert hash to hex
  std::stringstream stream;
  stream << std::setw(16) << std::setfill('0') << std::hex << std::uppercase
         << hash;
  return stream.str();
}

// Get hash and add to file
void hashFile(fs::path filename, bool save = false) {

  auto hash = getHash(filename);

  if (save) 
  {
   std::ofstream hashes("hash.txt", std::ios::app);

    if (!hashes.is_open())
    {
      std::cout << "[-] Failed to create hash.txt\n";
      return; 
    } 

      hashes << hash << " " << filename.string() << "\n";
  } 

  else 
    std::cout << hash << " " << filename.string() << "\n";
 
} 

// Get calculated hashes from hash.txt to memory.
void loadHash(std::vector<std::string> &hashes, std::vector<std::string> &filenames){ 
  
   std::ifstream hashesFile("hash.txt");

    if (!hashesFile.is_open()) 
     {
        std::cerr << "Hash file not found!\n";
        return;
     }
  
    std::string tempHash, tempFilename, line; 
   
   // Split the line into hash and filename.  
      while(std::getline(hashesFile, line))
      {
        std::istringstream ss(line);
        ss >> tempHash;
        std::getline( ss >> std::ws, tempFilename);
        hashes.push_back(tempHash);
        filenames.push_back(tempFilename);   
      }

}

// Compare hash in the file with the calculated hash.
void compare(fs::path filename, std::vector<std::string> &hashes) { 

    auto hash = getHash(filename);
    auto found = std::find( hashes.begin(), hashes.end(), hash);

    if ( !(found != hashes.end()) )  
        {
       invalidHash++;
       std::cout << "[-] Invalid\tHASH: " << hash << "\tFilename: " << filename.string() << "\n";
        }
}

// Get hash of directory.
void hashDirectory(fs::path path, bool saveHash = false) {

  for ( auto &files : fs::recursive_directory_iterator(path) ) 
    {
      if (files.is_regular_file())
        hashFile(files, saveHash);
    }
  }

// Compare hash of directory.
void compDirectory(fs::path path){

  std::vector<std::string> hash, files;
  loadHash(hash, files);

  for ( auto& files : fs::recursive_directory_iterator(path) )
      if(files.is_regular_file())
        compare(files, hash);

}
 
void usage() { 
  std::cout << "Usage: program.exe [options] filepath.\n"
                "\t-save : save hashes to hash.txt.\n"
                "\t-compare: compare hash. \n";
  }


int main(int argc, const char *argv[]) {
  
  const std::string info =
      "################################################\n"
      "#   Fast File Verification Tool by d06i        #\n"
      "#                                              #\n"
      "#  It's your life, live it however you wanna   #\n" 
      "################################################\n";

  std::cout << info << std::endl;

  try {
   
   // List a hash file or directory
    if (argc == 2) 
    {
      fs::path file = argv[1];
      if (fs::is_regular_file(file))
          hashFile(file);
      else
          hashDirectory(file);
    }

    // Save hash. 
    else if (argc == 3 && std::string(argv[1]) == "-save")
    {
      fs::path file = argv[2];

      if (fs::is_regular_file(file))
      {
        hashFile(file, false); 
        std::cout << "[+] Hashes saved to hash.txt.";
      }

      else if (fs::is_directory(file))
       {
      //  hashDirectory(file, true);
        measure("Elapsed time", [&file](){ hashDirectory(file, true); });
        std::cout << "[+] Hashes saved to hash.txt.";
      }
    }
    
    // Compare hash of directory
    else if (argc == 3 && std::string(argv[1]) == "-compare") 
    {
      fs::path file = argv[2]; 
      measure("Elapsed time", [&file](){ compDirectory(file); });
      if(invalidHash == 0 )
        std::cout << "All files OK!";
    }

    // get help info
    else if (argc <= 1) 
         usage();
    
} catch (const std::exception &e) {
    std::cerr << e.what() << "\n[!] Check file or directory name!";
  }

 if( invalidHash >= 1) 
  std::cout << "\nInvalid hash count is " << invalidHash;

  return 0;
}
