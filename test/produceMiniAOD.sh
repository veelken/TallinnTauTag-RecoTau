#!/usr/bin/sh

mode=$1
echo "produceMiniAOD.sh: mode = '$mode'"

version=$2
echo "produceMiniAOD.sh: version = '$version'"

cmode=$3

inputFile=file:produceTallinnTaus_ggH_htt_"$mode"_"$version"_AODSIM.root
echo "produceMiniAOD.sh: inputFile for process 'ggH_htt' = '$inputFile'"
outputFile=file:produceTallinnTaus_ggH_htt_"$mode"_"$version"_MINIAODSIM.root
echo "produceMiniAOD.sh: outputFile for process 'ggH_htt' = '$outputFile'"
configFile=produceTallinnTaus_ggH_htt_"$mode"_MiniAOD_cfg.py
cmsDriver.py produceTallinnTausMiniAOD --filein $inputFile --fileout $outputFile --mc --eventcontent MINIAODSIM --datatier MINIAODSIM --conditions auto:phase1_2021_realistic --step PAT --nThreads 1 --geometry DB:Extended --era Run3 --python_filename $configFile --no_exec --customise TallinnTauTag/RecoTau/tools/customizeMiniAOD.customizeTallinnTausMiniAOD -n -1
logFile=produceTallinnTaus_ggH_htt_"$mode"_MiniAOD.log
rm $logFile
sed_command="s/process = customizeTallinnTausMiniAOD(process)/process = customizeTallinnTausMiniAOD(process,'$cmode')/g" 
sed -i "$sed_command" $configFile
cmsRun $configFile >& $logFile

inputFile=file:produceTallinnTaus_qcd_"$mode"_"$version"_AODSIM.root
echo "produceMiniAOD.sh: inputFile for process 'qcd' = '$inputFile'"
outputFile=file:produceTallinnTaus_qcd_"$mode"_"$version"_MINIAODSIM.root
echo "produceMiniAOD.sh: outputFile for process 'qcd' = '$outputFile'"
configFile=produceTallinnTaus_qcd_"$mode"_MiniAOD_cfg.py
cmsDriver.py produceTallinnTausMiniAOD --filein $inputFile --fileout $outputFile --mc --eventcontent MINIAODSIM --datatier MINIAODSIM --conditions auto:phase1_2021_realistic --step PAT --nThreads 1 --geometry DB:Extended --era Run3 --python_filename $configFile --no_exec --customise TallinnTauTag/RecoTau/tools/customizeMiniAOD.customizeTallinnTausMiniAOD -n -1
logFile=produceTallinnTaus_qcd_"$mode"_MiniAOD.log
rm $logFile
sed_command="s/process = customizeTallinnTausMiniAOD(process)/process = customizeTallinnTausMiniAOD(process,'$cmode')/g" 
sed -i "$sed_command" $configFile
cmsRun $configFile >& $logFile


