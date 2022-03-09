#include "TallinnTauTag/RecoTau/interface/TFGraphCache.h"

#include "FWCore/Utilities/interface/Exception.h" // edm::Exception

using namespace reco::tau;

TFGraphCache::TFGraphCache(const edm::ParameterSet& cfg)
  : inputLayerName_(cfg.getParameter<std::string>("inputLayerName"))
  , outputLayerName_(cfg.getParameter<std::string>("outputLayerName"))
{
  std::string inputFileName = cfg.getParameter<std::string>("inputFile");
  std::string inputFileName_full = edm::FileInPath(inputFileName).fullPath();

  std::string graphName = cfg.getParameter<std::string>("graphName");
  std::cout << "<TFGraphCache::TFGraphCache>: loading graph = '" << graphName << "' from input file = '" << inputFileName_full << "'." << std::endl;
  graph_ = tensorflow::loadGraphDef(inputFileName_full);
  assert(graph_);

  // CV: read names of DNN input and output layers directly from the TensorFlow graph
  inputLayerName_ = (*graph_).node(0).name();
  outputLayerName_ = (*graph_).node((*graph_).node_size() - 1).name();
  std::cout << " inputLayerName = " << inputLayerName_ << std::endl;
  std::cout << " outputLayerName = " << outputLayerName_ << std::endl;

  // set tensorflow verbosity to warning level
  tensorflow::setLogging("2");
}

TFGraphCache::~TFGraphCache()
{
  delete graph_;
  graph_ = nullptr;
}

const tensorflow::GraphDef&
TFGraphCache::getGraph() const
{
  return *graph_;
}

const std::string&
TFGraphCache::getInputLayerName() const
{
  return inputLayerName_;
}

const std::string&
TFGraphCache::getOutputLayerName() const
{
  return outputLayerName_;
}

void 
TFGraphCache::fillDescriptions(edm::ParameterSetDescription& desc)
{
  desc.add<std::string>("inputFile", "");
  desc.add<std::string>("graphName", "");
  desc.add<std::string>("inputLayerName", "input");
  desc.add<std::string>("outputLayerName", "output");
}
