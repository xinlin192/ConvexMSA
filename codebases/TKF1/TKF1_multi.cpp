#include<iostream>
#include"TKF1.h"

using namespace std;

int main(int argc, char** argv){
	
	if( argc < 5 ){
		cerr << "./TKF1_multi [N] [length] [sub_rate] [del_rate] (output)" << endl;
		exit(0);
	}
	
	//srand(time(NULL));
	srand(1);
	int N = atoi(argv[1]);
	int L = atoi(argv[2]);
	double sub_rate = atof(argv[3]);
	double del_rate = atof(argv[4]);
	double ins_rate = del_rate*L/(1+L);

	double alpha = compute_alpha(del_rate);
	double beta = compute_beta(ins_rate,del_rate);
	double gamma = compute_gamma(ins_rate,del_rate);
	double subProb = compute_subProb(sub_rate);

    char* fname;
    string outputFile ("output");
    if ( argc > 5 ) outputFile = argv[5];
    string msa_file = outputFile+".msa";
    string gtco_file = outputFile+".gtco";
    string info_file = outputFile+".info";
    string pair_file = outputFile+".pair";
	
	// generate ancestral sequence (geometric with p = ins_rate/del_rate)
	vector<char> anc_seq;
	for(int i = 0; i < L; i++) anc_seq.push_back(sample_char());
	
    // print to .info file (ground truth)
    ofstream info_out(info_file.c_str());
    info_out << "par_seq: ";
    for(int i = 0; i < anc_seq.size(); i++)
        info_out << anc_seq[i] ;
    info_out << endl;
	info_out << "N= " << N << ", L=" << L << endl;
	info_out << "sub=" << sub_rate << ", ins=" << ins_rate << ", del=" << del_rate << endl;
	info_out << "P(taxon survive)=" << alpha << endl;
	info_out << "P(insert|taxon not die)=" << beta << endl;
    info_out << "P(insert|taxon die)=" << gamma << endl;
	info_out << "P(subsitution)=" << subProb << endl;
    info_out.close();

    vector<vector<char> > allModelSeqs, allDataSeqs;
    ofstream msa_out(msa_file.c_str());
    ofstream pair_out(pair_file.c_str());
	for(int n = 0; n < N; n++){
		//descent sequence 
		vector<vector<char> > des_seq;
		//insert before 1st match/delete
		des_seq.push_back(vector<char>());
		while(rand_u01() < beta) des_seq[0].push_back(sample_char());
		for(int i = 0; i < anc_seq.size(); i++){
			des_seq.push_back(vector<char>());
			//evolution for the i-th taxon of ancestral seq
			double has_insert_prob;
			if( rand_u01() < alpha ){ //taxon survive
				if (rand_u01() < subProb) des_seq[i+1].push_back(sample_char());
                else des_seq[i+1].push_back(anc_seq[i]);
				has_insert_prob = beta;
			} else {
				des_seq[i+1].push_back('-');
				has_insert_prob = gamma;
			}
			
			if( rand_u01() < has_insert_prob ){
				des_seq[i+1].push_back(sample_char());
				while(rand_u01() < beta)
					des_seq[i+1].push_back(sample_char());
			}
		}
        vector<char> line1;
        vector<char> line2;
        //initial insertion
        for(int j = 0; j < des_seq[0].size(); j++){
            line1.push_back('-');
            line2.push_back(des_seq[0][j]);
        }
        //for each ancestral taxon
        for(int i=0;i<anc_seq.size();i++){
            line1.push_back(anc_seq[i]);
            line2.push_back(des_seq[i+1][0]);
            for(int j=1;j<des_seq[i+1].size();j++){
                line1.push_back('-');
                line2.push_back(des_seq[i+1][j]);
            }
        }
        allModelSeqs.push_back(line1);
        allDataSeqs.push_back(line2);

        // print to .pair file
        for (int i =0; i < line1.size(); i++) 
            pair_out << line1[i];
        pair_out << endl;
        for (int i =0; i < line2.size(); i++) 
            pair_out << line2[i];
        pair_out << endl;

        // print to .msa file
        for(int i=0;i<des_seq.size();i++){
            for(int j=0;j<des_seq[i].size();j++)
                if(des_seq[i][j] != '-')
                    msa_out << des_seq[i][j];
        }
        msa_out << endl;
	}
    pair_out.close();
    msa_out.close();
    int numSeq = N;
    vector<vector<char> > allCOSeqs (numSeq, vector<char>());
    vector<int> pos(numSeq, 0);
    while (true) {
        set<int> insertion_ids;
        for (int i = 0; i < N; i ++) {
            if (pos[i] >= allModelSeqs[i].size()) continue;
            if (allModelSeqs[i][pos[i]] == '-') 
                insertion_ids.insert(i);
        }
        if (insertion_ids.size() != 0) {
            // insertion exists
            for (int i = 0; i < numSeq; i ++) {
                if (insertion_ids.find(i)==insertion_ids.end()) // not in set
                    allCOSeqs[i].push_back('-');
                else { // in set
                    allCOSeqs[i].push_back(allDataSeqs[i][pos[i]++]);
                }
            }
        } else { // no insertion
            for (int i = 0; i < numSeq; i ++) 
                allCOSeqs[i].push_back(allDataSeqs[i][pos[i]++]);
        }
        // terminating
        bool terminated = true;
        for (int i = 0; i < numSeq; i ++) 
            if (pos[i] != allModelSeqs[i].size()) {
                terminated = false; 
                break;
            }
        if (terminated) break;
    }
    // print to gtco
    ofstream gtco_out(gtco_file.c_str());
    for (int i = 0; i < numSeq; i ++) {
        for (int j = 0; j < allCOSeqs[i].size(); j++)  {
            gtco_out << allCOSeqs[i][j];
        }
        gtco_out << endl;
    }
    gtco_out.close();
}
