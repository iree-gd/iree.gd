#ifndef IREE_ERROR_H
#define IREE_ERROR_H

#include <iree/base/api.h>

#include <godot_cpp/variant/utility_functions.hpp>

#define IREE_ERR_MSG(mp_status, mp_message)                                    \
	{                                                                          \
		iree_status_t status = (mp_status);                                    \
		if ((status)) {                                                        \
			const iree_allocator_t allocator = iree_allocator_system();        \
			char *buffer = nullptr;                                            \
			iree_host_size_t length = 0;                                       \
			if (iree_status_to_string(status, &allocator, &buffer, &length)) { \
				ERR_PRINT(vformat("%s [IREE code: '%s'; %s]", (mp_message),    \
						iree_status_code_string(iree_status_code(status)),     \
						buffer));                                              \
				iree_allocator_free(allocator, buffer);                        \
			} else                                                             \
				ERR_PRINT(vformat("%s [IREE code: '%s']", (mp_message),        \
						iree_status_code_string(iree_status_code(status))));   \
			iree_status_free((status));                                        \
			return;                                                            \
		}                                                                      \
	}

#define IREE_ERR_V_MSG(mp_status, mp_retval, mp_message)                       \
	{                                                                          \
		iree_status_t status = (mp_status);                                    \
		if ((status)) {                                                        \
			const iree_allocator_t allocator = iree_allocator_system();        \
			char *buffer = nullptr;                                            \
			iree_host_size_t length = 0;                                       \
			if (iree_status_to_string(status, &allocator, &buffer, &length)) { \
				ERR_PRINT(vformat("%s [IREE code: '%s'; %s]", (mp_message),    \
						iree_status_code_string(iree_status_code(status)),     \
						buffer));                                              \
				iree_allocator_free(allocator, buffer);                        \
			} else                                                             \
				ERR_PRINT(vformat("%s [IREE code: '%s']", (mp_message),        \
						iree_status_code_string(iree_status_code(status))));   \
			iree_status_free((status));                                        \
			return (mp_retval);                                                \
		}                                                                      \
	}

#endif // IREE_ERROR_H
