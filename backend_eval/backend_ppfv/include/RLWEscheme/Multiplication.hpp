#include <iostream>
#include <cmath>
using namespace std;

typedef vector<int64_t> polynomial;
//extern intel::hexl::NTT* g_ntt; 


//relinearization key generation 
extern vector<vector<polynomial>> RelK_Gen(const polynomial & s, int N, int64_t ct_modulus, int64_t pt_modulus, int logq, 
  double var, int64_t b, int logb){
  polynomial s2 = NTTMul(s,s,N,pt_modulus);
  //print_polynomial(s2);
  return extRLWE(N,ct_modulus,pt_modulus,logq,var,s2,s,b,logb);
}


extern vector<polynomial> ct_multiplication(const vector<polynomial> & ct1, const vector<polynomial> & ct2, 
  const vector<vector<polynomial>> & RelK, int N, int64_t ct_modulus, int64_t pt_modulus, 
  int logq, int64_t b, int logb){

  //ct1 *= pt_modulus/ct_modulus
  vector<polynomial> r_ct1(2,polynomial(N,0));
  
  double alpha = (double)pt_modulus/(double)ct_modulus;

  //c2 = a1a2
  //polynomial c2 = NTTMul(ct1[0], ct2[0], N, ct_modulus);
  vector<double> d_c2 = mul_poly_double(ct1[0], ct2[0], N);

  //c0=(-a1s+m1+e1)(-a2s+m2+e2)
  //polynomial c0 = NTTMul(ct1[1],ct2[1],N,ct_modulus);
  vector<double> d_c0 = mul_poly_double(ct1[1], ct2[1], N);

  //c1 = a2(-a1s+m1+e1)+a1(-a2s+m2+e2)
  //polynomial c1 = NTTMul(ct1[1], ct2[0], N, ct_modulus);
  vector<double> d_c1 = mul_poly_double(ct1[1], ct2[0], N);
  //polynomial c1_2 = NTTMul(ct1[0], ct2[1], N, ct_modulus);
  vector<double> d_c1_2 = mul_poly_double(ct1[0], ct2[1], N);

  

  polynomial c2(N),c1(N),c0(N);

  double modulus = (double)ct_modulus;
  //c0,c1,c2 *= alpha
  for(int i = 0 ; i< N ; ++i){
    double temp = alpha*d_c0[i];
    if(temp < -1 * modulus){
      int tempscalar = (int)(-1*temp/modulus)+1;
      temp += modulus*(double)tempscalar;
    }
    else if(temp > modulus){
      int tempscalar = (int)(temp/modulus);
      temp -= modulus*(double)tempscalar;
    }
    int64_t temp2 = (int64_t)temp;
    if(temp - (double)temp2 >= 0.5 && temp > 0){
      temp2 ++;
    }
    else if((double)temp2 - temp >= 0.5 && temp < 0){
        temp2 --;
    }
    c0[i] = temp2;
  }
  modq_poly(c0,N,ct_modulus);

  for(int i = 0 ; i< N ; ++i){
    double temp = alpha*d_c1[i];
    if(temp < -1 * modulus){
      int tempscalar = (int)(-1*temp/modulus)+1;
      temp += modulus*(double)tempscalar;
    }
    else if(temp > modulus){
      int tempscalar = (int)(temp/modulus);
      temp -= modulus*(double)tempscalar;
    }
    int64_t temp2 = (int64_t)temp;
    if(temp - (double)temp2 >= 0.5 && temp > 0){
      temp2 ++;
    }
    else if((double)temp2 - temp >= 0.5 && temp < 0){
        temp2 --;
    }
    c1[i] = temp2;
    temp = alpha*d_c1_2[i];
    if(temp < -1 * modulus){
      int tempscalar = (int)(-1*temp/modulus)+1;
      temp += modulus*(double)tempscalar;
    }
    else if(temp > modulus){
      int tempscalar = (int)(temp/modulus);
      temp -= modulus*(double)tempscalar;
    }
    temp2 = (int64_t)temp;
    if(temp - (double)temp2 >= 0.5 && temp > 0){
      temp2 ++;
    }
    else if((double)temp2 - temp >= 0.5 && temp < 0){
        temp2 --;
    }
    c1[i] += temp2;
  }
  modq_poly(c1,N,ct_modulus);

  for(int i = 0 ; i< N ; ++i){
    double temp = alpha*d_c2[i];
    if(temp < -1 * modulus){
      int tempscalar = (int)(-1*temp/modulus)+1;
      temp += modulus*(double)tempscalar;
    }
    else if(temp > modulus){
      int tempscalar = (int)(temp/modulus);
      temp -= modulus*(double)tempscalar;
    }
    int64_t temp2 = (int64_t)temp;
    if(temp - (double)temp2 >= 0.5 && temp > 0){
      temp2 ++;
    }
    else if((double)temp2 - temp >= 0.5 && temp < 0){
        temp2 --;
    }
    c2[i] = temp2;
  }
  modq_poly(c2,N,ct_modulus);

/*
  //add_poly(c1,c1_2,N,2*ct_modulus*ct_modulus);

  polynomial s2 = mul_poly(s,s,N,ct_modulus);
  polynomial c2s2 = mul_poly(c2,s2,N,ct_modulus);

  polynomial c1s = mul_poly(c1,s,N,ct_modulus);
  polynomial testc0 = c0;
  add_poly(testc0,c1s,N,ct_modulus);
  add_poly(testc0,c2s2,N,ct_modulus);

  for(int i = 0 ; i< N ; ++i){
    double temp = alpha*(double)testc0[i];
    int64_t temp2 = (int64_t)temp;
    if(temp - (double)temp2 >= 0.5 && temp > 0){
      temp2 ++;
    }
    else if((double)temp2 - temp >= 0.5 && temp < 0){
        temp2 --;
    }
    testc0[i] = temp2;
  }
  cout <<"alpha*(c0+c1*s+c2*s2): "<<endl;
  print_polynomial(testc0);
*/

  //c2<>RelK = RLWE(a1a2s^2)
  vector<polynomial> c3 = bit_then_multiply(N, ct_modulus, logq, c2, RelK, b, logb);

  //cout <<"Decryption of c2 <> Relk: "<<endl;
  //print_polynomial(RLWE64_Dec(N,ct_modulus,ct_modulus,s,c3));

  
  
  //cout <<"s*s: "<<endl;
  //print_polynomial(s2);
  //cout <<"c2 * s2: "<<endl;
  //print_polynomial(c2s2);

  //c1+c3[0],c0+c3[1]
  add_poly(c3[0],c1,N,ct_modulus);
  add_poly(c3[1],c0,N,ct_modulus);

  return c3;

}
