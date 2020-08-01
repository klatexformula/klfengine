

# set $GIT, $CMAKE, $MAKE -- use existing values from environment variables, or
# deduce them from `which xxx`
GIT="${GIT:-`which git`}"
CMAKE="${CMAKE:-`which cmake`}"
MAKE="${MAKE:-`which make`}"

# Versions of the library to install.  I've hard-coded the latest versions (as
# of July 2020), change these to whatever you like ->
NLOHMANN_JSON_TAG=v3.9.0
MPARK_VARIANT_TAG=v1.4.0
GULRAK_FILESYSTEM_TAG=v1.3.2
CATCH2_TAG=v2.13.0


if [ ! -e 'include/klfengine/klfengine' ]; then

    echo >&2 "Please run this script in the root directory of the klfengine sources."
    exit 1

fi

mkdir -p deps_src deps_install

# "[ cond1 -o cond2 ]" is cond1 OR cond2
if [ -d deps_src/nlohmann-json  -o  \
     -d deps_src/mpark-variant  -o  \
     -d deps_src/gulrak-filesystem  -o  \
     -d deps_src/catch2 ]; then

    echo >&2 "Please remove (or rename) any existing downloads in deps_src/ before continuing."
    exit 2

fi

INSTALL_PREFIX=`pwd`/deps_install

#
# Download & install nlohmann/json in local deps_* dir
#

(cd deps_src && \
 "$GIT" clone https://github.com/nlohmann/json.git \
        --depth=1 --branch "$NLOHMANN_JSON_TAG" nlohmann-json   && \
 mkdir -p nlohmann-json/build  && \
 cd nlohmann-json/build  && \
 "$CMAKE" .. -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" -DJSON_BuildTests=off   && \
 "$MAKE" install) 

#
# Download & install mpark/variant in local deps_* dir
#

(cd deps_src && \
 "$GIT" clone https://github.com/mpark/variant.git \
        --depth=1 --branch "$MPARK_VARIANT_TAG" mpark-variant  && \
 mkdir -p mpark-variant/build  && \
 cd mpark-variant/build   && \
 "$CMAKE" .. -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"   && \
 "$MAKE" install)



#
# Download & install gulrak/filesystem in local deps_* dir
#

(cd deps_src && \
 "$GIT" clone https://github.com/gulrak/filesystem.git \
        --depth=1 --branch "$GULRAK_FILESYSTEM_TAG" gulrak-filesystem  && \
 mkdir -p gulrak-filesystem/build  && \
 cd gulrak-filesystem/build   && \
 "$CMAKE" .. -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
          -DGHC_FILESYSTEM_BUILD_TESTING=off  -DGHC_FILESYSTEM_BUILD_EXAMPLES=off \
          -DGHC_FILESYSTEM_WITH_INSTALL=on   && \
 "$MAKE" install)



#
# Download & install Catch2 in local deps_* dir
#

(cd deps_src && \
 "$GIT" clone https://github.com/catchorg/Catch2.git \
        --depth=1 --branch "$CATCH2_TAG" catch2  && \
 mkdir -p catch2/build  && \
 cd catch2/build   && \
 "$CMAKE" .. -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
        -DCATCH_BUILD_TESTING=off -DCATCH_BUILD_EXAMPLES=off \
        -DCATCH_BUILD_EXTRA_TESTS=off  && \
 "$MAKE" install)

