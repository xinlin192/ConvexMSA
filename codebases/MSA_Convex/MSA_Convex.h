/*###############################################################
## MODULE: MSA_Convex.h
## VERSION: 1.0 
## SINCE 2015-09-03 ##      
#################################################################
## Edited by MacVim
## Class Info auto-generated by Snippet 
################################################################*/

/* Imported Libraries */
using namespace std;
#include "stdio.h"
#include "stdlib.h"
#include <limits>
#include <vector>
#include <sstream> 
#include <iostream>
#include <fstream>

/* Self-defined Constants and Global Variables */
const double MIN_DOUBLE = -1*numeric_limits<double>::max();
const double MAX_DOUBLE = numeric_limits<double>::max();
const int NUM_DNA_TYPE = 4; 
const int NUM_MOVEMENT = 9;

/* Algorithmic Seeting */
const int MAX_1st_FW_ITER = 100;
const int MAX_2nd_FW_ITER = 100;
const int MAX_ADMM_ITER = 1000;

/* Define Scores and Other Constants */
const char GAP_NOTATION = '-';
const double C_I = 1; // penalty of insertion
const double C_D = 1.5; // penalty of deletion
const double C_MM = 1.8; // penalty of mismatch
const double C_M = 0; // penalty of match

/* Data Structure */
/*{{{*/
enum Action {
    INSERTION = 0,
    DELETION_A = 1,
    DELETION_T = 2, 
    DELETION_C = 3, 
    DELETION_G = 4,
    MATCH_A = 5,
    MATCH_T = 6, 
    MATCH_C = 7, 
    MATCH_G = 8, 
    UNDEFINED = 9
};
string action2str (Action action) {
    switch (action) {
        case INSERTION: return "Insertion";
        case DELETION_A: return "Deletion_A";
        case DELETION_T: return "Deletion_T";
        case DELETION_C: return "Deletion_C";
        case DELETION_G: return "Deletion_G";
        case MATCH_A: return "Match_A";
        case MATCH_T: return "Match_T";
        case MATCH_C: return "Match_C";
        case MATCH_G: return "Match_G";
        case UNDEFINED: return "Undefined";
    }
    return "";
}
class Cell {
    public:
        double score;   
        Action action;
        int dim;   
        vector<int> location; 
        char acidA, acidB;
        int ans_idx;
        Cell (int dim) {
            this->score = 0;
            this->action = UNDEFINED; this->dim = dim; for (int i = 0; i < dim; i ++) 
                location.push_back(-1);
            this->acidA = '?';
            this->acidB = '?';
            this->ans_idx = -1;
        }
        // convert to string:
        //    [(location vector), action, acidA, acidB, score] 
        string toString () {
            stringstream s;
            s << "[(";
            for (int i = 0 ; i < this->dim; i ++) {
                s << location[i];
                if (i < this->dim - 1) s << ",";
            }
            s << "), ";
            s << action2str(this->action) << ", ";
            s << this->acidA << ", " << this->acidB;
            s << ", " << this->score << ") ";
            return s.str();
        }
};

typedef vector<vector<char> > SequenceSet;
typedef vector<char> Sequence;

typedef vector<Cell> Trace;
typedef vector<Trace > Plane; // 2-d Cell Plane
typedef vector<Plane > Cube;  // 3-d Cell Cube

typedef vector<vector<double> > Matrix; // 2-d double matrix
typedef vector<Matrix > Tensor;  // 3-d double tensor
typedef vector<Tensor > Tensor4D; // 4-d double Tensor
typedef vector<Tensor4D > Tensor5D;  // 5-d double Tensor
/*}}}*/

/* T4 architecture */
const int INS_BASE_IDX = 0;
const int DEL_BASE_IDX = 1; // 1-A, 2-T, 3-C, 4-G
const int MTH_BASE_IDX = 5; // 5-A, 6-T, 7-C, 8-G
Action T4idx2Action [9] = {INSERTION, DELETION_A, DELETION_T, DELETION_C, DELETION_G, MATCH_A, MATCH_T, MATCH_C, MATCH_G};
int dna2T3idx (char dna) {
        if (dna == 'A') return 0;
        else if (dna == 'T') return 1;
        else if (dna == 'C') return 2;
        else if (dna == 'G') return 3;
        else { 
            cerr << "dna2T3idx issue: " << dna << endl;
            exit(1);
        }
    return -1;
}
char T3idx2dna (int idx) {
    if (idx == 0) return 'A';
    else if (idx == 1) return 'T';
    else if (idx == 2) return 'C';
    else if (idx == 3) return 'G';
    else {
        cerr << "T3idx2dna issue: " << idx << endl;
        exit(1);
    }
    return -1;
}

