// Compilation:
// - bash:
// g++ -O3 fastMatch.cpp $(root-config --cflags --libs) -o fastMatch
// - tcsh
// g++ -O3 -march=native fastMatch.cpp `root-config --cflags --libs` -o fastMatch
//
//
// Exectution:
// ./fastMatch <distrib_A_name> <file_A.root> <distrib_B_name> <file_B.root>
//
// Example:
// ./fastMatch HLT hlt_output.root Prompt prompt_output.root

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <cmath>
#include <string>
#include <map>

const int ETA_BINS = 100;
const int PHI_BINS = 100;
const int NEIGHBOURSTOSCAN = 20;
const double ETA_MIN = -4.0;
const double ETA_MAX = 4.0;
const double PHI_MIN = -M_PI;
const double PHI_MAX = M_PI;
const double DR_THRESHOLD = 0.1;

int get_hash(double eta, double phi) {
  int eta_bin = floor((eta - ETA_MIN)/(ETA_MAX-ETA_MIN)*ETA_BINS);
  int phi_bin = floor((phi - PHI_MIN)/(PHI_MAX-PHI_MIN)*PHI_BINS);
  eta_bin = std::max(0, std::min(ETA_BINS-1, eta_bin));
  phi_bin = std::max(0, std::min(PHI_BINS-1, phi_bin));
  return eta_bin * PHI_BINS + phi_bin;
}

double delta_r(double eta1, double phi1, double eta2, double phi2) {
  double deta = eta1 - eta2;
  double dphi = fmod(phi1 - phi2 + M_PI, 2*M_PI) - M_PI;
  return sqrt(deta*deta + dphi*dphi);
}

std::string branch_name(const std::string& var, int pdg_id) {
  std::string tag = (pdg_id < 0) ? "n" + std::to_string(-pdg_id)
                                 : std::to_string(pdg_id);
  return "PF_" + var + "_" + tag;
  // -> "PF_eta_n211"
}

