#!/usr/bin/sh

version=$1

cmsDriver.py produceTallinnTausMiniAOD --filein "file:produceTallinnTaus_ggH_htt_DNN_$version_AODSIM.root" --fileout "file:produceTallinnTaus_ggH_htt_DNN_$version_MINIAODSIM.root" --mc --eventcontent MINIAODSIM --datatier MINIAODSIM --conditions auto:phase1_2021_realistic --step PAT --nThreads 1 --geometry DB:Extended --era Run3 --python_filename produceTallinnTaus_ggH_htt_DNN_MiniAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeMiniAOD.customizeTallinnTausMiniAOD -n -1
rm produceTallinnTaus_ggH_htt_DNN_MiniAOD.log
cmsRun produceTallinnTaus_ggH_htt_DNN_MiniAOD_cfg.py >& produceTallinnTaus_ggH_htt_DNN_MiniAOD.log

cmsDriver.py produceTallinnTausMiniAOD --filein "file:produceTallinnTaus_qcd_DNN_$version_AODSIM.root" --fileout "file:produceTallinnTaus_qcd_DNN_$version_MINIAODSIM.root" --mc --eventcontent MINIAODSIM --datatier MINIAODSIM --conditions auto:phase1_2021_realistic --step PAT --nThreads 1 --geometry DB:Extended --era Run3 --python_filename produceTallinnTaus_qcd_DNN_MiniAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeMiniAOD.customizeTallinnTausMiniAOD -n -1
rm produceTallinnTaus_qcd_DNN_MiniAOD.log
cmsRun produceTallinnTaus_qcd_DNN_MiniAOD_cfg.py >& produceTallinnTaus_qcd_DNN_MiniAOD.log


