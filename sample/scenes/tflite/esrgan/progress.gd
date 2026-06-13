extends ProgressBar


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	pass


func _on_texture_rect_on_upscaling_start():
	visible = true


func _on_texture_rect_on_upscaling_stop():
	visible = false

func _on_texture_rect_on_upscaling_step(percentage):
	value = percentage
