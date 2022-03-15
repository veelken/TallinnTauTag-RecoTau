
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
        inputFile = cms.string("/home/tolange/tauReko/CMSSW_12_1_0/src/taugnn/realtest_adam_log/graph_e_66.pb"),
        graphName = cms.string(""),
        inputLayerNames = cms.vstring("points", "features", "mask"),
        outputLayerName = cms.string("Identity"),
        gnn = cms.bool(True),
        jetInputs = cms.vstring(""),
        pfCandInputs = cms.vstring("logpt", "logEnergy", "dEta_jet", "dPhi_jet", "particleId", "charge", "dR_jet", "dz", "logabsdz", "logrelpt_jet", "jetpt", "jeteta", "jetphi", "logReldR"),
        pointInputs = cms.vstring("dEta_jet", "dPhi_jet"),
        maskInputs = cms.vstring("logpt"),
        maxNumPFCands = cms.uint32(20),
        jetConstituent_order = cms.vint32(1, 2, 4, 3, 5) # h, e, gamma, mu, h0
    ),
    signalMinPFEnFrac = cms.double(0.),
    isolationMinPFEnFrac = cms.double(0.),
    signalConeSize = cms.string("max(min(0.1, 3.0/pt()), 0.05)"),
    isolationConeSize = PFRecoTauPFJetInputs.isolationConeSize,
    qualityCuts = PFTauQualityCuts,
    chargedHadrParticleIds = cms.vint32(1, 2, 3), # h, e, mu
    piZeros = tallinnTauPiZeros,
    saveInputs = cms.bool(True),
    jsonFileName = cms.string("TallinnTauProducer.json"),
    verbosity = cms.int32(0)
)
