#include <iostream>
#include <cmath>
using namespace std;

typedef vector<int64_t> polynomial;
//extern intel::hexl::NTT* g_ntt; 


//compute RLWE(ecd(f(x))) from RLWE(ecd(x)) and f(x)
vector<polynomial> eval_poly(const vector<polynomial> & RLWE_ct, const vector<vector<polynomial>> & RelK, int N, 
  int64_t ct_modulus, int64_t pt_modulus, int64_t alpha, int64_t primitive_root, int logq, int64_t b, int logb, const polynomial& s, const polynomial & f){
    int f_degree = f.size()-1;
    vector<polynomial> ct_out(2);

    ct_out[0] = RLWE_ct[0];
    ct_out[1] = RLWE_ct[1];

    //print_polynomial(RLWE64_Dec(N,ct_modulus,pt_modulus,s,ct_out));

    multi_scale_poly(f[f_degree],ct_out[0],N,ct_modulus);
    multi_scale_poly(f[f_degree],ct_out[1],N,ct_modulus);

    polynomial a(N,0);
    a[0] = f[f_degree-1]*alpha;

    add_poly(ct_out[1],a,N,ct_modulus);

    polynomial dec0 = RLWE64_Dec(N,ct_modulus,pt_modulus,s,ct_out);
    vector<int64_t> de_y0 = Decoding(pt_modulus,primitive_root,N,dec0);
    cout <<"Decoding result for i = "<<f_degree-1<<endl;
      for(int j = 0 ; j < N ; ++j){
        cout <<de_y0[j]<<" ";
    }
    cout <<endl;

    for(int i = f_degree-2 ; i >= 0 ; --i){
      //cout <<i<<endl;
      //ct_out = ct_out*x+f[i]
      vector<polynomial> ct_mul = ct_multiplication(ct_out, RLWE_ct, RelK, N, ct_modulus, pt_modulus, logq, b, logb,s);

      polynomial dec = RLWE64_Dec(N,ct_modulus,pt_modulus,s,ct_mul);
      vector<int64_t> de_y = Decoding(pt_modulus,primitive_root,N,dec);
      cout <<"Decoding result after multiplying x for i = "<<i<<endl;
      for(int j = 0 ; j < N ; ++j){
        cout <<de_y[j]<<" ";
      }
      cout <<endl;

      polynomial a(N,0);
      a[0] = f[i]*alpha;

      add_poly(ct_mul[1],a,N,ct_modulus);

      ct_out[0] = ct_mul[0];
      ct_out[1] = ct_mul[1];

      dec = RLWE64_Dec(N,ct_modulus,pt_modulus,s,ct_out);
      de_y = Decoding(pt_modulus,primitive_root,N,dec);
      cout <<"Decoding result for i = "<<i<<endl;
      for(int j = 0 ; j < N ; ++j){
        cout <<de_y[j]<<" ";
      }
      cout <<endl;
    }

    return ct_out;
}