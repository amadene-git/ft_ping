docker build    --build-arg USER=$USER \
                --build-arg UID=$(id -u) \
                --build-arg GID=$(id -g) \
                -t ft_ping_image .