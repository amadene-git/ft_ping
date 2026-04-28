docker run -it --rm \
  --name ${USER}_ft_ping \
  --user $USER \
  --cap-add=NET_RAW \
  --hostname ${HOSTNAME}_ft_ping \
  -w $PWD \
  -v $HOME:$HOME \
  ft_ping_image
