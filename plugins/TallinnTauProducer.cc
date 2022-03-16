#include "TallinnTauTag/RecoTau/plugins/TallinnTauProducer.h"

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"          // edm::ParameterSetDescription
#include "FWCore/Utilities/interface/Exception.h"                           // edm::Exception
#include "FWCore/MessageLogger/interface/MessageLogger.h"                   // edm::LogWarning

#include "DataFormats/TauReco/interface/PFTau.h"                            // reco::PFTau
#include "DataFormats/TauReco/interface/PFTauFwd.h"                         // reco::PFTauCollection
#include "DataFormats/TrackReco/interface/Track.h"                          // reco::Track
#include "DataFormats/Math/interface/deltaR.h"                              // reco::deltaR()
#include "DataFormats/Math/interface/deltaPhi.h"                            // reco::deltaPhi()
#include "DataFormats/Common/interface/RefToPtr.h"                          // edm::refToPtr()

#include "TallinnTauTag/RecoTau/interface/hashAuxFunctions.h"               // getHash_jet(), getHash_pfCand()
#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidate.h"             // reco::wrappedPFCandidate, reco::wrappedPFCandidateCollection
#include "TallinnTauTag/RecoTau/interface/wrappedPFCandidateAuxFunctions.h" // isHigherPt(), getSumP4()

#include <sstream>                                                          // std::ostringstream
#include <iostream>                                                         // std::cout, std::endl
#include <memory>                                                           // std::make_unique<>
#include <utility>                                                          // std::pair
#include <algorithm>                                                        // std::sort
#include <cmath>                                                            // log

using namespace reco::tau;

typedef std::vector<std::string> vstring;

