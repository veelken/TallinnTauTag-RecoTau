#include "TallinnTauTag/RecoTau/interface/TallinnTauBuilder.h"

#include "DataFormats/TauReco/interface/PFRecoTauChargedHadron.h"           // reco::PFRecoTauChargedHadron
#include "DataFormats/TauReco/interface/PFRecoTauChargedHadronFwd.h"        // reco::PFRecoTauChargedHadronCollection

#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidateAuxFunctions.h" // reco::tau::getPFCands_of_type(), reco::tau::getSumP4()

#include <math.h> // round

using namespace reco::tau;

TallinnTauBuilder::TallinnTauBuilder(const edm::ParameterSet& cfg)
  : signalConeSize_(cfg.getParameter<std::string>("signalConeSize"))
  , chargedHadrParticleIds_(cfg.getParameter<std::vector<int>>("chargedHadrParticleIds"))
  , piZeroBuilder_(cfg.getParameterSet("piZeros"))
{
  verbosity_ = cfg.getParameter<int>("verbosity");
}
 
TallinnTauBuilder::~TallinnTauBuilder()
{}

namespace
{
  enum { kPt, kEt };
  double
  getScalarSum(const reco::wrappedPFCandidateCollection& pfCands, int mode)
  {
    double scalarSum = 0.;
    for ( auto const& pfCand : pfCands )
    {
      if      ( mode == kPt ) scalarSum += pfCand.pt();
      else if ( mode == kEt ) scalarSum += pfCand.et();
      else assert(0);
    }
    return scalarSum;
  }

  reco::PFCandidatePtr
  getLeadingPFCandPtr(const reco::wrappedPFCandidateCollection& pfCands)
  {
    reco::PFCandidatePtr leadingPFCand;
    double leadingPFCandPt = 0.;
    for ( auto const& pfCand : pfCands )
    {
      if ( pfCand.pt() > leadingPFCandPt )
      {
        leadingPFCand = pfCand.pfCandPtr();
        leadingPFCandPt = pfCand.pt();
      }
    }
    return leadingPFCand;
  }

  //-------------------------------------------------------------------------------
  // CV: code of function setBendCorr copied from
  //       RecoTauTag/RecoTau/plugins/RecoTauBuilderCombinatoricPlugin.cc
  double
  square(double x)
  {
    return x*x;
  }

  void
  setBendCorr(reco::PFTau& pfTau)
  {
    double tauEn = pfTau.energy();
    double tauPz = pfTau.pz();
    const double chargedPionMass = 0.13957;  // GeV
    double tauMass = std::max(pfTau.mass(), chargedPionMass);
    double bendCorrMass2 = 0.;
    const std::vector<reco::RecoTauPiZero>& piZeros = pfTau.signalPiZeroCandidates();
    for ( auto const& piZero : piZeros ) 
    {
      double piZeroEn = piZero.energy();
      double piZeroPx = piZero.px();
      double piZeroPy = piZero.py();
      double piZeroPz = piZero.pz();
      double tau_wo_piZeroPx = pfTau.px() - piZeroPx;
      double tau_wo_piZeroPy = pfTau.py() - piZeroPy;
      // CV: Compute effect of varying strip four-vector by eta and phi correction on tau mass
      //    (derrivative of tau mass by strip eta, phi has been computed using Mathematica)
      bendCorrMass2 += square(((piZeroPz * tauEn - piZeroEn * tauPz) / tauMass) * piZero.bendCorrEta());
      bendCorrMass2 += square(((piZeroPy * tau_wo_piZeroPx - piZeroPx * tau_wo_piZeroPy) / tauMass) * piZero.bendCorrPhi());
    }
    pfTau.setBendCorrMass(sqrt(bendCorrMass2));
  }
  //-------------------------------------------------------------------------------

  std::vector<reco::CandidatePtr>
  convert_to_CandidatePtrs(const reco::wrappedPFCandidateCollection& pfCands)
  {
    std::vector<reco::CandidatePtr> cands;
    for ( auto const& pfCand : pfCands )
    {
      cands.push_back(reco::CandidatePtr(pfCand.pfCandPtr()));
    }
    return cands;
  }

  reco::PFRecoTauChargedHadronCollection
  convert_to_PFRecoTauChargedHadrons(const reco::wrappedPFCandidateCollection& pfCands)
  {
    reco::PFRecoTauChargedHadronCollection tauChargedHadrons;
    for ( auto const& pfCand : pfCands )
    {
      reco::PFRecoTauChargedHadron tauChargedHadron(
        pfCand.pfCand().charge(),
        pfCand.p4(),
        pfCand.pfCand().vertex(),
        0,
        true,
        reco::PFRecoTauChargedHadron::PFRecoTauChargedHadronAlgorithm::kChargedPFCandidate
      );
      tauChargedHadrons.push_back(tauChargedHadron);
    }
    return tauChargedHadrons;
  }
} // namespace

