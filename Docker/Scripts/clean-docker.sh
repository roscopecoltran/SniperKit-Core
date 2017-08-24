#!/bin/bash
# won't work unless docker is running

docker rm $(docker ps -q -f 'status=exited')
docker rmi $(docker images -q -f "dangling=true")