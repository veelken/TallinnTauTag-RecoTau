#include "TallinnTauTag/RecoTau/interface/TallinnTauPiZeroBuilder.h"

#include "DataFormats/Candidate/interface/Candidate.h" // reco::Candidate::LorentzVector

#include "TString.h"

//-------------------------------------------------------------------------------
# CV: function makeFunction copied from
#       RecoTauTag/RecoTau/plugins/RecoTauPiZeroStripPlugin3.cc
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
{
  inputParticleIds_ = pset.getParameter<std::vector<int> >("stripCandidatesParticleIds");
  const edm::ParameterSet& stripSize_eta_pset = pset.getParameterSet("stripEtaAssociationDistanceFunc");
  etaAssociationDistance_ = makeFunction("etaAssociationDistance", stripSize_eta_pset);
  const edm::ParameterSet& stripSize_phi_pset = pset.getParameterSet("stripPhiAssociationDistanceFunc");
  phiAssociationDistance_ = makeFunction("phiAssociationDistance", stripSize_phi_pset);
  verbosity_ = cfg.getParameter<int>("verbosity");
}
 
TallinnTauPiZeroBuilder::~TallinnTauPiZeroBuilder()
{}

//-------------------------------------------------------------------------------
# CV: code for function setBendCorr copied from
#       RecoTauTag/RecoTau/plugins/RecoTauPiZeroStripPlugin3.cc
namespace 
{
  double setBendCorr(reco::RecoTauPiZero& strip, const TFormula& etaAssociationDistance, const TFormula& phiAssociationDistance) 
  {
    double bendCorrEta = 0.;
    double bendCorrPhi = 0.;
    double energySum = 0.;
    for ( auto const& gamma : strip.daughterPtrVector() ) 
    {
      bendCorrEta += (gamma->energy() * etaAssociationDistance.Eval(gamma->pt()));
      bendCorrPhi += (gamma->energy() * phiAssociationDistance.Eval(gamma->pt()));
      energySum += gamma->energy();
    }
    if ( energySum > 1.e-2 ) 
    {
      bendCorrEta /= energySum;
      bendCorrPhi /= energySum;
    }
    //std::cout << "stripPt = " << strip.pt() << ": bendCorrEta = " << bendCorrEta << ", bendCorrPhi = " << bendCorrPhi << std::endl;
    strip.setBendCorrEta(bendCorrEta);
    strip.setBendCorrPhi(bendCorrPhi);
  }
} // namespace
//-------------------------------------------------------------------------------

reco::RecoTauPiZeroCollection 
TallinnTauPiZeroBuilder::operator()(const std::vector<edm::Ptr<reco::PFCandidate>>& pfCands, const reco::Vertex::Point& primaryVertexPos)
{
  int piZeroCharge = 0;
  reco::Candidate::LorentzVector piZeroP4;
  for ( auto const& pfCand : pfCands ) 
  {
    piZeroP4 += pfCand->p4();
  }
  RecoTauPiZero piZero(piZeroCharge, piZeroP4, primaryVertexPos);
  setBendCorr(piZero, *etaAssociationDistance_, *phiAssociationDistance_);
  reco::RecoTauPiZeroCollection piZeros.push_back(piZero);
  return piZeros;
}

namespace 
{
  fillDescriptions_stripAssociationDistanceFunc(edm::ParameterSetDescription& desc)
  {
    desc.add<std::string>("function", "");
    desc.add<double>("par0", 0.);
    desc.add<double>("par1", 0.);
  }
}

void 
TallinnTauPiZeroBuilder::fillDescriptions(edm::ParameterSetDescription& desc)
{
  desc.add<std::vector<int>>("stripCandidatesParticleIds", std::vector<int>({ 2, 4 }));
  edm::ParameterSetDescription desc_stripEtaAssociationDistanceFunc;
  fillDescriptions_stripAssociationDistanceFunc(desc_stripEtaAssociationDistanceFunc);
  desc.add<edm::ParameterSetDescription>("stripEtaAssociationDistanceFunc", desc_stripEtaAssociationDistanceFunc);
  edm::ParameterSetDescription desc_stripPhiAssociationDistanceFunc;
  fillDescriptions_stripAssociationDistanceFunc(desc_stripPhiAssociationDistanceFunc);
  desc.add<edm::ParameterSetDescription>("stripPhiAssociationDistanceFunc", desc_stripPhiAssociationDistanceFunc);
  desc.add<int>("verbosity", 0);
}