int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << " <distrib_A_name> <file_A.root> <distrib_B_name> <file_B.root>\n";
    return 1;
  }

  const std::string distrib_A = argv[1];
  const std::string file_A    = argv[2];
  const std::string distrib_B = argv[3];
  const std::string file_B    = argv[4];

  const char* TREE_NAME   = "scoutingCollectionNtuplizer/tree";

  // particle types
  const std::vector<int> pdg_ids = {211, -211, 130, 22, 13, -13, 1, 2};

  TFile f_A(file_A.c_str());
  TFile f_B(file_B.c_str());

  TTree* t_A = (TTree*)f_A.Get(TREE_NAME);
  TTree* t_B = (TTree*)f_B.Get(TREE_NAME);

  ULong64_t event_A, event_B;
  t_A->SetBranchAddress("event", &event_A);
  t_B->SetBranchAddress("event", &event_B);

  // per-particle branch pointers & histograms 
  struct ParticleData {
    // A
    std::vector<float>* eta_A = nullptr;
    std::vector<float>* phi_A = nullptr;
    std::vector<float>* pt_A  = nullptr;
    // B
    std::vector<float>* eta_B = nullptr;
    std::vector<float>* phi_B = nullptr;
    std::vector<float>* pt_B  = nullptr;
    // Histograms
    TH1F* h_pt_diff  = nullptr;
    TH1F* h_eta_diff = nullptr;
    TH1F* h_phi_diff = nullptr;
    TH1F* h_dr       = nullptr;
  };

  std::map<int, ParticleData> particles;

  for (int pdg : pdg_ids) {
    ParticleData& p = particles[pdg];
    std::string tag = std::to_string(pdg);

    // Wire up A branches
    t_A->SetBranchAddress(branch_name("eta", pdg).c_str(), &p.eta_A);
    t_A->SetBranchAddress(branch_name("phi", pdg).c_str(), &p.phi_A);
    t_A->SetBranchAddress(branch_name("pT",  pdg).c_str(), &p.pt_A);

    // Wire up B branches
    t_B->SetBranchAddress(branch_name("eta", pdg).c_str(), &p.eta_B);
    t_B->SetBranchAddress(branch_name("phi", pdg).c_str(), &p.phi_B);
    t_B->SetBranchAddress(branch_name("pT",  pdg).c_str(), &p.pt_B);

    // Book histograms
    p.h_pt_diff  = new TH1F(("pt_diff_"  + tag).c_str(), ("pT("  + distrib_B + "-" + distrib_A + ") pdg=" + tag).c_str(), 100, -5,   5  );
    p.h_eta_diff = new TH1F(("eta_diff_" + tag).c_str(), ("eta(" + distrib_B + "-" + distrib_A + ") pdg=" + tag).c_str(), 100, -0.1, 0.1);
    p.h_phi_diff = new TH1F(("phi_diff_" + tag).c_str(), ("phi(" + distrib_B + "-" + distrib_A + ") pdg=" + tag).c_str(), 100, -0.1, 0.1);
    p.h_dr       = new TH1F(("dr_"       + tag).c_str(), ("DeltaR pdg="           + tag).c_str(), 100,  0,   0.05);
  }

  // build B event index
  std::unordered_map<ULong64_t, ULong64_t> B_event_map;
  ULong64_t n_B = t_B->GetEntries();
  for (ULong64_t i = 0; i < n_B; i++) {
    t_B->GetEntry(i);
    B_event_map[event_B] = i;
  }

  // main matching loop
  ULong64_t n_A = t_A->GetEntries();

  for (ULong64_t i = 0; i < n_A; i++) {
    t_A->GetEntry(i);

    if (!B_event_map.count(event_A)) continue;
    t_B->GetEntry(B_event_map[event_A]);

    // loop over particle types
    for (int pdg : pdg_ids) {
      ParticleData& p = particles[pdg];

      const auto& eta_a = *p.eta_A;
      const auto& phi_a = *p.phi_A;
      const auto& pt_a  = *p.pt_A;

      const auto& eta_b = *p.eta_B;
      const auto& phi_b = *p.phi_B;
      const auto& pt_b  = *p.pt_B;

      // Build hash vectors
      std::vector<int> A_hash   (eta_a.size());
      std::vector<int> B_hash(eta_b.size());

      for (size_t j = 0; j < eta_a.size(); j++) A_hash[j] = get_hash(eta_a[j], phi_a[j]);
      for (size_t j = 0; j < eta_b.size(); j++) B_hash[j] = get_hash(eta_b[j], phi_b[j]);

      // B spatial map
      std::unordered_map<int, std::vector<int>> B_map;
      for (size_t j = 0; j < B_hash.size(); j++)
        B_map[B_hash[j]].push_back(j);

      std::set<int> used_B;

      // Match each A particle to its nearest B neighbour
      for (size_t h = 0; h < A_hash.size(); h++) {
        int hash    = A_hash[h];
        int eta_bin = hash / PHI_BINS;
        int phi_bin = hash % PHI_BINS;

        double best_dr  = DR_THRESHOLD;
        int    best_idx = -1;

        for (int d_eta = -NEIGHBOURSTOSCAN; d_eta <= NEIGHBOURSTOSCAN; d_eta++) {
          int eta_n = eta_bin + d_eta;
          if (eta_n < 0 || eta_n >= ETA_BINS) continue;

          for (int d_phi = -NEIGHBOURSTOSCAN; d_phi <= NEIGHBOURSTOSCAN; d_phi++) {
            int phi_n    = (phi_bin + d_phi + PHI_BINS) % PHI_BINS;
            int neighbor = eta_n * PHI_BINS + phi_n;

            if (!B_map.count(neighbor)) continue;

            for (int p_idx : B_map[neighbor]) {
              if (used_B.count(p_idx)) continue;

              double dr = delta_r(eta_a[h], phi_a[h], eta_b[p_idx], phi_b[p_idx]);
              if (dr < best_dr) { best_dr = dr; best_idx = p_idx; }
            }
          }
        }

        if (best_idx != -1) {
          used_B.insert(best_idx);
          p.h_pt_diff ->Fill(pt_b [best_idx] - pt_a [h]);
          p.h_eta_diff->Fill(eta_b[best_idx] - eta_a[h]);
          p.h_phi_diff->Fill(phi_b[best_idx] - phi_a[h]);
          p.h_dr      ->Fill(best_dr);
        }
      }
    } // end particle loop
  }   // end event loop

  // save histograms
  std::string out_file = "matching_" + distrib_A + "_vs_" + distrib_B + ".root";
  TFile fout(out_file.c_str(), "RECREATE");
  for (int pdg : pdg_ids) {
    ParticleData& p = particles[pdg];
    p.h_pt_diff ->Write();
    p.h_eta_diff->Write();
    p.h_phi_diff->Write();
    p.h_dr      ->Write();
  }
  fout.Close();

  std::cout << "Plots saved to " << out_file << std::endl;
}

