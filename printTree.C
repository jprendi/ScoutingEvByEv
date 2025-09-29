void printTree() {
    TFile* f = TFile::Open("output_numEvent100.root");
    TDirectoryFile* d = (TDirectoryFile*) f->Get("scoutingCollectionNtuplizer");
    TTree* tree = (TTree*) d->Get("tree");
    tree->Print();
}

