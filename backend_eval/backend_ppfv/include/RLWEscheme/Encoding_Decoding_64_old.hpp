#include <iostream>
#include <cmath>
using namespace std;

typedef vector<int64_t> polynomial;
extern intel::hexl::NTT* g_ntt; 

inline int64_t u_modq_64(int64_t number, int64_t q) {
//  cout <<"modulus = "<<q<<endl;
  /*
  while(number < 0){
    number += q;
  }
  */
  if(number < 0){
    int64_t tempnum = -1*number;
    int64_t temp = tempnum/q+1;
    number += (q*temp);
  }
  if(number >= q){
    int64_t temp = number/q;
    number -= (q*temp);
  }
  //number in [0,q)
  return number;
}

inline int u_modq_32(int number, int q) {
  while(number < 0){
    number += q;
  }
  while (number >= q){
    number -= q;
  }
  //number in [0,q)
 return number;
}

int64_t power_mod_q_slow(int64_t modulus, int64_t number, int power){
  if(power == 0){
    return 1;
  }
  int64_t ans = 1;
  for(int i = 1 ; i <= power ; ++i){
    ans *= number;
    ans = u_modq_64(ans,modulus);
   // cout << number<<endl;
  }
  return ans;
}

//number^power mod modulus
int64_t power_mod_q(int64_t modulus, int64_t number, int power){
  int64_t ans = 1%modulus;
  while(power){
    if(power&1){
      ans = (ans*number)%modulus;
    }
    number = (number*number)%modulus;
    power >>= 1;
  }
  return ans;
}

int64_t inverse(int64_t modulus, int64_t number){
  
  for(int i = 1 ; i < modulus ; ++i){
    int64_t temp = number * i;
   // temp = u_modq_64(temp, modulus);
    temp %= modulus;
    //cout <<temp<<" ";
    if(temp == 1){
      return (int64_t)i;
    }
  }
}

extern polynomial u_modq_poly(int64_t modulus, int N, const polynomial & a){
  polynomial out(a);
  for(int i = 1 ; i <= N ; ++i){
    out[i] = u_modq_64(a[i],modulus);
  }
  return out; 
}



extern polynomial Encoding(int64_t modulus, int64_t primitive_root, int N, const vector<int64_t> & m){
  
  //construct output polynomial 
  polynomial y(N+1);
  y[0]=N-1;

  //compute 2N-th primitive root
  int64_t root = power_mod_q(modulus, primitive_root, (modulus-1)/(2*N));
  //cout <<"2N-th primitive_root is: "<<root<<endl;

  //compute inverse of primitive root Inv_root = primitive_root^{2N-1}
  int64_t Inv_root = power_mod_q(modulus, root, 2*N-1);

  //cout <<"Inverse of primitive_root is: "<<Inv_root<<endl;

  int64_t Inv_N = inverse(modulus, (int64_t)N);

  //cout <<"Inverse of N is: "<<Inv_N<<endl;

  //cout <<"Encoding matrix: "<<endl;

  for(int i = 0 ; i < N ; ++i){
  //  cout <<i <<endl;

    int64_t temp_y = 0;

    int exp = 1;

    for(int j = 0 ; j < N ; ++j){
     
      int64_t temp_root = 0;
      if (j < N/2) {
        //temp_root = power_mod_q(modulus, Inv_root, u_modq_32(exp*i,2*N));
        temp_root = power_mod_q(modulus, Inv_root, (exp*i)%(2*N));
      }
      else {
        //temp_root = power_mod_q(modulus, primitive_root, u_modq_32(exp*i,2*N));
        temp_root = power_mod_q(modulus, root, (exp*i)%(2*N));
      }
      
     // cout <<temp_root<<" ";
      int64_t temp_m = u_modq_64(m[j],modulus);

      temp_y += (Inv_N*temp_root*temp_m);  // todo 15

      //temp_y = u_modq_64(temp_y,modulus);
      temp_y %= modulus;

      exp *= primitive_root;
      //exp = u_modq_32(exp,2*N);
      exp %= (2*N);
    }
  //  cout <<endl;

    y[i+1] = temp_y;
    if(y[i+1] > modulus/2){
      y[i+1] -= modulus;
    }
  }

  // modq_poly(y,modulus);

  return y;

}

extern vector<vector<int64_t>> ecd_matrix(int64_t modulus, int64_t primitive_root, int N){
  //compute 2N-th primitive root
  int64_t root = power_mod_q(modulus, primitive_root, (modulus-1)/(2*N));
  //cout <<"2N-th primitive_root is: "<<root<<endl;

  //compute inverse of primitive root Inv_root = primitive_root^{2N-1}
  int64_t Inv_root = power_mod_q(modulus, root, 2*N-1);

  //cout <<"Inverse of primitive_root is: "<<Inv_root<<endl;

  int64_t Inv_N = inverse(modulus, (int64_t)N);

  vector<vector<int64_t>> matrix(N,vector<int64_t>(N));

  for(int i = 0 ; i < N ; ++i){

    int exp = 1;

    for(int j = 0 ; j < N ; ++j){
     
      int64_t temp_root = 0;
      if (j < N/2) {
        temp_root = power_mod_q(modulus, Inv_root, (exp*i)%(2*N));
      }
      else {
        temp_root = power_mod_q(modulus, root, (exp*i)%(2*N));
      }
      
      matrix[i][j] = (temp_root*Inv_N) % modulus;

      exp *= primitive_root;
      exp %= (2*N);
    }

  }

  return matrix;
}

