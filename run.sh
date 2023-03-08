rm -f ./object/out.out
docker run -it --rm -v $PWD:/tmp -w /tmp gcc /bin/bash -c "gcc ./src/main.c -o ./object/out.out && ./object/out.out"
rm -f ./object/out.out