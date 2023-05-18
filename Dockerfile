FROM gcc 

# install hiredis
RUN git clone https://github.com/redis/hiredis.git && \
    cd hiredis && \
    make && \
    make install
RUN echo "/usr/local/lib" >> /etc/ld.so.conf && \
    ldconfig

# install postgresql
RUN apt-get install libpq-dev

# https://github.com/ufoscout/docker-compose-wait/
COPY --from=ghcr.io/ufoscout/docker-compose-wait:latest /wait /wait