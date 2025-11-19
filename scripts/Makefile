DOCKER_COMPOSE_FILE=../docker/docker-compose.yml

all: build
	./irssi_test.sh	docker

build:
	docker compose -f ${DOCKER_COMPOSE_FILE} up -d
	docker compose -f ${DOCKER_COMPOSE_FILE} exec irc-server make -f /app/Makefile re
