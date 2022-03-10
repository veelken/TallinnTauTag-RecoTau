import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.selectionLayer1.tauSelector_cfi import selectedPatTaus
from PhysicsTools.NanoAOD.common_cff import Var, P4Vars
from PhysicsTools.NanoAOD.nano_cff import linkedObjects
from PhysicsTools.NanoAOD.taus_cff import _tauId3WPMask, tausMCMatchLepTauForTable, tausMCMatchHadTauForTable, tauMCTable

def customizeTallinnTausNanoAOD(process):
    process.finalTausHPS = selectedPatTaus.clone(
        src = 'patTausHPS',
        cut = "pt > 18 && abs(eta) < 2.4 && tauID('decayModeFindingNewDMs')"
    )
    process.linkedObjects.taus = cms.InputTag('finalTausHPS')
    process.tauTable.variables.idIso = _tauId3WPMask("by%sCombinedIsolationDeltaBetaCorr3Hits", "HPS combined isolation")
    process.tauTable.variables.idChargedIso = _tauId3WPMask("by%sChargedIsolation", "HPS charged isolation")
    process.finalTausTallinn = selectedPatTaus.clone(
        src = 'patTausTallinn',
        cut = "pt > 18 && abs(eta) < 2.4 && tauID('decayModeFindingNewDMs')"
    )
    process.linkedObjectsTallinn = linkedObjects.clone(
        taus = 'finalTausTallinn'
    )
    process.tallinnTauTable = cms.EDProducer("SimpleCandidateFlatTableProducer",
        #src = cms.InputTag('finalTausTallinn'),
        src = cms.InputTag('linkedObjectsTallinn:taus'),
        cut = cms.string(""),
        name = cms.string("tallinnTau"),
        doc = cms.string("patTausTallinn after basic selection (" + process.finalTausTallinn.cut.value()+")"),
        singleton = cms.bool(False),
        extension = cms.bool(False),
        variables = cms.PSet(
            P4Vars,
            charge = Var("charge", int, doc="electric charge"),
            jetIdx = Var("?hasUserCand('jet')?userCand('jet').key():-1", int, doc="index of the associated jet (-1 if none)"),
            decayMode = Var("decayMode()", int),
            leadTkPtOverTauPt = Var("leadChargedHadrCand.pt/pt", float, doc="pt of the leading track divided by tau pt", precision=10),
            leadTkDeltaEta = Var("leadChargedHadrCand.eta - eta", float, doc="eta of the leading track, minus tau eta", precision=8),
            leadTkDeltaPhi = Var("deltaPhi(leadChargedHadrCand.phi, phi)", float, doc="phi of the leading track, minus tau phi", precision=8),
            rawIso = Var("tauID('byCombinedIsolationDeltaBetaCorrRaw3Hits')", float, doc = "combined isolation (deltaBeta corrections)", precision=10),
            chargedIso = Var("tauID('chargedIsoPtSum')", float, doc = "charged isolation", precision=10),
            neutralIso = Var("tauID('neutralIsoPtSum')", float, doc = "neutral (photon) isolation", precision=10),
            puCorr = Var("tauID('puCorrPtSum')", float, doc = "pileup correction", precision=10),
            photonsOutsideSignalCone = Var("tauID('photonPtSumOutsideSignalCone')", float, doc = "sum of photons outside signal cone", precision=10),
            idIso = _tauId3WPMask("by%sCombinedIsolationDeltaBetaCorr3Hits", "HPS combined isolation"),
            idChargedIso = _tauId3WPMask("by%sChargedIsolation", "HPS charged isolation")
        )
    )
    process.tallinnTausMCMatchLepTauForTable = tausMCMatchLepTauForTable.clone(
        src = process.tallinnTauTable.src
    )
    process.tallinnTausMCMatchHadTauForTable = tausMCMatchHadTauForTable.clone(
        src = process.tallinnTauTable.src
    )
    process.tallinnTauMCTable = tauMCTable.clone(
        src = process.tallinnTauTable.src,
        mcMap = 'tallinnTausMCMatchLepTauForTable',
        mcMapVisTau = 'tallinnTausMCMatchHadTauForTable',                         
        objName = process.tallinnTauTable.name
    )
    process.tauTask = cms.Task(
        process.patTauMVAIDsTask,
        process.finalTaus,
        process.finalTausHPS,
        process.finalTausTallinn,
        process.linkedObjectsTallinn
    )
    process.tauTablesTask = cms.Task(
        process.tauTable,
        process.tallinnTauTable
    )
    process.tauMCTask = cms.Task(
        process.genTauTask,
        process.tausMCMatchLepTauForTable,
        process.tausMCMatchHadTauForTable,
        process.tauMCTable,
        process.tallinnTausMCMatchLepTauForTable,
        process.tallinnTausMCMatchHadTauForTable,
        process.tallinnTauMCTable
    )
    return process
