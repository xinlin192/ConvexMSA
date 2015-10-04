/*################################################################ 
## MODULE: MSA_Convex.cpp
## VERSION: 1.0 
## SINCE 2015-09-01
##      
#################################################################
## Edited by MacVim
## Class Info auto-generated by Snippet 
################################################################*/

#include "MSA_Convex.h"

/* Debugging option */
//#define RECURSION_TRACE
//#define CUBE_SMITH_WATERMAN_DEBUG
//#define SECOND_SUBPROBLEM_DEBUG

/* Programming Setting option */
#define ADMM_EARLY_STOP

/* 
   The first sequence is observed. 
   The second sequence is the one to be aligned with the observed one.
   */
void usage () { cout << "./PSA_CUBE [seq_file]" << endl;
    cout << "seq_file should contain two DNA sequence in its first line and second line. " << endl;
    cout << "The first sequence is observed. " << endl;
    cout << "The second sequence is the one to be aligned with the observed one." << endl;
}

int get_init_model_length (vector<int>& lenSeqs) {
    int max_seq_length = -1;
    int numSeq = lenSeqs.size(); 
    for (int i = 0; i < numSeq; i ++)
        if (lenSeqs[i] > max_seq_length) 
            max_seq_length = lenSeqs[i];
    return max_seq_length;
}

void tensor4D_dump (Tensor4D& W) {
    int T1 = W.size();
    for (int i = 0; i < T1; i ++) {
        int T2 = W[i].size();
        for (int j = 0; j < T2; j ++) 
            for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                for (int m = 0; m < NUM_MOVEMENT; m ++)
                    if (W[i][j][d][m] > 0.0)
                        cout << "(i="  << i
                            << ", j=" << j
                            << ", d=" << d
                            << ", m=" << m
                            << "): " << W[i][j][d][m]
                            << endl;
    }
}

double get_sub1_cost (Tensor5D& W, Tensor5D& Z, Tensor5D& Y, Tensor5D& C, double& mu, SequenceSet& allSeqs) {
    int numSeq = W.size();
    int T2 = W[0].size();
    double lin_term = 0.0, qua_term = 0.0;
    for (int n = 0; n < numSeq; n ++) {
        int T1 = W[n].size();
        for (int i = 0; i < T1; i ++) 
            for (int j = 0; j < T2; j ++) 
                for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                    for (int m = 0; m < NUM_MOVEMENT; m ++) {
                        double sterm = W[n][i][j][d][m] - Z[n][i][j][d][m] + 1.0/mu*Y[n][i][j][d][m];
                        lin_term += C[n][i][j][d][m] * W[n][i][j][d][m];
                        qua_term += 0.5 * mu * sterm * sterm;
                    }
    }
    return lin_term + qua_term;
}

double get_sub2_cost (Tensor5D& W, Tensor5D& Z, Tensor5D& Y, double& mu, SequenceSet& allSeqs) {
    int numSeq = W.size();
    int T2 = W[0].size();
    double qua_term = 0.0;
    for (int n = 0; n < numSeq; n ++) {
        int T1 = W[n].size();
        for (int i = 0; i < T1; i ++) 
            for (int j = 0; j < T2; j ++) 
                for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                    for (int m = 0; m < NUM_MOVEMENT; m ++) {
                        double sterm = W[n][i][j][d][m] - Z[n][i][j][d][m] + 1.0/mu*Y[n][i][j][d][m];
                        qua_term += sterm * sterm;
                    }
    }
    return qua_term;
}

