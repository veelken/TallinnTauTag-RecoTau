#ifndef TallinnTauTag_RecoTau_TallinnTauProducer_h
#define TallinnTauTag_RecoTau_TallinnTauProducer_h

#include "FWCore/Framework/interface/stream/EDProducer.h"               // edm::stream::EDProducer<>
#include "FWCore/Framework/interface/Event.h"                           // edm::Event
#include "FWCore/Framework/interface/EventSetup.h"                      // edm::EventSetup
#include "FWCore/ParameterSet/interface/ParameterSet.h"                 // edm::ParameterSet
#include "FWCore/Utilities/interface/InputTag.h"                        // edm::InputTag
#include "FWCore/Utilities/interface/EDGetToken.h"                      // edm::EDGetTokenT<>
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"    // edm::ConfigurationDescriptions

#include "RecoTauTag/RecoTau/interface/RecoTauQualityCuts.h"            // reco::tau::RecoTauQualityCuts
#include "RecoTauTag/RecoTau/interface/RecoTauVertexAssociator.h"       // reco::tau::RecoTauVertexAssociator 
#include "PhysicsTools/TensorFlow/interface/TensorFlow.h"               // tensorflow::Tensor
#include "CommonTools/Utils/interface/StringObjectFunction.h"           // StringObjectFunction

#include "DataFormats/JetReco/interface/PFJet.h"                        // reco::PFJet, reco::PFJetRef
#include "DataFormats/JetReco/interface/PFJetCollection.h"              // reco::PFJetCollection
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"    // reco::PFCandidate, reco::PFCandidatePtr
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h" // reco::PFCandidateCollection
#include "DataFormats/VertexReco/interface/Vertex.h"                    // reco::Vertex::Point

#include "TallinnTauTag/RecoTau/interface/TallinnTauBuilder.h"          // reco::tau::TallinnTauBuilder
#include "TallinnTauTag/RecoTau/interface/TFGraphCache.h"               // reco::tau::TFGraphCache

#include <fstream>                                                      // std::ofstream
#include <string>                                                       // std::string
#include <vector>                                                       // std::vector

namespace reco 
{
  namespace tau 
  {
    class PFJetConstituent_order
    {
     public:
      PFJetConstituent_order(const std::vector<int>& particleIds)
        : particleIds_(particleIds)
      {}
      ~PFJetConstituent_order() {}
      bool 
      operator()(const reco::PFCandidatePtr& pfCand1, const reco::PFCandidatePtr& pfCand2) const
      {
        for ( auto particleId : particleIds_ )
        {
          if ( pfCand1->particleId() == particleId && pfCand2->particleId() != particleId ) return true;
          if ( pfCand2->particleId() == particleId && pfCand1->particleId() != particleId ) return false;
        }
        return pfCand1->pt() > pfCand2->pt();
      }
     private:
      std::vector<int> particleIds_;
    };

    class TallinnTauProducer : public edm::stream::EDProducer<edm::GlobalCache<TFGraphCache>>
    {
     public:
      explicit TallinnTauProducer(const edm::ParameterSet& cfg, const TFGraphCache* tfGraph);
      ~TallinnTauProducer() override;

      void 
      produce(edm::Event& evt, const edm::EventSetup& es) override;

      static std::unique_ptr<TFGraphCache> 
      initializeGlobalCache(const edm::ParameterSet& cfg);

      static void
      globalEndJob(const TFGraphCache* tfGraph);

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions); 

     private:
      double 
      compJetInput(const reco::PFJet& pfJet, 
                   const std::string& inputVariable, 
                   size_t numPFCands, const reco::Candidate::LorentzVector& pfCandSumP4, const reco::Track* leadTrack) const;

      double 
      compPFCandInput(const reco::PFCandidate& pfCand, 
                      const std::string& inputVariable, 
                      const reco::Vertex::Point& primaryVertexPos,
                      const reco::PFJet& pfJet, const reco::Track* leadTrack,
		      const reco::Candidate::LorentzVector pfCandSumP4) const;

      std::string moduleLabel_;

      edm::InputTag pfJetSrc_;
      edm::EDGetTokenT<reco::PFJetCollection> pfJetToken_;
      double minJetPt_;
      double maxJetAbsEta_;

      edm::InputTag pfCandSrc_;
      edm::EDGetTokenT<reco::PFCandidateCollection> pfCandToken_;

      std::vector<std::string> jetInputs_;
      mutable std::map<std::string, std::unique_ptr<StringObjectFunction<reco::PFJet>>> jetInputExtractors_;
      std::vector<std::string> pfCandInputs_;
      mutable std::map<std::string, std::unique_ptr<StringObjectFunction<reco::PFCandidate>>> pfCandInputExtractors_;

      std::vector<std::string> pointInputs_;
      std::vector<std::string> maskInputs_;


      size_t maxNumPFCands_;

      PFJetConstituent_order jetConstituent_order_;

      tensorflow::Session* tfSession_;
      std::unique_ptr<tensorflow::Tensor> nnInputs_features_;
      std::unique_ptr<tensorflow::Tensor> gnnInputs_points_;
      std::unique_ptr<tensorflow::Tensor> gnnInputs_mask_;
      size_t num_nnInputs_;
      std::vector<tensorflow::Tensor> nnOutputs_;
      size_t num_nnOutputs_;

      std::string gnnPointsLayerName_;
      std::string gnnMaskLayerName_;
      std::string nnFeatureLayerName_;
      std::string nnOutputLayerName_;

      bool isGNN_;

      double signalMinPFEnFrac_;
      double isolationMinPFEnFrac_;
      enum { kUndefined, kRegression, kClassification };
      int mode_;

      double isolationConeSize_;

      RecoTauQualityCuts signalQualityCuts_;
      RecoTauQualityCuts isolationQualityCuts_;
      RecoTauVertexAssociator vertexAssociator_;

      TallinnTauBuilder tauBuilder_;

      bool saveInputs_;
      std::string jsonFileName_;
      std::ofstream* jsonFile_;
      bool jsonFile_isFirstEvent_;

      int verbosity_;
    };
  }
}

#endif
