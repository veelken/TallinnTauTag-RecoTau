#!/usr/bin/sh

cmsDriver.py produceTallinnTausNANOAOD --filein file:produceTallinnTaus_MINIAODSIM.root --fileout file:produceTallinnTaus_NANOAODSIM.root --mc --eventcontent NANOAODSIM --datatier NANOAODSIM --conditions auto:phase1_2021_realistic --step NANO --nThreads 1 --era Run3 --python_filename produceTallinnTaus_nanoAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeNanoAOD.customizeTallinnTausNanoAOD -n 10
rm produceTallinnTaus_nanoAOD.log
cmsRun produceTallinnTaus_nanoAOD_cfg.py >& produceTallinnTaus_nanoAOD.log
