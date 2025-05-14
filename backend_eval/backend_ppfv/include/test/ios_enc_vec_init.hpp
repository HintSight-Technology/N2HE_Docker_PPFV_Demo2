// ================================================

//Code is designed for Facial verification in IOS

//Date: May 2024

// ================================================


#include <iostream>
#include <vector>
#include <ctime>
#include<sys/time.h>
#include<time.h>
#include<unistd.h>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <immintrin.h>
#include <string>

using namespace std;

// ======================================================  LWE Encryption Scheme  ==========================================================================

//mod q
inline void modq(int64_t number, int64_t q) {
  if(number < 0){
      int64_t temp = (-1*number)/q + 1;
      number += temp*q;
    }
    if(number >= q){
      int64_t temp = number/q;
      number -= temp*q;
    }
    if(number >= q/2){
      number -= q;
    }
  return ;
}

inline void modq_poly(vector<int64_t> &a, int n, int64_t q) {
  
  // INPUT: polynomial a, modulus q
  // OUTPUT: (modified) a mod q, stored in a

  for (int i = 0; i < n; ++i) {

    while (a[i]<0) {
      a[i] += q;
    }

    while (a[i]>=q) {
      a[i] -= q;
    }

    if(a[i] > (q-1)/2){
      a[i] -=q;
    }

  }
  
}

inline void modq_poly_large(vector<int64_t> &a, int n, int64_t q) {
  
  // INPUT: polynomial a, modulus q
  // OUTPUT: (modified) a mod q, stored in a

  for (int i = 0; i < n; ++i) {

    if(a[i]<0){
      int64_t temp = -1 * a[i];
      a[i] += q*(temp/q+1);
    }

    if(a[i] >= q){
      a[i] -= q*(a[i]/q);
    }

    if(a[i] > (q-1)/2){
      a[i] -=q;
    }

  }
  
}

// Addition between polynomials
  void add_poly(vector<int64_t> &a, const vector<int64_t> & b, int n, int64_t q) {
  // INPUT: polynomials a and b, modulus q
  // OUTPUT: (a + b) mod q, stored in a
  for(int i = 0 ; i < n ; ++i){
    a[i] += b[i];
  }
  // mod q
  modq_poly(a,n, q);
}

 // Scaling
  void multi_scale_poly(int64_t t, vector<int64_t> &a, int n, int64_t q) {
  // INPUT: scaler t, polynomial a, modulus q
  // OUTPUT: t*a mod q, stored in a

  for (int i = 0; i < n; ++i) {
    a[i] *= t;
  }
  modq_poly_large(a,n, q);
}

 vector<int64_t> mul_poly( const vector<int64_t>& aa, 
  const vector<int64_t>& bb,int n, int64_t q) {
  vector<int64_t> c(n,0);
  for(int i = 0 ; i <n ; ++i){
    for(int j = 0 ; j < n ; ++j){
      if(i+j < n){
        c[i+j] += aa[i]*bb[j];
      }
      else{
        c[i+j-n] -= aa[i]*bb[j];
      }
    }
  }
  modq_poly_large(c,n,q);
  return c;
}

vector<int64_t> RLWE64_SKGen(int n) {
  vector<int64_t> x;
  for (int i = 0; i < n; ++i) {
    int64_t tempx = rand() % 3;
    x.push_back(tempx-1);
  }
  return x;
}

vector< vector<int64_t> > RLWE64_PKGen(const vector<int64_t>& sk, int64_t q){
  int len = sk.size();
  vector<vector<int64_t> >ct(2, vector<int64_t>(len,0));

  //Generate random polynomial a
  for (int i = 0; i < len; ++i) {
    int64_t tempa = rand() % q-q/2;
    ct[0][i] = tempa;
  }

  //compute -(as)
  vector<int64_t> as = mul_poly(sk, ct[0], len, q);
  multi_scale_poly(-1, as,len,q);

  //Generate e and compute -as+e
  int non_zero_count = 0;
  for (int i = 0; i < len; ++i) {
    int64_t indexe=rand()%16;
      int64_t e=0;
      if(indexe == 0){
        e=1;
        non_zero_count++;
      }
      else if(indexe == 1){
        e=-1;
        non_zero_count++;
      }
      as[i] += e;
  }
  //cout <<"non zero ei = "<<non_zero_count<<endl;

  modq_poly_large(as,len,q);
  ct[1] = as;

  return ct;

}

int main(){
  srand(time(0));


  //set n,q,t for RLWE scheme
  const int n1 = 2048;                        //polynomial degree 
  const int q1 = 576460752154525697;           //ciphertext modulus
  const int p1 = 12289;                       //plaintext modulus

  vector < int64_t > x = RLWE64_SKGen(n1);
  ofstream fout;
  fout.open("rlwe_sk.txt");
  for(int i=0;i<n1;++i){
    fout<<x[i]<<" ";
  }
  fout.close();
  cout<<"Generate and store RLWE secret key."<<endl;

  vector<vector<int64_t> > pk = RLWE64_PKGen(x, q1);
  fout.open("rlwe_pk.txt");
  for(int i=0 ; i < 2 ; ++i){
    for (int j = 0; j < n1; ++j){
      fout << pk[i][j]<<" ";
    }
    fout <<endl;
  }
  fout.close();
  cout<<"Generate and store RLWE public key."<<endl;


  return 0;

}


