import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.producersLayer1.tauProducer_cfi import singleID, containerID
from PhysicsTools.PatAlgos.tools.helpers import cloneProcessingSnippet
from PhysicsTools.PatAlgos.tools.helpers import massSearchReplaceAnyInputTag
from PhysicsTools.PatAlgos.slimming.slimmedTaus_cfi import slimmedTaus

def customizeTallinnTausMiniAOD(process):
    process.PATTauSequence = cms.Sequence(process.makePatTaus + process.selectedPatTaus)
    process.PATTauSequenceHPS = cloneProcessingSnippet(process, process.PATTauSequence, "HPS", addToTask = True)
    process.selectedPatTausHPS.cut = cms.string("")
    containerID(process.patTausHPS.tauIDSources, 'hpsPFTauBasicDiscriminators', "IDWPdefinitions", [
        [ 'byLooseCombinedIsolationDeltaBetaCorr3Hits', "ByLooseCombinedIsolationDBSumPtCorr3Hits" ],
        [ 'byMediumCombinedIsolationDeltaBetaCorr3Hits', "ByMediumCombinedIsolationDBSumPtCorr3Hits" ],
        [ 'byTightCombinedIsolationDeltaBetaCorr3Hits', "ByTightCombinedIsolationDBSumPtCorr3Hits" ],
        [ 'byLooseChargedIsolation', "ByLooseChargedIsolation" ],
        [ 'byMediumChargedIsolation', "ByMediumChargedIsolation" ],
        [ 'byTightChargedIsolation', "ByTightChargedIsolation" ],
        [ 'byPhotonPtSumOutsideSignalCone', "ByPhotonPtSumOutsideSignalCone" ]
    ])
    process.PATTauSequenceTallinn = cloneProcessingSnippet(process, process.PATTauSequence, "Tallinn", addToTask = True)
    massSearchReplaceAnyInputTag(process.PATTauSequenceTallinn, 'hpsPFTauProducer', 'tallinnTaus')
    process.patTausTallinn.tauIDSources = cms.PSet()
    singleID(process.patTausTallinn.tauIDSources, 'tallinnTauDiscriminationByDecayModeFindingNewDMs', "decayModeFindingNewDMs")
    containerID(process.patTausTallinn.tauIDSources, 'tallinnTauBasicDiscriminators', "IDdefinitions", [
        [ 'chargedIsoPtSum', "ChargedIsoPtSum" ],
        [ 'neutralIsoPtSum', "NeutralIsoPtSum" ],
        [ 'puCorrPtSum', "PUcorrPtSum" ],
        [ 'neutralIsoPtSumWeight', "NeutralIsoPtSumWeight" ],
        [ 'footprintCorrection', "TauFootprintCorrection" ],
        [ 'photonPtSumOutsideSignalCone', "PhotonPtSumOutsideSignalCone" ],
        [ 'byCombinedIsolationDeltaBetaCorrRaw3Hits', "ByRawCombinedIsolationDBSumPtCorr3Hits" ]
    ])
    containerID(process.patTausTallinn.tauIDSources, 'tallinnTauBasicDiscriminators', "IDWPdefinitions", [
        [ 'byLooseCombinedIsolationDeltaBetaCorr3Hits', "ByLooseCombinedIsolationDBSumPtCorr3Hits" ],
        [ 'byMediumCombinedIsolationDeltaBetaCorr3Hits', "ByMediumCombinedIsolationDBSumPtCorr3Hits" ],
        [ 'byTightCombinedIsolationDeltaBetaCorr3Hits', "ByTightCombinedIsolationDBSumPtCorr3Hits" ],
        [ 'byLooseChargedIsolation', "ByLooseChargedIsolation" ],
        [ 'byMediumChargedIsolation', "ByMediumChargedIsolation" ],
        [ 'byTightChargedIsolation', "ByTightChargedIsolation" ],
        [ 'byPhotonPtSumOutsideSignalCone', "ByPhotonPtSumOutsideSignalCone" ]
    ])
    process.patTausTallinn.tauTransverseImpactParameterSource = cms.InputTag("")
    process.selectedPatTausTallinn.cut = cms.string("")
    myOutputCommands = process.MINIAODSIMoutput.outputCommands
    myOutputCommands.append("keep *_patTaus_*_*")
    myOutputCommands.append("keep *_patTausHPS_*_*")
    myOutputCommands.append("keep *_patTausTallinn_*_*")
    myOutputCommands.append("keep *_tallinnTaus_*_*")
    myOutputCommands.append("keep *_particleFlow_*_*")
    myOutputCommands.append("keep *_ak4PFJets_*_*")
    myOutputCommands.append("keep *_gtStage2Digis_*_*")
    process.MINIAODSIMoutput.outputCommands = myOutputCommands
    return process
