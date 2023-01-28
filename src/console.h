#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>

#include "event.h"

void console_init(void);
void console_destroy(void);
bool console_handle_event(event_t* event);
void console_update(void);
void console_draw(void);
void console_buffer_write(const char* line);
void console_buffer_clear(void);
void console_buffer_toggle(void);

#endif
