#include <iostream>
//#include <omp.h>
using namespace std;

typedef vector<int64_t> polynomial;
//extern intel::hexl::NTT* g_ntt;


// RLWE key generation algorithm
// INPUT: dimension n
// OUTPUT: a degree-(n-1) polynomial s, with coeffients from Uniform Random Distribution on {-1,0,1}
extern polynomial RLWE64_KeyGen(int n) {
  unsigned int len_out = n;
  unsigned char seed[SEED_LEN];
  int *x = new int [n];
  random_bytes(seed,SEED_LEN);
  int r = gen_ternary(x,len_out,seed);
  if (r == 1){
    cout <<"Error in RLWE Key Generation: NULL "<<endl;
  }
  if(r == 2){
    cout <<"Error in RLWE Key Generation: Length" <<endl;
  }

  polynomial s(n,0);
  //s[0] = (int64_t)n - 1;
  for (int i = 0; i < n; ++i) {
    s[i] = x[i];
  }
  return s;
}

// RLWE encryption algorithm
// INPUT: dimension n, modulus q, variance 2^(-k), k >= 1, plaintext (polynomial) m, RLWE key s
// OUTPUT: RLWE ciphertext ct = (a, b) = (a, [q/t]m + e - a * s)
extern vector<polynomial> RLWE64_Enc(int n, int64_t q, int64_t t,double k, const polynomial & m, const polynomial & s) {

  vector<polynomial> ct;

  int64_t alpha = q/t;
  //cout <<"[Q/t] = "<<alpha<<endl;

  //generate random a
  double logq=log(q)/log(2.0);
  int int_logq = (int) logq;
  if(logq > (double)int_logq){
    int_logq++;
  }

  unsigned int len_out = n;
  unsigned char seed[SEED_LEN];
  int64_t *array_a = new int64_t [len_out];
  random_bytes(seed,SEED_LEN);
  int r = gen_uniform_int64(array_a, len_out, q, int_logq, seed);

  if (r == 1){
    cout <<"Error in generation random array a of RLWE Encryption: NULL "<<endl;
  }
  if(r == 2){
    cout <<"Error in generation random array a of RLWE encryption: modulus" <<endl;
  }

  polynomial a(n,0);
  //a[0] = (int64_t)n - 1;
  for (int i = 0; i < n; ++i) {
     a[i]=array_a[i]-q/2;
  }
  
  ct.push_back(a);

  //cout <<"a generated."<<endl;

  //compute - a * s
  polynomial as = NTTMul(s,a,n,q);
  //print_polynomial(as);
  //cout <<"as computed."<<endl;
  multi_scale_poly(-1, as,n,q);

  //cout <<"as computed."<<endl;

  //generate  error e
  //generate error array e
  int *array_e = new int [len_out];
  random_bytes(seed,SEED_LEN);
  r = gen_discrete_normal(array_e, len_out, k, seed);
  if (r == 1){
    cout <<"Error in generation a random error of LWE Encryption: NULL "<<endl;
  }
  if(r == 2){
    cout <<"Error in generation a random error of LWE Encryption: modulus" <<endl;
  }

  polynomial as_m(n);
  for (int i = 0; i < n; ++i) {
    //as_m[i] = as[i] + alpha*m[i];
    if(array_e[i]>10 || array_e[i]<-10){
      cout <<"error > 10"<<endl;
    }
    as_m[i] = as[i] + alpha*m[i]+array_e[i];
  }
  modq_poly(as_m, n,q);
 // cout <<"as+m+e computed."<<endl;
  ct.push_back(as_m);

  return ct;
}


// RLWE Decryption algorithm  
// INPUT: dimension n, modulus q, RLWE key s, RLWE ciphertext ct (a, b)
// OUTPUT: polynomial (t/q)(b + a * s)
extern polynomial RLWE64_Dec(int n, int64_t q, int64_t t, const polynomial & s, const vector<polynomial> & ct) {
  //compute as
  polynomial as = NTTMul(s, ct[0],n,q);
  //compute b+as
  add_poly(as, ct[1],n,q);

  //int64_t alpha = q/t;
  double tq = (double)t/(double)q;

  //(t/q)*as
  for(int i = 0 ; i < n ; ++i){
   // as[i] *= t;
    //double tq = (double)t/(double)q;
    double temp = tq*(double)as[i] ;
    int64_t temp2 = (int64_t)temp;
    if(temp - (double)temp2 >= 0.5 && temp > 0){
      temp2 ++;
    }
    else if((double)temp2 - temp >= 0.5 && temp < 0){
        temp2 --;
    }
    as[i] = temp2;
  //  if(as[i] < 0){
   //   as[i] += t;
   // }
  }

  return as;
}

