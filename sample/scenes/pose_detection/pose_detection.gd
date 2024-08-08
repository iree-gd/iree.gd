extends Node2D

@export var vulkan_module: IREEModule
@export var metal_module: IREEModule
@export var llvm_module: IREEModule
@export var texture: Texture2D

var data := PackedFloat32Array() :
	set(p_value):
		if data == p_value: return
		data = p_value
		queue_redraw()

func select_module() -> IREEModule:
	var module : IREEModule = null
	match OS.get_name():
		"Windows", "Linux", "FreeBSD", "NetBSD", "OpenBSD", "BSD":
			module = vulkan_module
		"macOS", "iOS":
			module = metal_module
		"Android":
			module = llvm_module
		_:
			assert(false, "Unsupported platform.")
	return module

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

func _ready():
	var image := texture.get_image()
	var processed_image := rectify_image(image)
	var input_tensor := IREETensor.from_bytes(
		processed_image.get_data(),
		[1, 256, 256, 3]
	)
	var module := select_module()
	var result := await module.call_module("module.main", [input_tensor]).completed as Array
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
	

func _draw():
	if not texture: return
	draw_texture(texture, Vector2.ZERO)
	
	if data.size() != (17*3): return
	
	var width := texture.get_width()
	var height := texture.get_height()
	var resolution := Vector2(
		max(width, height),
		max(width, height)
	)
	
	var nose_coordinate := Vector2(data[1], data[0])
	draw_circle(nose_coordinate * resolution, 5.0, Color.RED)
	
	var left_eye := Vector2(data[4], data[3])
	draw_circle(left_eye * resolution, 5.0, Color.RED)
	
	var right_eye := Vector2(data[7], data[6])
	draw_circle(right_eye * resolution, 5.0, Color.RED)
	
	var left_ear := Vector2(data[10], data[9])
	draw_circle(left_ear * resolution, 5.0, Color.RED)
	
	var right_ear := Vector2(data[13], data[12])
	draw_circle(right_ear * resolution, 5.0, Color.RED)
	
	var left_shoulder := Vector2(data[16], data[15])
	draw_circle(left_shoulder * resolution, 5.0, Color.RED)
	
	var rigth_shoulder := Vector2(data[19], data[18])
	draw_circle(rigth_shoulder * resolution, 5.0, Color.RED)
	
	var left_elbow := Vector2(data[22], data[21])
	draw_circle(left_elbow * resolution, 5.0, Color.RED)
	
	# I think I demonstrate enough points.
