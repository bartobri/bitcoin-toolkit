/*
 * Copyright (c) 2017 Brian Barto
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPL License. See LICENSE for more details.
 */

#ifndef BTKTERMIO_H
#define BTKTERMIO_H 1

// Function prototypes
void btktermio_init_terminal(void);
void btktermio_restore_terminal(void);
int btktermio_get_rows(void);
int btktermio_get_cols(void);
int btktermio_get_cursor_row(void);
void btktermio_move_cursor(int, int);
void btktermio_refresh(void);
void btktermio_clear_input(void);
char btktermio_get_char(void);
void btktermio_show_cursor(void);
void btktermio_beep(void);
int btktermio_get_clearscr(void);
void btktermio_set_clearscr(int);
void btktermio_set_foregroundcolor(char *);


#endif
