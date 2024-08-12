@tool
extends Button

@export var option_button: OptionButton

# Called when the HTTP request is completed.
func _http_request_completed(result, response_code, headers, body: PackedByteArray):
	if result != HTTPRequest.RESULT_SUCCESS || response_code != 200:
		push_error("Can't downloaded.")
		return
	option_button.clear()
	for model in JSON.parse_string(body.get_string_from_utf8()):
		option_button.add_item(model.name)

func _ready():
	_pressed()

func _pressed() -> void:
	var http_request = HTTPRequest.new()
	add_child(http_request)
	http_request.use_threads = true
	http_request.request_completed.connect(self._http_request_completed)
	var url : String = "https://api.github.com/repos/appsinacup/iree.zoo/releases?per_page=100&page=1"
	var error = http_request.request(url)
	if error != OK:
		push_error("An error occurred in the HTTP request.")
