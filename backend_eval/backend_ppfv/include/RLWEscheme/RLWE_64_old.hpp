#include <iostream>
using namespace std;

typedef vector<int64_t> polynomial;
extern intel::hexl::NTT* g_ntt;

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

  polynomial s(n+1,0);
  s[0] = (int64_t)n - 1;
  for (int i = 1; i <= n; ++i) {
    s[i] = x[i-1];
  }
  return s;
}

// RLWE encryption algorithm
// INPUT: dimension n, modulus q, variance 2^(-k), k >= 1, plaintext (polynomial) m, RLWE key s
// OUTPUT: RLWE ciphertext ct = (a, b) = (a, [q/t]m + e - a * s)
extern vector<polynomial> RLWE64_Enc(int n, int64_t q, int64_t t,int k, const polynomial & m, const polynomial & s) {

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

  polynomial a(n+1,0);
  a[0] = (int64_t)n - 1;
  for (int i = 1; i <= n; ++i) {
    // a[i]=array_a[i-1]-q/2;
   a[i]=0; //debug, wangxn, feb 19
  }
  a[0]=0;//debug
  ct.push_back(a);

  //cout <<"a generated."<<endl;

  //compute - a * s
  polynomial as = NTTMul(s,a,n,q);
  //print_polynomial(as);
  //cout <<"as computed."<<endl;
  multi_scale_poly(-1, as,q);

  //cout <<"as computed."<<endl;

  //generate  error e
  //generate error array e
  int *array_e = new int [len_out];
  random_bytes(seed,SEED_LEN);
  r = gen_ternary_var(array_e, len_out, k, seed);
  if (r == 1){
    cout <<"Error in generation a random error of LWE Encryption: NULL "<<endl;
  }
  if(r == 2){
    cout <<"Error in generation a random error of LWE Encryption: modulus" <<endl;
  }

  polynomial as_m(n+1);
  as_m[0] = n-1;
  for (int i = 1; i <= n; ++i) {
    as_m[i] = as[i] + alpha*m[i];
    //as[i] += (alpha*m[i]+array_e[i-1]);
  }
  modq_poly(as_m, q);
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
  add_poly(as, ct[1],q);

  print_polynomial(ct[1]);
  print_polynomial(as);
 // cout <<endl;

  //(t/q)*as
  for(int i = 1 ; i <= n ; ++i){
    as[i] *= t;
    double temp = (double)as[i]/(double)q;
    int64_t temp2 = (int64_t)temp;
    if(temp - (double)temp2 >= 0.5 && temp > 0){
      temp2 ++;
    }
    else if((double)temp2 - temp >= 0.5 && temp < 0){
        temp2 --;
    }
    as[i] = temp2;
  }


  return as;
}

//sample extended RLWE encryption
// INPUT: dimension n, modulus q, k = log(q), variance 2^(-var), var >= 1, plaintext (polynomial) m, RLWE key s, decomposition base b, log(b)
// OUTPUT: ~RLWE ciphertext.
vector<vector<polynomial>> extRLWE(int n, int64_t q, int64_t t, int k, int var, const polynomial & m, 
  const polynomial & s, int64_t b, int logb) {

  vector<vector<polynomial>> extRLWEct;

  //~RLWE(m,2^i)
  int64_t temp2k = b;
  polynomial mi = copy(m);
  for (int i = 0; i < (k/logb); ++i) {
   // cout <<i <<endl;
    if (i > 0) {
      multi_scale_poly(temp2k, mi, q);
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
vector<polynomial> bit_then_multiply(int n, int64_t q, int k, const polynomial & r, const vector<vector<polynomial>> & ct, int b, int logb) {
  //b-decomption of r
  vector<polynomial> rbit = bit_poly(k, r,q,b,logb);

  polynomial ip1 = NTTMul(ct[0][0], rbit[0],n,q);

  polynomial ip2 = NTTMul(ct[0][1], rbit[0],n,q);

  //compute ri*cti
  for (int i = 1; i < (k/logb); ++i) {
    polynomial temp1 = NTTMul(ct[i][0], rbit[i],n,q);
    add_poly(ip1, temp1,q);

    polynomial temp2 = NTTMul(ct[i][1], rbit[i],n,q);
    add_poly(ip2, temp2,q);
  }
  //return (ip1,ip2)
  vector<polynomial> ans;
  ans.push_back(ip1);
  ans.push_back(ip2);

  return ans;
}



// Extract the const term of an RLWE encrypted polynomial, return LWE ciphertext 
// INPUT: modulus q, scaler delta, dimension n, RLWE ciphertext RLWE_ct of polynomial k
// OUTPUT: LWE ciphertext of the const term of k
extern vector<int64_t> Extract0(int64_t q, int64_t delta, int n, const vector<polynomial> & RLWE_ct) {

  if(RLWE_ct[0][0] == 0){
    
    vector<int64_t> b(n,0);
    b.push_back(RLWE_ct[1][1]);
    return b;
  }
  else{
    vector<int64_t> a(RLWE_ct[0].begin()+2,RLWE_ct[0].end());
    int size = a.size();

    for (int i = 0; i <size; ++i) {
      a[i] *= -1;
    }
    a.push_back(RLWE_ct[0][1]);

    a.push_back(RLWE_ct[1][1]);

    return a;
  }
}

extern vector<uint64_t> Extract0_u(int64_t q, int64_t delta, int n, const vector<vector<uint64_t>> & RLWE_ct) {

  if(RLWE_ct[0][0] == 0){
    
    vector<uint64_t> b(n,0);
    b.push_back(RLWE_ct[1][1]);
    return b;
  }
  else{
    vector<uint64_t> a(RLWE_ct[0].begin()+2,RLWE_ct[0].end());
    int size = a.size();
    //cout <<size<<endl;
    for (int i = 0; i <size; ++i) {
      //a[i] *= -1;
      a[i] = (uint64_t)q-a[i];
    }
    a.push_back(RLWE_ct[0][1]);

    a.push_back(RLWE_ct[1][1]);

    return a;
  }
}



