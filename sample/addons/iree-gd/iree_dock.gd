@tool
extends EditorPlugin

var dock

func _enter_tree():
	dock = preload("res://addons/iree-gd/iree_dock.tscn").instantiate()
	add_control_to_bottom_panel(dock, "IREE Models")


func _exit_tree():
	remove_control_from_bottom_panel(dock)
	dock.free()
