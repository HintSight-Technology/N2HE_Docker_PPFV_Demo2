#include <iostream>
#include <cmath>
using namespace std;

typedef vector<int64_t> polynomial;
//extern intel::hexl::NTT* g_ntt; 

polynomial mapk(polynomial a, int N, int k, int primitive_root){
  polynomial ans(N);
  //ans[0] = N-1;
  //exp = 3^k
  int exp = 1;
  for(int i = 0; i < k ; ++i){
    //exp *= primitive_root;
    exp *= 5;
    exp = u_modq_32(exp, 2*N);
  }

  for(int i = 0; i < N ; ++i){
    int new_exp = i*exp;
    new_exp = u_modq_32(new_exp,2*N);
    if(new_exp < N){
      ans[new_exp]=a[i];
    }
    else{
      ans[new_exp-N] = -1*a[i];
    }
  }

  return ans;
}

polynomial inv_mapk(polynomial a, int N){
  polynomial ans(N);

  int exp = 2*N-1;

  for(int i = 0; i < N ; ++i){
    int new_exp = i*exp;
    new_exp = u_modq_32(new_exp,2*N);
    if(new_exp < N){
      ans[new_exp]=a[i];
    }
    else{
      ans[new_exp-N] = -1*a[i];
    }
  }

  return ans;
}


vector<int64_t> Rot_Vec(vector<int64_t> input, int k){
  int len = input.size();
  vector<int64_t> out(len);
  for(int j = 0 ; j <2 ; ++j){
    for(int i = 0 ; i < len/2 ; ++i){
      int index = i+k;
      if(index >= len/2){
        index -= len/2;
      }
      out[index+j*len/2] = input[i+j*len/2];
    }
  }
  
  return out;
}

//RotK = ~RLWE_s^{N,ct_modulus}(\phik(s))
vector<vector<polynomial>> RotK_Gen(const polynomial & s, int N, int k, int64_t ct_modulus, int64_t pt_modulus, int logq,
 double var, int64_t b, int logb, int primitive_root){
  polynomial phi_k_s;
  if(k == -1){
    phi_k_s=inv_mapk(s,N);
  } 
  else{
    phi_k_s= mapk(s, N, k, primitive_root);
  }
  //return extRLWE(N,ct_modulus,pt_modulus,logq,var,phi_k_s,s, b, logb);
  vector<vector<polynomial>> out = extRLWE(N,ct_modulus,pt_modulus,logq,var,phi_k_s,s, b, logb);

  return out;
}


//Rotation
vector<polynomial> Rotation(const vector<polynomial> & RLWE_ct, const vector<vector<polynomial>> & RotK, 
  int N, int k, int64_t ct_modulus, int logq, int64_t b, int logb, int primitive_root){

  //RLWE_ct = (a, -as+m+e)
  //phi_k(a) <> RotK

  polynomial phi_k_a, phi_k_as;
  if (k == -1) {
    phi_k_a = inv_mapk(RLWE_ct[0],N);
    phi_k_as = inv_mapk(RLWE_ct[1],N);
  }
  else{
    phi_k_a = mapk(RLWE_ct[0],N,k,primitive_root);
    phi_k_as = mapk(RLWE_ct[1],N,k,primitive_root);
  }
  
  //cout <<"phi_k(ct[0]): "<<endl;
  //print_polynomial(phi_k_a);

 // out[0] = phi_k_a;
 // out[1] = phi_k_as;
  vector<polynomial> out = bit_then_multiply(N, ct_modulus, logq, phi_k_a, RotK, b, logb);
  //vector<polynomial> out(2);
  //out[0] = NTTMul(phi_k_a,RotK[0][0],N,ct_modulus);
  //out[1] = NTTMul(phi_k_a,RotK[0][1],N,ct_modulus);

  //phi_k(a) <> RotK + phi_k(-as+m+e)
  //phi_k(-as+m+e)
  

  //multi_scale_poly(inv_alpha,out[0],N,ct_modulus);
  //multi_scale_poly(inv_alpha,out[1],N,ct_modulus);

  add_poly(out[1],phi_k_as,N,ct_modulus);

  //cout <<"Ciphertext after rotation: "<<endl;
  //print_polynomial(out[0]);
 // print_polynomial(out[1]);



  return out;
}


