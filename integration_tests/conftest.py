import pytest
from irc_test_utils import ExecutableIrcServer, DockerIrcServer
import os

def pytest_addoption(parser):
    parser.addini("test_target", "The test target to use: 'executable' or 'docker'.", default='executable')
    
    # Options for 'executable' target
    parser.addini("executable_path", "Path to the IRC server executable relative to the project root.")
    parser.addini("executable_password", "Password for the IRC server.", default="pass")

    # Options for 'docker' target
    parser.addini("docker_compose_path", "Path to the docker-compose.yml file relative to the project root.")
    parser.addini("docker_host", "Hostname for the Dockerized IRC server.", default="localhost")
    parser.addini("docker_port", "Port for the Dockerized IRC server.", default="6668")
    parser.addini("docker_password", "Password for the Dockerized IRC server.", default="pass")

@pytest.fixture(scope="function")
def irc_server(request):
    test_target = request.config.getini("test_target")
    
    server = None
    if test_target == 'executable':
        relative_path = request.config.getini("executable_path")
        password = request.config.getini("executable_password")
        absolute_path = os.path.join(request.config.rootdir, relative_path)
        server = ExecutableIrcServer(executable_path=absolute_path, password=password)
    
    elif test_target == 'docker':
        relative_path = request.config.getini("docker_compose_path")
        host = request.config.getini("docker_host")
        port = int(request.config.getini("docker_port"))
        password = request.config.getini("docker_password")
        absolute_path = os.path.join(request.config.rootdir, relative_path)
        server = DockerIrcServer(compose_path=absolute_path, host=host, port=port, password=password)

    else:
        pytest.fail(f"Invalid test_target specified in pytest.ini: {test_target}")

    if server:
        server.start()
        yield server
        server.stop()
    else:
        pytest.fail("Failed to initialize a server fixture.")
