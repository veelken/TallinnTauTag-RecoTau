#include "TallinnTauTag/RecoTau/plugins/TallinnTauProducer.h"

#include "FWCore/ParameterSet/interface/ParameterSetDescription.h" // edm::ParameterSetDescription 
#include "FWCore/Utilities/interface/Exception.h"                  // edm::Exception

#include "DataFormats/TauReco/interface/PFTau.h"                   // reco::PFTau
#include "DataFormats/TauReco/interface/PFTauFwd.h"                // reco::PFTauCollection

#include <iostream>                                                // std::cout, std::endl
#include <memory>                                                  // std::make_unique<>

using namespace reco::tau;

TallinnTauProducer::TallinnTauProducer(const edm::ParameterSet& cfg) 
  : moduleLabel_(cfg.getParameter<std::string>("@module_label"))
  , signalConeSize_(cfg.getParameter<std::string>("signalConeSize"))
  , isolationConeSize_(cfg.getParameter<double>("isolationConeSize"))
  , signalQualityCuts_(cfg.getParameterSet("qualityCuts").getParameterSet("signalQualityCuts"))
  , isolationQualityCuts_(cfg.getParameterSet("qualityCuts").getParameterSet("isolationQualityCuts"))
  , vertexAssociator_(cfg.getParameterSet("qualityCuts"), consumesCollector())
  , signalMinPFEnFrac_(cfg.getParameter<double>("signalMinPFEnFrac"))
  , isolationMinPFEnFrac_(cfg.getParameter<double>("isolationMinPFEnFrac"))
  , verbosity_(cfg.getParameter<int>("verbosity"))
{
  std::cout << "<TallinnTauProducer::TallinnTauProducer (moduleLabel = " << moduleLabel_ << ")>:" << std::endl;

  jetSrc_ = cfg.getParameter<edm::InputTag>("jetSrc");
  jetToken_ = consumes<reco::PFJetCollection>(jetSrc_);

  pfCandSrc_ = cfg.getParameter<edm::InputTag>("pfCandSrc");
  pfCandToken_ = consumes<reco::PFCandidateCollection>(pfCandSrc_);

  produces<reco::PFTauCollection>();
  produces<reco::PFCandidateCollection>("splittedPFCands");
}

TallinnTauProducer::~TallinnTauProducer()
{}

void TallinnTauProducer::produce(edm::Event& evt, const edm::EventSetup& es)
{
  std::cout << "<TallinnTauProducer::produce (moduleLabel = " << moduleLabel_ << ")>:" << std::endl;

  std::unique_ptr<reco::PFTauCollection> pfTaus = std::make_unique<reco::PFTauCollection>();
  std::unique_ptr<reco::PFCandidateCollection> splittedPFCands = std::make_unique<reco::PFCandidateCollection>();

  reco::PFCandidateRefProd splittedPFCandsRefProd = evt.getRefBeforePut<reco::PFCandidateCollection>();
  // CV: to get Ref, see lines 135-136 of 
  //       https://cmssdt.cern.ch/lxr/source/Calibration/EcalCalibAlgos/src/ElectronRecalibSuperClusterAssociator.cc

  edm::Handle<reco::PFJetCollection> jets;
  evt.getByToken(jetToken_, jets);

  edm::Handle<reco::PFCandidateCollection> pfCands;
  evt.getByToken(pfCandToken_, pfCands);

  size_t numJets = jets->size();
  for ( size_t idxJet = 0; idxJet < numJets; ++idxJet ) {
    reco::PFJetRef jetRef(jets, idxJet);

    // CV: to convert reco::PFJetRef to edm::RefToBase<reco::Jet>, do
    //       edm::RefToBase<Jet> jetBaseRef(jetRef);

    reco::VertexRef primaryVertexRef = vertexAssociator_.associatedVertex(*jetRef);
    signalQualityCuts_.setPV(primaryVertexRef);
    isolationQualityCuts_.setPV(primaryVertexRef);

  }

  evt.put(std::move(pfTaus));
  evt.put(std::move(splittedPFCands), "splittedPFCands");
}

void TallinnTauProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("jetSrc", edm::InputTag("ak4PFJets"));
  desc.add<double>("minJetPt", 14.0);
  desc.add<double>("maxJetAbsEta", 2.5);
  desc.add<edm::InputTag>("pfCandSrc", edm::InputTag("particleFlow"));
  desc.add<std::string>("signalConeSize", "max(min(0.1, 3.0/pt()), 0.05)");
  desc.add<double>("isolationConeSize", 0.5);
  edm::ParameterSetDescription desc_qualityCuts;
  RecoTauQualityCuts::fillDescriptions(desc_qualityCuts);
  desc.add<edm::ParameterSetDescription>("qualityCuts", desc_qualityCuts);
  desc.add<double>("signalMinPFEnFrac", 0.);
  desc.add<double>("isolationMinPFEnFrac", 0.);
  desc.add<int>("verbosity", 0);
  descriptions.add("tallinnRecoTaus", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"                // DEFINE_FWK_MODULE()
DEFINE_FWK_MODULE(TallinnTauProducer);
