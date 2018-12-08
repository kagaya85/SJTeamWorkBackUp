MAKE = make

COMMON_MAKE = COMMON
COMMON_DIR = common

DFS_MAKE = DFS

exclude_dirs := include bin

###########
##get dir##
###########

subdir = $(shell find . -maxdepth 1 -type d)
dirs := $(basename $(patsubst ./%,%,$(subdir)))
dirs := $(filter-out $(exclude_dirs),$(dirs))

.PHONY: clean 
	#avoid to make clean.*

all: $(COMMON_MAKE) $(DFS_MAKE)

$(COMMON_MAKE):
	$(MAKE) -C $(COMMON_DIR) all || exit 1;

$(DFS_MAKE):
	@for dir in $(dirs); \
	do \
		if [ "$$dir"x = "$(COMMON_DIR)"x ]; \
		then \
			continue; \
		fi; \
		$(MAKE) -C $$dir all || exit 1; \
	done
	
clean:
	@for dir in $(dirs); \
	do \
		$(MAKE) -C $$dir clean || exit 1; \
	done
