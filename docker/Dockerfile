FROM debian:13

# ENV DEBIAN_FRONTEND=noninteractive
ARG USER
ARG UID=1000
ARG GID=1000

ENV USER=${USER}
ENV UID=${UID}
ENV GID=${GID}

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    zsh \
    vim \
    git \
    build-essential \
    sudo \
    wget

RUN groupadd -g ${GID} ${USER} \
 && useradd -m -u ${UID} -g ${GID} -s /bin/bash ${USER} \
 && usermod -aG sudo ${USER} \
 && echo "${USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${USER} \
 && chmod 440 /etc/sudoers.d/${USER}

RUN wget -O /tmp/pkg.deb http://ftp.debian.org/debian/pool/main/i/inetutils/inetutils-ping_2.0-1+deb11u2_amd64.deb \
 && dpkg -i /tmp/pkg.deb || apt-get -f install -y \
 && rm /tmp/pkg.deb


USER ${USER}

CMD echo "ft_ping container started" && zsh
