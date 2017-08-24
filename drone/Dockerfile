FROM alpine
MAINTAINER feisky <feiskyer@gmail.com>

EXPOSE 8000

ENV DATABASE_DRIVER=sqlite3
ENV DRONE_SERVER=http://localhost:8000
ENV DATABASE_CONFIG=/var/lib/drone/drone.sqlite
ENV GODEBUG=netdns=go
ENV GOPATH=/go
ENV PATH=/go/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

COPY ca-certificates.crt /etc/ssl/certs/ca-certificates.crt
COPY start.sh /drone
VOLUME /var/lib/drone
CMD ["/drone"]

RUN apk update && \
    apk add docker git go build-base && \
    git clone https://github.com/drone/drone.git /go/src/github.com/drone/drone && \
    cd /go/src/github.com/drone/drone && \
    make deps gen build_static && \
    cp release/drone /usr/bin/ && \
    rm -rf /go
