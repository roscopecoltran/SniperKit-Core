#!/bin/bash
openssl req -new -newkey rsa:4096 -days 365 -nodes -x509 -subj "/CN=nginx/O=Nginx Server" -keyout "nginx.key" -out "nginx.cert" && chmod 660 nginx.key nginx.cert
