#!/usr/bin/sh

cmsDriver.py produceTallinnTausNANOAOD --filein "file:produceTallinnTaus_ggH_htt_DNN_$version_MINIAODSIM.root" --fileout "file:produceTallinnTaus_ggH_htt_DNN_$version_NANOAODSIM.root" --mc --eventcontent NANOAODSIM --datatier NANOAODSIM --conditions auto:phase1_2021_realistic --step NANO --nThreads 1 --era Run3 --python_filename produceTallinnTaus_ggH_htt_DNN_nanoAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeNanoAOD.customizeTallinnTausNanoAOD -n -1
rm produceTallinnTaus_ggH_htt_DNN_nanoAOD.log
cmsRun produceTallinnTaus_ggH_htt_DNN_nanoAOD_cfg.py >& produceTallinnTaus_ggH_htt_DNN_nanoAOD.log

cmsDriver.py produceTallinnTausNANOAOD --filein "file:produceTallinnTaus_qcd_DNN_$version_MINIAODSIM.root" --fileout "file:produceTallinnTaus_qcd_DNN_$version_NANOAODSIM.root" --mc --eventcontent NANOAODSIM --datatier NANOAODSIM --conditions auto:phase1_2021_realistic --step NANO --nThreads 1 --era Run3 --python_filename produceTallinnTaus_qcd_DNN_nanoAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeNanoAOD.customizeTallinnTausNanoAOD -n -1
rm produceTallinnTaus_qcd_DNN_nanoAOD.log
cmsRun produceTallinnTaus_qcd_DNN_nanoAOD_cfg.py >& produceTallinnTaus_qcd_DNN_nanoAOD.log

