# VOXMO
A Utility for Bundling Kernel Modules for the Voxia OS
### How To Use
```bash 
    mkdir build && cd build
```
```bash
    cmake .. && make
```
```bash
    ./voxmo -o {output name}.voxmo {file1} {file2} ...
```

file you must included
1.  binary elf
2.  manifest.yml

### manifest example
```yml
name: test
version: 0.2
author: mohammad arfan

main: flui.elf

cap:
  - PCI
```