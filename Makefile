CC = gcc
CFLAGS = -Wall -Wextra -O2

OBJS = main.o tracefs.o file_util.o tracer_ctrl.o output.o

litetrace: $(OBJS)
	$(CC) $(CFLAGS) -o litetrace $(OBJS)

main.o: main.c tracefs.h tracer_ctrl.h output.h
tracefs.o: tracefs.c tracefs.h
file_util.o: file_util.c file_util.h
tracer_ctrl.o: tracer_ctrl.c tracer_ctrl.h tracefs.h file_util.h output.h
output.o: output.c output.h

.PHONY: clean
clean:
	$(RM) *.o litetrace


