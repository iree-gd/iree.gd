import argparse
import tensorflow as tf
import iree.compiler.tflite as iree_tflite_compile
import urllib.request
import os

# usage: main.py {TFLITE_MODEL_URL} {MODEL_NAME}
# This program downloads tflite model and generate bytecodes.

def main():
    parser = argparse.ArgumentParser(
            prog="tflite-vulkan-vmfb-generate",
            description="Generate vmfb in vulkan format from tflite model."
            )
    parser.add_argument("tflite_model_url", help="URL to tflite model to be downloaded.")
    parser.add_argument("model_name", help="Name of the model.")
    args = parser.parse_args()
    tflite_url = args.tflite_model_url
    model_name = args.model_name
    output_dir = os.getcwd()
    output_tflite_path = os.path.join(output_dir, f"{model_name}.tflite")
    output_tflite_ir_path = os.path.join(output_dir, f"{model_name}-tflite.mlir")
    output_tosa_ire_path = os.path.join(output_dir, f"{model_name}-tosa.mlir")
    output_vmfb_path = os.path.join(output_dir, f"{model_name}.vmfb")
    output_detail_dump_path = os.path.join(output_dir, f"{model_name}-detail-dump.txt")

    # Get model.
    print("Downloading tflite model.")
    urllib.request.urlretrieve(tflite_url, output_tflite_path)

    # Compile.
    print("Compiling tflite model.")
    iree_tflite_compile.compile_file(
        output_tflite_path,
        input_type="tosa",
        output_file=output_vmfb_path,
        save_temp_tfl_input=output_tflite_ir_path,
        save_temp_iree_input=output_tosa_ire_path,
        target_backends=["vulkan"],
        import_only=False
    )

    # Dump details.
    print("Generating detail dump.")
    interpreter = tf.lite.Interpreter(output_tflite_path)
    with open(output_detail_dump_path, "w") as file:
        file.writelines([
                "Input details: \n\n",
                str(interpreter.get_input_details()),
                "\n\nOutput details: \n\n",
                str(interpreter.get_output_details()),
                "\n\nSignature details: \n\n",
                str(interpreter.get_signature_list()),
                "\n\nTensor details: \n\n",
                str(interpreter.get_tensor_details()),
            ])
    return 0

if __name__ == "__main__": exit(main())
