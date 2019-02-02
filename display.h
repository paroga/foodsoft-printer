#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct display_s display_t;

display_t* display_new(const char* path);
void display_free(display_t* display);
uint32_t display_slot_count(display_t* display);
void display_update_antenna(display_t* display, int level);
void display_update_connection(display_t* display, bool active);
void display_update_cloud(display_t* display, bool active);
void display_update_printer(display_t* display, bool active);
void display_update_slot(display_t* display, uint32_t slot, char icon, unsigned char id, const char* message);
