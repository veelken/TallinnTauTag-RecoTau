#!/usr/bin/sh

#mode="dnn"
mode="gnn"

version="2022Mar20"

source produceAOD.sh $mode $version
source produceMiniAOD.sh $mode $version
source produceNanoAOD.sh $mode $version
