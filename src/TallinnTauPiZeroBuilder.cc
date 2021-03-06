#include "TallinnTauTag/RecoTau/interface/TallinnTauPiZeroBuilder.h"

#include "DataFormats/Candidate/interface/Candidate.h"                      // reco::Candidate::LorentzVector

#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidateAuxFunctions.h" // reco::tau::getPFCands_of_type(), reco::tau::getSumP4()

#include "TString.h"

using namespace reco::tau;

//-------------------------------------------------------------------------------
// CV: function makeFunction copied from
//       RecoTauTag/RecoTau/plugins/RecoTauPiZeroStripPlugin3.cc
namespace 
{
  std::unique_ptr<TFormula> makeFunction(const std::string& functionName, const edm::ParameterSet& pset) 
  {
    TString formula = pset.getParameter<std::string>("function");
    formula = formula.ReplaceAll("pT", "x");
    std::unique_ptr<TFormula> function(new TFormula(functionName.data(), formula.Data()));
    int numParameter = function->GetNpar();
    for ( int idxParameter = 0; idxParameter < numParameter; ++idxParameter ) {
      std::string parameterName = Form("par%i", idxParameter);
      double parameter = pset.getParameter<double>(parameterName);
      function->SetParameter(idxParameter, parameter);
    }
    return function;
  }
} // namespace
//-------------------------------------------------------------------------------

TallinnTauPiZeroBuilder::TallinnTauPiZeroBuilder(const edm::ParameterSet& cfg)
  : inputParticleIds_(cfg.getParameter<std::vector<int>>("stripCandidatesParticleIds"))
  , minStripPt_(cfg.getParameter<double>("minStripPt"))
  , verbosity_(cfg.getParameter<int>("verbosity"))
{
  const edm::ParameterSet& cfg_stripSizeEta = cfg.getParameterSet("stripEtaAssociationDistanceFunc");
  etaAssociationDistance_ = makeFunction("etaAssociationDistance", cfg_stripSizeEta);
  const edm::ParameterSet& cfg_stripSizePhi = cfg.getParameterSet("stripPhiAssociationDistanceFunc");
  phiAssociationDistance_ = makeFunction("phiAssociationDistance", cfg_stripSizePhi);
}
 
TallinnTauPiZeroBuilder::~TallinnTauPiZeroBuilder()
{}

//-------------------------------------------------------------------------------
// CV: code for function setBendCorr copied from
//       RecoTauTag/RecoTau/plugins/RecoTauPiZeroStripPlugin3.cc
namespace 
{
  void setBendCorr(reco::RecoTauPiZero& piZero, 
                   const reco::wrappedPFCandidateCollection& gammas, 
                   const TFormula& etaAssociationDistance, const TFormula& phiAssociationDistance) 
  {
    double bendCorrEta = 0.;
    double bendCorrPhi = 0.;
    double energySum = 0.;
    for ( auto const& gamma : gammas ) 
    {
      bendCorrEta += (gamma.energy() * etaAssociationDistance.Eval(gamma.pt()));
      bendCorrPhi += (gamma.energy() * phiAssociationDistance.Eval(gamma.pt()));
      energySum += gamma.energy();
    }
    if ( energySum > 1.e-2 ) 
    {
      bendCorrEta /= energySum;
      bendCorrPhi /= energySum;
    }
    //std::cout << "piZero pT = " << piZero.pt() << ": bendCorrEta = " << bendCorrEta << ", bendCorrPhi = " << bendCorrPhi << std::endl;
    piZero.setBendCorrEta(bendCorrEta);
    piZero.setBendCorrPhi(bendCorrPhi);
  }
} // namespace
//-------------------------------------------------------------------------------

reco::RecoTauPiZeroCollection 
TallinnTauPiZeroBuilder::operator()(const reco::wrappedPFCandidateCollection& pfCands, const reco::Vertex::Point& primaryVertexPos)
{
  int piZeroCharge = 0;
  reco::wrappedPFCandidateCollection gammas = getPFCands_of_type(pfCands, inputParticleIds_);
  reco::Candidate::LorentzVector piZeroP4 = getSumP4(gammas);
  RecoTauPiZero piZero(piZeroCharge, piZeroP4, primaryVertexPos);
  for ( auto const& gamma : gammas ) 
  {
    piZero.addDaughter(reco::CandidatePtr(gamma.pfCandPtr()));
  }
  setBendCorr(piZero, gammas, *etaAssociationDistance_, *phiAssociationDistance_);
  reco::RecoTauPiZeroCollection piZeros;
  if ( piZero.pt() > minStripPt_ ) 
  {
    piZeros.push_back(piZero);
  }
  return piZeros;
}

namespace 
{
  void
  fillDescriptions_stripAssociationDistanceFunc(edm::ParameterSetDescription& desc)
  {
    desc.add<std::string>("function", "");
    desc.add<double>("par0", 0.);
    desc.add<double>("par1", 0.);
  }
} // namespace

void 
TallinnTauPiZeroBuilder::fillDescriptions(edm::ParameterSetDescription& desc)
{
  desc.add<std::vector<int>>("stripCandidatesParticleIds", std::vector<int>({ 2, 4 })); // e, gamma
  edm::ParameterSetDescription desc_stripEtaAssociationDistanceFunc;
  fillDescriptions_stripAssociationDistanceFunc(desc_stripEtaAssociationDistanceFunc);
  desc.add<edm::ParameterSetDescription>("stripEtaAssociationDistanceFunc", desc_stripEtaAssociationDistanceFunc);
  edm::ParameterSetDescription desc_stripPhiAssociationDistanceFunc;
  fillDescriptions_stripAssociationDistanceFunc(desc_stripPhiAssociationDistanceFunc);
  desc.add<edm::ParameterSetDescription>("stripPhiAssociationDistanceFunc", desc_stripPhiAssociationDistanceFunc);
  desc.add<double>("minStripPt", 2.5);
  desc.add<int>("verbosity", 0);
}
