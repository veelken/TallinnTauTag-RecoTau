#!/usr/bin/sh

inputFilePath_ggH_htt="\/hdfs\/local\/tolange\/step3\/val\/"
inputFilePath_qcd="\/hdfs\/cms\/store\/mc\/Run3Winter21DRMiniAOD\/QCD_Pt-15to7000_TuneCUETP8M1_Flat_14TeV-pythia8\/AODSIM\/112X_mcRun3_TL180fb_Th180fb_realistic_v1-v2\/"

mode=$1
echo "produceAOD.sh: mode = '$mode'"

version=$2
echo "produceAOD.sh: version = '$version'"

configFile=produceTallinnTaus_ggH_htt_"$mode"_AOD_cfg.py
rm $configFile
sed_command="s/##inputFilePath/inputFilePath/; s/\$inputFilePath/$inputFilePath_ggH_htt/; s/##mode/mode/; s/\$mode/$mode/"
sed "$sed_command" produceTallinnTaus_AOD_cfg.py >& $configFile

logFile=produceTallinnTaus_ggH_htt_"$mode"_AOD.log
rm $logFile
cmsRun $configFile >& $logFile
outputFile=produceTallinnTaus_ggH_htt_"$mode"_"$version"_AODSIM.root
echo "produceAOD.sh: outputFile for process 'ggH_htt' = '$outputFile'"
mv produceTallinnTaus_AODSIM.root $outputFile

configFile=produceTallinnTaus_qcd_"$mode"_AOD_cfg.py
rm $configFile
sed_command="s/##inputFilePath/inputFilePath/; s/\$inputFilePath/$inputFilePath_qcd/; s/##mode/mode/; s/\$mode/$mode/"
sed "$sed_command" produceTallinnTaus_AOD_cfg.py >& $configFile

logFile=produceTallinnTaus_qcd_"$mode"_AOD.log
rm $logFile
cmsRun $configFile >& $logFile
outputFile=produceTallinnTaus_qcd_"$mode"_"$version"_AODSIM.root
echo "produceAOD.sh: outputFile for process 'qcd' = '$outputFile'"
mv produceTallinnTaus_AODSIM.root $outputFile