reco::PFTau
TallinnTauBuilder::operator()(const reco::PFJetRef& jetRef,
                              const reco::wrappedPFCandidateCollection& signalPFCands,
                              const reco::wrappedPFCandidateCollection& isolationPFCands, 
                              const reco::Vertex::Point& primaryVertexPos)
{
  reco::PFTau pfTau;
  pfTau.setjetRef(edm::RefToBase<reco::Jet>(jetRef));
  reco::Candidate::LorentzVector signalPFCandP4 = getSumP4(signalPFCands);
  pfTau.setP4(signalPFCandP4);
  double signalConeSize = signalConeSize_(pfTau);
  pfTau.setSignalConeSize(signalConeSize);
  reco::RecoTauPiZeroCollection piZeros = piZeroBuilder_(signalPFCands, primaryVertexPos);
  pfTau.setsignalPiZeroCandidates(piZeros);
  double numTracks_float = 0;
  double charge_float = 0.;
  for ( auto const& signalPFCand : signalPFCands )
  {
    if ( signalPFCand.charge() != 0. ) numTracks_float += signalPFCand.enFrac();
    charge_float += signalPFCand.enFrac()*signalPFCand.charge();
  }
  int numTracks = round(numTracks_float);
  reco::PFTau::hadronicDecayMode decayMode = reco::PFTau::kNull;
  if      ( numTracks == 1 && piZeros.size() == 0 ) decayMode = reco::PFTau::kOneProng0PiZero;
  else if ( numTracks == 1 && piZeros.size() >= 1 ) decayMode = reco::PFTau::kOneProng1PiZero;
  else if ( numTracks == 2 && piZeros.size() == 0 ) decayMode = reco::PFTau::kTwoProng0PiZero;
  else if ( numTracks == 2 && piZeros.size() >= 1 ) decayMode = reco::PFTau::kTwoProng1PiZero;
  else if ( numTracks == 3 && piZeros.size() == 0 ) decayMode = reco::PFTau::kThreeProng0PiZero;
  else if ( numTracks == 3 && piZeros.size() >= 1 ) decayMode = reco::PFTau::kThreeProng1PiZero;
  else                                              decayMode = reco::PFTau::kRareDecayMode;
  pfTau.setDecayMode(decayMode);
  int charge = round(charge_float);
  pfTau.setCharge(charge);
  int pdgId = ( charge_float >= 0. ) ? -15 : +15; 
  pfTau.setPdgId(pdgId);
  pfTau.setVertex(primaryVertexPos);
  reco::wrappedPFCandidateCollection signalPFChargedHadrCands = getPFCands_of_type(signalPFCands, chargedHadrParticleIds_);
  reco::wrappedPFCandidateCollection isolationPFChargedHadrCands = getPFCands_of_type(isolationPFCands, chargedHadrParticleIds_);
  reco::wrappedPFCandidateCollection signalPFNeutralHadrCands = getPFCands_of_type(signalPFCands, { reco::PFCandidate::h0 });
  reco::wrappedPFCandidateCollection isolationPFNeutralHadrCands = getPFCands_of_type(isolationPFCands, { reco::PFCandidate::h0 });
  reco::wrappedPFCandidateCollection signalPFGammaCands = getPFCands_of_type(signalPFCands, { reco::PFCandidate::gamma });
  reco::wrappedPFCandidateCollection isolationPFGammaCands = getPFCands_of_type(isolationPFCands, { reco::PFCandidate::gamma });
  double isolationPFChargedHadrCandsPtSum = getScalarSum(isolationPFChargedHadrCands, kPt);
  double isolationPFGammaCandsEtSum = getScalarSum(isolationPFGammaCands, kEt);
  pfTau.setisolationPFChargedHadrCandsPtSum(isolationPFChargedHadrCandsPtSum);
  pfTau.setisolationPFGammaCandsEtSum(isolationPFGammaCandsEtSum);
  double signalPFGammaCandsPtSum = getScalarSum(signalPFGammaCands, kPt);
  double signalPFCandsPtSum = getScalarSum(signalPFCands, kPt);
  double emFraction = ( signalPFGammaCandsPtSum > 0. && signalPFCandsPtSum > 0. ) ? signalPFGammaCandsPtSum/signalPFCandsPtSum : 0.;
  pfTau.setemFraction(emFraction);
  pfTau.setleadCand(getLeadingPFCandPtr(signalPFCands));
  pfTau.setleadChargedHadrCand(getLeadingPFCandPtr(signalPFChargedHadrCands));
  pfTau.setleadNeutralCand(getLeadingPFCandPtr(signalPFGammaCands));
  pfTau.setsignalCands(convert_to_CandidatePtrs(signalPFCands));
  pfTau.setsignalChargedHadrCands(convert_to_CandidatePtrs(signalPFChargedHadrCands));
  pfTau.setSignalTauChargedHadronCandidates(convert_to_PFRecoTauChargedHadrons(signalPFChargedHadrCands));
  pfTau.setsignalNeutrHadrCands(convert_to_CandidatePtrs(signalPFNeutralHadrCands));
  pfTau.setsignalGammaCands(convert_to_CandidatePtrs(signalPFGammaCands));
  pfTau.setisolationCands(convert_to_CandidatePtrs(isolationPFCands));
  pfTau.setisolationChargedHadrCands(convert_to_CandidatePtrs(isolationPFChargedHadrCands));
  pfTau.setIsolationTauChargedHadronCandidates(convert_to_PFRecoTauChargedHadrons(isolationPFChargedHadrCands));
  pfTau.setisolationNeutrHadrCands(convert_to_CandidatePtrs(isolationPFNeutralHadrCands));
  pfTau.setisolationGammaCands(convert_to_CandidatePtrs(isolationPFGammaCands));
  setBendCorr(pfTau);
  return pfTau;
}

void 
TallinnTauBuilder::fillDescriptions(edm::ParameterSetDescription& desc)
{
  desc.add<std::string>("signalConeSize", "max(min(0.1, 3.0/pt()), 0.05)");
  desc.add<std::vector<int>>("chargedHadronParticleIds", { 1, 2, 3 });
  edm::ParameterSetDescription desc_piZeros;
  TallinnTauPiZeroBuilder::fillDescriptions(desc_piZeros);
  desc.add<edm::ParameterSetDescription>("piZeros", desc_piZeros);
}
