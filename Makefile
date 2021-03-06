targets = common.c dsa.c expand.c load.c solution.c reduce.c

THREADS = 16
SMALL_SOL_SIZE = 100
LARGE_SOL_SIZE = 550
SMALL_N = 2000
SMALL_M = 2000
LARGE_N = 3000
LARGE_M = 3000
LARGE_HEAP_PARTITION = 4000
SMALL_HEAP_PARITION = 4000

all: dc_dismsemin dc_dismsesum dc_dishaumin dc_dishausum dc_discli dc_bes dc_ran \
	dc_dismseminL dc_dismsesumL dc_dishauminL dc_dishausumL dc_discliL dc_besL dc_ranL \
	randomhc dc_disclione dc_disclinwi

bin:
	mkdir -p bin
tests:
	mkdir -p tests
dc_dismsemin: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_MSE -D FDISMODE_MINDIST \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_dismsemin -lm -lpthread
dc_dismseminL: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(LARGE_SOL_SIZE) -D MAX_FACILITIES=$(LARGE_N) -D MAX_CLIENTS=$(LARGE_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_MSE -D FDISMODE_MINDIST \
		-D HEAP_PARTITION=$(LARGE_HEAP_PARTITION) \
		-o ../bin/dc_dismseminL -lm -lpthread
dc_dismsesum: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_MSE -D FDISMODE_SUMOFDELTAS \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_dismsesum -lm -lpthread
dc_dismsesumL: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(LARGE_SOL_SIZE) -D MAX_FACILITIES=$(LARGE_N) -D MAX_CLIENTS=$(LARGE_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_MSE -D FDISMODE_SUMOFDELTAS \
		-D HEAP_PARTITION=$(LARGE_HEAP_PARTITION) \
		-o ../bin/dc_dismsesumL -lm -lpthread
dc_dishaumin: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_HAUSDORFF -D FDISMODE_MINDIST \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_dishaumin -lm -lpthread
dc_dishauminL: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(LARGE_SOL_SIZE) -D MAX_FACILITIES=$(LARGE_N) -D MAX_CLIENTS=$(LARGE_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_HAUSDORFF -D FDISMODE_MINDIST \
		-D HEAP_PARTITION=$(LARGE_HEAP_PARTITION) \
		-o ../bin/dc_dishauminL -lm -lpthread
dc_dishausum: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_HAUSDORFF -D FDISMODE_SUMOFDELTAS \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_dishausum -lm -lpthread
dc_dishausumL: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(LARGE_SOL_SIZE) -D MAX_FACILITIES=$(LARGE_N) -D MAX_CLIENTS=$(LARGE_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_HAUSDORFF -D FDISMODE_SUMOFDELTAS \
		-D HEAP_PARTITION=$(LARGE_HEAP_PARTITION) \
		-o ../bin/dc_dishausumL -lm -lpthread
dc_discli: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_CLIENTDELTA \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_discli -lm -lpthread
dc_discliL: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(LARGE_SOL_SIZE) -D MAX_FACILITIES=$(LARGE_N) -D MAX_CLIENTS=$(LARGE_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_CLIENTDELTA \
		-D HEAP_PARTITION=$(LARGE_HEAP_PARTITION) \
		-o ../bin/dc_discliL -lm -lpthread
dc_bes: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_BESTS \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_bes -lm -lpthread
dc_besL: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(LARGE_SOL_SIZE) -D MAX_FACILITIES=$(LARGE_N) -D MAX_CLIENTS=$(LARGE_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_BESTS \
		-D HEAP_PARTITION=$(LARGE_HEAP_PARTITION) \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_besL -lm -lpthread
dc_ran: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_RANDOM \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_ran -lm -lpthread
dc_ranL: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(LARGE_SOL_SIZE) -D MAX_FACILITIES=$(LARGE_N) -D MAX_CLIENTS=$(LARGE_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_RANDOM \
		-D HEAP_PARTITION=$(LARGE_HEAP_PARTITION) \
		-o ../bin/dc_ranL -lm -lpthread
randomhc: bin
		cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) mainhc.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D RANDOMHC \
		-D REDUCTION_BESTS \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/randomhc -lm -lpthread
dc_disclione: bin
	cd src; gcc -std=c99 -g -D THREADS=0 -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D NDEBUG \
		-D REDUCTION_DISPERSE -D DISSIM_CLIENTDELTA \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_disclione -lm -lpthread
dc_disclinwi: bin
	cd src; gcc -std=c99 -g -D THREADS=$(THREADS) -Wall $(targets) main.c \
		-D MAX_SOL_SIZE=$(SMALL_SOL_SIZE) -D MAX_FACILITIES=$(SMALL_N) -D MAX_CLIENTS=$(SMALL_M) \
		-D LOCAL_SEARCH -D LOCALSEARCH_DONT_USE_WHITAKER \
		-D REDUCTION_DISPERSE -D DISSIM_CLIENTDELTA \
		-D HEAP_PARTITION=$(SMALL_HEAP_PARITION) \
		-o ../bin/dc_disclinwi -lm -lpthread

