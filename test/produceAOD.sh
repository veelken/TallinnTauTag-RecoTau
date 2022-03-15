#!/usr/bin/sh

inputFilePath_ggH_htt="\/hdfs\/local\/tolange\/step3\/val\/"
inputFilePath_qcd="\/hdfs\/cms\/store\/mc\/Run3Winter21DRMiniAOD\/QCD_Pt-15to7000_TuneCUETP8M1_Flat_14TeV-pythia8\/AODSIM\/112X_mcRun3_TL180fb_Th180fb_realistic_v1-v2\/"

version=$1

rm produceTallinnTaus_ggH_htt_DNN_AOD_cfg.py
sed_command="s/##inputFilePath/inputFilePath/; s/\$inputFilePath/$inputFilePath_ggH_htt/; s/##mode/mode/; s/\$mode/dnn/"
sed "$sed_command" produceTallinnTaus_AOD_cfg.py >& produceTallinnTaus_ggH_htt_DNN_AOD_cfg.py

rm produceTallinnTaus_ggH_htt_DNN_AOD.log
#cmsRun produceTallinnTaus_ggH_htt_DNN_AOD_cfg.py >& produceTallinnTaus_ggH_htt_DNN_AOD.log
#mv produceTallinnTaus_AODSIM.root "produceTallinnTaus_ggH_htt_DNN_$version_AODSIM.root"

#rm produceTallinnTaus_qcd_DNN_AOD_cfg.py
#sed_command="s/##inputFilePath/inputFilePath/; s/\$inputFilePath/$inputFilePath_qcd/; s/##mode/mode/; s/\$mode/dnn/"
#sed "$sed_command" produceTallinnTaus_AOD_cfg.py >& produceTallinnTaus_qcd_DNN_AOD_cfg.py

#rm produceTallinnTaus_qcd_DNN_AOD.log
#cmsRun produceTallinnTaus_qcd_DNN_AOD_cfg.py >& produceTallinnTaus_qcd_DNN_AOD.log
#mv produceTallinnTaus_AODSIM.root "produceTallinnTaus_qcd_DNN_$version_AODSIM.root"

