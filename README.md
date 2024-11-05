# IREE.gd

![IREE.gd logo](./graphics/logo.svg)

[IREE](https://github.com/iree-org/iree) runtime in Godot through GDExtension, a mission to run machine learning model (e.g. Tensorflow lite) natively in Godot.


Authored by [Richie Kho](https://github.com/RechieKho) and its contributors.

## Supported Platforms

| Platform                                  | HAL Backend used |
| ----------------------------------------- | ---------------- |
| Apple products (macOS, IOS)               | `metal`          |
| Desktops (Windows, Linux, \*BSD, Android) | `vulkan`         |
| The rest                                  | `vmvx`           |

## Documentation and samples

The documentation is hosted using [Github page](https://iree-gd.github.io/iree.gd.docs/).
The sample project is in `sample` directory.

## Build from source

We'll use Git and CMake to build this project.

```sh
git clone https://github.com/iree-gd/iree.gd.git # clone this repo
cd iree.gd
git submodule init thirdparty # initialize all the thirdparty
git submodule deinit thirdparty/iree/third_party/llvm-project # Deinitialize LLVM, as we are not compiling the compiler.
git submodule update --recursive # Pull submodule content, this will take a while.
mkdir build
cd build
cmake ..
cmake --build .
```

If you would like to compile LLVM from source, you'll need to set `IREE_BUILD_BUNDLED_LLVM` CMake option to `ON` when generating build files with CMake. You also need to initialize `llvm-project` submodule under `thirdparty/iree/third_party/llvm-project`.

After compilation, the library will be in `build/lib` directory.
It will also install the library into the sample if `COPY_LIBS_TO_SAMPLE` CMake option is `ON` which is enabled by default.
