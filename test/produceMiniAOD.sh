#!/usr/bin/sh

cmsDriver.py produceTallinnTausMiniAOD --filein file:produceTallinnTaus_AODSIM.root --fileout file:produceTallinnTaus_MINIAODSIM.root --mc --eventcontent MINIAODSIM --datatier MINIAODSIM --conditions auto:phase1_2021_realistic --step PAT --nThreads 1 --geometry DB:Extended --era Run3 --python_filename produceTallinnTaus_MiniAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeMiniAOD.customizeTallinnTausMiniAOD -n 10
rm produceTallinnTaus_MiniAOD.log
cmsRun produceTallinnTaus_MiniAOD_cfg.py >& produceTallinnTaus_MiniAOD.log
