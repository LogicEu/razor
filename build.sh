#!/bin/bash

name=razor

cc=gcc
src=src/*.c

flags=(
    -Wall
    -Wextra
    -pedantic
    -O2
    -std=c99
)

sub=(
    imgtool
    fract
    mass
    utopia
)

inc=(
    -I.
    -Ispxe
)

lib=(
    -Llib
    -lz
    -lpng
    -ljpeg
    -lglfw
    -lfreetype
)

for mod in ${sub[*]}
do
    inc+=(-I$mod)
    lib+=(-l$mod)
done

if echo "$OSTYPE" | grep -q "darwin"; then
    lib+=(
        -framework OpenGL
    )
elif echo "$OSTYPE" | grep -q "linux"; then
    inc+=(-I/usr/include/freetype2)
    lib+=(
        -lGL
        -lGLEW
        -lm
    )
else
    echo "This OS is not supported by this shell script yet..." && exit
fi

cmd() {
    echo "$@" && $@
}

lib_build() {
    cmd pushd $1/ && ./build.sh $2 && cmd mv bin/*.a ../lib/ && cmd popd
}

build() {
    cmd mkdir -p lib/
    for mod in ${sub[*]}
    do
        lib_build $mod static
    done
}

objs() {
    [ ! -d lib/ ] && build
    cmd mkdir -p tmp/
    cmd $cc -c $src ${flags[*]} ${inc[*]} && cmd mv *.o tmp/
}

comp() {
    objs && cmd $cc tmp/*.o $rt -o $name ${flags[*]} ${lib[*]} ${inc[*]}
}

cleanf() {
    [ -f $1 ] && cmd rm $1
}

cleand() {
    [ -d $1 ] && cmd rm -r $1
}

cleanr() {
    cleand $1/tmp/
    cleand $1/bin/
}

clean() {
    for mod in ${sub[*]}
    do
        cleanr $mod
    done
    cleand lib
    cleand tmp
    cleanf $name
    return 0
}

if [[ $# -eq 0 ]]; then
    comp && exit
fi

case "$1" in
    "build")
        build;;
    "all")
        build && comp;;
    "clean")
        clean;;
    *)
        echo "Illegal option. Use with build, all or clean"
esac
