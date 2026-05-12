
docker ps -a | grep ft_ping > /dev/null
if [[ $? -eq 1 ]]
then
  docker run -it --rm \
    --name ft_ping_container \
    --user root \
    --cap-add=NET_RAW \
    --cap-add=NET_ADMIN \
    --hostname ${HOSTNAME}_ft_ping \
    -w /root \
    -v $HOME:/root \
    ft_ping_image \
    zsh
else
  docker exec -it ft_ping_container zsh
fi


