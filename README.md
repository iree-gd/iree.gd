# IREE.gd

> Warning: This project is still under development!

[IREE](https://github.com/openxla/iree) runtime in Godot through GDExtension, a mission to run machine learning model (e.g. Tensorflow lite) natively in Godot.

## Progress
What I have done:
- [*] vmvx backend (synchronous call).
- [*] Resource for loading `.vmfb` bytecodes.
- [*] Data conversion from numeric array (e.g. `PackedByteArray`), `Vector*` and `Color` to IREE's tensor data type (`iree_hal_buffer_view_t`).
- [*] Data conversion from IREE's tensor data type to array.
- [*] Calling functions in bytecode and retrieving the output `call_vmfb`.

Yet to be implemented:
- [] Data conversion from Godot's Image to `iree_hal_buffer_view_t`.
- [] Data conversion from `iree_hal_buffer_view_t` to Godot's Image.
- [] Nested list of `IREEBufferView`.
- [] Resource loader to auto import `.vmfb` bytecodes.

Things not going to be included in this version:
- [] Expose `.vmfb` bytecodes as classes instead of resource for better method hinting.
- [] Asynchronous call
- [] CUDA backend
- [] Vulkan backend

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