double first_subproblem_log (int fw_iter, Tensor4D& W, Tensor4D& Z, Tensor4D& Y, Tensor4D& C, double mu) {
    double cost = 0.0, lin_term = 0.0, qua_term = 0.0;
    double Ws = tensor4D_frob_prod (W, W); 
    int T1 = W.size();
    int T2 = W[0].size();
    for (int i = 0; i < T1; i ++) 
        for (int j = 0; j < T2; j ++) 
            for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                for (int m = 0; m < NUM_MOVEMENT; m ++) {
                    double sterm =  (W[i][j][d][m] - Z[i][j][d][m] + 1.0/mu*Y[i][j][d][m]);
                    lin_term += C[i][j][d][m] * W[i][j][d][m];
                    qua_term += 0.5*mu *sterm * sterm;
                }
    cost = lin_term + qua_term;
    cout << "[FW1] iter=" << fw_iter
         << ", ||W||^2: " << Ws 
         << ", lin_term: " << lin_term 
         << ", qua_sterm: " << qua_term
         << ", cost=" << cost 
         << endl;
}
double second_subproblem_log (int fw_iter, Tensor5D& W, Tensor5D& Z, Tensor5D& Y, double mu) {
    double cost = 0.0,  qua_term = 0.0;
    int numSeq = W.size();
    double Ws = 0.0;
    for (int n = 0; n < numSeq; n ++) 
        Ws += tensor4D_frob_prod (W[n], W[n]); 
    for (int n = 0; n < numSeq; n ++) {
        int T1 = W[n].size();
        int T2 = W[n][0].size();
        for (int i = 0; i < T1; i ++) 
            for (int j = 0; j < T2; j ++) 
                for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                    for (int m = 0; m < NUM_MOVEMENT; m ++) {
                        double sterm =  (W[n][i][j][d][m] - Z[n][i][j][d][m] + 1.0/mu*Y[n][i][j][d][m]);
                        qua_term += 0.5*mu *sterm * sterm;
                    }
    }
    cost = qua_term;
    cout << "[FW2] iter=" << fw_iter 
         << ", ||W||^2: " << Ws  
         << ", qua_sterm: " << qua_term
         << ", cost=" << cost  
         << endl;
}

/* We resolve the first subproblem through the frank-wolfe algorithm */
void first_subproblem (Tensor4D& W, Tensor4D& Z, Tensor4D& Y, Tensor4D& C, double& mu, Sequence data_seq) {
    /*{{{*/
    // 1. Find the update direction
    int T1 = W.size();
    int T2 = W[0].size();
    Tensor4D M (T1, Tensor(T2, Matrix(NUM_DNA_TYPE, vector<double>(NUM_MOVEMENT, 0.0)))); 
    // reinitialize to all-zero matrix
    for (int i = 0; i < T1; i ++) 
        for (int j = 0; j < T2; j ++) 
            for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                for (int m = 0; m < NUM_MOVEMENT; m ++)
                    W[i][j][d][m] = 0.0;
    int fw_iter = -1;
    first_subproblem_log(fw_iter, W, Z, Y, C, mu);
    while (fw_iter < MAX_1st_FW_ITER) {
        fw_iter ++;
        for (int i = 0; i < T1; i ++) 
            for (int j = 0; j < T2; j ++) 
                for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                    for (int m = 0; m < NUM_MOVEMENT; m ++)
                        M[i][j][d][m] = mu*(W[i][j][d][m] - Z[i][j][d][m]) + Y[i][j][d][m]; 
        Tensor4D S (T1, Tensor(T2, Matrix(NUM_DNA_TYPE, vector<double>(NUM_MOVEMENT, 0.0)))); 
        Trace trace (0, Cell(3));
        cube_smith_waterman (S, trace, M, C, data_seq);
        tensor4D_dump(S);

        // 2. Exact Line search: determine the optimal step size \gamma
        // gamma = [ ( C + Y_1 + mu*W_1 - mu*Z ) dot (W_1 - S) ] / (mu* || W_1 - S ||^2)
        double numerator = 0.0, denominator = 0.0;
        for (int i = 0; i < T1; i ++) 
            for (int j = 0; j < T2; j ++) 
                for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                    for (int m = 0; m < NUM_MOVEMENT; m ++) {
                        double wms = W[i][j][d][m] - S[i][j][d][m];
                        numerator += (C[i][j][d][m] + Y[i][j][d][m] + mu*W[i][j][d][m] - mu*Z[i][j][d][m]) * wms;
                        denominator += mu * wms * wms;
                    }
        double gamma = numerator / denominator;
        if (fw_iter == 0) gamma = 1.0;
        gamma = max(gamma, 0.0);
        gamma = min(gamma, 1.0);
        cout << "gamma: " << gamma << ", mu*||W-S||^2: " << denominator << endl;

        // 3. update W
        for (int i = 0; i < T1; i ++) 
            for (int j = 0; j < T2; j ++) 
                for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                    for (int m = 0; m < NUM_MOVEMENT; m ++)
                        W[i][j][d][m] = (1-gamma) * W[i][j][d][m] + gamma* S[i][j][d][m];

        // 4. output iteration tracking info
        first_subproblem_log(fw_iter, W, Z, Y, C, mu);
        // 5. early stop condition
        // if (-1e-6 < gamma and gamma < 1e-6) {
        if (fabs(gamma) < EPS_1st_FW) {
            cout << "gamma=" << gamma << ", early stop!" << endl;
            break; 
        }
        // NOTE: remove this after debug the second subproblem
        // if (fw_iter == 0) break;
    }
    return; 
    /*}}}*/
}

