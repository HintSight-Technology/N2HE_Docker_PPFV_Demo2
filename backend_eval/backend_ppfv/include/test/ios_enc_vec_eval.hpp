#include <iostream>
using namespace std;


extern void Multiplication_test(){
  cout <<"Task: Test for ciphertexts multiplication. "<<endl;
  int64_t ct_modulus = 576460752154525697;
  int64_t delta = 1;
  int logq = 60;
  int degree = 2048;
  double var = 3.2;
  int64_t b = 2;
  int logb = 1;
  int64_t modulus = 12289; 
  int64_t primitive_root = 11;
  cout <<"Parameters: Ciphertext Modulus = "<<ct_modulus<<", degree = "<<degree<<", variance = "<<var<<endl;
  cout <<"base of decomposition = "<<b<<endl;
  cout <<"Plaintext Modulus = "<<modulus <<", primitive_root = "<<primitive_root<<endl;

  polynomial s=RLWE64_KeyGen(degree);
  int *lwe_s = new int [degree];
  for(int i = 0 ; i < degree ; ++i){
    lwe_s[i] = (int)s[i];
  }
 // cout <<"secret key: "<<endl;
 // print_polynomial(s);


  vector<vector<polynomial>> RelK=RelK_Gen(s,degree,ct_modulus,ct_modulus,logq,var,b,logb);

  cout <<"Relinearization key generated. "<<endl;
  cout <<endl;

/*
  for(int i = 0 ; i < RelK.size() ; ++i){
    polynomial out = RLWE64_Dec(degree,ct_modulus,ct_modulus,s,RelK[i]);
    cout <<"Decryption of "<<i<<"-th RLWE ct in RelK: "<<endl;
    print_polynomial(out);
  }
*/

  vector<int64_t> test_input(degree);
  for(int i = 0 ; i < degree ; ++i){
    test_input[i] = 1;
  }
  cout <<"test input vector 1: "<<endl;
  for(int i = 0 ; i < degree ; ++i){
    cout <<test_input[i]<<" ";
  }
  cout <<endl;

  //polynomial y = Encoding(modulus,primitive_root,degree,test_input);
  //cout <<"Encoding output: "<<endl;
  //print_polynomial(y);

  vector<polynomial> RLWE_y=RLWE64_Enc(degree,ct_modulus,modulus,var,test_input,s);

  vector<int64_t> test_input_2(degree);
  for(int i = 0 ; i < degree ; ++i){
    test_input_2[i] = 2;
  }
  cout <<"test input vector 2: "<<endl;
  for(int i = 0 ; i < degree ; ++i){
    cout <<test_input_2[i]<<" ";
  }
  cout <<endl;

 // polynomial y2 = Encoding(modulus,primitive_root,degree,test_input_2);
 // cout <<"Encoding output: "<<endl;
 // print_polynomial(y2);

  vector<polynomial> RLWE_y2=RLWE64_Enc(degree,ct_modulus,modulus,var,test_input_2,s);

  vector<polynomial> RLWE_mul = ct_multiplication(RLWE_y,RLWE_y2, RelK, degree, ct_modulus,modulus, 
    logq, b, logb,s);

  RLWE_y[0] = RLWE_mul[0];
  RLWE_y[1] = RLWE_mul[1];



  polynomial dec = RLWE64_Dec(degree,ct_modulus,modulus,s,RLWE_y);

  cout <<"Decryption output: "<<endl;
  for(int i = 0 ; i < degree ; ++i){
    cout <<dec[i]<<" ";
  }
  cout <<endl;

  vector<int64_t> LWE_ip = extract_0(RLWE_y,degree);
  cout <<LWE64_Dec(modulus,ct_modulus,degree,LWE_ip,lwe_s)<<endl;
}
  


