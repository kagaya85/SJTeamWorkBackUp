MAKE = make

COMMON_DIR = common

DFS_DIR = DFS

exclude_dirs := include bin

###########
##get dir##
###########

subdir = $(shell find . -maxdepth 1 -type d)
dirs := $(basename $(patsubst ./%,%,$(subdir)))
dirs := $(filter-out $(exclude_dirs),$(dirs))

.PHONY: clean 
	#avoid to make clean.*

all: $(COMMON_DIR) $(DFS_DIR)

$(COMMON_DIR):
	$(MAKE) -C $$COMMON_DIR all || exit 1; 

$(DFS_DIR):
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
