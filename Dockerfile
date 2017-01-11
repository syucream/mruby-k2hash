FROM ubuntu
MAINTAINER syu_cream
WORKDIR /opt
ADD ./docker_config.rb /opt

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

# QUICKFIX install fullock required by k2hash
RUN git clone --depth 1 https://github.com/yahoojapan/fullock.git &&   \
    cd fullock/ && ./autogen.sh && ./configure && make && make install
RUN ldconfig

#
# Build mruby
#
RUN git clone --depth 1 https://github.com/mruby/mruby.git
RUN cd mruby &&                                 \
    MRUBY_CONFIG=../docker_config.rb ./minirake

#
# For development
#
RUN apt-get install -y       \
            gdb              \
            vim