/* We resolve the second subproblem through sky-plane projection */
void second_subproblem (Tensor5D& W, Tensor5D& Z, Tensor5D& Y, double& mu, SequenceSet& allSeqs, vector<int> lenSeqs) {
    int numSeq = allSeqs.size();
    int T2 = W[0][0].size();
    // reinitialize W_2 to all-zero matrix
    for (int n = 0; n < numSeq; n ++) {
        int T1 = W[n].size();
        for (int i = 0; i < T1; i ++)  
            for (int j = 0; j < T2; j ++) 
                for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                    for (int m = 0; m < NUM_MOVEMENT; m ++) 
                        W[n][i][j][d][m] = 0.0;
    }

    vector<Tensor4D> delta (numSeq, Tensor4D(0, Tensor(T2, Matrix(NUM_DNA_TYPE,
                        vector<double>(NUM_MOVEMENT, 0.0)))));  
    tensor5D_init (delta, allSeqs, lenSeqs, T2);
    Tensor tensor (T2, Matrix (NUM_DNA_TYPE, vector<double>(NUM_DNA_TYPE, 0.0)));
    Matrix mat_insertion (T2, vector<double>(NUM_DNA_TYPE, 0.0));

    int fw_iter = -1;
    while (fw_iter < MAX_2nd_FW_ITER) {
        fw_iter ++;
        // 1. compute delta
        for (int n = 0; n < numSeq; n ++) {
            int T1 = W[n].size();
            for (int i = 0; i < T1; i ++) { 
                for (int j = 0; j < T2; j ++) 
                    for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                        for (int m = 0; m < NUM_MOVEMENT; m ++) {
                            delta[n][i][j][d][m] = -1.0 * mu * (W[n][i][j][d][m] - Z[n][i][j][d][m] + (1.0/mu)*Y[n][i][j][d][m]);
#ifdef SECOND_SUBPROBLEM_DEBUG
                            if (delta[n][i][j][d][m] > 0)
                                cout <<"delta: " << n << "," << i << "," << j << "," << d  << "," << m << ": "
                                     << delta[n][i][j][d][m] << endl;
#endif
                            if (m == DELETION_A or m == MATCH_A)
                                tensor[j][d][dna2T3idx('A')] += max(0.0, delta[n][i][j][d][m]);
                            else if (m == DELETION_T or m == MATCH_T)
                                tensor[j][d][dna2T3idx('T')] += max(0.0, delta[n][i][j][d][m]);
                            else if (m == DELETION_C or m == MATCH_C)
                                tensor[j][d][dna2T3idx('C')] += max(0.0, delta[n][i][j][d][m]);
                            else if (m == DELETION_G or m == MATCH_G)
                                tensor[j][d][dna2T3idx('G')] += max(0.0, delta[n][i][j][d][m]);
                            else if (m == INSERTION) {
                                mat_insertion[j][d] += max(0.0, delta[n][i][j][d][m]);
                            }
                        }
            }
        }
#ifdef SECOND_SUBPROBLEM_DEBUG
        cout << "tensor transition input list:" << endl;
        for (int j = 0; j < T2; j ++) 
            for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                for (int k = 0; k < NUM_DNA_TYPE; k ++) {
                    if (tensor[j][d][k] > 0)
                    cout << "(" << j << ", " << d << ", " << k << ")=" << tensor[j][d][k] << endl;
                }
#endif

        double delta_square = 0.0;
        for (int n = 0; n < numSeq; n ++) 
            delta_square += tensor4D_frob_prod (delta[n], delta[n]);
        cout << "delta_square: " << delta_square << endl;
        
        if ( delta_square < 10e-6 ) {
            cout << "small delta. early stop." << endl;
            break;
        }
        // 2. determine the trace: run viterbi algorithm
        Trace trace (0, Cell(2)); // 1d: j, 2d: ATCG
        refined_viterbi_algo (trace, tensor, mat_insertion);
        Tensor5D S (numSeq, Tensor4D(0, Tensor(T2, Matrix(NUM_DNA_TYPE, vector<double>(NUM_MOVEMENT, 0.0))))); 
        tensor5D_init (S, allSeqs, lenSeqs, T2);

        // 3. recover values for S 
        for (int n = 0; n < numSeq; n ++) {
            int T1 = S[n].size();
            for (int i = 0; i < T1; i ++) {
                // 3b. set a number of selected elements to 1
                for (int t = 0; t < trace.size(); t++) {
                    int sj = trace[t].location[0];
                    int sd = trace[t].location[1];
                    for (int m = 0; m < NUM_MOVEMENT; m ++)
                        if (delta[n][i][sj][sd][m] > 0.0) 
                            S[n][i][sj][sd][m] = 1.0;
                }
            }
        }
#ifdef SECOND_SUBPROBLEM_DEBUG
       cout << "Result of viterbi:" << endl;
       for (int t = 0; t < trace.size(); t++) 
            cout << "(" <<  trace[t].location[0] << ", " << trace[t].acidA << ", "<< trace[t].acidB << ")=" << trace[t].score << endl;
#endif
        double S_s = 0.0;
        for (int n = 0; n < numSeq; n ++) 
            S_s += tensor4D_frob_prod (S[n], S[n]);
        cout << "S_s: " << S_s << endl;

        // 4. Exact Line search: determine the optimal step size \gamma
        // gamma = [ ( Y_2 + mu*W_2 - mu*Z ) dot (W_2 - S) ] / || W_2 - S ||^2
        //           ---------------combo------------------
        double numerator = 0.0, denominator = 0.0;
        for (int n = 0; n < numSeq; n ++) {
            int T1 = S[n].size();
            for (int i = 0; i < T1; i ++) 
                for (int j = 0; j < T2; j ++) 
                    for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                        for (int m = 0; m < NUM_MOVEMENT; m ++) {
                            double wms = W[n][i][j][d][m] - S[n][i][j][d][m];
                            numerator += (Y[n][i][j][d][m] + mu*W[n][i][j][d][m] - mu*Z[n][i][j][d][m]) * wms;
                            denominator += mu * wms * wms;
                        }
        }
        cout << "numerator: " << numerator << ", denominator: " << denominator << endl;
        if ( denominator < 10e-6) {
            cout << "small denominator: " << denominator << endl;
            break;
        }
        double gamma = numerator / denominator;
        // if (fw_iter == 0) gamma = 1.0;
        gamma = max(gamma, 0.0);
        gamma = min(gamma, 1.0);
        cout << "gamma: " << gamma << ", mu*||W-S||^2: " << denominator << endl;

        // 3. update W
        for (int n = 0; n < numSeq; n ++) {
            int T1 = S[n].size();
            for (int i = 0; i < T1; i ++) 
                for (int j = 0; j < T2; j ++) 
                    for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                        for (int m = 0; m < NUM_MOVEMENT; m ++)
                            W[n][i][j][d][m] = (1-gamma) * W[n][i][j][d][m] + gamma* S[n][i][j][d][m];
        }

        // 4. output iteration tracking info
        second_subproblem_log(fw_iter, W, Z, Y, mu);
        // 5. early stop condition
        if (fabs(gamma) < EPS_2nd_FW) {
            cout << "gamma=" << gamma << ", early stop!" << endl;
            break; 
        }
    }
    return;
}

