#include "TallinnTauTag/RecoTau/plugins/TallinnTauProducer.h"

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h" // edm::ParameterSetDescription
#include "FWCore/Utilities/interface/Exception.h"                  // edm::Exception

#include "DataFormats/TauReco/interface/PFTau.h"                   // reco::PFTau
#include "DataFormats/TauReco/interface/PFTauFwd.h"                // reco::PFTauCollection
#include "DataFormats/TrackReco/interface/Track.h"                 // reco::Track
#include "DataFormats/Math/interface/deltaR.h"                     // reco::deltaR()
#include "DataFormats/Math/interface/deltaPhi.h"                   // reco::deltaPhi()

#include <iostream>                                                // std::cout, std::endl
#include <memory>                                                  // std::make_unique<>
#include <algorithm>                                               // std::sort

using namespace reco::tau;

typedef std::vector<std::string> vstring;

TallinnTauProducer::TallinnTauProducer(const edm::ParameterSet& cfg, const TallinnTauCache* dnn) 
  : moduleLabel_(cfg.getParameter<std::string>("@module_label"))
  , minJetPt_(cfg.getParameter<double>("minJetPt"))
  , maxJetAbsEta_(cfg.getParameter<double>("maxJetAbsEta"))
  , jetInputs_(cfg.getParameter<vstring>("jetInputs"))
  , pfCandInputs_(cfg.getParameter<vstring>("pfCandInputs"))
  , maxNumPFCands_(cfg.getParameter<unsigned>("maxNumPFCands"))
  , dnn_(dnn)
  , signalMinPFEnFrac_(cfg.getParameter<double>("signalMinPFEnFrac"))
  , isolationMinPFEnFrac_(cfg.getParameter<double>("isolationMinPFEnFrac"))
  , isolationConeSize_(cfg.getParameter<double>("isolationConeSize"))
  , signalQualityCuts_(cfg.getParameterSet("qualityCuts").getParameterSet("signalQualityCuts"))
  , vertexAssociator_(cfg.getParameterSet("qualityCuts"), consumesCollector())
  , tauBuilder_(cfg)
  , verbosity_(cfg.getParameter<int>("verbosity"))
{
  std::cout << "<TallinnTauProducer::TallinnTauProducer (moduleLabel = " << moduleLabel_ << ")>:" << std::endl;

  pfJetSrc_ = cfg.getParameter<edm::InputTag>("pfJetSrc");
  pfJetToken_ = consumes<reco::PFJetCollection>(pfJetSrc_);  

  pfCandSrc_ = cfg.getParameter<edm::InputTag>("pfCandSrc");
  pfCandToken_ = consumes<reco::PFCandidateCollection>(pfCandSrc_);

  for ( auto const& jetInput : jetInputs_ )
  {
    jetInputExtractors_[jetInput] = std::make_unique<StringObjectFunction<reco::PFJet>>(jetInput);
  }
  for ( auto const& pfCandInput : pfCandInputs_ )
  {
    pfCandInputExtractors_[pfCandInput] = std::make_unique<StringObjectFunction<reco::PFCandidate>>(pfCandInput);
  }

  num_dnnInputs_ = jetInputs_.size() + pfCandInputs_.size()*maxNumPFCands_;
  dnnInputs_ = std::make_unique<tensorflow::Tensor>(tensorflow::DT_FLOAT, tensorflow::TensorShape{ 1, (long)num_dnnInputs_ });
  dnnInputs_->flat<float>().setZero();

  num_dnnOutputs_ = maxNumPFCands_;

  dnnInputLayerName_ = dnn_->getGraph().node(0).name();
  dnnOutputLayerName_ = dnn_->getGraph().node(dnn_->getGraph().node_size() - 1).name();
  const auto& dnnInputLayer = dnn_->getGraph().node(0).attr().at("shape").shape();
  if ( (size_t)dnnInputLayer.dim(1).size() != num_dnnInputs_ )
    throw cms::Exception("TallinnTauProducer")
      << "Size of DNN input layer = " << dnnInputLayer.dim(1).size() << " does not match number of DNN input variables = " << num_dnnInputs_ << " !!";

  produces<reco::PFTauCollection>();
  produces<reco::PFCandidateCollection>("splittedPFCands");
}

TallinnTauProducer::~TallinnTauProducer()
{}

double 
TallinnTauProducer::compJetInput(const reco::PFJet& pfJet, const std::string& inputVariable, const reco::Track* leadTrack) const
{
  double retVal = 0.;
  if      ( inputVariable == "dR_leadTrack"   ) retVal = deltaR(pfJet.p4(), leadTrack->momentum());
  else if ( inputVariable == "dEta_leadTrack" ) retVal = std::fabs(pfJet.eta() - leadTrack->eta());
  else if ( inputVariable == "dPhi_leadTrack" ) retVal = deltaPhi(pfJet.phi(), leadTrack->phi());
  else
  {
    auto jetInputExtractor = jetInputExtractors_.find(inputVariable);
    assert(jetInputExtractor != jetInputExtractors_.end());
    retVal = (*jetInputExtractor->second)(pfJet);
  }
  return retVal;
}

