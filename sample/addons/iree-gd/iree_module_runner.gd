class_name IREEModuleRunner
extends Node

@export var max_threads := 2
@export var max_results := 2
var module_semaphore := Semaphore.new()
var results_semaphore := Semaphore.new()

var modules: Array[IREEModule]

var results: Dictionary
var tasks: Dictionary
var mutex := Mutex.new()

func _init() -> void:
	for idx in max_results:
		results_semaphore.post()
	for idx in max_threads:
		module_semaphore.post()
	modules.resize(max_threads)
	for idx in modules.size():
		modules.push_back(_load_module())

func _load_module() -> IREEModule:
	assert("This is abstract and needs to be implemented")
	return null

func _write_results(outputs: Array[IREETensor], id: int):
	results_semaphore.wait()
	mutex.lock()
	results[id] = outputs
	mutex.unlock()

func _get_module() -> IREEModule:
	var module: IREEModule
	module_semaphore.wait()
	mutex.lock()
	module = modules.pop_back()
	mutex.unlock()
	return module

func _return_module(module: IREEModule):
	mutex.lock()
	modules.push_back(module)
	mutex.unlock()
	module_semaphore.post()

func _run(method: String, inputs: Array[IREETensor], id: int):
	var thread_id := OS.get_thread_caller_id()
	var module:= _get_module()
	var outputs = module.call_module(method, inputs)
	_write_results(outputs, id)
	_return_module(module)

func run(method: String, inputs: Array[IREETensor]) -> int:
	var thread_id = randi()
	var task_id = WorkerThreadPool.add_task(_run.bind(method, inputs, thread_id))
	tasks[task_id] = thread_id
	return task_id

func result(task: int) -> Array[IREETensor]:
	if !WorkerThreadPool.is_task_completed(task):
		if WorkerThreadPool.wait_for_task_completion(task) != OK:
			push_error("Task error")
		var object = results[tasks[task]]
		results.erase(tasks[task])
		results_semaphore.post()
		return object
	elif results.has(tasks[task]):
		var object = results[tasks[task]]
		results.erase(tasks[task])
		results_semaphore.post()
		return object
	push_error("Task error")
	return []
