# TallinnTauTag-RecoTau
TallinnTauTag/RecoTau package for DNN/GNN-based PFTau reconstruction developed by Tallinn group

# To checkout the code:
git clone https://github.com/veelken/TallinnTauTag-RecoTau $CMSSW_BASE/src/TallinnTauTag/RecoTau

git remote set-url origin  git+ssh://git@github.com/veelken/TallinnTauTag-RecoTau.git

# To produce nanoAOD Ntuples:
Open the file $CMSSW_BASE/src/TallinnTauTag/RecoTau/test/produceAll.sh in your preferred editor.
Set the parameter "mode" to either "dnn" (for the DNN trained by Laurits) or "gnn" (for the GNN trained by Torben) and set the parameter "version" to today's date.

Then open a new screen session, set the CMSSW variables by executing the commands
  source /cvmfs/cms.cern.ch/cmsset_default.sh
  cd $CMSSW_BASE/src/TallinnTauTag/RecoTau/test
  cmsenv
in the screen session and then execute
  source produceAll.sh
This will run for a few hours during which time you can disconnect from the screen session.

