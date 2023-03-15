    # image +/- 350 MiB

    # podman
    alias starcry='xhost + && podman run --rm --name starcry -p 18080:18080 -i -t -v `pwd`:`pwd` -w `pwd` -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --entrypoint=/starcry docker.io/rayburgemeestre/starcry:v8'

    # docker (non-rootless version)
    alias starcry='xhost + && docker run --rm --name starcry -i -t -v `pwd`:`pwd` -w `pwd` -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --entrypoint=/starcry docker.io/rayburgemeestre/starcry:v8'

    # force software rendering
    alias starcry='xhost + && docker run --rm --name starcry -i -t -v `pwd`:`pwd` -w `pwd` -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri/card0:/dev/dri/card0 --entrypoint=/starcry docker.io/rayburgemeestre/starcry:v8'

