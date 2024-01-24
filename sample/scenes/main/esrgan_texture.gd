extends TextureRect

@export var metal_module: IREEModule
@export var vulkan_module: IREEModule
@export var vmvx_module: IREEModule

signal on_upscaling_start()
signal on_upscaling_step(percentage)
signal on_upscaling_stop()

func downscale():
	var image := texture.get_image()
	image.resize(int(image.get_width() / 4), int(image.get_height() / 4))
	texture = ImageTexture.create_from_image(image)

func upscale():
	var width := texture.get_width()
	var last_box_width := width % 50 if (width % 50) != 0 else 50
	var box_column_count := (float(width) / 50 if last_box_width == 50 else ceil(float(width) / 50)) as int
	var height := texture.get_height()
	var last_box_height := height % 50 if (height % 50) != 0 else 50
	var box_row_count := (float(height) / 50 if last_box_height == 50 else ceil(float(height) / 50)) as int
	var old_image := texture.get_image()
	var new_image_data := PackedByteArray()
	new_image_data.resize(width * 4 * height * 4 * 3)
	var new_image := Image.create_from_data(width * 4, height * 4, false, Image.FORMAT_RGB8, new_image_data)
	old_image.convert(Image.FORMAT_RGB8)
	new_image.convert(Image.FORMAT_RGB8)
	
	var processing_image_data := PackedByteArray()
	processing_image_data.resize(50 * 50 * 3)
	
	on_upscaling_start.emit()
	print("Start upscaling.")
	for i in box_column_count: 
		for j in box_row_count:
			on_upscaling_step.emit(float(i * box_column_count + j) / (box_column_count * box_row_count) * 100)
			print(float(i * box_column_count + j) / (box_column_count * box_row_count) * 100, "% is complete.")
			var x_offset := i * 50
			var y_offset := j * 50
			var box_width := 50 if i != box_column_count - 1 else last_box_width
			var box_height := 50 if j != box_row_count - 1 else last_box_height
			var processing_image := Image.create_from_data(50, 50, false, Image.FORMAT_RGB8, processing_image_data)
			processing_image.convert(Image.FORMAT_RGB8)
			processing_image.blit_rect(
				old_image, 
				Rect2i(x_offset, y_offset, box_width, box_height),
				Vector2i.ZERO
			)
			var raw_input_data := processing_image.get_data()
			var clean_input_data := PackedFloat32Array()
			clean_input_data.resize(raw_input_data.size())
			for k in raw_input_data.size(): clean_input_data[k] = raw_input_data[k]
			var input_tensor := IREETensor.from_float32s(
				clean_input_data,
				[1, 50, 50, 3]
			)
			var module : IREEModule = null
			match OS.get_name():
				"Windows", "Linux", "FreeBSD", "NetBSD", "OpenBSD", "BSD":
					module = vulkan_module
				"macOS", "iOS":
					module = metal_module
				"Android":
					module = vmvx_module
				_:
					assert(false, "Unsupported platform.")
			# Synchronous execution
			#var output_tensor := module.bind("module.main", [input_tensor]).call_module().front() as IREETensor
			# Asynchronous execution
			var output_tensor := (await module.bind("module.main", [input_tensor]).call_module_async().completed).front() as IREETensor
			var raw_output_data := output_tensor.get_data().to_float32_array()
			var clean_output_data := PackedByteArray()
			clean_output_data.resize(raw_output_data.size())
			for k in raw_output_data.size(): clean_output_data[k] = clamp(raw_output_data[k], 0, 255)
			var output_image := Image.create_from_data(
				200, 200, false, Image.FORMAT_RGB8, clean_output_data
			)
			new_image.blit_rect(
				output_image,
				Rect2i(0, 0, box_width * 4, box_height * 4),
				Vector2i(x_offset * 4, y_offset * 4)
			)
	on_upscaling_stop.emit()
	print("Done upscaling.")
	
	
	texture = ImageTexture.create_from_image(new_image)
