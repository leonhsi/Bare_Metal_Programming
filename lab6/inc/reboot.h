#ifndef REBOOT__H
#define REBOOT__H

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

#endif