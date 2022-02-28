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

#include "DataFormats/JetReco/interface/PFJet.h"                        // reco::PFJet, reco::PFJetRef
#include "DataFormats/JetReco/interface/PFJetCollection.h"              // reco::PFJetCollection
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"    // reco::PFCandidate
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h" // reco::PFCandidateCollection
#include "DataFormats/VertexReco/interface/Vertex.h"                    // reco::Vertex::Point

#include "TallinnTauTag/RecoTau/interface/TallinnTauBuilder.h"          // reco::tau::TallinnTauBuilder
#include "TallinnTauTag/RecoTau/interface/TallinnTauCache.h"            // reco::tau::TallinnTauCache

#include <fstream>                                                      // std::ofstream
#include <string>                                                       // std::string
#include <vector>                                                       // std::vector

namespace reco 
{
  namespace tau 
  {
    class TallinnTauProducer : public edm::stream::EDProducer<edm::GlobalCache<TallinnTauCache>>
    {
     public:
      explicit TallinnTauProducer(const edm::ParameterSet& cfg, const TallinnTauCache* dnn);
      ~TallinnTauProducer() override;

      void 
      produce(edm::Event& evt, const edm::EventSetup& es) override;

      static std::unique_ptr<TallinnTauCache> 
      initializeGlobalCache(const edm::ParameterSet& cfg);

      static void
      globalEndJob(const TallinnTauCache* dnn);

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions); 

     private:
      double 
      compJetInput(const reco::PFJet& pfJet,
                   const std::string& inputVariable,
                   const reco::Track* leadTrack) const;
      double 
      compPFCandInput(const reco::PFCandidate& pfCand, 
                      const std::string& inputVariable, 
                      const reco::Vertex::Point& primaryVertexPos,
                      const reco::PFJet& pfJet, const reco::Track* leadTrack) const;

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
      size_t maxNumPFCands_;

      const TallinnTauCache* dnn_;
      std::unique_ptr<tensorflow::Tensor> dnnInputs_;
      size_t num_dnnInputs_;
      size_t num_dnnOutputs_;
      std::string dnnInputLayerName_;
      std::string dnnOutputLayerName_;

      double signalMinPFEnFrac_;
      double isolationMinPFEnFrac_;

      double isolationConeSize_;

      RecoTauQualityCuts signalQualityCuts_;
      RecoTauVertexAssociator vertexAssociator_;

      TallinnTauBuilder tauBuilder_;

      bool saveInputs_;
      std::string jsonFileName_;
      std::ofstream* jsonFile_;

      int verbosity_;
    };
  }
}

#endif
