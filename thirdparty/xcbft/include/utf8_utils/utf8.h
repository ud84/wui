#pragma once

#include <fontconfig/fontconfig.h>

struct utf_holder {
	FcChar32 *str;
	unsigned int length;
};

struct utf_holder char_to_uint32(const char *str);
void utf_holder_destroy(struct utf_holder holder);