//-------------------------------------------------------------------------------
// CV: function getTrack copied from
//       RecoTauTag/RecoTau/src/RecoTauQualityCuts.cc
namespace 
{
  const reco::Track* 
  getTrack(const reco::PFCandidate& pfCand) 
  {
    // Get the KF track if it exists. Otherwise, see if PFCandidate has a GSF track.
    if      ( pfCand.trackRef().isNonnull()    ) return pfCand.trackRef().get();
    else if ( pfCand.gsfTrackRef().isNonnull() ) return pfCand.gsfTrackRef().get();
    else                                         return nullptr;
  }
} // namespace
//-------------------------------------------------------------------------------

double 
TallinnTauProducer::compPFCandInput(const reco::PFCandidate& pfCand, 
                                    const std::string& inputVariable, 
                                    const reco::Vertex::Point& primaryVertexPos,
                                    const reco::Track* leadTrack) const
{
  double retVal = 0.;
  if ( inputVariable == "dz" || inputVariable == "dxy" )
  {
    const reco::Track* track = getTrack(pfCand);
    if ( track && leadTrack )
    {
      if      ( inputVariable == "dz"  ) retVal = std::fabs(track->dz(primaryVertexPos) - leadTrack->dz(primaryVertexPos));
      else if ( inputVariable == "dxy" ) retVal = std::fabs(track->dxy(primaryVertexPos));
      else                               assert(0);
    }
  }
  else if ( inputVariable == "dR_leadTrack"   ) retVal = deltaR(pfCand.p4(), leadTrack->momentum());
  else if ( inputVariable == "dEta_leadTrack" ) retVal = std::fabs(pfCand.eta() - leadTrack->eta());
  else if ( inputVariable == "dPhi_leadTrack" ) retVal = deltaPhi(pfCand.phi(), leadTrack->phi());
  else
  {
    auto pfCandInputExtractor = pfCandInputExtractors_.find(inputVariable);
    assert(pfCandInputExtractor != pfCandInputExtractors_.end());
    retVal = (*pfCandInputExtractor->second)(pfCand);
  }
  return retVal;
}

namespace
{
  void
  set_dnnInput(tensorflow::Tensor& dnnInputs, size_t idx, double value)
  {
    dnnInputs.matrix<float>()(0, idx) = value;
  }

  reco::PFCandidate
  clonePFCand(const reco::PFCandidate& pfCand, double pfEnFrac)
  {
    reco::PFCandidate clonedPFCand(pfCand);
    clonedPFCand.setP4(pfEnFrac*pfCand.p4());
    return clonedPFCand;
  }

  bool
  isHigherPt(const reco::PFCandidatePtr& pfCand1, const reco::PFCandidatePtr& pfCand2)
  {
    return pfCand1->pt() > pfCand2->pt();
  }
}

