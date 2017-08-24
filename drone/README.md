Dockerfile for building [drone](https://github.com/drone/drone).

```
docker run \
  --env DRONE_GITHUB=true \
  --env DRONE_GITHUB_CLIENT=... \
  --env DRONE_GITHUB_SECRET=... \
  --env DRONE_SECRET=... \
  --env DRONE_OPEN=true  \
  --env DRONE_ADMIN=...  \
  -v /var/lib/drone:/var/lib/drone \
  --restart=always \
  --publish=80:8000 \
  --detach=true \
  --name=drone \
  feisky/drone
```
