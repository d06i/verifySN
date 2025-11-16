<p align="center">
<img width="781" height="620" alt="ss" src="https://github.com/user-attachments/assets/b9466497-aa12-47bd-a017-201ec59fbfb5" />
</p>

# VerifySN
   
 A tool for quickly verifying files. You can hash and compare dozens or even hundreds of gigabytes in seconds.

  
````
Usage: program.exe [options] filepath.
      --save    or -s : save hashes to hash.txt
      --compare or -c : compare hash

Example -> verifysn.exe --save "C:\gcc"
           verifysn.exe --compare "C:\gcc"
or you can directly list
            verifysn.exe "C:\gcc"
````

>[!WARNING]
> Not guaranteed for highly secure tasks.
 
# Demo
<p align="center">
<img alt="demo" src="https://github.com/user-attachments/assets/7f795801-e76f-4942-aa75-6aa93a2645c3" />
</p>
   
## **Installation**

### Cmake (Default)
~~~bash
git clone https://github.com/d06i/verifySN.git
cd verifySN
cmake -DCMAKE_BUILD_TYPE=Release CMakeLists.txt
cmake --build . --config Release
~~~

## Credits
 -  https://github.com/ekpyron/xxhashct
 -  https://github.com/ArthurSonzogni/FTXUI 
