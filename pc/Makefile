#Docker Makefile

CONTAINER_NAME=node_red
CONTAINER=$(shell docker ps | grep ${CONTAINER_NAME})

#Add volumes to docker
#--volume="Host_Pc_Path:Container_Path:Permissions"

DOCKER_VOLUMES= \
	--volume="${PWD}/flows:/data:rw"
#--volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
#--volume="/usr/share/icons:/usr/share/icons:ro" \


DOCKER_ENV_VARS=

DOCKER_ARGS = ${DOCKER_VOLUMES} ${DOCKER_ENV_VARS}

.PHONY: build run help list start

help:
	@echo "Run 'make build' to create or recreate the docker image"
	@echo "Run 'make start' to start the container"
	@echo "Run 'make run' to connect a terminal to the container"
	@echo "Run 'make list' to list running containers"

build:
	clear
	@echo "Building Image"
	@docker build -t ${CONTAINER_NAME} .
	clear
	@echo "Images built"

list:
	@docker ps

setup:

start: setup
	clear
	@docker run -it --net=host ${DOCKER_ARGS} ${CONTAINER_NAME} bash

run:
	clear
	@docker exec -it $(word 12,${CONTAINER}) bash
