# IREE.gd

> Warning: This project is still under development!

[IREE](https://github.com/openxla/iree) runtime in Godot through GDExtension, a mission to run machine learning model (e.g. Tensorflow lite) natively in Godot.

## Progress
What I have done:
- [x] vmvx backend (synchronous call).
- [x] Resource for loading `.vmfb` bytecodes.
- [x] Data conversion from numeric array (e.g. `PackedByteArray`), `Vector*` and `Color` to IREE's tensor data type (`iree_hal_buffer_view_t`).
- [x] Data conversion from IREE's tensor data type to array.
- [x] Calling functions in bytecode and retrieving the output `call_vmfb`.
- [x] Data conversion from Godot's Image to `iree_hal_buffer_view_t`.
- [x] Data conversion from `iree_hal_buffer_view_t` to Godot's Image.
- [x] Resource loader to import `.vmfb` bytecodes.

Things not going to be included in this version:
- [ ] Nested list of `IREEBufferView`.
- [ ] Expose `.vmfb` bytecodes as classes instead of resource for better method hinting.
- [ ] Asynchronous call
- [ ] CUDA backend
- [ ] Vulkan backend

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
