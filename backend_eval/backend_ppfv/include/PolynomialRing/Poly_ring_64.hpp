//code for polynomial ring whose coefficients are at most 64 bits integers. 
#include <iostream>
#include <vector>
//#include <omp.h>
using namespace std;

//extern intel::hexl::NTT* g_ntt;

typedef vector<int64_t> polynomial;

// Copy one polynomial
polynomial copy(const polynomial & p) {
  // INPUT: polynomial p
  // OUTPUT: a copy of p
  polynomial ans(p);
  return ans;
}

// Print polynomial
 void print_polynomial(const polynomial & p) {
  // INPUT: polynomial p
  // OUTPUT: print on screen
  //cout <<"coefficients from x^0 - x^{n-1}: "<<endl;
  for(int i = 0 ; i < p.size(); ++i){
    cout <<p[i]<<" ";
  }
  cout <<endl;
}

// polynomial a mod q
//a[i] in [-q/2,q/2)
inline void modq_poly(polynomial &a, int n, int64_t q) {
  
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

inline void modq_poly_large(polynomial &a, int n, int64_t q) {
  
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
 extern void add_poly(polynomial &a, const polynomial & b, int n, int64_t q) {
  // INPUT: polynomials a and b, modulus q
  // OUTPUT: (a + b) mod q, stored in a
  for(int i = 0 ; i < n ; ++i){
    a[i] += b[i];
  }
  // mod q
  modq_poly(a,n, q);

}

 // Scaling
 extern void multi_scale_poly(int64_t t, polynomial &a, int n, int64_t q) {
  // INPUT: scaler t, polynomial a, modulus q
  // OUTPUT: t*a mod q, stored in a

  for (int i = 0; i < n; ++i) {
    a[i] *= t;
  }
  modq_poly_large(a,n, q);
}




// NTT multiplication of two polynomials
polynomial NTTMul( const polynomial& aa, const polynomial& bb,int n, int64_t q) {
  vector<uint64_t> a(n),b(n);
  vector<uint64_t> c(n);
  uint64_t modulus = (uint64_t)q;

  for(int k=0;k<n;++k){
    if(aa[k]>=0) a[k]=(uint64_t)aa[k];
    else if(aa[k]<0) a[k]=(uint64_t)(aa[k]+q);
  }
  for(int k=0;k<n;++k){
    if(bb[k]>=0) b[k]=(uint64_t)bb[k];
    else if(bb[k]<0) b[k]=(uint64_t)(bb[k]+q);
  
  }
  
  //intel::hexl::NTT ntt(n, modulus);
  intel::hexl::NTT g_ntt((uint64_t)n,modulus);
 //ntt(a) and ntt(b)
  g_ntt.ComputeForward(a.data(), a.data(), 1, 1);
 // cout <<"a.ntt"<<endl;
  g_ntt.ComputeForward(b.data(), b.data(), 1, 1);

 // cout <<"b.ntt"<<endl;

  //c=ntt(a)*ntt(b)
  intel::hexl::EltwiseMultMod(c.data(), a.data(), b.data(), c.size(), modulus, 1);

 // cout <<"pmul"<<endl;


  //intt(c)
  g_ntt.ComputeInverse(c.data(), c.data(), 1, 1);

  //cout <<"intt"<<endl;
  
  //vector<uint64_t> c -> polynomial cc
    
    polynomial cc(n);
    for(int k=0;k<n;++k){
      cc[k]=(int64_t)c[k];
    }
    modq_poly(cc,n,q);
    return cc;
}



extern polynomial mul_poly( const polynomial& aa, const polynomial& bb,int n, int64_t q) {
  polynomial c(n,0);
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
  modq_poly(c,n,q);
  return c;
}

extern vector<double> mul_poly_double( const polynomial& aa, const polynomial& bb,int n) {
  vector<double> c(n,0);
  for(int i = 0 ; i <n ; ++i){
    for(int j = 0 ; j < n ; ++j){
      if(i+j < n){
        c[i+j] += (double)aa[i]*(double)bb[j];
      }
      else{
        c[i+j-n] -= (double)aa[i]*(double)bb[j];
      }
    }
  }
  //modq_poly(c,n,q);
  return c;
}

// Extend polynomial w.r.t. base b: 
// Convert one polynomial to k/logb polynomials with coeff 0 or b-1
 vector<polynomial> bit_poly(int k, const polynomial & p, int n, int64_t q, int64_t b, int logb) {
  // INPUT: k is about log(q), polynomial p, modulus q, base b, log(b)
  // OUTPUT: a vector of k/log(b) polynomials

  // initialize
  int out_size = k/logb;
  //cout <<out_size<<endl;
  
  vector<polynomial> ans(out_size, polynomial(n,0));
 // cout <<"size of output in bit_poly: "<<ans.size()<<endl;

  // fulfill ans slot by slot
  for (register int i = 0; i <n; ++i) {
    //cout <<"i = "<<i<<endl;
    int64_t tempcoeff= p[i];
    while (tempcoeff < 0) {
      tempcoeff = tempcoeff + q;
    }
    
    int index = 0;
    while (tempcoeff != 0) {
      int64_t t = tempcoeff % b;
    //  cout <<tempcoeff<<", "<<t<<", "<<b<<endl;
      if (t> 0) {
        ans[index][i] = t;
        tempcoeff -= ans[index][i];
      }
      else {
        ans[index][i] = 0;
      }
      tempcoeff /= (b);
      index++;
    }
  }
  
  return ans;
}

