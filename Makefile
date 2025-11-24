CC = smpicc
CFLAGS = -Wall -O2
LDFLAGS = -lm

TARGET = suara2

# --- Utils sources / objects ---
UTILS_SRCS = $(wildcard utils/*.c)
UTILS_OBJS = $(UTILS_SRCS:.c=.o)

# --- Main program object files ---
OBJS = \
	suara2.o est_time.o globals.o \
	linear_allreduce.o rabenseifner_allreduce.o \
	ring_allreduce.o recursive_doubling_allreduce.o \
	ring_seg_allreduce.o

all: suara2

# --- Link final executable ---
suara2: $(OBJS) $(UTILS_OBJS)
	smpicc -o suara2 \
		suara2.o est_time.o globals.o \
		linear_allreduce.o rabenseifner_allreduce.o \
		ring_allreduce.o recursive_doubling_allreduce.o \
		ring_seg_allreduce.o \
		$(UTILS_OBJS) \
		$(LDFLAGS)

# --------------------------------------------------------------------
# Explicit compilation rules (NO shorthand)
# --------------------------------------------------------------------

suara2.o: suara2.c est_time.h macros.h
	smpicc -Wall -O2 -c suara2.c -o suara2.o

est_time.o: est_time.c est_time.h $(UTILS_SRCS)
	smpicc -Wall -O2 -c est_time.c -o est_time.o

globals.o: globals.c macros.h
	smpicc -Wall -O2 -c globals.c -o globals.o

linear_allreduce.o: linear_allreduce.c macros.h
	smpicc -Wall -O2 -c linear_allreduce.c -o linear_allreduce.o

rabenseifner_allreduce.o: rabenseifner_allreduce.c macros.h
	smpicc -Wall -O2 -c rabenseifner_allreduce.c -o rabenseifner_allreduce.o

ring_allreduce.o: ring_allreduce.c macros.h
	smpicc -Wall -O2 -c ring_allreduce.c -o ring_allreduce.o

recursive_doubling_allreduce.o: recursive_doubling_allreduce.c macros.h
	smpicc -Wall -O2 -c recursive_doubling_allreduce.c -o recursive_doubling_allreduce.o

ring_seg_allreduce.o: ring_seg_allreduce.c macros.h
	smpicc -Wall -O2 -c ring_seg_allreduce.c -o ring_seg_allreduce.o

# --------------------------------------------------------------------
# Utils shorthand rule (pattern OK here)
# --------------------------------------------------------------------
utils/%.o: utils/%.c
	smpicc $(CFLAGS) -c $< -o $@

# --- Clean ---
clean:
	rm -f $(OBJS) $(UTILS_OBJS) suara2
