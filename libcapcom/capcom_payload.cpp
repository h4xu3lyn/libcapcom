#include "capcom_payload.h"

namespace capcom
{
	capcom_payload* build_capcom_payload(uintptr_t user_function_wrapper) 
	{
		capcom_payload* final_payload = (capcom_payload*)VirtualAlloc(nullptr, sizeof(capcom_payload), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		memcpy(final_payload->code, code_template, sizeof(code_template));

		final_payload->ptr_to_code = final_payload->code; 

		*(uintptr_t*)(final_payload->code + user_function_ptr_offset) = (uintptr_t)user_function_wrapper; 

		return final_payload;
	}
}