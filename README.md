# IREE.gd

> Warning: This project is still under development!

IREE runtime in Godot through GDExtension.

## Build from source
```sh
git clone https://github.com/RechieKho/IREE.gd.git # clone this repo
cd IREE.gd
git submodule init thirdparty # initialize all the thirdparty
git submodule deinit thirdparty/iree/third_party/llvm-project # deinitialize llvm; we are not compiling the compiler
git submodule update --recursive # pull submodule content; this will take a while
mkdir build
cd build
cmake ..
cmake --build . # building the project; this will take a while; add `-j` flag to make it faster
```

With this, the gdextension library will be in `build/lib` directory.
If you have a sample godot project in `sample` directory, it will copy the library to `sample/extension/iree/` directory.