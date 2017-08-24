#!/bin/sh
set -e
trap 'kill $(jobs -p) 2> /dev/null' EXIT

docker daemon &
docker_pid=$!

/usr/bin/drone server &
server_pid=$!

/usr/bin/drone agent &
client_pid=$!

wait ${docker_pid} ${server_pid} #{client_pid} 2> /dev/null
