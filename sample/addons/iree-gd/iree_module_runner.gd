class_name IREEModuleRunner
extends Node

@export var max_threads := 2
var module_semaphore := Semaphore.new()
var modules: Array[IREEModule]
var mutex := Mutex.new()

class IREEResult:
	signal result(outputs: Array[IREETensor])

func _init() -> void:
	for idx in max_threads:
		module_semaphore.post()
	modules.resize(max_threads)
	for idx in modules.size():
		modules.push_back(_load_module())

func _load_module() -> IREEModule:
	assert("This is abstract and needs to be implemented")
	return null

func _write_results(outputs: Array[IREETensor], iree_result: IREEResult):
	iree_result.result.emit.call_deferred(outputs)

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

func _run(method: String, inputs: Array[IREETensor], iree_result: IREEResult):
	var thread_id := OS.get_thread_caller_id()
	var module:= _get_module()
	var outputs = module.call_module(method, inputs)
	_write_results(outputs, iree_result)
	_return_module(module)

func run(method: String, inputs: Array[IREETensor]) -> IREEResult:
	var iree_result = IREEResult.new()
	WorkerThreadPool.add_task(_run.bind(method, inputs, iree_result))
	return iree_result
