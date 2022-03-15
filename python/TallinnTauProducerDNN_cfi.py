
import FWCore.ParameterSet.Config as cms
from RecoTauTag.RecoTau.PFRecoTauQualityCuts_cfi import PFTauQualityCuts
from RecoTauTag.RecoTau.PFRecoTauPFJetInputs_cfi import PFRecoTauPFJetInputs
from TallinnTauTag.RecoTau.TallinnTauPiZeroBuilder_cfi import tallinnTauPiZeros
'''

Configuration for GNN-based PFTau producer developed by Tallinn group.

Author: Christian Veelken, Tallinn


'''

tallinnTaus = cms.EDProducer("TallinnTauProducer",
    pfJetSrc = PFRecoTauPFJetInputs.inputJetCollection,
    minJetPt = PFRecoTauPFJetInputs.minJetPt,
    maxJetAbsEta = PFRecoTauPFJetInputs.maxJetAbsEta,
    pfCandSrc = cms.InputTag("particleFlow"),
    tfGraph = cms.PSet(
        inputFile = cms.string("TallinnTauTag/RecoTau/data/dnn_2022Mar09_Laurits.pb"),
        graphName = cms.string(""),
        inputLayerNames = cms.vstring("flatten_input"),
        outputLayerName = cms.string("Identity"),
        gnn = cms.bool(False),
        jetInputs = cms.vstring("numPFCands", "pt", "eta", "phi", "mass"),
        pfCandInputs = cms.vstring("pt", "eta", "phi", "particleId", "charge", "dR_jet", "dEta_jet", "dPhi_jet", "dz"),
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
