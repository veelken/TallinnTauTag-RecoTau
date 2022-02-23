#include "TallinnTauTag/RecoTau/interface/TallinnTauBuilder.h"

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
  std::vector<reco::PFCandidatePtr>
  getPFCands_of_type(const std::vector<reco::PFCandidatePtr>& pfCands, const std::vector<int>& particleIds)
  {
    std::vector<reco::PFCandidatePtr> pfCands_of_type;
    for ( auto const& pfCand : pfCands )
    {
      bool isPFCand_of_type = false;
      for ( auto const& particleId : particleIds )
      {
        if ( pfCand->particleId() == particleId )
        {
          isPFCand_of_type = true;
          break;
        }
      }
      if ( isPFCand_of_type ) pfCands_of_type.push_back(pfCand);
    }
    return pfCands_of_type;
  }

  reco::Candidate::LorentzVector
  getSumP4(const std::vector<reco::PFCandidatePtr>& pfCands)
  {
    reco::Candidate::LorentzVector sumP4;
    for ( auto const& pfCand : pfCands )
    {
      sumP4 += pfCand->p4();
    }
    return sumP4;
  }

  enum { kPt, kEt };
  double
  getScalarSum(const std::vector<reco::PFCandidatePtr>& pfCands, int mode)
  {
    double scalarSum = 0.;
    for ( auto const& pfCand : pfCands )
    {
      if      ( mode == kPt ) scalarSum += pfCand->pt();
      else if ( mode == kEt ) scalarSum += pfCand->et();
      else assert(0);
    }
    return scalarSum;
  }

  reco::PFCandidatePtr
  getLeadingPFCand(const std::vector<reco::PFCandidatePtr>& pfCands)
  {
    reco::PFCandidatePtr leadingPFCand;
    double leadingPFCandPt = 0.;
    for ( auto const& pfCand : pfCands )
    {
      if ( pfCand->pt() > leadingPFCandPt )
      {
        leadingPFCand = pfCand;
        leadingPFCandPt = pfCand->pt();
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
  convert_to_CandidatePtrs(const std::vector<reco::PFCandidatePtr>& pfCands)
  {
    std::vector<reco::CandidatePtr> cands;
    for ( auto const& pfCand : pfCands )
    {
      cands.push_back(reco::CandidatePtr(pfCand));
    }
    return cands;
  }
} // namespace

reco::PFTau
TallinnTauBuilder::operator()(const reco::PFJetRef& jetRef,
                              const std::vector<std::pair<reco::PFCandidatePtr, double>>& signalPFCands_and_EnFracs,
                              const std::vector<reco::PFCandidatePtr>& isolationPFCands, 
                              const reco::Vertex::Point& primaryVertexPos)
{
  reco::PFTau pfTau;
  pfTau.setjetRef(edm::RefToBase<reco::Jet>(jetRef));
  std::vector<reco::PFCandidatePtr> signalPFCands;
  for ( auto const& signalPFCand_and_EnFrac : signalPFCands_and_EnFracs )
  {
    signalPFCands.push_back(signalPFCand_and_EnFrac.first);
  }
  reco::Candidate::LorentzVector signalPFCandP4 = getSumP4(signalPFCands);
  pfTau.setP4(signalPFCandP4);
  double signalConeSize = signalConeSize_(pfTau);
  pfTau.setSignalConeSize(signalConeSize);
  reco::RecoTauPiZeroCollection piZeros = piZeroBuilder_(signalPFCands, primaryVertexPos);
  pfTau.setsignalPiZeroCandidates(piZeros);
  double numTracks_float = 0;
  double charge_float = 0.;
  for ( auto const& signalPFCand_and_EnFrac : signalPFCands_and_EnFracs )
  {
    const reco::PFCandidatePtr& signalPFCand = signalPFCand_and_EnFrac.first;
    double signalPFEnFrac = signalPFCand_and_EnFrac.second;
    if ( signalPFCand->charge() != 0. ) numTracks_float += signalPFEnFrac;
    charge_float += signalPFEnFrac*signalPFCand->charge();
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
  std::vector<reco::PFCandidatePtr> signalPFChargedHadrCands = getPFCands_of_type(signalPFCands, chargedHadrParticleIds_);
  std::vector<reco::PFCandidatePtr> isolationPFChargedHadrCands = getPFCands_of_type(isolationPFCands, chargedHadrParticleIds_);
  std::vector<reco::PFCandidatePtr> signalPFNeutralHadrCands = getPFCands_of_type(signalPFCands, { reco::PFCandidate::h0 });
  std::vector<reco::PFCandidatePtr> isolationPFNeutralHadrCands = getPFCands_of_type(isolationPFCands, { reco::PFCandidate::h0 });
  std::vector<reco::PFCandidatePtr> signalPFGammaCands = getPFCands_of_type(signalPFCands, { reco::PFCandidate::gamma });
  std::vector<reco::PFCandidatePtr> isolationPFGammaCands = getPFCands_of_type(isolationPFCands, { reco::PFCandidate::gamma });
  double isolationPFChargedHadrCandsPtSum = getScalarSum(isolationPFChargedHadrCands, kPt);
  double isolationPFGammaCandsEtSum = getScalarSum(isolationPFGammaCands, kEt);
  pfTau.setisolationPFChargedHadrCandsPtSum(isolationPFChargedHadrCandsPtSum);
  pfTau.setisolationPFGammaCandsEtSum(isolationPFGammaCandsEtSum);
  double signalPFGammaCandsPtSum = getScalarSum(signalPFGammaCands, kPt);
  double signalPFCandsPtSum = getScalarSum(signalPFCands, kPt);
  double emFraction = ( signalPFGammaCandsPtSum > 0. && signalPFCandsPtSum > 0. ) ? signalPFGammaCandsPtSum/signalPFCandsPtSum : 0.;
  pfTau.setemFraction(emFraction);
  reco::PFCandidatePtr leadPFCand = getLeadingPFCand(signalPFCands);
  pfTau.setleadCand(leadPFCand);
  reco::PFCandidatePtr leadPFChargedHadrCand = getLeadingPFCand(signalPFChargedHadrCands);
  pfTau.setleadChargedHadrCand(leadPFChargedHadrCand);
  reco::PFCandidatePtr leadPFGammaCand = getLeadingPFCand(signalPFGammaCands);
  pfTau.setleadNeutralCand(leadPFGammaCand);
  pfTau.setsignalCands(convert_to_CandidatePtrs(signalPFCands));
  pfTau.setsignalChargedHadrCands(convert_to_CandidatePtrs(signalPFChargedHadrCands));
  pfTau.setsignalNeutrHadrCands(convert_to_CandidatePtrs(signalPFNeutralHadrCands));
  pfTau.setsignalGammaCands(convert_to_CandidatePtrs(signalPFGammaCands));
  pfTau.setisolationCands(convert_to_CandidatePtrs(isolationPFCands));
  pfTau.setisolationChargedHadrCands(convert_to_CandidatePtrs(isolationPFChargedHadrCands));
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
