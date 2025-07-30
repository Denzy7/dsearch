# dsearch
cross platform search tool inspired by [voidtools Everything](https://voidtools.com/support/everything)

# key features
- file watching
- fast search
- open, open location and properties

# coming soon
- drag and drop
- hiding deleted files
- persistent database

# building
## depends
- qt5 or qt6
- cmake

## cmake configure
```
cmake -S . -B build [extra-options]
```
### extra options
#### to force a specific qt version 
to force a specific qt version (eg. Qt6) add:  
```
-DQT_DIR=/usr/lib/cmake/Qt6
```
this directory should contain a file Qt6Config.cmake (or the version of qt you want)  
if -DQT_DIR is not provided or is invalid, it defaults to qt5 if installed  

## build and install
```bash
cmake --build build
cmake --install build --prefix out
```
