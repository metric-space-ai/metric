FROM quay.io/pypa/manylinux2014_x86_64

ENV PLAT manylinux2014_x86_64

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]