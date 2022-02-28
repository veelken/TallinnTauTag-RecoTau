#include "TallinnTauTag/RecoTau/interface/hashAuxFunctions.h"

#include <cstring>       // strcpy
#include <iomanip>       // std::fixed, std::setprecision()
#include <sstream>       // std::ostringstream
#include <openssl/sha.h> // SHA256

std::string
reco::tau::getHash_jet(const reco::Candidate::LorentzVector& p4)
{
  std::ostringstream input;
  input << std::fixed << std::setprecision(1) << p4.pt();
  input << std::fixed << std::setprecision(3) << p4.eta();
  input << std::fixed << std::setprecision(3) << p4.phi();
  input << std::fixed << std::setprecision(1) << p4.mass();
  const char* input_char = input.str().c_str();
  size_t input_length = input.str().length();
  unsigned char input_uchar[input_length];
  strcpy((char*)input_uchar, input_char);
  unsigned char sha256[SHA256_DIGEST_LENGTH];
  SHA256(input_uchar, input_length, sha256);
  std::string hash((char*)sha256);
  std::cout << "<getHash_jet>:" << std::endl;
  std::cout << " pT = " << p4.pt() << ", eta = " << p4.eta() << ", phi = " << p4.phi() << ", mass = " << p4.mass() << ":" 
            << " input = " << input.str() << ", hash = " << hash << std::endl;
  return hash;
}

std::string
reco::tau::getHash_pfCand(const reco::Candidate::LorentzVector& p4, reco::PFCandidate::ParticleType particleId, int charge)
{
  std::ostringstream input;
  input << std::fixed << std::setprecision(1) << p4.pt();
  input << std::fixed << std::setprecision(3) << p4.eta();
  input << std::fixed << std::setprecision(3) << p4.phi();
  input << (int)particleId;
  input << charge;
  const char* input_char = input.str().c_str();
  size_t input_length = input.str().length();
  unsigned char input_uchar[input_length];
  strcpy((char*)input_uchar, input_char);
  unsigned char sha256[SHA256_DIGEST_LENGTH];
  SHA256(input_uchar, input_length, sha256);
  std::string hash((char*)sha256);
  std::cout << "<getHash_pfCand>:" << std::endl;
  std::cout << " pT = " << p4.pt() << ", eta = " << p4.eta() << ", phi = " << p4.phi() << ", particleId = " << particleId << ", charge = " << charge << ":" 
            << " input = " << input.str() << ", hash = " << hash << std::endl;
  return hash;
}
