# esrgan-tf2 bytecodes
This directory is for storing [esrgan-tf2](https://tfhub.dev/captain-pool/lite-model/esrgan-tf2/1) tflite model.

> **Note**
> Remember to setup the [tools](../../../tools) first before generating the bytecodes.

Run these commands to generate the bytecodes:
```sh
cd ./sample/bytecodes/esrgan # Enter this directory.
source ../../../tools/tflite-vulkan-vmfb-generate/venv/bin/activate # Activate the virtualenv.
python ../../../tools/tflite-vulkan-vmfb-generate/main.py ESRGAN_TFLITE_URL esrgan # replace `ESRGAN_TFLITE_URL` with a real url that downloads esrgan tflite model.
```
