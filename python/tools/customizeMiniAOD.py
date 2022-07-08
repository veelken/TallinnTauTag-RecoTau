import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.producersLayer1.tauProducer_cfi import singleID, containerID
from PhysicsTools.PatAlgos.tools.helpers import cloneProcessingSnippet
from PhysicsTools.PatAlgos.tools.helpers import massSearchReplaceAnyInputTag
from PhysicsTools.PatAlgos.slimming.slimmedTaus_cfi import slimmedTaus

def customizeTallinnTausMiniAOD(process, tallinTauBmode=None):
    process.PATTauSequence = cms.Sequence(process.makePatTaus + process.selectedPatTaus)
    process.PATTauSequenceHPS = cloneProcessingSnippet(process, process.PATTauSequence, "HPS", addToTask = True)
    process.selectedPatTausHPS.cut = cms.string("")
    singleID(process.patTausHPS.tauIDSources, 'hpsPFTauDiscriminationByLeadingTrackFinding', "leadingTrackFinding")
    singleID(process.patTausHPS.tauIDSources, 'hpsPFTauDiscriminationByLeadingTrackPtCut', "leadingTrackPtCut")
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
    singleID(process.patTausTallinn.tauIDSources, 'tallinnTauDiscriminationByDecayModeFinding', "decayModeFinding")
    singleID(process.patTausTallinn.tauIDSources, 'tallinnTauDiscriminationByDecayModeFindingNewDMs', "decayModeFindingNewDMs")
    singleID(process.patTausTallinn.tauIDSources, 'tallinnTauDiscriminationByLeadingTrackFinding', "leadingTrackFinding")
    singleID(process.patTausTallinn.tauIDSources, 'tallinnTauDiscriminationByLeadingTrackPtCut', "leadingTrackPtCut")
    
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
    containerID(process.patTausTallinn.tauIDSources, "hpsPFTauBasicDiscriminatorsdR03", "IDdefinitions", [
     ["chargedIsoPtSumdR03", "ChargedIsoPtSumdR03"],
     ["neutralIsoPtSumdR03", "NeutralIsoPtSumdR03"],
     ["neutralIsoPtSumWeightdR03", "NeutralIsoPtSumWeightdR03"],
     ["footprintCorrectiondR03", "TauFootprintCorrectiondR03"],
     ["photonPtSumOutsideSignalConedR03", "PhotonPtSumOutsideSignalConedR03"]
    ])
    process.patTausTallinn.tauTransverseImpactParameterSource = cms.InputTag("tallinnTauTransverseImpactParameters")
    process.selectedPatTausTallinn.cut = cms.string("")
    process.slimmedTallinnTaus = cms.EDProducer("PATTauSlimmer",
                                 src = cms.InputTag("selectedPatTausTallinn"),
                                 linkToPackedPFCandidates = cms.bool(True),
                                 dropPiZeroRefs = cms.bool(True),
                                 dropTauChargedHadronRefs = cms.bool(True),
                                 dropPFSpecific = cms.bool(True),
                                 packedPFCandidates = cms.InputTag("packedPFCandidates"),
                                 modifyTaus = cms.bool(True),
                                 modifierConfig = cms.PSet( modifications = cms.VPSet() ),
                                 linkToLostTracks = cms.bool(True),
                                 lostTracks = cms.InputTag("lostTracks")
                             )
    if tallinTauBmode == "classification":
        process.patTask.add(process.slimmedTallinnTaus)

    #----------------------------------------------------------------------------
    # CV: add collection 'rekeyLowPtGsfElectronSeedValueMaps' to avoid exception in PATElectronProducer
    process.load("RecoEgamma.EgammaElectronProducers.lowPtGsfElectronSeedValueMaps_cff")
    process.makePatElectronsTask = cms.Task(process.rekeyLowPtGsfElectronSeedValueMaps, process.makePatElectronsTask)
    #----------------------------------------------------------------------------
    myOutputCommands = process.MINIAODSIMoutput.outputCommands
    myOutputCommands.append("keep *_patTaus_*_*")
    myOutputCommands.append("keep *_patTausHPS_*_*")
    myOutputCommands.append("keep *_patTausTallinn_*_*")
    myOutputCommands.append("keep *_tallinnTaus_*_*")
    myOutputCommands.append("keep *_slimmed*_*_*")
    myOutputCommands.append("keep *_particleFlow_*_*")
    myOutputCommands.append("keep *_ak4PFJets_*_*")
    myOutputCommands.append("keep *_gtStage2Digis_*_*")
    process.MINIAODSIMoutput.outputCommands = myOutputCommands
    #----------------------------------------------------------------------------
    # CV: the following line is necessary,
    #     because some of the reco::Track collections used as input to the PF reconstruction
    #     have been dropped from the event content
    process.options.SkipEvent = cms.untracked.vstring('ProductNotFound')
    #----------------------------------------------------------------------------
    return process