/* Define match identification function */
bool isMatch1 (char DNA1, char DNA2) {
    if (DNA1 > DNA2) {
        char temp = DNA1;
        DNA1 = DNA2;
        DNA2 = temp;
    }
    if ((DNA1 == 'A' and DNA2 == 'T') or (DNA1 == 'C' and DNA2 == 'G')) 
        return true;
    else return false;
}
bool isMatch2 (char DNA1, char DNA2) { return DNA1==DNA2; }

void set_C (Tensor5D& C, SequenceSet allSeqs) {
    int T0 = C.size();
    int T2 = C[0][0].size();
    int T3 = NUM_DNA_TYPE;
    int T4 = NUM_MOVEMENT;
    for (int n = 0; n < T0; n ++) {
        int T1 = C[n].size();
        for (int i = 0; i < T1; i ++) {
            for (int j = 0; j < T2; j ++) {
                for (int k = 0; k < T3; k ++) {
                    for (int m = 0; m < T4; m ++) {
                        if (m == INS_BASE_IDX) 
                            C[n][i][j][k][m] = C_I;
                        else if (DEL_BASE_IDX <= m and m < MTH_BASE_IDX) 
                            C[n][i][j][k][m] = C_D;
                        else if (MTH_BASE_IDX <= m) {
                            if (dna2T3idx(allSeqs[n][i]) == k)
                                C[n][i][j][k][m] = C_M;
                            else
                                C[n][i][j][k][m] = C_MM;
                        }
                        
                    }
                }
            }
        }
    }
   
}


/* Tensors auxiliary function */
/*{{{*/
void tensor5D_init (vector<Tensor4D>& C, SequenceSet& allSeqs, vector<int>& lenSeqs, int init_T2) {
    int numSeq = allSeqs.size();
    for (int n = 0; n < numSeq; n ++) {
        for (int i = 0; i < lenSeqs[n]; i ++) {
            Tensor tmp_tensor (init_T2, Matrix(NUM_DNA_TYPE, vector<double>(NUM_MOVEMENT, 0.0)));
            C[n].push_back(tmp_tensor);
        }
    }
}
void tensor4D_average (Tensor4D& dest, Tensor4D& src1, Tensor4D& src2) {
    int T1 = src1.size();
    for (int i = 0; i < T1; i ++) {
        int T2 = src1[i].size();
        for (int j = 0; j < T2; j ++) 
            for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                for (int m = 0; m < NUM_MOVEMENT; m ++)
                    dest[i][j][d][m] = 0.5*(src1[i][j][d][m] + src2[i][j][d][m]);
    }
}
double tensor4D_frob_prod (Tensor4D& src1, Tensor4D& src2) {
    double prod = 0.0;
    int T1 = src1.size();
    for (int i = 0; i < T1; i ++) {
        int T2 = src1[i].size();
        for (int j = 0; j < T2; j ++) 
            for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                for (int m = 0; m < NUM_MOVEMENT; m ++)
                    prod += src1[i][j][d][m] * src2[i][j][d][m];
    }
    return prod;
}
void tensor4D_lin_update (Tensor4D& dest, Tensor4D& src1, Tensor4D& src2, double ratio) {
    int T1 = src1.size();
    for (int i = 0; i < T1; i ++) {
        int T2 = src1[i].size();
        for (int j = 0; j < T2; j ++) 
            for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                for (int m = 0; m < NUM_MOVEMENT; m ++)
                    dest[i][j][d][m] += ratio * (src1[i][j][d][m] - src2[i][j][d][m]);
    }
}
void tensor4D_copy (Tensor4D& dest, Tensor4D& src1) {
    int N = src1.size();
    int T1 = src1.size();
    for (int i = 0; i < T1; i ++) {
        int T2 = src1[i].size();
        for (int j = 0; j < T2; j ++) 
            for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                for (int m = 0; m < NUM_MOVEMENT; m ++)
                    dest[i][j][d][m] = src1[i][j][d][m];
    }
    return ;
}
/*}}}*/

