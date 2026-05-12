docker run -it --rm \
  --name ft_ping_container \
  --user $USER \
  --cap-add=NET_RAW \
  --hostname ${HOSTNAME}_ft_ping \
  -w $PWD \
  -v $HOME:$HOME \
  ft_ping_image \
  zsh
