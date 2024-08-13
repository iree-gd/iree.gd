@tool
extends Button

@export var option_button: OptionButton

static func unzip(path_to_zip: String) -> void:
	var zr : ZIPReader = ZIPReader.new()
	if zr.open(path_to_zip) == OK:		
		for filepath in zr.get_files():
			var zip_directory : String = path_to_zip.get_base_dir()
			var trimmed_path : String = path_to_zip.trim_suffix(".zip")
			var fa : FileAccess = FileAccess.open("%s/%s" % [zip_directory, filepath], FileAccess.WRITE)
			fa.store_buffer(zr.read_file(filepath))
			ResourceLoader.load(fa.get_path(), "", ResourceLoader.CacheMode.CACHE_MODE_REPLACE_DEEP)
		DirAccess.remove_absolute(path_to_zip)

# Called when the HTTP request is completed.
func _http_request_completed(result, response_code, headers, body, file_path):
	if result != HTTPRequest.RESULT_SUCCESS || response_code != 200:
		push_error("Can't downloaded.")
		return
	print(file_path)
	unzip(file_path)
	EditorInterface.get_resource_filesystem().scan()
	print("Download successful. Check " + file_path)

func _pressed() -> void:
	var http_request = HTTPRequest.new()
	add_child(http_request)
	http_request.use_threads = true
	var model = option_button.get_item_text(option_button.get_selected_id())
	var folder_path : String = "res://addons/iree-zoo/" + model
	var file_path : = folder_path + "/model.zip"
	DirAccess.make_dir_recursive_absolute(folder_path)
	http_request.request_completed.connect(self._http_request_completed.bind(file_path))
	http_request.download_file = file_path
	var url : String = "https://github.com/appsinacup/iree.zoo/releases/download/" + model + "/model.zip"
	print("Downloading file from " + url)
	var error = http_request.request(url)
	if error != OK:
		push_error("An error occurred in the HTTP request.")