Tensor5D CVX_ADMM_MSA (SequenceSet& allSeqs, vector<int>& lenSeqs, int T2) {
    /*{{{*/
    // 1. initialization
    int numSeq = allSeqs.size();
    vector<Tensor4D> Z (numSeq, Tensor4D(0, Tensor(T2, Matrix(NUM_DNA_TYPE,
                        vector<double>(NUM_MOVEMENT, 0.0)))));  
    vector<Tensor4D> C (numSeq, Tensor4D(0, Tensor(T2, Matrix(NUM_DNA_TYPE,
                        vector<double>(NUM_MOVEMENT, 0.0)))));  
    vector<Tensor4D> W_1 (numSeq, Tensor4D(0, Tensor(T2, Matrix(NUM_DNA_TYPE,
                        vector<double>(NUM_MOVEMENT, 0.0)))));  
    vector<Tensor4D> W_2 (numSeq, Tensor4D(0, Tensor(T2, Matrix(NUM_DNA_TYPE,
                        vector<double>(NUM_MOVEMENT, 0.0)))));  
    vector<Tensor4D> Y_1 (numSeq, Tensor4D(0, Tensor(T2, Matrix(NUM_DNA_TYPE,
                        vector<double>(NUM_MOVEMENT, 0.0)))));  
    vector<Tensor4D> Y_2 (numSeq, Tensor4D(0, Tensor(T2, Matrix(NUM_DNA_TYPE,
                        vector<double>(NUM_MOVEMENT, 0.0)))));  
    tensor5D_init (Z, allSeqs, lenSeqs, T2);
    tensor5D_init (C, allSeqs, lenSeqs, T2);
    tensor5D_init (W_1, allSeqs, lenSeqs, T2);
    tensor5D_init (W_2, allSeqs, lenSeqs, T2);
    tensor5D_init (Y_1, allSeqs, lenSeqs, T2);
    tensor5D_init (Y_2, allSeqs, lenSeqs, T2);
    set_C (C, allSeqs);

    // 2. ADMM iteration
    int iter = 0;
    double mu = 0.001;
    double prev_CoZ = MAX_DOUBLE;
    while (iter < MAX_ADMM_ITER) {
        // 2a. Subprogram: FrankWolf Algorithm
        // NOTE: parallelize this for to enable parallelism
        for (int n = 0; n < numSeq; n++) {
            cout << "----------------------n=" << n <<"-----------------------------------------" << endl;
            first_subproblem (W_1[n], Z[n], Y_1[n], C[n], mu, allSeqs[n]);
            tensor4D_dump (W_1[n]);
        }
        double sub1_cost = get_sub1_cost (W_1, Z, Y_1, C, mu, allSeqs);
        cout << "=============================================================================" << endl;
        // 2b. Subprogram: 
        second_subproblem (W_2, Z, Y_2, mu, allSeqs, lenSeqs);
        double sub2_cost = get_sub2_cost (W_2, Z, Y_2, mu, allSeqs);
        cout << "=============================================================================" << endl;

        // 2c. update Z: Z = (W_1 + W_2) / 2
        // NOTE: parallelize this for to enable parallelism
        for (int n = 0; n < numSeq; n ++) 
            tensor4D_average (Z[n], W_1[n], W_2[n]);
        // 2d. update Y_1 and Y_2: Y_1 += mu * (W_1 - Z)
        // NOTE: parallelize this for to enable parallelism
        for (int n = 0; n < numSeq; n ++)
            tensor4D_lin_update (Y_1[n], W_1[n], Z[n], mu);
        for (int n = 0; n < numSeq; n ++)
            tensor4D_lin_update (Y_2[n], W_2[n], Z[n], mu);

        // 2e. print out tracking info
        double CoZ = 0.0;
        for (int n = 0; n < numSeq; n++) {
            CoZ += tensor4D_frob_prod(C[n], Z[n]);
        }
        double W1mW2 = 0.0;
        for (int n = 0; n < numSeq; n ++) {
            int T1 = W_1[n].size();
            for (int i = 0; i < T1; i ++) 
                for (int j = 0; j < T2; j ++) 
                    for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                        for (int m = 0; m < NUM_MOVEMENT; m ++) {
                            double value = (W_1[n][i][j][d][m] - W_2[n][i][j][d][m]);
                            W1mW2 += value * value;
                        }
        }
        cerr << "=============================================================================" << endl;
        cerr << "ADMM_iter = " << iter 
             << ", sub1_and_sub2_cost = " << sub1_cost + sub2_cost 
             << ", C o Z = " << CoZ  
             << ", || W_1 - W_2 ||_2 = " << W1mW2
             << endl;
        // cerr << "sub1_Obj = CoW_1+0.5*mu*||W_1-Z+1/mu*Y_1||^2 = " << sub1_cost << endl;
        // cerr << "sub2_Obj = ||W_2-Z+1/mu*Y_2||^2 = " << sub2_cost << endl;

        // 2f. stopping conditions
#ifdef ADMM_EARLY_STOP
        if ( iter > MIN_ADMM_ITER)
            if ( fabs(prev_CoZ - CoZ)  < EPS_ADMM_CoZ  and W1mW2 < 10e-3) {
                cerr << "CoZ Converges. ADMM early stop!" << endl;
                break;
            }
#endif
        prev_CoZ = CoZ;
        iter ++;
    }
    return Z;
    /*}}}*/
}

