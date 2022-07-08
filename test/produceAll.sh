#!/usr/bin/sh

mode="dnn"
#mode="gnn"

version="2022Jul6"

cmode="classification"

source produceAOD.sh $mode $version $cmode
source produceMiniAOD.sh $mode $version $cmode
source produceNanoAOD.sh $mode $version
