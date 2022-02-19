SHELL := /bin/bash
FILE := $(lastword $(MAKEFILE_LIST))
PWD := $(shell pwd)
DIR := $(shell basename ${PWD})

TARGET := debug
USER:= ga74ped
STYLE := google

isFormatAvailable = $(shell command -v clang-format)
isServerFolderAvailable = $(shell ls server)

.PHONY: all
all: clean build

.PHONY: help
help: 
	@echo ""

.PHONY: reset-img
reset-img:
	@echo -e '\n** RESETTING IMAGES'
	@rm -fv .imgs/*.png

.PHONY: reset
reset: reset-img
	@echo -e '\n** RESETTING RESULTS'
	@rm -fv results/debug/*.npz
	@rm -fv results/small/*.npz
	@rm -fv results/normal/*.npz
	@rm -fv results/*.png

.PHONY: clean
clean:
	@echo -e '\n** CLEANING PROJECT'
	@if [ -n "$(shell ls build/)" ]; then \
		rm -rvf build/*; \
	fi
	@touch build/.gitkeep
	@rm -fv *.log
	@rm -fv *.zip
	@rm -rfv .cache
	@rm -rfv .pytest_cache

.PHONY: docs
docs:
	@echo -e '\n** GENERATING DOCUMENTATION'
	@doxygen docs/Doxyfile

.PHONY: build
build:
	@echo -e '\n** COMPILING PROJECT'
	@cd build/ && cmake ..
	@$(MAKE) -C build/

.PHONY: project
project: clean reset convert build
	@echo -e '\n** RUNNING FINAL PROJECT RESULTS'
	@for t in debug small; do \
		for n in 4 6 8; do \
			pushd build; \
				mpirun -np $$n --hostfile ../hostfile --mca btl_tcp_if_include lan0 ./project $$t ; \
			popd; \
		done; \
	done

.PHONY: run
run:
	@echo -e '\n** RUNNING PROJECT'
	@for n in 2 4; do \
		pushd build; \
			mpirun -np $$n --hostfile ../hostfile --mca btl_tcp_if_include lan0 ./project ${TARGET} ; \
		popd; \
	done

.PHONY: local
local:
	@echo -e '\n** RUNNING PROJECT LOCALLY'
	@for n in 2; do \
		pushd build; \
			mpirun -np $$n ./project ${TARGET} ; \
		popd; \
	done

.PHONY: bench
bench: reset-img
	@echo -e '\n** BENCHMARKING PROJECT'
	@python3 scripts/main.py

.PHONY: view
view:
	@echo -e '\n** VIEWING IMAGES'
	@sxiv .imgs

.PHONY: convert
convert:
	@echo -e '\n** CONVERTING PICKLE FILES'
	@python3 scripts/convert.py

.PHONY: test
test:
	@python3 -m pytest tests/

.PHONY: debug
debug: 
	@echo -e '\n** DEBUGGING PROJECT'
	$(MAKE) -C build/ debug

.PHONY: pack
pack: clean
	@echo -e '\n** PACKING PROJECT'
	@zip -r archive.zip ../${DIR}

.PHONY: send
send: pack
	@echo -e '\n** SENDING CURRENT PROJECT TO SERVER'
	@scp -r archive.zip ${USER}@hpc02.clients.eikon.tum.de:/home/${USER}/

.PHONY: get
get:
	@echo -e '\n** GETTING RESULTS FROM SERVER'
	@if [ -n "$(call isServerFolderAvailable)" ]; then \
		rm -vrf server; \
	fi
	@scp -r ${USER}@hpc04.clients.eikon.tum.de:/home/${USER}/${DIR}/results ./server

.PHONY: transfer
transfer:
	@echo -e '\n** TRANSFERING RESULTS'
	@if [ -n "$(call isServerFolderAvailable)" ]; then \
		${MAKE} reset; \
		find server/debug -name '*.npz' -exec cp -v {} results/debug/ \; ; \
		find server/small -name '*.npz' -exec cp -v {} results/small/ \; ; \
		find server/normal -name '*.npz' -exec cp -v {} results/normal/ \; ; \
	fi

.PHONY: format
format:
	@echo -e '\n** FORMATTING PROJECT'
	@if [ -n "$(call isFormatAvailable)" ]; then \
		for f in $$(ls src/*.cpp); do \
			clang-format -style=${STYLE} -i $$f ; \
			echo Formatting $$f ; \
		done; \
		for f in $$(ls include/*.h); do \
			clang-format -style=${STYLE} -i $$f ; \
			echo Formatting $$f ; \
		done; \
	fi
