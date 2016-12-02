
OUTPUTDIRECTORY=codebrowser
DATADIRECTORY=$OUTPUTDIRECTORY/data
BUILDIRECTORY=$PWD
VERSION=`git describe --always --tags`


cmake . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry

typeset WOBOC=~/Downloads/woboq/woboq_codebrowser-2.0.1

rm -rf $OUTPUTDIRECTORY
#$WOBOC/generator/codebrowser_generator -b $BUILDIRECTORY -a -o $OUTPUTDIRECTORY -p codebrowser:$BUILDIRECTORY:$VERSION
$WOBOC/generator/codebrowser_generator -b $BUILDIRECTORY -a -o $OUTPUTDIRECTORY -p starcry:$BUILDIRECTORY:$VERSION
$WOBOC/indexgenerator/codebrowser_indexgenerator $OUTPUTDIRECTORY
cp -rv $WOBOC/data $DATADIRECTORY


