#!/usr/bin/sh

#SBATCH --cpus-per-task 2
#SBATCH --output CONF/slurm-JOB_%A_%a.out
#SBATCH --error CONF/slurm_JOB_%A_%a.err

mode=dnn
cmode='classification'
filepath=FILEPATH
#filepath="file:\/hdfs\/cms\/store\/mc\/Run3Winter21DRMiniAOD\/QCD_Pt-15to7000_TuneCUETP8M1_Flat_14TeV-pythia8\/AODSIM\/112X_mcRun3_TL180fb_Th180fb_realistic_v1-v2\/280000\/ee9df072-9a33-4fbe-be31-e9f037507758.root"
#filepath="file:\/hdfs\/local\/tolange\/step3\/val\/71\/step3_htt.root"
cwd=$(pwd)
#workpath="${cwd}/JOB"
workpath="/scratch/tolange/JOB"
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test
cmsenv
mkdir $workpath
cd $workpath
configFile=produceTallinnTaus_JOB_"$mode"_AOD_cfg.py
sed_command="s/inputFileNames = getInputFileNames(inputFilePath)/inputFileNames = \[\'${filepath}\'\]/; s/##mode/mode/; s/\$mode/$mode/; s/#process.tallinnTaus.mode = cms.string('regression')/process.tallinnTaus.mode = cms.string('$cmode')/"
sed "$sed_command" /home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test/produceTallinnTaus_AOD_cfg.py >& $configFile
logFile=produceTallinnTaus_JOB_"$mode"_AOD.log
cmsRun $configFile >& $logFile
outputFile=produceTallinnTaus_JOB_"$mode"_AODSIM.root
mv produceTallinnTaus_AODSIM.root $outputFile
inputFile=file:${workpath}/produceTallinnTaus_JOB_"$mode"_AODSIM.root
outputFile=file:${workpath}/produceTallinnTaus_JOB_"$mode"_MINIAODSIM.root
configFile=${workpath}/produceTallinnTaus_JOB_"$mode"_MiniAOD_cfg.py
cd /home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test
cmsDriver.py produceTallinnTausMiniAOD --filein $inputFile --fileout $outputFile --mc --eventcontent MINIAODSIM --datatier MINIAODSIM --conditions auto:phase1_2021_realistic --step PAT --nThreads 1 --geometry DB:Extended --era Run3 --python_filename $configFile --no_exec --customise TallinnTauTag/RecoTau/tools/customizeMiniAOD.customizeTallinnTausMiniAOD -n -1
logFile=$workpath/produceTallinnTaus_JOB_"$mode"_MiniAOD.log
sed_command="s/process = customizeTallinnTausMiniAOD(process)/process = customizeTallinnTausMiniAOD(process,'$cmode')/g" 
sed -i "$sed_command" $configFile
cmsRun $configFile >& $logFile
mv $workpath /hdfs/local/tolange/MINIAODFORDEEPTAU/
cd /home/tolange/tauReko/CMSSW_12_1_0/src/TallinnTauTag/RecoTau/test