/* 3-d smith waterman algorithm */
void cube_smith_waterman (Tensor4D& S, Trace& trace, Tensor4D& M, Tensor4D& C, Sequence& data_seq) {
    /*{{{*/
    // 1. set up 3-d model
    int T1 = S.size() + 1;
    int T2 = S[0].size() + 1;
    int T3 = S[0][0].size();
    Cube cube (T1, Plane (T2, Trace (T3, Cell(3))));
    // 2. fill in the tensor
    double global_min_score = MAX_DOUBLE;
    int gmin_i = -1, gmin_j = -1, gmin_k = -1;
    for (int i = 0; i < T1; i ++) 
        for (int k = 0; k < T3; k ++) 
            cube[i][0][k].score = i * C_I;
    for (int i = 0; i < T1; i ++) {
        for (int j = 0; j < T2; j ++) {
            for (int k = 0; k < T3; k ++) {
                // cout << "i=" << i << ", j=" << j << ", k=" << k << endl;
                cube[i][j][k].location[0] = i;
                cube[i][j][k].location[1] = j;
                cube[i][j][k].location[2] = k;
                if (i == 0 or j == 0) continue;
                char data_dna = data_seq[i-1];
                int dna_idx = dna2T3idx(data_dna);
                vector<double> scores (NUM_MOVEMENT, 0.0); 
                // 1a. get insertion score
                // FIXME: no preference M for the insertion
                double ins_score = cube[i-1][j][k].score + C[i-1][j-1][k][INS_BASE_IDX];
                scores[INS_BASE_IDX] = ins_score;
                // 1b. get deletion score
                double del_score;
                for (int d = 0; d < NUM_DNA_TYPE ; d ++) {
                    del_score = cube[i][j-1][d].score +
                                  M[i-1][j-1][d][DEL_BASE_IDX+k] +
                                  C[i-1][j-1][d][DEL_BASE_IDX+k];
                    scores[DEL_BASE_IDX+d] = del_score;
                }
                // 1c. get max matach/mismatch score
                double mth_score;
                // cout << "dna: " << data_dna << ", " <<  dna_idx << ", k = " << k << endl;
                // d is inheriter, FIXME: verify the semantics of M
                for (int d = 0; d < NUM_DNA_TYPE ; d ++) {
                    // double mscore = (dna_idx==k)?C_M:C_MM;
                    mth_score = cube[i-1][j-1][d].score + 
                                   M[i-1][j-1][d][MTH_BASE_IDX+k] +
                                   C[i-1][j-1][d][MTH_BASE_IDX+k]; 
                    scores[MTH_BASE_IDX+d] = mth_score;
                }
                // 1d. get optimal action for the current cell
                double min_score = MAX_DOUBLE;
                int min_ansid = -1;
                Action min_action;
#ifdef CUBE_SMITH_WATERMAN_DEBUG
                cout << "scores: ";
#endif
                for (int ansid = 0; ansid < scores.size(); ansid++) {
#ifdef CUBE_SMITH_WATERMAN_DEBUG
                    cout << scores[ansid] << "," ;
#endif
                    if (scores[ansid] < min_score) {
                        min_ansid = ansid;
                        min_score = scores[ansid];
                        if (ansid == INS_BASE_IDX) 
                            min_action = INSERTION;
                        else if (DEL_BASE_IDX <= ansid and ansid < MTH_BASE_IDX) 
                            min_action = T4idx2Action[DEL_BASE_IDX+dna2T3idx(data_dna)];
                        else if (MTH_BASE_IDX <= ansid and ansid < NUM_MOVEMENT) 
                            min_action = T4idx2Action[MTH_BASE_IDX + k];
                    }
                }
#ifdef CUBE_SMITH_WATERMAN_DEBUG
                cout << endl;
#endif
                // 1e. assign the optimal score/action to the cell
                cube[i][j][k].score = min_score;
                cube[i][j][k].action = min_action;
                cube[i][j][k].ans_idx = min_ansid;
                switch (cube[i][j][k].action) {
                    case INSERTION: 
                        cube[i][j][k].acidA = data_dna; // data dna
                        cube[i][j][k].acidB = GAP_NOTATION; // model dna
                        break;
                    case DELETION_A: 
                        cube[i][j][k].acidA = GAP_NOTATION; 
                        cube[i][j][k].acidB = 'A'; 
                        break;
                    case DELETION_T: 
                        cube[i][j][k].acidA = GAP_NOTATION; 
                        cube[i][j][k].acidB = 'T'; 
                        break;
                    case DELETION_C: 
                        cube[i][j][k].acidA = GAP_NOTATION; 
                        cube[i][j][k].acidB = 'C'; 
                        break;
                    case DELETION_G: 
                        cube[i][j][k].acidA = GAP_NOTATION; 
                        cube[i][j][k].acidB = 'G'; 
                        break;
                    case MATCH_A:
                        cube[i][j][k].acidA = data_dna; 
                        cube[i][j][k].acidB = 'A'; 
                        break;
                    case MATCH_T: 
                        cube[i][j][k].acidA = data_dna; 
                        cube[i][j][k].acidB = 'T'; 
                        break;
                    case MATCH_C: 
                        cube[i][j][k].acidA = data_dna; 
                        cube[i][j][k].acidB = 'C'; 
                        break;
                    case MATCH_G:
                        cube[i][j][k].acidA = data_dna; 
                        cube[i][j][k].acidB = 'G'; 
                        break;
                    case UNDEFINED: cerr << "uncatched action." << endl; break;
                }
            }
        }
    }
    // 3. trace back
    // 1f. keep track of the globally optimal cell
    for (int i = T1-1, j = 1; j < T2; j ++) {
        for (int k = 0; k < T3; k ++) {
            double min_score = cube[i][j][k].score;
            if (min_score < global_min_score) {
                global_min_score = min_score;
                gmin_i = i;
                gmin_j = j;
                gmin_k = k;
            }
        }
    }
    // cout << "min_i: " << gmin_i << ", min_j: " << gmin_j << ", min_k: " << gmin_k << endl;
    if (gmin_i == 0 or gmin_j == 0) {
        trace.push_back(cube[gmin_i][gmin_j][gmin_k]);
        return; 
    }
    int i,j,k;
    for (i = gmin_i, j = gmin_j, k = gmin_k; i > 0 and j > 0; ) {
        trace.insert(trace.begin(), cube[i][j][k]);
        // cout << cube[i][j][k].toString() << endl;
        switch (cube[i][j][k].ans_idx) {
            case INSERTION: i--; break;
            case DELETION_A: j--; k = dna2T3idx('A'); break;
            case DELETION_T: j--; k = dna2T3idx('T'); break;
            case DELETION_C: j--; k = dna2T3idx('C'); break;
            case DELETION_G: j--; k = dna2T3idx('G'); break;
            case MATCH_A: i--; j--; k = dna2T3idx('A'); break;
            case MATCH_T: i--; j--; k = dna2T3idx('T'); break;
            case MATCH_C: i--; j--; k = dna2T3idx('C'); break;
            case MATCH_G: i--; j--; k = dna2T3idx('G'); break;
            case UNDEFINED: cerr << "uncatched action." << endl; break;
        }
    }
    // if (i == 0 and j == 0) return;
    // else trace.insert(trace.begin(), cube[1][1][dna2T3idx(data_seq[0])]);
    // 4. reintepret it as 4-d data structure
    int ntr = trace.size();
    for (int t = 0; t < ntr; t ++) {
        Cell tmp_cell = trace[t];
        i = tmp_cell.location[0];
        j = tmp_cell.location[1];
        k = tmp_cell.location[2];
        int m = tmp_cell.action;
        // NOTE: k now is the dna of j-1 position
        // we set the first d to be 'G'
        if (t == 0) 
            S[i-1][j-1][3][m] = 1.0;
        else
            S[i-1][j-1][trace[t-1].location[2]][m] = 1.0;
    }
    /*}}}*/
}

