# `iree.gd`

> **Warning**
> This GDExtension requires Godot compiled with [this Pull Request](https://github.com/godotengine/godot/pull/80322).

![iree.gd logo](./graphics/logo.svg)

[IREE](https://github.com/openxla/iree) runtime in Godot through GDExtension, a mission to run machine learning model (e.g. Tensorflow lite) natively in Godot.

## Overview

> **Note**
> We are still working on examples, please wait patiently.

This GDExtension provides:
* `IREETensor` - Hold data to be fed into or output by the model. 
* `IREEModule` - Load model and run it.

### Preparation
Before using `iree.gd`, you'll do some preparation on the model (following this [guide](https://openxla.github.io/iree/guides/)).

Your `.vmfb` bytecode must be for vulkan, which is currently the only supported format.
You should also use `iree-opts` from `iree-dist` package [here](https://github.com/openxla/iree/releases)
to get the textual version of your `.mlir` bytecode compiled from the model chosen in order to 
inspect the input, output argument type and the function name (it is usually `"module.main"`) 
by finding the function signature. Failure of giving correct type will result errors.

Here is an example of a function signature in a dump of `iree-opts`:
```
...
func.func @main(%arg0: tensor<1x50x50x3xf32> {iree.identifier = "input_0"}) -> (tensor<1x200x200x3xf32> {iree.identifier = "Identity"}) {
...
```

Here we can know that: 
* The function name is `"module.main"`
* The function takes one `1x50x50x3` float 32 `IREETensor` (`input_0`) as input.
* The function takes one `1x200x200x3` float 32 `IREETensor` (`Identity`) as input.

### Using `iree.gd`
After having your `.vmfb` bytecode ready and the model input type determined, you could start using `iree.gd`.

There are 4 steps:
1. Load model with `IREEModule.load`.
2. Prepare input by feeding data into `IREETensor` through `IREETensor.from_*` variant or `IREETensor.capture_*` variant.
3. Send `IREETensor`s into loaded `IREEModule`.
4. Interpreting the output `IREETensors` from `IREEModule`.

```swift
var module := IREEModule.load("res://model.vmfb")
var input := IREETensor.from_bytes(image.get_data(), [1, 50, 50, 3]) # Remember to consider the input type.
var outputs := module.run_module("module.main", [input])
for output in outputs:
    pass # Do something with the `output`.
```

## Build from source
Run these commands: 
```sh
git clone https://github.com/RechieKho/iree.gd.git # clone this repo
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
If you have a sample godot project in `sample` directory, it will copy the library to `sample/extension/iree/` directory.
