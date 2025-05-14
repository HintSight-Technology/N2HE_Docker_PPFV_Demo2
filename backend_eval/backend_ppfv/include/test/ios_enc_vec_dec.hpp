#include <iostream>
using namespace std;

extern void poly_eval_test(){
  cout <<"Task: Test for polynomial evaluation. "<<endl;
  int64_t ct_modulus = 576460752154525697;
  int64_t delta = 1;
  int logq = 60;
  int degree = 16;
  double var = 3.2;
  int64_t b = 2;
  int logb = 1;
  int64_t modulus = 12289; 
  int64_t primitive_root = 11;
  cout <<"Parameters: Ciphertext Modulus = "<<ct_modulus<<", degree = "<<degree<<", variance = "<<var<<endl;
  cout <<"base of decomposition = "<<b<<endl;
  cout <<"Plaintext Modulus = "<<modulus <<", primitive_root = "<<primitive_root<<endl;

  int64_t alpha = ct_modulus/modulus;

  polynomial s=RLWE64_KeyGen(degree);
  cout <<"secret key: "<<endl;
  print_polynomial(s);


  vector<vector<polynomial>> RelK=RelK_Gen(s,degree,ct_modulus,ct_modulus,logq,var,b,logb);

  cout <<"Relinearization key generated. "<<endl;
  cout <<endl;


  vector<int64_t> test_input(degree);
  for(int i = 0 ; i < degree ; ++i){
    test_input[i] = i;
  }
  cout <<"test input vector 1: "<<endl;
  for(int i = 0 ; i < degree ; ++i){
    cout <<test_input[i]<<" ";
  }
  cout <<endl;

  polynomial y = Encoding(modulus,primitive_root,degree,test_input);
  cout <<"Encoding output: "<<endl;
  print_polynomial(y);

  vector<polynomial> RLWE_y=RLWE64_Enc(degree,ct_modulus,modulus,var,y,s);

 polynomial f(8,2);
 //f[1] = 2;
// f[2] = 3;
 cout <<"polynomial to be evaluated: ";
 print_polynomial(f);

  vector<polynomial> RLWE_poly = eval_poly(RLWE_y,RelK, degree, ct_modulus,modulus, alpha, primitive_root,
    logq, b, logb,s,f);

  polynomial dec = RLWE64_Dec(degree,ct_modulus,modulus,s,RLWE_poly);

  cout <<"Decryption output after polynomial evaluation: "<<endl;
  print_polynomial(dec);
 // cout<<endl;

  //cout <<"Decryption result mod pt_modulus: "<<endl;
  //polynomial dec2 = u_modq_poly(modulus, degree, dec);
 // print_polynomial(dec2);
 // cout<<endl;

  vector<int64_t> de_y = Decoding(modulus,primitive_root,degree,dec);

  cout <<"Decoding output: "<<endl;
  for(int i = 0 ; i < degree ; ++i){
    cout <<de_y[i]<<" ";
  }
  cout <<endl;

}