extern polynomial ecd_with_M(int64_t modulus, int N, const vector<vector<int64_t>> & M, const vector<int64_t>& m){
  //construct output polynomial 
  polynomial y(N+1);
  y[0]=N-1;

  for(int i = 0 ; i < N ; ++i){
    int64_t temp_y = 0;
    for(int j = 0 ; j < N ; ++j){
      int64_t temp_m = u_modq_64(m[j],modulus);
      temp_y += (M[i][j]*temp_m)%modulus;
    }
    temp_y %= modulus;
    y[i+1] = temp_y;
    if(y[i+1] > modulus/2){
      y[i+1] -= modulus;
    }
  }

  return y;
}

extern vector<int64_t> Decoding(int64_t modulus, int64_t primitive_root, int N, const polynomial& y){

  vector<int64_t> m(N);

  int degree = y[0];

  int64_t root = power_mod_q(modulus, primitive_root, (modulus-1)/(2*N));
 // cout <<"2N-th primitive_root is: "<<root<<endl;

  //compute inverse of primitive root Inv_root = primitive_root^{2N-1}
  int64_t Inv_root = power_mod_q(modulus, root, 2*N-1);

 // cout <<"Inverse of primitive_root is: "<<Inv_root<<endl;

   //cout <<"Decoding matrix: "<<endl;

  int exp = 1;

  for(int i = 0 ; i < N ; ++i){
    //cout <<i <<endl;
    int64_t temp_m = 0;

    //cout <<"3^i = "<<exp<<endl;
    int64_t temp_root = 0;
    if (i < N/2) {
      temp_root = power_mod_q(modulus, root, exp);
    }
    else {
      temp_root = power_mod_q(modulus, Inv_root, exp);
    }
    

    for(int j = 0 ; j < N ; ++j){
      int64_t root = power_mod_q(modulus, temp_root, j);
    //  cout <<root<<" ";
    //  int64_t tempy = u_modq_64(y[j+1],modulus);
      int64_t tempy = y[j+1];
      
      if(tempy < 0){
        tempy += modulus;
      }
      
      //temp_m += root*y[j+1];
      temp_m += root*tempy;
      //temp_m = u_modq_64(temp_m,modulus);
      temp_m %= modulus;
    }
    //cout <<endl;

    m[i] = temp_m;

    exp *= primitive_root;
    //exp = u_modq_32(exp,2*N);
    exp %= (2*N);
  }

  return m;
}

extern vector<vector<int64_t>> dcd_matrix(int64_t modulus, int64_t primitive_root, int N){

  int64_t root = power_mod_q(modulus, primitive_root, (modulus-1)/(2*N));
 // cout <<"2N-th primitive_root is: "<<root<<endl;

  //compute inverse of primitive root Inv_root = primitive_root^{2N-1}
  int64_t Inv_root = power_mod_q(modulus, root, 2*N-1);

 // cout <<"Inverse of primitive_root is: "<<Inv_root<<endl;

   //cout <<"Decoding matrix: "<<endl;
  vector<vector<int64_t>> matrix(N,vector<int64_t>(N));

  int exp = 1;

  for(int i = 0 ; i < N ; ++i){

    int64_t temp_root = 0;

    if (i < N/2) {
      temp_root = power_mod_q(modulus, root, exp);
    }
    else {
      temp_root = power_mod_q(modulus, Inv_root, exp);
    }
    
    for(int j = 0 ; j < N ; ++j){
      matrix[i][j] = power_mod_q(modulus, temp_root, j);
     
    }

    exp *= primitive_root;
    exp %= (2*N);
  }

  return matrix;
}

extern vector<int64_t> dcd_with_M(int64_t modulus, int N, const vector<vector<int64_t>> & M, const polynomial& y){
  vector<int64_t> m(N);
   for(int i = 0 ; i < N ; ++i){
    int64_t temp_m = 0;
    for(int j = 0 ; j < N ; ++j){
      //int64_t tempy = y[j+1];
      //int64_t tempy = u_modq_64(y[j+1],modulus);
      int64_t tempy = y[j+1];
      
      if(tempy < 0){
        tempy += modulus;
      }
      temp_m += (M[i][j]*tempy)%modulus;
    }
    temp_m %= modulus;
    m[i] = temp_m;
  }
  return m;
}