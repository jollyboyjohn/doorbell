OBJS = doorbell.o hw.o

all : doorbell

doorbell: $(OBJS)
	$(CC) $^ -o $@

doorbell.o: doorbell.h hw.h
hw.o: hw.h

.PHONY: clean
clean:
	rm doorbell $(OBJS)
