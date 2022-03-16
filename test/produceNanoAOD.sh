#!/usr/bin/sh

version=$1
echo "produceNanoAOD.sh: version = '$version'"

inputFile=file:produceTallinnTaus_ggH_htt_DNN_"$version"_MINIAODSIM.root
echo "produceNanoAOD.sh: inputFile for process 'ggH_htt' = '$inputFile'"
outputFile=file:produceTallinnTaus_ggH_htt_DNN_"$version"_NANOAODSIM.root
echo "produceNanoAOD.sh: outputFile for process 'ggH_htt' = '$outputFile'"
cmsDriver.py produceTallinnTausNANOAOD --filein $inputFile --fileout $outputFile --mc --eventcontent NANOAODSIM --datatier NANOAODSIM --conditions auto:phase1_2021_realistic --step NANO --nThreads 1 --era Run3 --python_filename produceTallinnTaus_ggH_htt_DNN_nanoAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeNanoAOD.customizeTallinnTausNanoAOD -n -1
rm produceTallinnTaus_ggH_htt_DNN_nanoAOD.log
cmsRun produceTallinnTaus_ggH_htt_DNN_nanoAOD_cfg.py >& produceTallinnTaus_ggH_htt_DNN_nanoAOD.log

inputFile=file:produceTallinnTaus_qcd_DNN_"$version"_MINIAODSIM.root
echo "produceNanoAOD.sh: inputFile for process 'qcd' = '$inputFile'"
outputFile=file:produceTallinnTaus_qcd_DNN_"$version"_NANOAODSIM.root
echo "produceNanoAOD.sh: outputFile for process 'qcd' = '$outputFile'"
cmsDriver.py produceTallinnTausNANOAOD --filein $inputFile --fileout $outputFile --mc --eventcontent NANOAODSIM --datatier NANOAODSIM --conditions auto:phase1_2021_realistic --step NANO --nThreads 1 --era Run3 --python_filename produceTallinnTaus_qcd_DNN_nanoAOD_cfg.py --no_exec --customise TallinnTauTag/RecoTau/tools/customizeNanoAOD.customizeTallinnTausNanoAOD -n -1
rm produceTallinnTaus_qcd_DNN_nanoAOD.log
cmsRun produceTallinnTaus_qcd_DNN_nanoAOD_cfg.py >& produceTallinnTaus_qcd_DNN_nanoAOD.log

