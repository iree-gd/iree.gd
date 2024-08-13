@icon("res://addons/iree-gd/logo.svg")
extends IREERunner
class_name IREEModule_google_movenet_tfLite_singlepose_lightning_tflite_int8

func _load_module() -> IREEModule:
	match OS.get_name():
		"Windows", "Linux", "FreeBSD", "NetBSD", "OpenBSD", "BSD":
			return IREEModule.new().load("res://addons/iree-zoo/google_movenet_tfLite_singlepose_lightning_tflite_int8/iree.vulkan-spirv.vmfb")
		"macOS", "iOS":
			return IREEModule.new().load("res://addons/iree-zoo/google_movenet_tfLite_singlepose_lightning_tflite_int8/iree.metal-spirv.vmfb")
		"Android":
			return IREEModule.new().load("res://addons/iree-zoo/google_movenet_tfLite_singlepose_lightning_tflite_int8/iree.llvm-cpu.vmfb")
		_:
			assert(false, "Unsupported platform.")
	return null

## INPUTS
## serving_default_input:0: <class 'numpy.uint8'> [  1 192 192   3]
## ---
## OUTPUTS
## StatefulPartitionedCall:0: <class 'numpy.float32'> [ 1  1 17  3]
func main(inputs: Array[IREETensor]) -> IREEResult:
	return run("module.main", inputs)
