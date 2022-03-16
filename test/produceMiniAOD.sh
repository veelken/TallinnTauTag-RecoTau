#!/usr/bin/sh

version=$1
echo "produceMiniAOD.sh: version = '$version'"

inputFile=file:produceTallinnTaus_ggH_htt_DNN_"$version"_AODSIM.root
echo "produceMiniAOD.sh: inputFile for process 'ggH_htt' = '$inputFile'"
outputFile=file:produceTallinnTaus_ggH_htt_DNN_"$version"_MINIAODSIM.root
echo "produceMiniAOD.sh: outputFile for process 'ggH_htt' = '$outputFile'"
cmsDriver.py produceTallinnTausMiniAOD --filein $inputFile --fileout $outputFile --mc --eventcontent MINIAODSIM --datatier MINIAODSIM --conditions auto:phase1_2021_realistic --step PAT --nThreads 1 --geometry DB:Extended --era Run3 --python_filename produceTallinnTaus_ggH_htt_DNN_MiniAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeMiniAOD.customizeTallinnTausMiniAOD -n -1
rm produceTallinnTaus_ggH_htt_DNN_MiniAOD.log
cmsRun produceTallinnTaus_ggH_htt_DNN_MiniAOD_cfg.py >& produceTallinnTaus_ggH_htt_DNN_MiniAOD.log

inputFile=file:produceTallinnTaus_qcd_DNN_"$version"_AODSIM.root
echo "produceMiniAOD.sh: inputFile for process 'qcd' = '$inputFile'"
outputFile=file:produceTallinnTaus_qcd_DNN_"$version"_MINIAODSIM.root
echo "produceMiniAOD.sh: outputFile for process 'qcd' = '$outputFile'"
cmsDriver.py produceTallinnTausMiniAOD --filein $inputFile --fileout $outputFile --mc --eventcontent MINIAODSIM --datatier MINIAODSIM --conditions auto:phase1_2021_realistic --step PAT --nThreads 1 --geometry DB:Extended --era Run3 --python_filename produceTallinnTaus_qcd_DNN_MiniAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeMiniAOD.customizeTallinnTausMiniAOD -n -1
rm produceTallinnTaus_qcd_DNN_MiniAOD.log
cmsRun produceTallinnTaus_qcd_DNN_MiniAOD_cfg.py >& produceTallinnTaus_qcd_DNN_MiniAOD.log


