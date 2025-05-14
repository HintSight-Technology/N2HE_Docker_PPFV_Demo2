// ================================================

//Code is designed for Facial verification in IOS

//Date: May 2024

// ================================================


#include <iostream>
#include <vector>
#include<unistd.h>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <string>

using namespace std;



// Test set 

vector<int>  test_vector;
int scaler0 = 50;                         // scaler for input vector

int scaler1 = 50;                       // Scaler for Weights 

// ======================================================  Neural Network  ==========================================================================

//read test vectors and labels
void read_vector() {
  ifstream infile;
  infile.open("input_Img.txt");
  double sum_check = 0;
  for (int j = 0; j < 512; ++j) {
    double t;
    infile >> t;
    sum_check += t*t;
    t = t * scaler0;
    int tt=(int)t;
    if (t > 0 && t - tt > 0.5) {
      test_vector.push_back( tt + 1);
    }
    else if (t < 0 && tt - t>0.5) {
      test_vector.push_back( tt - 1);
    }
    else {
      test_vector.push_back( tt);
    }
  }
  infile.close();
  cout <<"l2 - norm ^2 = " << sum_check << endl;
}

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
  modq_poly_large(a,n, q);
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
        modq(c[i+j],q);
      }
      else{
        c[i+j-n] -= aa[i]*bb[j];
        modq(c[i+j-n],q);
      }
    }
  }
  modq_poly_large(c,n,q);
  return c;
}


vector<vector<int64_t> >RLWE64_Enc(const vector<int64_t>& m, 
  const vector<vector<int64_t> > & pk, int64_t p, int64_t q){
  int64_t alpha = q/p;
  int len = pk[0].size();

  vector<vector<int64_t> >ct(2, vector<int64_t>(len,0));

  //generate random polynomial u
  vector<int64_t> u(len,0);
  for (int i = 0; i < len; ++i) {
    int64_t tempa = rand() % 2;
    u[i] = tempa;
  }

  //compute ct[0] = pk[0]u+e1

  vector<int64_t> pk0u = mul_poly(pk[0], u, len, q);

  //Generate e and compute -as+e
  for (int i = 0; i < len; ++i) {
    int64_t indexe=rand()%16;
      int64_t e=0;
      if(indexe == 0){
        e=1;
      }
      else if(indexe == 1){
        e=-1;
      }
      pk0u[i] += e;
  }

  modq_poly_large(pk0u,len,q);
  ct[0] = pk0u;

  //compute ct[1] = pk[1]u+e1+alpham

  vector<int64_t> pk1u = mul_poly(pk[1], u, len, q);

  //Generate e and compute -as+e
  for (int i = 0; i < len; ++i) {
    int64_t indexe=rand()%16;
      int64_t e=0;
      if(indexe == 0){
        e=1;
      }
      else if(indexe == 1){
        e=-1;
      }
      pk1u[i] += e;
      pk1u[i] += (alpha*m[i]);
  }

  modq_poly_large(pk1u,len,q);
  ct[1] = pk1u;

  return ct;
}

vector<int64_t> RLWE64_Dec(const vector<vector<int64_t> >& ct, const vector<int64_t> & sk,
  int64_t p, int64_t q){
  int64_t alpha = q/p;
  int len = ct[0].size();

  vector<int64_t> pt = mul_poly(ct[0],sk,len,q);
  add_poly(pt, ct[1], len, q);

  for (int i = 0; i < len; ++i){
    pt[i] = pt[i]/alpha;
  }

  return pt;


}

int main(){

  srand(time(0));
  read_vector();

  //set n,q,t for RLWE scheme
  const int n1 = 2048;                        //polynomial degree 
  const int q1 = 576460752154525697;           //ciphertext modulus
  const int p1 = 12289;                       //plaintext modulus

  //network parameter
  const int l1 = 512;

  //read lwe secret key
  ifstream fin;

  /*
  vector<int64_t> x(n1,0);
  fin.open("rlwe_sk.txt");
  for(int i=0;i<n1;++i){
    fin>>x[i];
  }
  fin.close();
  cout <<"Read RLWE secret key. (only for test)"<<endl;
  */

  //read lwe public key
  fin.open("rlwe_pk.txt");
  vector< vector<int64_t> > pk(2, vector<int64_t>(n1,0));
  for(int i=0 ; i < 2 ; ++i){
    for (int j = 0; j < n1; ++j){
      fin>> pk[i][j];
    }
  }
  fin.close();
  cout <<"Read RLWE public key. "<<endl;

  
  vector<int64_t> input(n1,0);
  for (int i = 0; i < l1; ++i){
    input[i] = test_vector[i];
  }
  vector<vector<int64_t> > ct=RLWE64_Enc(input,pk,p1,q1);          
  

  ofstream fout;
  fout.open("ciphertext.txt");

  for (int i = 0; i < 2; ++i){
    for(int j=0;j<n1;++j){
      fout<<ct[i][j]<<" ";
    }
    fout << endl;
    //for test
    //cout <<"( "<<test_vector[i]<<", "<<Decrypt(q1,p1,n1,ct[i],x)<<") ";
  }
  //cout <<endl;

  
  fout.close();
  cout<<"Encrypt the input and store the ciphertext. "<<endl;

/*
  //for test
  vector<int64_t> pt = RLWE64_Dec(ct, x, p1, q1);
  for(int i = 0 ; i < n1 ; ++i){
    cout <<"( "<<input[i]<<", "<<pt[i]<<") ";
  }
*/
  

  return 0;

}