void 
TallinnTauProducer::produce(edm::Event& evt, const edm::EventSetup& es)
{
  std::cout << "<TallinnTauProducer::produce (moduleLabel = " << moduleLabel_ << ")>:" << std::endl;

  std::unique_ptr<reco::PFTauCollection> pfTaus = std::make_unique<reco::PFTauCollection>();
  std::unique_ptr<reco::PFCandidateCollection> splittedPFCands = std::make_unique<reco::PFCandidateCollection>();

  reco::PFCandidateRefProd splittedPFCandsRefProd = evt.getRefBeforePut<reco::PFCandidateCollection>();

  edm::Handle<reco::PFJetCollection> pfJets;
  evt.getByToken(pfJetToken_, pfJets);

  edm::Handle<reco::PFCandidateCollection> pfCands;
  evt.getByToken(pfCandToken_, pfCands);

  size_t numPFJets = pfJets->size();
  for ( size_t idxPFJet = 0; idxPFJet < numPFJets; ++idxPFJet )
  {
    reco::PFJetRef pfJetRef(pfJets, idxPFJet);

    // CV: require that jet that seeds the tau reconstruction passes loose cuts on pT and eta
    if ( !(pfJetRef->pt() > minJetPt_ && std::fabs(pfJetRef->eta()) < maxJetAbsEta_) ) continue;
    std::vector<reco::PFCandidatePtr> pfJetConstituents = pfJetRef->getPFConstituents();
    if ( verbosity_ >= 1 )
    {
      std::cout << "jet #" << pfJetRef.key() << ":" 
                << " pT = " << pfJetRef->pt() << "," 
                << " eta = " << pfJetRef->eta() << "," 
                << " phi = " << pfJetRef->phi() 
                << " (#constituents = " << pfJetConstituents.size() << ")" << std::endl;
    }

    reco::VertexRef primaryVertexRef = vertexAssociator_.associatedVertex(*pfJetRef);
    if ( !primaryVertexRef.get() ) continue;
    reco::Vertex::Point primaryVertexPos = primaryVertexRef->position();
    if ( verbosity_ >= 1 )
    {
      std::cout << "associated primary vertex:" 
                << " x = " << primaryVertexPos.x() << ","
                << " y = " << primaryVertexPos.y() << ","
                << " z = " << primaryVertexPos.z() << std::endl;
    }
    const reco::Track* leadTrack = vertexAssociator_.getLeadTrack(*pfJetRef);
    if ( !leadTrack ) continue;
    if ( verbosity_ >= 1 )
    {
      std::cout << "lead. track:" 
                << " pT = " << leadTrack->pt() << "," 
                << " eta = " << leadTrack->eta() << "," 
                << " phi = " << leadTrack->phi() << std::endl;
    }

    // CV: compute and set DNN input variables
    dnnInputs_->flat<float>().setZero();
    size_t idx_dnnInput = 0;
    for ( auto const& jetInput : jetInputs_ )
    {
      set_dnnInput(*dnnInputs_, idx_dnnInput, compJetInput(*pfJetRef, jetInput, leadTrack));
      ++idx_dnnInput;
    }
    for ( auto const& pfJetConstituent : pfJetConstituents )
    {
      for ( auto const& inputVariable : pfCandInputs_ )
      {      
        set_dnnInput(*dnnInputs_, idx_dnnInput, compPFCandInput(pfJetConstituent, inputVariable, primaryVertexRef->position(), leadTrack));
        ++idx_dnnInput;
      }
    }
    
    // CV: compute DNN output
    std::vector<tensorflow::Tensor> dnnOutputs;
    tensorflow::run(&dnn_->getSession(), {{ dnnInputLayerName_, *dnnInputs_ }}, { dnnOutputLayerName_ }, &dnnOutputs);
    if ( verbosity_ >= 1 ) 
    {
      std::cout << "output = { ";
      for ( size_t idx_dnnOutput = 0; idx_dnnOutput < dnnOutputs.size(); ++idx_dnnOutput )
      {
        float dnnOutput = dnnOutputs[0].flat<float>()(idx_dnnOutput);
        if ( idx_dnnOutput > 0 ) std::cout << ", ";
        std::cout << dnnOutput;
      }
      std::cout << " }" << std::endl;
    }

    // CV: check that DNN output vector has the expected length and all DNN outputs are between 0 and 1
    if ( dnnOutputs.size() != num_dnnOutputs_ )
      throw cms::Exception("TallinnTauProducer")
        << "Size of DNN output vector = " << dnnOutputs.size() << " does not match expected size = " << num_dnnOutputs_ << " !!";
    for ( size_t idx_dnnOutput = 0; idx_dnnOutput < num_dnnOutputs_; ++idx_dnnOutput )
    {
      float dnnOutput = dnnOutputs[0].flat<float>()(idx_dnnOutput);
      if ( !(dnnOutput >= 0. && dnnOutput <= 1.) )
        throw cms::Exception("TallinnTauProducer")
          << "Invalid DNN output #" << idx_dnnOutput << ": value = " << dnnOutput << " is not within the interval [0,1] !!";
    }
    
    signalQualityCuts_.setPV(primaryVertexRef);
    signalQualityCuts_.setLeadTrack(*leadTrack);

    std::vector<reco::PFCandidatePtr> signalPFCands;
    std::vector<double> signalPFEnFracs;
    std::vector<reco::PFCandidatePtr> isolationPFCands;

    for ( size_t idxPFJetConstituent = 0; idxPFJetConstituent < std::min(pfJetConstituents.size(), num_dnnOutputs_); ++idxPFJetConstituent )
    {
      const reco::PFCandidate& pfJetConstituent = *pfJetConstituents[idxPFJetConstituent];
      double signalPFEnFrac = dnnOutputs[0].flat<float>()(idxPFJetConstituent);
      if ( signalPFEnFrac >= signalMinPFEnFrac_ && signalQualityCuts_.filterCand(pfJetConstituent) )
      {
        reco::PFCandidate signalPFCand = clonePFCand(pfJetConstituent, signalPFEnFrac);
        splittedPFCands->push_back(signalPFCand);
        edm::Ptr<reco::PFCandidate> signalPFCandPtr(splittedPFCandsRefProd.id(), splittedPFCands->size() - 1, splittedPFCandsRefProd.productGetter());
        signalPFCands.push_back(signalPFCandPtr);
        signalPFEnFracs.push_back(signalPFEnFrac);
      }
      double isolationPFEnFrac = 1. - signalPFEnFrac;
      if ( isolationPFEnFrac >= isolationMinPFEnFrac_ )
      {
        reco::PFCandidate isolationPFCand = clonePFCand(pfJetConstituent, isolationPFEnFrac);
        splittedPFCands->push_back(isolationPFCand);
        edm::Ptr<reco::PFCandidate> isolationPFCandPtr(splittedPFCandsRefProd.id(), splittedPFCands->size() - 1, splittedPFCandsRefProd.productGetter());
        isolationPFCands.push_back(isolationPFCandPtr);
      }
    }

    // CV: process PFCandidates within isolation cone, which are not constituents of the jet that seeds the tau reconstruction;
    //     center isolation cone on the direction given by the momentum sum of jet constituents indentified as tau decay products by the DNN
    reco::Candidate::LorentzVector pfTauP4;
    for ( auto const& signalPFCand : signalPFCands )
    {
      pfTauP4 += signalPFCand->p4();
    }
    size_t numPFCands = pfCands->size();
    for ( size_t idxPFCand = 0; idxPFCand < numPFCands; ++idxPFCand ) 
    {
      reco::PFCandidatePtr pfCandPtr(pfCands, idxPFCand);
      double dR_tau = deltaR(pfCandPtr->p4(), pfTauP4);
      if ( dR_tau < isolationConeSize_ )
      {
        bool isPFJetConstituent = false;
        for ( auto const& pfJetConstituent : pfJetConstituents )
        {
          double dR_jetConstituent = deltaR(pfCandPtr->p4(), pfJetConstituent->p4());
          if ( dR_jetConstituent > 1.e-3 ) continue;
          if ( pfCandPtr->particleId() == pfJetConstituent->particleId() ) continue;
          if ( !(pfCandPtr->energy() > 0.99*pfJetConstituent->energy() && pfCandPtr->energy() < 1.01*pfJetConstituent->energy()) ) continue;
          isPFJetConstituent = true;
          break;
        }
        if ( !isPFJetConstituent ) isolationPFCands.push_back(pfCandPtr);
      }
    }
    
    // CV: sort signal and PFCandidates by decreasing pT
    std::sort(signalPFCands.begin(), signalPFCands.end(), isHigherPt);
    std::sort(isolationPFCands.begin(), isolationPFCands.end(), isHigherPt);

    reco::PFTau pfTau = tauBuilder_(pfJetRef, signalPFCands, signalPFEnFracs, isolationPFCands, primaryVertexPos);
    if ( verbosity_ >= 1 )
    {
      std::cout << pfTau << std::endl;
    }
    pfTaus->push_back(pfTau);
  }

  evt.put(std::move(pfTaus));
  evt.put(std::move(splittedPFCands), "splittedPFCands");
}

