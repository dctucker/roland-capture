#pragma once

typedef void (capmix_listener_t)(uint8_t *, int);

int   capmix_setup_midi();
int   capmix_read_midi();
int   capmix_send_midi(uint8_t *, int);
void  capmix_cleanup_midi();