int main (int argn, char** argv) {
    // 1. usage
    if (argn < 2) {
        usage();
        exit(1);
    }

    // 2. input DNA sequence file
    SequenceSet allSeqs (0, Sequence());
    ifstream seq_file(argv[1]);
    string tmp_str;
    int numSeq = 0;
    while (getline(seq_file, tmp_str)) {
        Sequence tmp_seq (tmp_str.begin(), tmp_str.end());
        allSeqs.push_back(tmp_seq);
        ++ numSeq;
    }
    seq_file.close();
    cout << "#########################################################" << endl;
    cout << "ScoreMatch: " << C_M;
    cout << ", ScoreInsertion: " << C_I;
    cout << ", ScoreDeletion: " << C_D;
    cout << ", ScoreMismatch: " << C_MM << endl;
    for (int n = 0; n < numSeq; n ++) {
        char buffer [50];
        sprintf (buffer, "Seq%5d", n);
        cout << buffer << ": ";
        for (int j = 0; j < allSeqs[n].size(); j ++) 
            cout << allSeqs[n][j];
        cout << endl;
    }
    vector<int> lenSeqs (numSeq, 0);
    for (int n = 0; n < numSeq; n ++) 
        lenSeqs[n] = allSeqs[n].size();

    // 3. relaxed convex program: ADMM-based algorithm
    int T2 = get_init_model_length (lenSeqs); // model_seq_length
    vector<Tensor4D> W = CVX_ADMM_MSA (allSeqs, lenSeqs, T2);

    // 4. output the result
    /*
    cout << ">>>>>>>>>>>>>>>>>>>>>>>Summary<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    
       cout << "Length of Trace: " << trace.size();
       cout << ", Score: " << trace.back().score;
       cout << endl;
       int numInsertion = 0, numDeletion = 0, numMatch = 0, numMismatch = 0, numUndefined = 0;
       for (int i = 0; i < trace.size(); i ++) {
           switch (trace[i].action) {
               case MATCH: ++numMatch; break;
               case INSERTION: ++numInsertion; break;
               case DELETION: ++numDeletion; break;
               case MISMATCH: ++numMismatch; break;
               case UNDEFINED: ++numUndefined; break;
           }
       }
       cout << "numMatch: " << numMatch;
       cout << ", numInsertion: " << numInsertion;
       cout << ", numDeletion: " << numDeletion;
       cout << ", numMismatch: " << numMismatch;
       cout << ", numUndefined: " << numUndefined;
       cout << endl;
       */
    // a. tuple view
    cout << ">>>>>>>>>>>>>>>>>>>>>>>TupleView<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    for (int n = 0; n < numSeq; n ++) {
        int T1 = W[n].size();
        for (int i = 0; i < T1; i ++) 
            for (int j = 0; j < T2; j ++) 
                for (int d = 0; d < NUM_DNA_TYPE; d ++) 
                    for (int m = 0; m < NUM_MOVEMENT; m ++)
                        if (W[n][i][j][d][m] > 0.0)
                            cout << "(" << n 
                                 << ", " << i
                                 << ", " << j
                                 << ", " << d
                                 << ", " << m
                                 << "): " << W[n][i][j][d][m]
                                 << endl;
    }
    // b. sequence view
    cout << ">>>>>>>>>>>>>>>>>>>>>>>SequenceView<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    Tensor tensor (T2, Matrix (NUM_DNA_TYPE, vector<double>(NUM_DNA_TYPE, 0.0)));
    Matrix mat_insertion (T2, vector<double> (NUM_DNA_TYPE, 0.0));
    for (int n = 0; n < numSeq; n ++) {
        int T1 = W[n].size();
        for (int i = 0; i < T1; i ++) { 
            for (int j = 0; j < T2; j ++) {
                for (int d = 0; d < NUM_DNA_TYPE; d ++) {
                    for (int m = 0; m < NUM_MOVEMENT; m ++) {
                        if (m == DELETION_A or m == MATCH_A)
                            tensor[j][d][dna2T3idx('A')] += max(0.0, W[n][i][j][d][m]);
                        else if (m == DELETION_T or m == MATCH_T)
                            tensor[j][d][dna2T3idx('T')] += max(0.0, W[n][i][j][d][m]);
                        else if (m == DELETION_C or m == MATCH_C)
                            tensor[j][d][dna2T3idx('C')] += max(0.0, W[n][i][j][d][m]);
                        else if (m == DELETION_G or m == MATCH_G)
                            tensor[j][d][dna2T3idx('G')] += max(0.0, W[n][i][j][d][m]);
                        else if (m == INSERTION) 
                            mat_insertion[j][d] += max(0.0, W[n][i][j][d][m]);
                    }
                }
            }
        }
    }
    Trace trace (0, Cell(2)); // 1d: j, 2d: ATCG
    refined_viterbi_algo (trace, tensor, mat_insertion);
    for (int n = 0; n < numSeq; n ++) {
        char buffer [50];
        sprintf (buffer, "Seq%5d", n);
        cout << buffer << ": ";
        for (int j = 0; j < allSeqs[n].size(); j ++) 
            cout << allSeqs[n][j];
        cout << endl;
    }
    cout << "SeqRecov: ";
    for (int i = 0; i < trace.size(); i ++) 
        cout <<  T3idx2dna(trace[i].location[1]);
    cout << endl;
    cout << "#########################################################" << endl;
    return 0;
}
