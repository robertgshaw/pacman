### builder image
FROM cpp-builder-base:latest AS builder

WORKDIR /app

# copy code and complie
COPY server server
COPY shared shared
COPY GNUmakefile .

RUN make server

### runtime image
FROM debian:stable-slim

COPY --from=builder /app/server.out .

EXPOSE 6169

CMD ["./server.out"]