//sample extended RLWE encryption
// INPUT: dimension n, modulus q, k = log(q), variance 2^(-var), var >= 1, plaintext (polynomial) m, RLWE key s, decomposition base b, log(b)
// OUTPUT: ~RLWE ciphertext.
vector<vector<polynomial>> extRLWE(int n, int64_t q, int64_t t, int k, double var, const polynomial & m, 
  const polynomial & s, int64_t b, int logb) {

  vector<vector<polynomial>> extRLWEct;

  //~RLWE(m,2^i)
  int64_t temp2k = b;
  polynomial mi = copy(m);
  for (int i = 0; i < (k/logb); ++i) {
   // cout <<i <<endl;
    if (i > 0) {
      multi_scale_poly(temp2k, mi,n, q);
    }
    vector<polynomial> c1i = RLWE64_Enc(n, q, t, var, mi, s);
   // cout <<"enc "<<c1i.size()<<endl;
    extRLWEct.push_back(c1i);
  }

  return extRLWEct;
}



//Operator <> (known polynomial r multiply ~RLWE ciphertext)
// INPUT: dimension n, modulus q, k = log(q), polynomial r, ~RLWE ciphertext ct, decomposition base b, log(b)
// OUTPUT: r <> ct := (b-decomption of r) slot-wise-multiply (ct)
vector<polynomial> bit_then_multiply(int n, int64_t q, int k, const polynomial & r, const vector<vector<polynomial>> & ct, int64_t b, int logb) {
  //b-decomption of r
  vector<polynomial> rbit = bit_poly(k, r,n,q,b,logb);
  //cout <<"size after bit_poly: "<<rbit.size();

  polynomial ip1 = NTTMul(ct[0][0], rbit[0],n,q);

  polynomial ip2 = NTTMul(ct[0][1], rbit[0],n,q);

  //compute ri*cti
  for (int i = 1; i < (k/logb); ++i) {
    polynomial temp1 = NTTMul(ct[i][0], rbit[i],n,q);
    add_poly(ip1, temp1,n,q);

    polynomial temp2 = NTTMul(ct[i][1], rbit[i],n,q);
    add_poly(ip2, temp2,n,q);
  }
  //return (ip1,ip2)
  vector<polynomial> ans;
  ans.push_back(ip1);
  ans.push_back(ip2);

  return ans;
}

/*
vector<polynomial> RLWE64_ModSwitch(const vector<polynomial> & ct, int degree, int64_t old_modulus, int64_t new_modulus){
  // int64_t alpha = old_modulus/new_modulus;
  double delta = 1.0/(double)old_modulus;

  vector<polynomial> out(2,polynomial(degree,0));
  for(int i = 0 ; i < 2 ; ++i){
    for(int j = 0 ; j < degree ; ++j){
      double temp = delta*(double)ct[i][j];
      temp *= (double) new_modulus;
      int64_t temp2 = (int64_t)(temp);
      if(temp - (double)temp2 >= 0.5 && temp > 0){
      temp2 ++;
      }
      else if((double)temp2 - temp >= 0.5 && temp < 0){
        temp2 --;
      }
      out[i][j] = temp2;
    }
  }
  return out;
}

vector<vector<int64_t>> Extract(const vector<polynomial> & ct, int degree){
  vector<vector<int64_t>> out(degree, vector<int64_t>(degree+1,0));
  for(int i = 0 ; i < degree ; ++i){
    for(int j = 0 ; j < degree ; ++j){
      if(j <= i){
        out[i][j] = ct[0][i-j];
      }
      else{
        out[i][j] = -1 * ct[0][degree-j+i];
      }
    }
    out[i][degree] = ct[1][i];
  }
  return out;
}
*/

vector<int64_t> extract_0(const vector<polynomial> & ct, int degree){
  vector<int64_t> out(degree+1,0);
  out[0] = ct[0][0];
  for (int i = 1; i < degree; ++i){
    out[i] = -1 * ct[0][degree-i];
  }
  out[degree] = ct[1][0];
  return out;
}







