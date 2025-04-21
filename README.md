# File-Reporter
A really really bad file reporter written for a school project
This project implements a File Deduplicator which reports the true duplicates of a file along with its hard links and symbolic links. Using a MD5 Hash

## Implementation
We have three seperate hashmaps  

Hard Links -> <inode_number,          list_of_paths>

Soft Links -> <target_inode_number,   list_of_paths>

Duplicates -> <md5_hash,              list_of_paths>

We use [nftw()](https://pubs.opengroup.org/onlinepubs/9799919799/functions/nftw.html) to traverse through the file directory computing their md5_hash and placing them into all three hashes. Then we print the output in order.

```
  ./detect_dups <file_name> 
```

Output Example 
```
File 1
        MD5 Hash: 636fad1d8e7287e49011a53c7ca5b509
                Hard Link (2): 7545
                        Paths:  ./dup5
                                ./dup
                        Soft Link 1(2): 7597
                                Paths:  ./dup3
                                        ./dup2
                        Soft Link 2(1): 7598
                                Paths:  ./dup8
                Hard Link (2): 7602
                        Paths:  ./dup10
                                ./dup15
                        Soft Link 1(2): 7600
                                Paths:  ./dup25
                                        ./dup21
                        Soft Link 2(1): 7599
                                Paths:  ./dup20
```

Under the MD5 consists of different files with their respective Hard Links. These files are true duplicates. 
NOTE: Soft Links can have Hard links too. When I first started working on this project that's what confused me most. 

 ## Why this is terrible

 - I'm using arrays instead of linked lists to allocate memory for file paths this is just bad since I'm allocating extra memory which never gets used if you try to run this program on a insanely huge batch of files the OS just decides to kill it cause it's taking too much memory.
 - Alot of this code could be compressed to make it more readable and modular 

Credits to Troy D. Hanson, Arthur O'Dwyer for the UTHash library
