clean:
	rm csm prd

build:
	gcc producer.c -o prd -lrt -lpthread
	gcc consumer.c -o csm -lrt -lpthread

run-prd:
	./prd

run-csm:
	./csm