/* 2-d viterbi algorithm */
void viterbi_algo (Trace& trace, Tensor& transition) {
/*{{{*/
    int J = transition.size();
    int D1 = transition[0].size();
    int D2 = transition[0][0].size();
    Plane plane (J+1, Trace(D2, Cell(2)));
    // 1. pass forward
    for (int j = 0; j < J; j++) {
        vector<double> max_score (D2, MIN_DOUBLE);
        vector<int> max_d1 (D1, -1);
        for (int d1 = 0; d1 < D1; d1 ++) {
            for (int d2 = 0; d2 < D2; d2 ++) {
                double score = plane[j][d1].score + transition[j][d1][d2];
                if (score > max_score[d2]) {
                    max_score[d2] = score;
                    max_d1[d2] = d1;
                }
            }
        }
        for (int d2 = 0; d2 < D2; d2 ++) {
            int jp = j + 1;
            plane[jp][d2].location[0] = j;
            plane[jp][d2].location[1] = max_d1[d2];
            plane[jp][d2].score = max_score[d2];
            plane[jp][d2].acidA = T3idx2dna(max_d1[d2]);
            plane[jp][d2].acidB = T3idx2dna(d2);
        }
    }
    // 2. trace backward
    int j = J;
    double max_score = MIN_DOUBLE;
    int max_d2 = -1;
    for (int d2 = 0; d2 < D2; d2++) {
        if (plane[j][d2].score > max_score) {
            max_score = plane[j][d2].score;
            max_d2 = d2;
        }
    }
    trace.insert(trace.begin(), plane[j][max_d2]);   
    for (j = J-1; j > 0; j--) {
        int last_d2 = dna2T3idx(trace[0].acidA);
        trace.insert(trace.begin(), plane[j][last_d2]);   
    }
    // cout << "viterbi_max: " << trace[J-1].score << endl;
    return ;
}
/*}}}*/
