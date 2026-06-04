extends Node2D

@export var vulkan_module : IREEModule
@export var metal_module : IREEModule
@export var texture: Texture2D

func select_model() -> IREEModule:
	match OS.get_name():
		"Windows", "Android", "Linux", "OpenBSD", "FreeBSD", "NetBSD", "BSD": return vulkan_module
		"macOS", "iOS": return metal_module
	assert(false)
	return null

func rectify_image(p_image: Image) -> Image:
	# Because the input image must be a square (224 x 224).
	# It scales down the image while keeping the aspect ratio.
	
	var image := p_image.duplicate(true) as Image
	image.convert(Image.FORMAT_RGB8)
	var resolution := image.get_size()
	var aspect_ratio := float(resolution.y) / resolution.x
	var is_landscape := resolution.x > resolution.y
	if is_landscape: image.resize(224, int(224.0 * aspect_ratio))
	else: image.resize(int(224.0 / aspect_ratio), 224)
	
	var padded_image := Image.create(224, 224, false, Image.FORMAT_RGB8)
	padded_image.blit_rect(image, Rect2i(Vector2i.ZERO, image.get_size()), Vector2i.ZERO)
	return padded_image

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	var image := texture.get_image()
	var processed_image := rectify_image(image)
	var processed_image_byte_data := processed_image.get_data()
	var processed_image_float32_data : PackedFloat32Array
	
	for byte in processed_image_byte_data: 
		processed_image_float32_data.append(float(byte) / 255)
	
	var input_tensor := IREETensor.from_float32s(
		processed_image_float32_data,
		[1, 3, 224, 224]
	)
	
	var result := select_model().call_module("module.main", [input_tensor])
	var output_tensor := result.front() as IREETensor
	var output_data := output_tensor.get_data().to_float32_array()
	var dimension := output_tensor.get_dimension()
	
	print(output_data)
	print(output_data.size())
	print(dimension)
	
