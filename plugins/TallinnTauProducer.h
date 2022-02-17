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
#include "CommonTools/Utils/interface/StringObjectFunction.h"           // StringObjectFunction

#include "DataFormats/JetReco/interface/PFJet.h"                        // reco::PFJet, reco::PFJetRef
#include "DataFormats/JetReco/interface/PFJetCollection.h"              // reco::PFJetCollection
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"    // reco::PFCandidate
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h" // reco::PFCandidateCollection

#include <string>
#include <vector>

namespace reco {
  namespace tau {

    class TallinnTauProducer : public edm::stream::EDProducer<>
    {
     public:
      explicit TallinnTauProducer(const edm::ParameterSet& cfg);
      ~TallinnTauProducer() override;

      void produce(edm::Event& evt, const edm::EventSetup& es) override;

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions); 

     private:
      std::string moduleLabel_;

      edm::InputTag jetSrc_;
      edm::EDGetTokenT<reco::PFJetCollection> jetToken_;

      edm::InputTag pfCandSrc_;
      edm::EDGetTokenT<reco::PFCandidateCollection> pfCandToken_;

      double minJetPt_;
      double maxJetAbsEta_;

      StringObjectFunction<reco::PFTau> signalConeSize_;
      double isolationConeSize_;

      RecoTauQualityCuts signalQualityCuts_;
      RecoTauQualityCuts isolationQualityCuts_;
      RecoTauVertexAssociator vertexAssociator_;

      double signalMinPFEnFrac_;
      double isolationMinPFEnFrac_;

      int verbosity_;
    };
  }
}

#endif
