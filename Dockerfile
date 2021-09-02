### builder image
FROM cpp-build-base:0.1.0 AS builder

WORKDIR /app

# copy code and complie
COPY server server
COPY shared shared
COPY GNUmakefile .

RUN make server

### runtime image
FROM debian:stable-slim

COPY --from=builder /app/server.out .

CMD ["./server.out"]