TallinnTauProducer::TallinnTauProducer(const edm::ParameterSet& cfg, const TFGraphCache* tfGraph) 
  : moduleLabel_(cfg.getParameter<std::string>("@module_label"))
  , minJetPt_(cfg.getParameter<double>("minJetPt"))
  , maxJetAbsEta_(cfg.getParameter<double>("maxJetAbsEta"))
  , jetConstituent_order_(tfGraph->getJetConstituent_order())
  , tfSession_(tensorflow::createSession(&tfGraph->getGraph()))
  , signalMinPFEnFrac_(cfg.getParameter<double>("signalMinPFEnFrac"))
  , isolationMinPFEnFrac_(cfg.getParameter<double>("isolationMinPFEnFrac"))
  , isolationConeSize_(cfg.getParameter<double>("isolationConeSize"))
  , signalQualityCuts_(cfg.getParameterSet("qualityCuts").getParameterSet("signalQualityCuts"))
  , vertexAssociator_(cfg.getParameterSet("qualityCuts"), consumesCollector())
  , tauBuilder_(cfg)
  , saveInputs_(cfg.getParameter<bool>("saveInputs"))
  , jsonFileName_(cfg.getParameter<std::string>("jsonFileName"))
  , jsonFile_(nullptr)
  , jsonFile_isFirstEvent_(true)
  , verbosity_(cfg.getParameter<int>("verbosity"))
{
  std::cout << "<TallinnTauProducer::TallinnTauProducer (moduleLabel = " << moduleLabel_ << ")>:" << std::endl;
  pfJetSrc_ = cfg.getParameter<edm::InputTag>("pfJetSrc");
  pfJetToken_ = consumes<reco::PFJetCollection>(pfJetSrc_);  

  pfCandSrc_ = cfg.getParameter<edm::InputTag>("pfCandSrc");
  pfCandToken_ = consumes<reco::PFCandidateCollection>(pfCandSrc_);

  isGNN_ = tfGraph->isGNN();
  jetInputs_ = tfGraph->getJetInputs();
  pfCandInputs_ = tfGraph->getPFCandInputs();
  pointInputs_ = tfGraph->getPointInputs();
  maskInputs_ = tfGraph->getMaskInputs();
  maxNumPFCands_= tfGraph->getMaxNumPFCands();

  if ( isGNN_ )
  {
    num_nnInputs_ =  pfCandInputs_.size();
    gnnInputs_points_ = std::make_unique<tensorflow::Tensor>(tensorflow::DT_FLOAT, tensorflow::TensorShape{ (long)1, (long)maxNumPFCands_, (long)pointInputs_.size() });
    gnnPointsLayerName_ = tfGraph->getInputLayerNames()[0];
    gnnInputs_points_->flat<float>().setZero();
    const auto& nnPointsLayer = tfGraph->getGraph().node(0).attr().at("shape").shape();
    if ( ((size_t)nnPointsLayer.dim(1).size() != maxNumPFCands_) || ((size_t)nnPointsLayer.dim(2).size() != pointInputs_.size()) )
      throw cms::Exception("TallinnTauProducer")
	<< "Size of GNN points input layer = { " << nnPointsLayer.dim(1).size() << ", " << nnPointsLayer.dim(2).size() << " }" 
        << " does not match number of maxPfCandidates, point input variables = { " << num_nnInputs_ << ", " << pointInputs_.size() << "} !!";
    nnInputs_features_ = std::make_unique<tensorflow::Tensor>(tensorflow::DT_FLOAT, tensorflow::TensorShape{ (long)1, (long)maxNumPFCands_, (long)pfCandInputs_.size() });
    nnFeatureLayerName_ = tfGraph->getInputLayerNames()[1];
    nnInputs_features_->flat<float>().setZero();
    gnnInputs_mask_ = std::make_unique<tensorflow::Tensor>(tensorflow::DT_FLOAT, tensorflow::TensorShape{ (long)1, (long)maxNumPFCands_, (long)maskInputs_.size() });
    gnnMaskLayerName_ = tfGraph->getInputLayerNames()[2];
    gnnInputs_mask_->flat<float>().setZero();
    nnOutputLayerName_ = tfGraph->getOutputLayerName();
    num_nnOutputs_ = maxNumPFCands_;
  }
  else
  {
    num_nnInputs_ = jetInputs_.size() + pfCandInputs_.size()*maxNumPFCands_;
    nnInputs_features_ = std::make_unique<tensorflow::Tensor>(tensorflow::DT_FLOAT, tensorflow::TensorShape{ (long)num_nnInputs_ });
    nnInputs_features_->flat<float>().setZero();

    num_nnOutputs_ = maxNumPFCands_;

    nnFeatureLayerName_ = tfGraph->getInputLayerNames()[0];
    nnOutputLayerName_ = tfGraph->getOutputLayerName();
    const auto& nnInputLayer = tfGraph->getGraph().node(0).attr().at("shape").shape();
    if ( (size_t)nnInputLayer.dim(1).size() != num_nnInputs_ )
      throw cms::Exception("TallinnTauProducer")
        << "Size of DNN input layer = " << nnInputLayer.dim(1).size() << " does not match number of DNN input variables = " << num_nnInputs_ << " !!";
  }

  produces<reco::PFTauCollection>();
  produces<reco::PFCandidateCollection>("splittedPFCands");
}

TallinnTauProducer::~TallinnTauProducer()
{
  if ( saveInputs_ )
  {
    (*jsonFile_) << std::endl;
    (*jsonFile_) << "}";
    delete jsonFile_;
  }
}

