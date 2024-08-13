class_name IREERunner
extends Node

# For now just 1 instance of it
var module: IREEModule = _load_module()
var semaphore := Semaphore.new()

class IREEResult:
	signal result(outputs: Array[IREETensor])

func _init() -> void:
	semaphore.post()

func _load_module() -> IREEModule:
	assert("This is abstract and needs to be implemented")
	return null

func _write_results(outputs: Array[IREETensor], iree_result: IREEResult):
	iree_result.result.emit.call_deferred(outputs)

func _run(method: String, inputs: Array[IREETensor], iree_result: IREEResult):
	var thread_id := OS.get_thread_caller_id()
	semaphore.wait()
	var outputs = module.call_module(method, inputs)
	_write_results(outputs, iree_result)
	semaphore.post()

func run(method: String, inputs: Array[IREETensor]) -> IREEResult:
	var iree_result = IREEResult.new()
	WorkerThreadPool.add_task(_run.bind(method, inputs, iree_result))
	return iree_result
