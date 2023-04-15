#ifndef TIME_H
#define TIME_H

void init_timer_queue();
void enqueue_timer(char *message, int sec);
void pop_timer();
void insert_timer();
void invoke_timer();
void timeout();
void add_timer(void (*call_back)(char *fmt, ...), char *message, int sec);
void setTimeout(char *message, int sec);

#endif