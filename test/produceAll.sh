#!/usr/bin/sh

mode = "dnn"
#mode = "gnn"

version="2022Mar15"

source produceAOD.sh $mode $version
source produceMiniAOD.sh $version
source produceNanoAOD.sh $version
