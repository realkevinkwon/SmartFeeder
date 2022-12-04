#ifndef __LOAD_CELL_H__
#define __LOAD_CELL_H__

#pragma once

#include "driver/gpio.h"
#include "hx711.h"

void load_cell_init(void);
void load_cell_write(void);
void load_cell_read(void);

#endif /* __LOAD_CELL_H__ */