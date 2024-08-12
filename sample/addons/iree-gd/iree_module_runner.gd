class_name IREEModuleRunner
extends Node

var results: Dictionary
var tasks: Dictionary
var mutex: Mutex

func _run(module: IREEModule, method: String, inputs: Array[IREETensor], id: int):
	var outputs = module.call_module(method, inputs)
	mutex.lock()
	results[id] = outputs
	mutex.unlock()

func run(module: IREEModule, method: String, inputs: Array[IREETensor]) -> int:
	var thread_id = randi()
	var task_id = WorkerThreadPool.add_task(_run.bind(module, method, inputs, thread_id))
	tasks[task_id] = thread_id
	return task_id

func result(task: int) -> Array[IREETensor]:
	if !WorkerThreadPool.is_task_completed(task):
		if WorkerThreadPool.wait_for_task_completion(task) != OK:
			push_error("Task error")
		var object = results[tasks[task]]
		results.erase(tasks[task])
		return object
	return []
