rm -f ./obj/out.out
docker run -it --rm -v $PWD:/tmp -w /tmp gcc /bin/bash -c "gcc ./src/main.c -o ./obj/out.out && ./obj/out.out"
rm -f ./obj/out.out