std::unique_ptr<TallinnTauCache> 
TallinnTauProducer::initializeGlobalCache(const edm::ParameterSet& cfg) 
{
  return std::make_unique<TallinnTauCache>(cfg.getParameter<edm::VParameterSet>("graphs"));
}

void
TallinnTauProducer::globalEndJob(const TallinnTauCache* dnn)
{}

void 
TallinnTauProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("jetSrc", edm::InputTag("ak4PFJets"));
  desc.add<double>("minJetPt", 14.0);
  desc.add<double>("maxJetAbsEta", 2.5);
  desc.add<edm::InputTag>("pfCandSrc", edm::InputTag("particleFlow"));
  desc.add<vstring>("jetInputs", {});
  desc.add<vstring>("pfCandInputs", {});
  desc.add<unsigned>("maxNumPFCands", 25);
  edm::ParameterSetDescription desc_graphs;
  TallinnTauCache::fillDescriptions(desc_graphs);
  desc.addVPSet("graphs", desc_graphs, {});
  desc.add<double>("signalMinPFEnFrac", 0.);
  desc.add<double>("isolationMinPFEnFrac", 0.);
  desc.add<double>("isolationConeSize", 0.5);
  edm::ParameterSetDescription desc_qualityCuts;
  RecoTauQualityCuts::fillDescriptions(desc_qualityCuts);
  desc.add<edm::ParameterSetDescription>("qualityCuts", desc_qualityCuts);
  TallinnTauBuilder::fillDescriptions(desc);
  desc.add<int>("verbosity", 0);
  descriptions.add("tallinnRecoTaus", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"                // DEFINE_FWK_MODULE()
DEFINE_FWK_MODULE(TallinnTauProducer);
