# esrgan-tf2 bytecodes
This directory is for storing [esrgan-tf2](https://tfhub.dev/captain-pool/lite-model/esrgan-tf2/1) tflite model.

> **Note**
> Remember to setup the [tools](../../../tools) first before generating the bytecodes.

Run these commands to generate the bytecodes:
```sh
cd ./sample/bytecodes/esrgan # Enter this directory.
source ../../../tools/tflite/venv/bin/activate # Activate the virtualenv.
python ../../../tools/tflite/main.py https://tfhub.dev/captain-pool/lite-model/esrgan-tf2/1 esrgan
```
