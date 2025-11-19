DOCKER_COMPOSE_FILE=../docker/docker-compose.yml

.PHONY: irssi_test docker nc_test nc_test_docker build clean

# Default target for host environment irssi test
all: irssi_test

irssi_test:
	@echo "Running irssi tests on host..."
	cd ../../ && make re
	./irssi_test.sh

# Target for docker environment irssi test
docker: build
	@echo "Running irssi tests in docker..."
	./irssi_test.sh docker

# Target for host environment nc_test
nc_test:
	@echo "Running netcat tests on host..."
	cd ../../ && make re
	./nc_test.sh

# Target for docker environment nc_test
nc_test_docker: build
	@echo "Running netcat tests in docker..."
	./nc_test.sh docker

build:
	@echo "Building ircserv in docker..."
	docker compose -f ${DOCKER_COMPOSE_FILE} up -d
	docker compose -f ${DOCKER_COMPOSE_FILE} exec irc-server make -f /app/Makefile re

clean:
	@echo "Cleaning docker containers..."
	docker compose -f ${DOCKER_COMPOSE_FILE} down