# `iree.gd`

![iree.gd logo](./graphics/logo.svg)

[IREE](https://github.com/openxla/iree) runtime in Godot through GDExtension, a mission to run machine learning model (e.g. Tensorflow lite) natively in Godot.

Authored by [Richie Kho](https://github.com/RechieKho/iree.gd), published by [V-Sekai](https://github.com/V-Sekai/IREE.gd).

## Supported Platforms

| Platform                         | HAL Backend used |
| -------------------------------- | ---------------- |
| Apple products (macOS, IOS)      | `metal`          |
| Desktops (Windows, Linux, \*BSD) | `vulkan`         |
| The rest (Android)               | `vmvx`           |

## Overview

This GDExtension provides:

- `IREETensor` - Hold data to be fed into or output by the model.
- `IREEModule` - Load model and run it.

### Preparation

You'll need to compile your models following this [guide](https://openxla.github.io/iree/guides/).
Make sure the backends is supported by your compiled models, based on [supported platforms](#supported-platforms).

You can use `iree-dump-module` from IREE's tools to inspect the byte code.
Here is an important section of the dump of esrgan:

```
Exported Functions:
  [  0] main(!vm.ref<?>) -> (!vm.ref<?>)
        iree.abi.declaration: sync func @main(%input0: tensor<1x50x50x3xf32> {ml_program.identifier = "input_0"}) -> (%output0: tensor<1x200x200x3xf32> {ml_program.identifier = "Identity"})
  [  1] __init() -> ()
```

Here we can know that:

- The function name is `"main"`.
- The function takes one `1x50x50x3` float 32 `IREETensor` (`input_0`) as input.
- The function takes one `1x200x200x3` float 32 `IREETensor` (`Identity`) as output.

### Using `iree.gd`

After having your `.vmfb` bytecode ready, you could start using `iree.gd`.

There are 4 steps:

1. Load model with `IREEModule.load`.
2. Prepare input by feeding data into `IREETensor` through `IREETensor.from_*` variant or `IREETensor.capture_*` variant.
3. Send `IREETensor`s into loaded `IREEModule`.
4. Interpreting the output `IREETensor`s from `IREEModule`.

```swift
var module := IREEModule.load("res://model.vmfb")
var input := IREETensor.from_bytes(image.get_data(), [1, 50, 50, 3]) # Remember to consider the input type.
# var outputs := module.bind("module.main", [input]).call_module() # Synchronous execution.
var outputs := await module.bind("module.main", [input]).call_module_async().completed # Asynchronous execution.
for output in outputs:
    pass # Do something with the `output`.
```

> [!NOTE]
>
> You need to prefix `module.` in front of the function you want to call when passing into `IREEModule`.
> In this case, the function is `main` so you pass `module.main` as function name.

## Sample project

The sample project is in `sample` directory, it demonstrate running esrgan using `iree.gd`.

## Build from source

Run these commands:

```sh
git clone https://github.com/V-Sekai/iree.gd.git # clone this repo
cd iree.gd
git submodule init thirdparty # initialize all the thirdparty
git submodule deinit thirdparty/iree/third_party/llvm-project # Deinitialize llvm, we are not compiling the compiler.
git submodule update --recursive # Pull submodule content, this will take a while.
mkdir build
cd build
cmake ..
cmake --build . # Building the project, this will take a while, add `-j` flag to make it faster.
```

If you would like to compile LLVM from source, you'll need to set `IREE_BUILD_BUNDLED_LLVM` to `ON` when generating build files with cmake.

After compilation, the library will be in `build/lib` directory.
It will also install the library into the sample.
