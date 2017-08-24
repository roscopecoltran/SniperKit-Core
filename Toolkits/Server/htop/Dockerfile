# htop in a container
#
# $ docker build -t htop --no-cache .
# 
# $ docker run --rm -it \
# 	--pid host \
# 	jess/htop
#
FROM alpine:latest
LABEL maintainer "Jessie Frazelle <jess@linux.com>"

RUN apk --no-cache add \
	htop

CMD [ "htop" ]