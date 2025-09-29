
// -*- C++ -*-
//
// Package:    ??/??
// Class:      ScoutingCollectionNtuplizer
//
/**\class ScoutingCollectionNtuplizer ScoutingCollectionNtuplizer.cc 
          ??/??/plugins/ScoutingCollectionNtuplizer.cc

Description: ScoutingCollectionNtuplizer ...
Getting the full collection as in ScoutingCollectionMonitor out ! 
*/
//
// Original Author:  Jessica Prendi
//         Created:  Mon, 29 Sep 2025 Xx:Yy:Zz GMT
//
//
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include <TLorentzVector.h>
#include <cmath>
#include <memory>
#include <vector>
#include <TTree.h>
#include <vector>
#include <string>

#include "DataFormats/Scouting/interface/Run3ScoutingParticle.h"




class ScoutingCollectionNtuplizer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit ScoutingCollectionNtuplizer(const edm::ParameterSet&);
 ~ScoutingCollectionNtuplizer() override = default;
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
private:

    void createTree();
    void analyze(const edm::Event&, const edm::EventSetup&) override;

    template <typename T>
    void setToken(edm::EDGetTokenT<T>& token, const edm::ParameterSet& iConfig, std::string name) {
        const auto inputTag = iConfig.getParameter<edm::InputTag>(name);
        if (!inputTag.encode().empty()) {
            token = consumes(inputTag);
        }
    }

    template <typename T>
    bool getValidHandle(const edm::Event& iEvent,
                        const edm::EDGetTokenT<T>& token,
                        edm::Handle<T>& handle,
                        const std::string& label) {
      iEvent.getByToken(token, handle);
      return handle.isValid();
    }
    
    const edm::EDGetTokenT<std::vector<Run3ScoutingParticle>> pfcandsToken_;
    TTree* tree_;

   // PF candidates pT
    std::vector<float> PF_pT_211_;
    std::vector<float> PF_pT_n211_;
    std::vector<float> PF_pT_130_;
    std::vector<float> PF_pT_22_;
    std::vector<float> PF_pT_13_;
    std::vector<float> PF_pT_n13_;
    std::vector<float> PF_pT_1_;
    std::vector<float> PF_pT_2_;

};


ScoutingCollectionNtuplizer::ScoutingCollectionNtuplizer(const edm::ParameterSet& iConfig): 
    pfcandsToken_             (consumes<std::vector<Run3ScoutingParticle> >     (iConfig.getParameter<edm::InputTag>("pfcands")))
{
    usesResource("TFileService");
    createTree();
}


// ScoutingCollectionNtuplizer::~ScoutingCollectionNtuplizer() {
// // do anything here that needs to be done at desctruction time
//  // (e.g. close files, deallocate resources etc.)
//}

void ScoutingCollectionNtuplizer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup){
    using namespace edm;
    using namespace std;
    edm::Handle<std::vector<Run3ScoutingParticle>> pfcandsH;

    PF_pT_211_.clear();
    PF_pT_n211_.clear();
    PF_pT_130_.clear();
    PF_pT_22_.clear();
    PF_pT_13_.clear();
    PF_pT_n13_.clear();
    PF_pT_1_.clear();
    PF_pT_2_.clear();

    if (!getValidHandle(iEvent, pfcandsToken_, pfcandsH, "PF candidates")) {
    return;
  }

  for (const auto& cand : *pfcandsH) {
    switch (cand.pdgId()) {
      case 211:
        PF_pT_211_.push_back(cand.pt());
        break;

      case -211:
        PF_pT_n211_.push_back(cand.pt());
        break;

      case 130:
        PF_pT_130_.push_back(cand.pt());
        break;

      case 22:
        PF_pT_22_.push_back(cand.pt());
        break;

      case 13:
        PF_pT_13_.push_back(cand.pt());
        break;

      case -13:
        PF_pT_n13_.push_back(cand.pt());
        break;

      case 1:
        PF_pT_1_.push_back(cand.pt());
        break;

      case 2:
        PF_pT_2_.push_back(cand.pt());
        break;
    }
  }

    tree_->Fill();
}


void ScoutingCollectionNtuplizer::createTree(){
    edm::Service<TFileService> fs;
    tree_ = fs->make<TTree>("tree", "tree");

    tree_->Branch("PF_pT_211", &PF_pT_211_);
    tree_->Branch("PF_pT_n211", &PF_pT_n211_);
    tree_->Branch("PF_pT_130", &PF_pT_130_);
    tree_->Branch("PF_pT_22", &PF_pT_22_);
    tree_->Branch("PF_pT_13", &PF_pT_13_);
    tree_->Branch("PF_pT_n13", &PF_pT_n13_);
    tree_->Branch("PF_pT_1", &PF_pT_1_);
    tree_->Branch("PF_pT_2", &PF_pT_2_);


}

void ScoutingCollectionNtuplizer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("pfcands", edm::InputTag("hltScoutingPFPacker"));
  descriptions.addWithDefaultLabel(desc);
}


DEFINE_FWK_MODULE(ScoutingCollectionNtuplizer);
