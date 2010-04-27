# common script functionality

function die() {
    echo "error: $1"
    exit 1
}

function is_sbox() {
    if [ -z "$SBOX_UNAME_MACHINE" ]; then
        return 1
    fi
    return 0
}

d=`dirname $0`
bd=$d/..

if is_sbox; then
    sudo="fakeroot"

    QT_SRC_DIR=${QT_SRC_DIR:-$bd/qt4-maemo5}
    target="$SBOX_DPKG_INST_ARCH"
else
    sudo="sudo"
    QT_SRC_DIR=${QT_SRC_DIR:-$bd/qt4-maemo5}
    target="host"
fi

WEBKIT_SRC_DIR=${WEBKIT_SRC_DIR:-$bd/webkit}

builddir=${builddir:-$PWD/builddir-$target}

cc_vars=
if [ -n "$DISTCC_HOSTS" ]; then
    cc_vars="CC='distcc gcc' CXX='distcc g++'"
    makeargs=-j8
fi

# most people have dual-cores..
makeargs=${makeargs:-"-j3"}

build_webkit_args="--engine-thread --no-video --no-svg"
build_webkit_args="--no-video --no-svg"


webkit_release=
release=
while [ $# -gt 0 ]; do
    case $1 in
        --release)
            release=1
            shift
            ;;
        --webkit_release)
            webkit_release=1
            shift
            ;;
        *)
            break
            ;;
    esac
done

function is_release() {
    if [ -z "$release" ]; then
        return 1
    fi
    return
}

function is_webkit_release() {
    if [ -z "$webkit_release" ]; then
        return 1
    fi
    return 0
}


if is_release; then
    CUSTOM_QT_PREFIX=${CUSTOM_QT_PREFIX:-/opt/qt4-custom}
else
    CUSTOM_QT_PREFIX=${CUSTOM_QT_PREFIX:-/opt/qt4-custom-debug}
fi
