FROM ubuntu
MAINTAINER syu_cream
WORKDIR /opt
ADD ./build_config_example.rb /opt

#
# Prepare build environment
#
RUN apt-get dist-upgrade
RUN apt-get -y update
RUN apt-get install -y       \
            git              \
            build-essential  \
            autoconf         \
            automake         \
            autotools-dev    \
            libtool          \
            pkg-config       \
            libssl-dev

#
# Prepare build environment for mruby
#
RUN apt-get install -y       \
            bison            \
            libreadline6     \
            libreadline6-dev \
            ncurses-dev      \
            ruby             \
            unzip

RUN ldconfig


#
# Build and install latest fullock and k2hash
#
RUN git clone https://github.com/yahoojapan/k2hash.git
RUN cd k2hash &&                               \
    git submodule update --init --recursive && \
    cd fullock &&                              \
    ./autogen.sh &&                            \
    ./configure &&                             \
    make &&                                    \
    make install &&                            \
    cd ../ &&                                  \
    ./autogen.sh &&                            \
    ./configure &&                             \
    make &&                                    \
    make install

RUN ldconfig

#
# Build mruby with mruby-k2hash
#
RUN git clone --depth 1 https://github.com/mruby/mruby.git
RUN cd mruby &&                                                              \
    mkdir -p build/host/mrbgems/ &&                                          \
    cd build/host/mrbgems/ &&                                                \
    git clone --depth 1 https://github.com/syucream/mruby-k2hash.git &&      \
    cd mruby-k2hash/ && git submodule update --init --recursive && cd ../ && \
    cd ../../../ &&                                                          \
    MRUBY_CONFIG=../build_config_example.rb ./minirake

#
# For development
#
RUN apt-get install -y       \
            gdb              \
            vim
