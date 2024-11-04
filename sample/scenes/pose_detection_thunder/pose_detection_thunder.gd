extends Node2D

@export var vulkan_module : IREEModule
@export var metal_module : IREEModule
@export var texture: Texture2D

var data := PackedFloat32Array() :
	set(p_value):
		if data == p_value: return
		data = p_value
		call_deferred("queue_redraw")

func select_model() -> IREEModule:
	match OS.get_name():
		"Windows", "Android", "Linux", "OpenBSD", "FreeBSD", "NetBSD", "BSD": return vulkan_module
		"macOS", "iOS": return metal_module
	assert(false)
	return null

func rectify_image(p_image: Image) -> Image:
	# Because the input image must be a square (256 x 256).
	# It scales down the image while keeping the aspect ratio.
	
	var image := p_image.duplicate(true) as Image
	image.convert(Image.FORMAT_RGB8)
	var resolution := image.get_size()
	var aspect_ratio := float(resolution.y) / resolution.x
	var is_landscape := resolution.x > resolution.y
	if is_landscape: image.resize(256, int(256.0 * aspect_ratio))
	else: image.resize(int(256.0 / aspect_ratio), 256)
	
	var padded_image := Image.create(256, 256, false, Image.FORMAT_RGB8)
	padded_image.blit_rect(image, Rect2i(Vector2i.ZERO, image.get_size()), Vector2i.ZERO)
	return padded_image

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	var image := texture.get_image()
	var processed_image := rectify_image(image)
	var processed_image_byte_data := processed_image.get_data()
	var processed_image_float32_data : PackedFloat32Array
	
	for byte in processed_image_byte_data: 
		processed_image_float32_data.append(float(byte))
	
	var input_tensor := IREETensor.from_float32s(
		processed_image_float32_data,
		[1, 256, 256, 3]
	)
	
	var result := select_model().call_module("module.main", [input_tensor])
	var output_tensor := result.front() as IREETensor
	var output_data := output_tensor.get_data().to_float32_array()
	var dimension := output_tensor.get_dimension()
	print(output_data)
	print(output_data.size())
	print(dimension)
	
	data = output_data
	# The first two channels of the last dimension represents the yx coordinates 
	# (normalized to image frame, i.e. range in [0.0, 1.0]) of the 17 keypoints 
	
	# (in the order of: 
	# [nose, left eye, right eye, left ear, right ear, left shoulder, right shoulder, 
	# left elbow, right elbow, left wrist, right wrist, left hip, right hip, left knee, 
	# right knee, left ankle, right ankle]).
	
	# The third channel of the last dimension represents the prediction confidence scores of each keypoint, also in the range [0.0, 1.0].


func draw_keypoint(index, color_base, resolution):
	var x_index = index * 3 + 1
	var y_index = index * 3
	var conf_index = index * 3 + 2
	var coordinate = Vector2(data[x_index], data[y_index]) * resolution
	var confidence = data[conf_index]
	var lightness = confidence
	var hue = Color.from_ok_hsl(color_base.h, color_base.s, lightness, 1.0)
	draw_circle(coordinate, 5.0, hue)


func _draw():
	if not texture:
		return
	draw_texture(texture, Vector2.ZERO)
	if data.size() != (17 * 3):
		return
	var width := texture.get_width()
	var height := texture.get_height()
	var resolution := Vector2(
		max(width, height),
		max(width, height)
	)
	var base_color = Color.RED
	for i in range(17):
		draw_keypoint(i, base_color, resolution)
