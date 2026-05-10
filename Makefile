DEBUG = 1

CXX = gcc
LD_FLAGS = -lrt -lpthread

ifeq ($(DEBUG), 1)
	CXX_FLAGS += -g -O0
else
	CXX_FLAGS += -O3
endif

COMPILER_CALL = $(CXX) $(CXX_FLAGS)

build:
	$(COMPILER_CALL) producer.c -o prd $(LD_FLAGS)
	$(COMPILER_CALL) consumer.c -o csm $(LD_FLAGS)

run-prd:
	./prd

run-csm:
	./csm

clean:
	rm csm prd
