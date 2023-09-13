# TFlite Vulkan VMFB Generate
The `main.py` python script contains easy and quick compilation for tflite models to vulkan-formatted `.vmfb` file, together with its intermediate representation (`.mlir`) and dump.

You'll need to setup a virtual environment with these [prerequisites](https://openxla.github.io/iree/guides/ml-frameworks/tflite/#prerequisites) installed.

```
usage: python main.py {TFLITE_URL} {NAME}
```
