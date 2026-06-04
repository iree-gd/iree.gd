import torch
import torchvision.models as models
import iree.turbine.aot as aot


def main():
    # 1. Load your pre-trained PyTorch model
    # Make sure to set it to evaluation mode to disable dropout/batchnorm training behavior
    print("Loading pre-trained model...")
    model = models.resnet18(weights=models.ResNet18_Weights.DEFAULT)
    model.eval()

    # 2. Create example inputs
    # This defines the static shape and dtype the compiled model will accept
    example_input = torch.randn(1, 3, 224, 224)

    # 3. Export the model using Turbine's AOT API
    print("Exporting model to Turbine representation...")
    export_output = aot.export(model, example_input)

    # 4. Save intermediate MLIR (Optional, good for debugging)
    export_output.save_mlir("resnet18.mlir")
    print("Saved MLIR representation to resnet18.mlir")

    # 5. Compile to an IREE deployable binary (.vmfb)
    # By default, this compiles for the host CPU.
    print("Compiling to IREE VM FlatBuffer...")
    export_output.compile(
        save_to="resnet18.vulkan.vmfb",
        target_backends=["vulkan-spirv"],
    )
    export_output.compile(
        save_to="resnet18.metal.vmfb",
        target_backends=["metal-spirv"],
    )
    print("Successfully compile resnet18")


if __name__ == "__main__":
    main()
