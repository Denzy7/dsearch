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
depends: qt5, cmake

```bash
cmake -S . -B build
cmake --build build
cmake --install build --prefix out
```
