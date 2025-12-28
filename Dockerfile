FROM debian:13
LABEL authors="guard3"

# Install necessary packages
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt -y full-upgrade && apt install -y wget clang gdb cmake libc++-dev libssl-dev zlib1g-dev libsqlite3-dev

# Install Boost
RUN wget -qO- https://archives.boost.io/release/1.90.0/source/boost_1_90_0.tar.gz | tar -xz
WORKDIR "/boost_1_90_0"
RUN ./bootstrap.sh && ./b2 cxxflags=-stdlib=libc++ linkflags=-stdlib=libc++ install
WORKDIR "/"
RUN rm -rf boost_*