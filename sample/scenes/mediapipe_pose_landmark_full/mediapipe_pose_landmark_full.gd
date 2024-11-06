extends Node2D

@export var vulkan_module : IREEModule
@export var metal_module : IREEModule
@export var texture: Texture2D

var padded_tex : ImageTexture
var resolution : Vector2;
var depthtex : ImageTexture
var points = []

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
	image.convert(Image.FORMAT_RGBF)
	resolution = image.get_size()
	var aspect_ratio := float(resolution.x) / resolution.y
	image.resize(int(256.0 * aspect_ratio), 256)

	var padded_image := Image.create(256, 256, false, Image.FORMAT_RGBF)
	padded_image.blit_rect(image, Rect2i(Vector2i.ZERO, image.get_size()), Vector2i())
	padded_tex = ImageTexture.create_from_image(padded_image)
	return padded_image


func dump_to_file(input_tensor: IREETensor, result: Array) -> void:
	var dump_file := FileAccess.open("res://mediapipe_pose_landmark_full.dump.log", FileAccess.WRITE)
	dump_file.store_line("Input:")
	dump_file.store_line(str(input_tensor.get_dimension()))
	dump_file.store_line(str(input_tensor.get_data().to_float32_array()))
	dump_file.store_line("\n")
	
	dump_file.store_line("Output:")
	for tensor: IREETensor in result:
		dump_file.store_line("Dimensions: "+str(tensor.get_dimension()))
		dump_file.store_line("Data: "+str(tensor.get_data().to_float32_array()))
		dump_file.store_line("\n")
	dump_file.close()


func _ready() -> void:
	var image := texture.get_image()
	var processed_image := rectify_image(image)
	var processed_image_float32_data := processed_image.get_data().to_float32_array()
	
	var input_tensor := IREETensor.from_float32s(
		processed_image_float32_data,
		[1, 256, 256, 3]
	)
	
	var result := select_model().call_module("module.main", [input_tensor])
	# dump_to_file(input_tensor, result)
	var pointdata = result[0].get_data().to_float32_array()
	points = []
	for i in range(0, len(pointdata)-6, 5):
		points.append(Vector2(pointdata[i], pointdata[i+1]))

	var depthmap = result[2].get_data().to_float32_array()
	var depthmap_corrected = PackedFloat32Array()
	depthmap_corrected.resize(len(depthmap))
	var minmax : Vector2
	for p in depthmap:
		if p > minmax.y: minmax.y = p
		elif p < minmax.x: minmax.x = p
		
	for i in len(depthmap):
		depthmap_corrected[i] = clamp(inverse_lerp(minmax.x, minmax.y, depthmap[i]), 0.0, 1.0)
	var depthimg = Image.create_from_data(256, 256, false, Image.FORMAT_RF, depthmap_corrected.to_byte_array())
	depthtex = ImageTexture.create_from_image(depthimg)

	queue_redraw()


var point_names = [
	"nose", "left eye (inner)", "left eye", "left eye (outer)", "right eye (inner)", "right eye", "right eye (outer)",
	"left ear", "right ear", "mouth (left)", "mouth (right)", "left shoulder", "right shoulder", "left elbow",
	"right elbow", "left wrist", "right wrist", "left pinky", "right pinky", "left index", "right index", "left thumb",
	"right thumb", "left hip", "right hip", "left knee", "right knee", "left ankle", "right ankle", "left heel",
	"right heel", "left foot index", "right foot index" 
]


var lines = [
	[11, 12], [11, 23], [23, 24], [12, 24],
	[12, 14], [14, 16], [11, 13], [13, 15],
	[24, 26], [26, 28], [23, 25], [25, 27]
]

var default_font = ThemeDB.fallback_font


func _draw():
	if not texture:
		return

	if points.size() < 29:
		return

	draw_texture(texture, Vector2.ZERO)

	var scale_factor := texture.get_height() / float(padded_tex.get_height())

	for line in lines:
		draw_line(points[line[0]] * scale_factor, points[line[1]] * scale_factor, Color(0.39, 0.58, 0.93), 2.0)

	for i in range(points.size()):
		for line in lines:
			if i == line[0] or i == line[1]:
				draw_circle(points[i] * scale_factor, 5.0, Color.SALMON)
				var offset := Vector2(10, 10)
				var text_pos: Vector2 = points[i] * scale_factor + offset
				draw_rect(Rect2(text_pos - Vector2(2, 12), Vector2(default_font.get_string_size(point_names[i]).x + 4, 16)), Color(0.1, 0.1, 0.1, 0.5))
				draw_string(default_font, text_pos, point_names[i], HORIZONTAL_ALIGNMENT_LEFT, -1, 16, Color(1, 1, 1))
