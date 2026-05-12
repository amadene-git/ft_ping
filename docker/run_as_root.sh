docker run -it --rm \
  --name ${USER}_ft_ping \
  --user root \
  --cap-add=NET_RAW \
  --hostname ${HOSTNAME}_ft_ping \
  -w /root \
  -v $HOME:/root \
  ft_ping_image \
  zsh

