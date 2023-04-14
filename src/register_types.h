/* godot-cpp integration testing project.
 *
 * This is free and unencumbered software released into the public domain.
 */

#ifndef IREE_GD_REGISTER_TYPES_H
#define IREE_GD_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_iree_gd_module(ModuleInitializationLevel p_level);
void uninitialize_iree_gd_module(ModuleInitializationLevel p_level);

#endif // IREE_GD_REGISTER_TYPES_H
