FROM debian:12
LABEL authors="guard3"

# Install necessary packages
RUN apt update && apt full-upgrade && apt install -y g++ lsb-release wget software-properties-common gnupg git cmake libssl-dev zlib1g-dev libsqlite3-dev

# Install LLVM 19
RUN bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)" -- 19 all

# Install Boost
RUN wget -qO- https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.gz | tar -xz
WORKDIR "/boost_1_88_0"
RUN ./bootstrap.sh && ./b2 cxxflags=-stdlib=libc++ linkflags=-stdlib=libc++ toolset=clang-19 install
WORKDIR "/"
RUN rm -rf boost_*

ENTRYPOINT [ "bash" ]