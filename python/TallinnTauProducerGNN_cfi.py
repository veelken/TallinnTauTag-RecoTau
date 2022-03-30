
import FWCore.ParameterSet.Config as cms
from RecoTauTag.RecoTau.PFRecoTauQualityCuts_cfi import PFTauQualityCuts
from RecoTauTag.RecoTau.PFRecoTauPFJetInputs_cfi import PFRecoTauPFJetInputs
from TallinnTauTag.RecoTau.TallinnTauPiZeroBuilder_cfi import tallinnTauPiZeros
'''

Configuration for DNN/GNN-based PFTau producer developed by Tallinn group.

Author: Christian Veelken, Tallinn


'''

tallinnTaus = cms.EDProducer("TallinnTauProducer",
    pfJetSrc = PFRecoTauPFJetInputs.inputJetCollection,
    minJetPt = PFRecoTauPFJetInputs.minJetPt,
    maxJetAbsEta = PFRecoTauPFJetInputs.maxJetAbsEta,
    pfCandSrc = cms.InputTag("particleFlow"),
    tfGraph = cms.PSet(
        inputFile = cms.string("TallinnTauTag/RecoTau/data/gnn_2022Mar10_Torben.pb"),
        graphName = cms.string(""),
        inputLayerNames = cms.vstring("points", "features", "mask"),
        outputLayerName = cms.string("Identity"),
        gnn = cms.bool(True),
        jetInputs = cms.vstring(""),
        pfCandInputs = cms.vstring("log(pt)", "log(energy)", "dEta_pfCandSum", "dPhi_pfCandSum", "particleId", "charge", "dR_pfCandSum", "dz", "log(abs(dz))", "log(pfCandPt/jetPt)", "pfCandSumPt", "pfCandSumEta", "pfCandSumPhi", "log(dR_pfCandSum)"),
        pointInputs = cms.vstring("dEta_pfCandSum", "dPhi_pfCandSum"),
        maskInputs = cms.vstring("log(pt)"),
        maxNumPFCands = cms.uint32(20),
        jetConstituent_order = cms.vint32(1, 2, 4, 3, 5) # h, e, gamma, mu, h0
    ),
    signalMinPFEnFrac = cms.double(0.),
    isolationMinPFEnFrac = cms.double(0.),
    # CV: Valid settings for the mode parameter are "regression" and "classification".
    #     If the mode is set to regression, each PFCandidate constituent of the jet that seeds the tau reconstruction 
    #     is split into "signal" and "isolation" candidates, depending on the signalPFEnFrac value returned for this PFCandidate by the DNN.
    #     If the mode is set to classification, the PFCandidate is taken as signal candidate of the tau if the signalPFEnFrac value 
    #     exceeds the value of the signalMinPFEnFrac parameter and contributes to the isolation of the tau if the signalPFEnFrac value 
    #     exceeds the value of the isolationMinPFEnFrac parameter.
    mode = cms.string("regression"),
    signalConeSize = cms.string("max(min(0.1, 3.0/pt()), 0.05)"),
    isolationConeSize = PFRecoTauPFJetInputs.isolationConeSize,
    qualityCuts = PFTauQualityCuts,
    chargedHadrParticleIds = cms.vint32(1, 2, 3), # h, e, mu
    piZeros = tallinnTauPiZeros,
    saveInputs = cms.bool(True),
    jsonFileName = cms.string("TallinnTauProducer.json"),
    verbosity = cms.int32(0)
)
tallinnTaus.qualityCuts.isolationQualityCuts.minTrackHits = cms.uint32(3)
tallinnTaus.qualityCuts.isolationQualityCuts.minGammaEt = cms.double(1.0)
tallinnTaus.qualityCuts.isolationQualityCuts.minTrackPt = cms.double(0.5)
