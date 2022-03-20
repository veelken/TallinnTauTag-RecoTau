#!/usr/bin/sh

mode=$1
echo "produceNanoAOD.sh: mode = '$mode'"

version=$2
echo "produceNanoAOD.sh: version = '$version'"

inputFile=file:produceTallinnTaus_ggH_htt_"$mode"_"$version"_MINIAODSIM.root
echo "produceNanoAOD.sh: inputFile for process 'ggH_htt' = '$inputFile'"
outputFile=file:produceTallinnTaus_ggH_htt_"$mode"_"$version"_NANOAODSIM.root
echo "produceNanoAOD.sh: outputFile for process 'ggH_htt' = '$outputFile'"
configFile=produceTallinnTaus_ggH_htt_"$mode"_nanoAOD_cfg.py
cmsDriver.py produceTallinnTausNANOAOD --filein $inputFile --fileout $outputFile --mc --eventcontent NANOAODSIM --datatier NANOAODSIM --conditions auto:phase1_2021_realistic --step NANO --nThreads 1 --era Run3 --python_filename $configFile --no_exec --customise TallinnTauTag/RecoTau/tools/customizeNanoAOD.customizeTallinnTausNanoAOD -n -1
logFile=produceTallinnTaus_ggH_htt_"$mode"_nanoAOD.log
rm $logFile
cmsRun $configFile >& $logFile

inputFile=file:produceTallinnTaus_qcd_"$mode"_"$version"_MINIAODSIM.root
echo "produceNanoAOD.sh: inputFile for process 'qcd' = '$inputFile'"
outputFile=file:produceTallinnTaus_qcd_"$mode"_"$version"_NANOAODSIM.root
echo "produceNanoAOD.sh: outputFile for process 'qcd' = '$outputFile'"
configFile=produceTallinnTaus_qcd_"$mode"_nanoAOD_cfg.py
cmsDriver.py produceTallinnTausNANOAOD --filein $inputFile --fileout $outputFile --mc --eventcontent NANOAODSIM --datatier NANOAODSIM --conditions auto:phase1_2021_realistic --step NANO --nThreads 1 --era Run3 --python_filename $configFile --no_exec --customise TallinnTauTag/RecoTau/tools/customizeNanoAOD.customizeTallinnTausNanoAOD -n -1
logFile=produceTallinnTaus_qcd_"$mode"_nanoAOD.log
rm $logFile
cmsRun $configFile >& $logFile

