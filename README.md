# IREE.gd

![IREE.gd logo](./graphics/logo.svg)

[IREE](https://github.com/iree-org/iree) runtime in Godot through GDExtension, a mission to run a machine learning model (e.g. Tensorflow lite) natively in Godot.

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
git clone https://github.com/iree-gd/iree.gd.git # Clone this repository.
cd iree.gd
git submodule update --init
git submodule init ./thirdparty/iree/third_party/*
git submodule deinit ./thirdparty/iree/third_party/llvm-project # Deinitialize `llvm-project` since we don't want to compile the compiler.
git submodule update --recursive
mkdir build
cd build
cmake ..
cmake --build .
```

If you would like to compile LLVM from the source, you'll need to set the `IREE_BUILD_BUNDLED_LLVM` CMake option to `ON` when generating build files with CMake and also initialize the `llvm-project` submodule under the `thirdparty/iree/third_party/llvm-project`.

After compilation, the library will be outputted directly in the sample (`sample/addons/iree-gd/`) directory.