double 
TallinnTauProducer::compJetInput(const reco::PFJet& pfJet, 
                                 const std::string& inputVariable, 
                                 size_t numPFCands, const reco::Candidate::LorentzVector& pfCandSumP4, const reco::Track* leadTrack) const
{
  //std::cout << "<TallinnTauProducer::compJetInput>:" << std::endl;
  //std::cout << " jet: pT = " << pfJet.pt() << ", eta = " << pfJet.eta() << ", phi = " << pfJet.phi() << std::endl;
  //std::cout << " inputVariable = '" << inputVariable << "'" << std::endl;
  double retVal = 0.;
  if      ( inputVariable == "pfCandSumPt"        ) retVal = pfCandSumP4.pt();
  else if ( inputVariable == "pfCandSumEta"       ) retVal = pfCandSumP4.eta();
  else if ( inputVariable == "pfCandSumPhi"       ) retVal = pfCandSumP4.phi();
  else if ( inputVariable == "pfCandSumMass"      ) retVal = pfCandSumP4.mass();
  else if ( inputVariable == "dR_leadTrack"        ) retVal = deltaR(pfJet.p4(), leadTrack->momentum());
  else if ( inputVariable == "dEta_leadTrack"      ) retVal = std::fabs(pfJet.eta() - leadTrack->eta());
  else if ( inputVariable == "dPhi_leadTrack"      ) retVal = deltaPhi(pfJet.phi(), leadTrack->phi());
  else if ( inputVariable == "numPFCands"          ) retVal = pfJet.getPFConstituents().size();
  else if ( inputVariable == "numSelectedPFCands"  ) retVal = numPFCands;
  else if ( inputVariable == "leadTrackPt/jetPt" ) retVal = leadTrack->pt()/pfJet.pt();
  else
  {
    auto jetInputExtractor = jetInputExtractors_.find(inputVariable);
    if ( jetInputExtractor == jetInputExtractors_.end() )
    {
      jetInputExtractors_[inputVariable] = std::make_unique<StringObjectFunction<reco::PFJet>>(inputVariable);
      jetInputExtractor = jetInputExtractors_.find(inputVariable);      
    }
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
                                    const reco::PFJet& pfJet, const reco::Track* leadTrack,
				    const reco::Candidate::LorentzVector pfCandSumP4) const
{
  //std::cout << "<TallinnTauProducer::compPFCandInput>:" << std::endl;
  //std::cout << " pfCand: pT = " << pfCand.pt() << ", eta = " << pfCand.eta() << ", phi = " << pfCand.phi() << std::endl;
  //std::cout << " inputVariable = '" << inputVariable << "'" << std::endl;
  double retVal = 0.;
  if ( inputVariable == "dz" || inputVariable == "dxy" || inputVariable == "log(abs(dz))" || inputVariable == "log(abs(dxy))" )
  {
    const reco::Track* track = getTrack(pfCand);
    if ( track && leadTrack )
    {
      if      ( inputVariable == "dz"                    ) retVal = track->dz(primaryVertexPos);
      else if ( inputVariable == "dxy"                   ) retVal = std::fabs(track->dxy(primaryVertexPos));
      else if ( inputVariable == "log(abs(dz))"          ) retVal = std::fabs(track->dz(primaryVertexPos)) > 0. ? log(std::fabs(track->dz(primaryVertexPos)))  : 0.;
      else if ( inputVariable == "log(abs(dxy))"         ) retVal = std::fabs(track->dz(primaryVertexPos)) > 0. ? log(std::fabs(track->dxy(primaryVertexPos))) : 0.;
      else assert(0);
    }
  }
  else if ( inputVariable == "dR_leadTrack"              ) retVal = deltaR(pfCand.p4(), leadTrack->momentum());
  else if ( inputVariable == "dEta_leadTrack"            ) retVal = pfCand.eta() - leadTrack->eta();
  else if ( inputVariable == "dPhi_leadTrack"            ) retVal = deltaPhi(pfCand.phi(), leadTrack->phi());
  else if ( inputVariable == "dR_jet"                    ) retVal = deltaR(pfCand.p4(), pfJet.p4());
  else if ( inputVariable == "dEta_jet"                  ) retVal = pfCand.eta() - pfJet.eta();
  else if ( inputVariable == "dPhi_jet"                  ) retVal = deltaPhi(pfCand.phi(), pfJet.phi());
  else if ( inputVariable == "dR_pfCandSum"              ) retVal = deltaR(pfCand.p4(), pfCandSumP4);
  else if ( inputVariable == "dEta_pfCandSum"            ) retVal = pfCand.eta() - pfCandSumP4.eta();
  else if ( inputVariable == "dPhi_pfCandSum"            ) retVal = deltaPhi(pfCand.phi(), pfCandSumP4.phi());
  else if ( inputVariable == "pfCandPt/jetPt"            ) retVal = pfCand.pt()/pfJet.pt();
  else if ( inputVariable == "log(pfCandPt/jetPt)"       ) retVal = log(pfCand.pt()/pfJet.pt());
  else if ( inputVariable == "log(pfCandPt/pfCandSumPt)" ) retVal = log(pfCand.pt()/pfCandSumP4.pt());
  else if ( inputVariable == "log(pt)"                   ) retVal = log(pfCand.pt());
  else if ( inputVariable == "log(energy)"               ) retVal = log(pfCand.energy());
  else if ( inputVariable == "log(dR_jet)"               ) retVal = deltaR(pfCand.p4(), pfJet.p4()) > 0. ? log(deltaR(pfCand.p4(), pfJet.p4())/pfJet.pt()) : 0.;
  else if ( inputVariable == "log(dR_pfCandSum)"         ) retVal = deltaR(pfCand.p4(), pfCandSumP4) > 0. ? log(deltaR(pfCand.p4(), pfCandSumP4)/pfCandSumP4.pt()) : 0.;
  else if ( inputVariable == "jetPt"                     ) retVal = pfJet.pt();
  else if ( inputVariable == "jetEta"                    ) retVal = pfJet.eta();
  else if ( inputVariable == "jetPhi"                    ) retVal = pfJet.phi();
  else if ( inputVariable == "pfCandSumPt"               ) retVal = pfCandSumP4.pt();
  else if ( inputVariable == "pfCandSumEta"              ) retVal = pfCandSumP4.eta();
  else if ( inputVariable == "pfCandSumPhi"              ) retVal = pfCandSumP4.phi();
  else
  {
    auto pfCandInputExtractor = pfCandInputExtractors_.find(inputVariable);
    if ( pfCandInputExtractor == pfCandInputExtractors_.end() )
    {
      pfCandInputExtractors_[inputVariable] = std::make_unique<StringObjectFunction<reco::PFCandidate>>(inputVariable);
      pfCandInputExtractor = pfCandInputExtractors_.find(inputVariable);
    }
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
    dnnInputs.flat<float>()(idx) = value;
  }

  void
  set_gnnInput(tensorflow::Tensor& gnnInputs, size_t idx2, size_t idx3, double value)
  {
    gnnInputs.tensor<float, 3>()(0, idx2, idx3) = value;
  }

  reco::PFCandidate
  clonePFCand(const reco::PFCandidate& pfCand, double pfEnFrac)
  {
    reco::PFCandidate clonedPFCand(pfCand);
    clonedPFCand.setP4(pfEnFrac*pfCand.p4());
    return clonedPFCand;
  }

  std::string
  format_vfloat(const std::vector<float>& values)
  {
    std::ostringstream output;
    output << "[ ";
    for ( size_t idxValue = 0; idxValue < values.size(); ++idxValue )
    {
      if ( idxValue != 0 )
      {
        output << ", ";
      }
      output << values[idxValue];
    }
    output << " ]";
    return output.str();
  }
}

void 
TallinnTauProducer::produce(edm::Event& evt, const edm::EventSetup& es)
{
  std::unique_ptr<reco::PFTauCollection> pfTaus = std::make_unique<reco::PFTauCollection>();
  std::unique_ptr<reco::PFCandidateCollection> splittedPFCands = std::make_unique<reco::PFCandidateCollection>();

  reco::PFCandidateRefProd splittedPFCandsRefProd = evt.getRefBeforePut<reco::PFCandidateCollection>("splittedPFCands");

  edm::Handle<reco::PFJetCollection> pfJets;
  evt.getByToken(pfJetToken_, pfJets);

  edm::Handle<reco::PFCandidateCollection> pfCands;
  evt.getByToken(pfCandToken_, pfCands);

  vertexAssociator_.setEvent(evt);

  size_t numPFJets = pfJets->size();
  for ( size_t idxPFJet = 0; idxPFJet < numPFJets; ++idxPFJet )
  {
    reco::PFJetRef pfJetRef(pfJets, idxPFJet);

    // CV: require that jet that seeds the tau reconstruction passes loose cuts on pT and eta
    if ( !(pfJetRef->pt() > minJetPt_ && std::fabs(pfJetRef->eta()) < maxJetAbsEta_) ) continue;    
    if ( verbosity_ >= 1 )
    {
      std::cout << "pfJet:" 
                << " pT = " << pfJetRef->pt() << "," 
                << " eta = " << pfJetRef->eta() << "," 
                << " phi = " << pfJetRef->phi() << "," 
                << " mass = " << pfJetRef->mass() << std::endl;
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
                << " phi = " << leadTrack->phi() << "," 
                << " dz = " << leadTrack->dz(primaryVertexPos) << std::endl;
    }

    signalQualityCuts_.setPV(primaryVertexRef);
    signalQualityCuts_.setLeadTrack(*leadTrack);

    // CV: apply signal quality cuts to jet constituents 
    std::vector<reco::PFCandidatePtr> allPFJetConstituents = pfJetRef->getPFConstituents();
    std::vector<reco::PFCandidatePtr> selPFJetConstituents;
    for ( auto const& pfJetConstituent : allPFJetConstituents )
    {
      if ( signalQualityCuts_.filterCand(*pfJetConstituent) )
      {
        selPFJetConstituents.push_back(pfJetConstituent);
      }
    }

    // CV: sort jet constituents by PFCandidate type and pT
    std::sort(selPFJetConstituents.begin(), selPFJetConstituents.end(), jetConstituent_order_);
    if ( verbosity_ >= 1 )
    {
      std::cout << "jet #" << pfJetRef.key() << ":" 
                << " pT = " << pfJetRef->pt() << "," 
                << " eta = " << pfJetRef->eta() << "," 
                << " phi = " << pfJetRef->phi() 
                << " (#constituents = " << selPFJetConstituents.size() << ")" << std::endl;
    }

    reco::Candidate::LorentzVector pfCandSumP4;
    for ( auto const& pfJetConstituent : selPFJetConstituents )
    {
      pfCandSumP4 += pfJetConstituent->p4();
    }

    if ( isGNN_ )
    {
      // TL: compute and set GNN input variables
      gnnInputs_points_->flat<float>().setZero();
      size_t idx_cand = 0;
      for  ( auto const& pfJetConstituent : selPFJetConstituents )
      {
	size_t idx_var = 0;
	for ( auto const& inputVariable : pointInputs_ )
        {
	  set_gnnInput(*gnnInputs_points_, idx_cand, idx_var, compPFCandInput(pfJetConstituent, inputVariable, primaryVertexRef->position(), *pfJetRef, leadTrack, pfCandSumP4));
	  idx_var++;
	}
	idx_cand++;
      }
      gnnInputs_mask_->flat<float>().setZero();
      idx_cand = 0;
      for  ( auto const& pfJetConstituent : selPFJetConstituents )
      {
	size_t idx_var = 0;
	for ( auto const& inputVariable : maskInputs_ )
        {
	  set_gnnInput(*gnnInputs_mask_, idx_cand, idx_var, compPFCandInput(pfJetConstituent, inputVariable, primaryVertexRef->position(), *pfJetRef, leadTrack, pfCandSumP4));
	  idx_var++;
	}
	idx_cand++;
      }
      nnInputs_features_->flat<float>().setZero();
      idx_cand = 0;
      for  ( auto const& pfJetConstituent : selPFJetConstituents )
      {
	size_t idx_var = 0;
	for ( auto const& inputVariable : pfCandInputs_ )
        {
	  set_gnnInput(*nnInputs_features_, idx_cand, idx_var, compPFCandInput(pfJetConstituent, inputVariable, primaryVertexRef->position(), *pfJetRef, leadTrack, pfCandSumP4));
	  idx_var++;
	}
	idx_cand++;
      }
    }
    else
    {
      // CV: compute and set DNN input variables
      nnInputs_features_->flat<float>().setZero();
      size_t idx_dnnInput = 0;
      for ( auto const& jetInput : jetInputs_ )
      {
        set_dnnInput(*nnInputs_features_, idx_dnnInput, compJetInput(*pfJetRef, jetInput, selPFJetConstituents.size(), pfCandSumP4, leadTrack));
        ++idx_dnnInput;
      }
      for ( auto const& pfJetConstituent : selPFJetConstituents )
      {
        for ( auto const& inputVariable : pfCandInputs_ )
        {
          set_dnnInput(*nnInputs_features_, idx_dnnInput, compPFCandInput(pfJetConstituent, inputVariable, primaryVertexRef->position(), *pfJetRef, leadTrack, pfCandSumP4));
          ++idx_dnnInput;
        }
      }
    }
    nnOutputs_.clear();
    if ( isGNN_ )
    {
      // TL: compute GNN output
      tensorflow::run(
        tfSession_,
        {{ gnnPointsLayerName_, *gnnInputs_points_ }, { nnFeatureLayerName_, *nnInputs_features_ }, { gnnMaskLayerName_, *gnnInputs_mask_ } },
        { nnOutputLayerName_ }, &nnOutputs_
      );
    }
    else
    {
      // CV: compute DNN output
      tensorflow::run(
        tfSession_,
        {{ nnFeatureLayerName_, *nnInputs_features_ }},
        { nnOutputLayerName_ }, &nnOutputs_
      );
    }
    // CV: check that DNN/GNN output vector has the expected length and all DNN outputs are between 0 and 1
    if ( !(nnOutputs_.size() == 1 && (size_t)nnOutputs_[0].flat<float>().size() == num_nnOutputs_) )
      throw cms::Exception("TallinnTauProducer")
        << "Size of NN output vector = " << nnOutputs_[0].flat<float>().size() << " does not match expected size = " << num_nnOutputs_ << " !!";
    for ( size_t idx_nnOutput = 0; idx_nnOutput < num_nnOutputs_; ++idx_nnOutput )
    {
      float nnOutput = nnOutputs_[0].flat<float>()(idx_nnOutput);
      if ( !(nnOutput >= 0. && nnOutput <= 1.) )
      {
        edm::LogWarning("TallinnTauProducer")
          << "Invalid DNN output #" << idx_nnOutput << ": value = " << nnOutput << " is not within the interval [0,1] !!";
        if ( nnOutput < 0. ) nnOutput = 0.;
        if ( nnOutput > 1. ) nnOutput = 1.;
      }
    }

    if ( verbosity_ >= 1)
    {
      std::cout << "output = { ";
      for ( size_t idx_nnOutput = 0; idx_nnOutput < num_nnOutputs_; ++idx_nnOutput )
      {
        float nnOutput = nnOutputs_[0].flat<float>()(idx_nnOutput);
        if ( idx_nnOutput > 0 ) std::cout << ", ";
        std::cout << nnOutput;
      }
      std::cout << " }" << std::endl;
    }

    reco::wrappedPFCandidateCollection signalPFCands;
    reco::wrappedPFCandidateCollection isolationPFCands;

    for ( size_t idxPFJetConstituent = 0; idxPFJetConstituent < std::min(selPFJetConstituents.size(), num_nnOutputs_); ++idxPFJetConstituent )
    {
      const reco::PFCandidate& pfJetConstituent = selPFJetConstituents.at(idxPFJetConstituent);
      double signalPFEnFrac = nnOutputs_[0].flat<float>()(idxPFJetConstituent);
      if ( signalPFEnFrac >= signalMinPFEnFrac_ && signalQualityCuts_.filterCand(pfJetConstituent) )
      {
        reco::PFCandidate signalPFCand = clonePFCand(pfJetConstituent, signalPFEnFrac);
        splittedPFCands->push_back(signalPFCand);
        edm::Ptr<reco::PFCandidate> signalPFCandPtr(edm::refToPtr(reco::PFCandidateRef(splittedPFCandsRefProd, splittedPFCands->size() - 1)));
        signalPFCands.push_back(reco::wrappedPFCandidate(signalPFCand, signalPFCandPtr, signalPFEnFrac));
      }
      double isolationPFEnFrac = 1. - signalPFEnFrac;
      if ( isolationPFEnFrac >= isolationMinPFEnFrac_ )
      {
        reco::PFCandidate isolationPFCand = clonePFCand(pfJetConstituent, isolationPFEnFrac);
        splittedPFCands->push_back(isolationPFCand);
        edm::Ptr<reco::PFCandidate> isolationPFCandPtr(edm::refToPtr(reco::PFCandidateRef(splittedPFCandsRefProd, splittedPFCands->size() - 1)));
        isolationPFCands.push_back(reco::wrappedPFCandidate(isolationPFCand, isolationPFCandPtr, isolationPFEnFrac));
      }
    }

    // CV: process PFCandidates within isolation cone, which are not constituents of the jet that seeds the tau reconstruction;
    //     center isolation cone on the direction given by the momentum sum of jet constituents indentified as tau decay products by the DNN
    reco::Candidate::LorentzVector pfTauP4 = getSumP4(signalPFCands);
    size_t numPFCands = pfCands->size();
    for ( size_t idxPFCand = 0; idxPFCand < numPFCands; ++idxPFCand ) 
    {
      reco::PFCandidatePtr pfCandPtr(pfCands, idxPFCand);
      double dR_tau = deltaR(pfCandPtr->p4(), pfTauP4);
      if ( dR_tau < isolationConeSize_ )
      {
        bool isPFJetConstituent = false;
        for ( auto const& pfJetConstituent : allPFJetConstituents )
        {
          double dR_jetConstituent = deltaR(pfCandPtr->p4(), pfJetConstituent->p4());
          if ( dR_jetConstituent > 1.e-3 ) continue;
          if ( pfCandPtr->particleId() == pfJetConstituent->particleId() ) continue;
          if ( !(pfCandPtr->energy() > 0.99*pfJetConstituent->energy() && pfCandPtr->energy() < 1.01*pfJetConstituent->energy()) ) continue;
          isPFJetConstituent = true;
          break;
        }
        if ( !isPFJetConstituent ) isolationPFCands.push_back(reco::wrappedPFCandidate(*pfCandPtr, pfCandPtr, 1.));
      }
    }

    // CV: sort signal and PFCandidates by decreasing pT
    std::sort(signalPFCands.begin(), signalPFCands.end(), isHigherPt);
    std::sort(isolationPFCands.begin(), isolationPFCands.end(), isHigherPt);

    if ( saveInputs_) 
    {
      if ( jsonFile_isFirstEvent_ )
      {
        jsonFile_ = new std::ofstream(jsonFileName_.data());
        (*jsonFile_) << "{" << std::endl;
        jsonFile_isFirstEvent_ = false;
      } 
      else
      {
        (*jsonFile_) << "," << std::endl;
      }
      const edm::EventID& evtId = evt.id();
      std::ostringstream key_evt;
      key_evt << evtId.run() << ":" << evtId.luminosityBlock() << ":" << evtId.event();
      //std::string key_jet = getHash_jet(pfJetRef->p4());
      std::string key_jet = getHash_jet(pfCandSumP4); // CV: ONLY FOR TESTING !!
      (*jsonFile_) << "    " << "\"" << key_evt.str() << "|" << key_jet << "\": {" << std::endl;
      if (!isGNN_)
	{
	  std::vector<float> nnInputs_jet;
	  for ( auto const& jetInput : jetInputs_ )
	    {
	      nnInputs_jet.push_back(compJetInput(*pfJetRef, jetInput, selPFJetConstituents.size(), pfCandSumP4, leadTrack));
	    }
	  (*jsonFile_) << "        " << "\"inputs\": " << format_vfloat(nnInputs_jet) << "," << std::endl;
	  std::vector<float> nnInputs_vfloat;
	  for ( size_t idx_nnInput = 0; idx_nnInput < num_nnInputs_; ++idx_nnInput )
	    {
	      nnInputs_vfloat.push_back(nnInputs_features_->flat<float>()(idx_nnInput));
	    }
	  (*jsonFile_) << "        " << "\"full_input\": " << format_vfloat(nnInputs_vfloat) << "," << std::endl;
	}
      std::vector<float> nnOutputs_vfloat;
      for ( size_t idx_nnOutput = 0; idx_nnOutput < num_nnOutputs_; ++idx_nnOutput )
      {
        nnOutputs_vfloat.push_back(nnOutputs_[0].flat<float>()(idx_nnOutput));
      }
      (*jsonFile_) << "        " << "\"full_output\": " << format_vfloat(nnOutputs_vfloat);
      if ( selPFJetConstituents.size() > 0 )
      {
        (*jsonFile_) << ",";
        (*jsonFile_) << std::endl;
      }
      for ( size_t idxPFJetConstituent = 0; idxPFJetConstituent < std::min(selPFJetConstituents.size(), num_nnOutputs_); ++idxPFJetConstituent )
      {
        const reco::PFCandidate& pfJetConstituent = selPFJetConstituents.at(idxPFJetConstituent);
        std::vector<float> nnInputs_pfCand;
        for ( auto const& inputVariable : pfCandInputs_ )
        {
          nnInputs_pfCand.push_back(compPFCandInput(pfJetConstituent, inputVariable, primaryVertexRef->position(), *pfJetRef, leadTrack, pfCandSumP4));
        }
	std::vector<float> gnnPointInputs_pfCand;
        for ( auto const& inputVariable : pointInputs_ )
        {
          gnnPointInputs_pfCand.push_back(compPFCandInput(pfJetConstituent, inputVariable, primaryVertexRef->position(), *pfJetRef, leadTrack, pfCandSumP4));
        }
	std::vector<float> gnnMaskInputs_pfCand;
        for ( auto const& inputVariable : maskInputs_ )
        {
          gnnMaskInputs_pfCand.push_back(compPFCandInput(pfJetConstituent, inputVariable, primaryVertexRef->position(), *pfJetRef, leadTrack, pfCandSumP4));
        }
        float nnOutput = nnOutputs_[0].flat<float>()(idxPFJetConstituent);
        std::string key_pfCand = getHash_pfCand(pfJetConstituent.p4(), pfJetConstituent.particleId(), pfJetConstituent.charge());
        if ( idxPFJetConstituent != 0 )
        {
          (*jsonFile_) << "," << std::endl;
        }
        (*jsonFile_) << "        " << "\"" << key_pfCand << "\": {" << std::endl;
        (*jsonFile_) << "            " << "\"inputs\": " << format_vfloat(nnInputs_pfCand) << "," << std::endl;
	if (isGNN_){
	  (*jsonFile_) << "            " << "\"mask\": " << format_vfloat(gnnMaskInputs_pfCand) << "," << std::endl;
	  (*jsonFile_) << "            " << "\"points\": " << format_vfloat(gnnPointInputs_pfCand) << "," << std::endl;
	}
        (*jsonFile_) << "            " << "\"output\": " << nnOutput << std::endl;
        (*jsonFile_) << "        }";
      }
      (*jsonFile_) << std::endl;
      (*jsonFile_) << "    }";
    }

    reco::PFTau pfTau = tauBuilder_(pfJetRef, signalPFCands, isolationPFCands, primaryVertexPos);
    if ( verbosity_ >= 1 )
    {
      std::cout << "pfTau:" 
                << " pT = " << pfTau.pt() << ","
                << " eta = " << pfTau.eta() << ","
                << " phi = " << pfTau.phi() << ","
                << " mass = " << pfTau.mass() << ","
                << " decayMode = " << pfTau.decayMode() << std::endl;
    }
    pfTaus->push_back(pfTau);
  }

  evt.put(std::move(pfTaus));
  evt.put(std::move(splittedPFCands), "splittedPFCands");
}

std::unique_ptr<TFGraphCache> 
TallinnTauProducer::initializeGlobalCache(const edm::ParameterSet& cfg) 
{
  edm::ParameterSet cfg_graph = cfg.getParameterSet("tfGraph");
  TFGraphCache* tfGraph = new TFGraphCache(cfg_graph);
  return std::unique_ptr<TFGraphCache>(tfGraph);
}

void
TallinnTauProducer::globalEndJob(const TFGraphCache* tfGraph)
{}

void 
TallinnTauProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("pfJetSrc", edm::InputTag("ak4PFJets"));
  desc.add<double>("minJetPt", 14.0);
  desc.add<double>("maxJetAbsEta", 2.5);
  desc.add<edm::InputTag>("pfCandSrc", edm::InputTag("particleFlow"));
  edm::ParameterSetDescription desc_tfGraph;
  TFGraphCache::fillDescriptions(desc_tfGraph);
  desc.add<edm::ParameterSetDescription>("tfGraph", desc_tfGraph);
  desc.add<double>("signalMinPFEnFrac", 0.);
  desc.add<double>("isolationMinPFEnFrac", 0.);
  desc.add<double>("isolationConeSize", 0.5);
  edm::ParameterSetDescription desc_qualityCuts;
  RecoTauQualityCuts::fillDescriptions(desc_qualityCuts);
  desc.add<edm::ParameterSetDescription>("qualityCuts", desc_qualityCuts);
  desc.add<std::vector<int>>("chargedHadrParticleIds", { 1, 2, 3 }); // h, e, mu
  TallinnTauBuilder::fillDescriptions(desc);
  desc.add<bool>("saveInputs", false);
  desc.add<std::string>("jsonFileName", "TallinnTauProducer.json");
  desc.add<int>("verbosity", 0);
  descriptions.addWithDefaultLabel(desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"                // DEFINE_FWK_MODULE()
DEFINE_FWK_MODULE(TallinnTauProducer);
