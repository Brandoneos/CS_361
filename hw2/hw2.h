#ifndef HW2_H
#define HW2_H

#define exit(N) {fflush(stdout); fflush(stderr); _exit(N); }
#define BSIZE 1024

void run_cmd(char * line);
void execute(char * input);
void pipeline(char* head, char* tail);
void sequence(char *head, char *tail);

int read_line(int fd, char *buf, int max);
#endif
