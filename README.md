# VerifySN
   
 It is a tool for verifying files very quickly. You can save hashes to a file and compare them.
 
````
Usage: program.exe [options] filepath.
      -save   : save hashes to hash.txt.
      -compare: compare hash.

Example -> verifysn.exe -save "C:\gcc"
           verifysn.exe -compare "C:\gcc"
or you can directly list
            verifysn.exe "C:\gcc"
````

**How it works?**

  -> So easy. Get bytes from the files and calculate the hash. That's all. 
  
****"Not guaranteed for highly secure tasks."**** 

# Results
  Test on 12.2 GB file. 
  
  **XXH64 -> 15 seconds**
  
![xxhash](https://github.com/user-attachments/assets/faf745f3-242d-492f-964f-3c722c899eb1)

  **VerifySN -> 0.016 seconds**
  
![verify](https://github.com/user-attachments/assets/91cd8975-c2c4-47b2-b6d4-c08924df56c6)

## **Installation**

### Cmake (Default)
````
git clone https://github.com/d06i/verifySN.git
cd verifysn
cmake CMakeLists.txt
cmake --build .
````
### Cmake for MSVC:
````
git clone https://github.com/d06i/verifySN.git
cd verifysn
cmake CMakeLists.txt -G "Visual Studio 17 2022"
cmake --build . or open the .sln
````
### Compile with Clang \ G++ : 
````
clang -O3 verifysn.cpp -o verifysn.exe
````
or
````
g++ -O3 -s verifysn.cpp -o verifysn.exe
````

## Issues
  * It works slowly with small and numerous files (such as more than 10,000 files). -> Async might be applicable.

## Credits
  https://github.com/ztanml/fast-hash

  
