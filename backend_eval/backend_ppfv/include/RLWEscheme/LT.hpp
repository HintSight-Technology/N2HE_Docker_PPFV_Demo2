#include <iostream>
#include <cmath>
#include <omp.h>
using namespace std;

typedef vector<int64_t> polynomial;
//extern intel::hexl::NTT* g_ntt; 


//Linear Transformation 
//Input: ct in RLWE(Ecd(z,\delta)), \delta', plain matrix M, vector t
extern vector<polynomial> LT_small_matrix(const vector<polynomial> & RLWE_ct, const int64_t delta, const int64_t new_delta, 
  const vector<vector<int64_t>> & M_1, const vector<vector<int64_t>> & M_2, int N, int64_t ct_modulus, int logq, int64_t b, int logb,
  int64_t modulus, int64_t primitive_root, const vector<vector<vector<polynomial>>> & RotK, const polynomial & s){

  int M_l = M_1.size(); 
  int M_n = M_2[0].size();

  //cout <<"Matrix M, Line = "<<M_l<<" , column = "<<M_n<<endl;

  int n1,n2;
  if(M_l >= M_n){
    n1 = M_l;
    n2 = M_n;
  }
  else{
    n1 = M_n;
    n2 = M_l;
  }

  //cout <<"n1 = "<<n1 <<", n2 = "<<n2;

  //construct n2 vectors
  vector<vector<int64_t>> M2(2*n2, vector<int64_t>(2*n1,0));
  for(int i = 0 ; i < n2 ; ++i){
    for(int j = 0 ; j < n1 ; ++j){
      //index1 = j mod M_l
      int index1 = j;
      while(index1 >= M_l){
        index1 -= M_l;
      }
      //index2 = j+i mod M_n
      int index2 = j+i;
      while(index2 >= M_n){
        index2 -= M_n;
      }

      M2[i][j] = M_1[index1][index2];
      M2[i+n2][j+n2] = M_2[index1][index2];
    }
  }
/*
  cout <<"{vector m}_j in [n2]: "<<endl;
  for(int i = 0 ; i < 2*n2 ; ++i){
    for(int j = 0 ; j < 2*n1 ; ++j){
      cout <<M2[i][j]<<" ";
    }
    cout <<endl;
  }
*/
  n2 *= 2;
  n1 *= 2;
  M_l *= 2;
  M_n *= 2;

  //baby step

  double sqrt_n2 = sqrt((double)n2);
 // cout <<"sqrt n2 = "<<sqrt_n2<<endl;

  //rounding sqrt_n2
  int r_sqrt_n2 = (int)sqrt_n2;
  if(sqrt_n2-(double)r_sqrt_n2 > 0){
    r_sqrt_n2 ++;
  }
 // cout <<"rounding of sqrt n2 = "<<r_sqrt_n2<<endl;

  vector<vector<polynomial>> c_g(r_sqrt_n2);
  c_g[0] = RLWE_ct;


  for(int i = 1 ; i < r_sqrt_n2 ; ++i){
    c_g[i] = Rotation(RLWE_ct, RotK[i], N, i, ct_modulus, logq, b, logb,(int)primitive_root);
  }

/*
  cout <<"test for baby step: "<<endl;
  for(int i = 0 ; i < r_sqrt_n2 ; ++i){
    polynomial dec = RLWE64_Dec(N,ct_modulus,modulus,s,c_g[i]);
    vector<int64_t> de_y = Decoding(modulus,primitive_root,N,dec);
    cout <<"Decoding of decryption of c_g "<<i<<": ";
    for(int j = 0 ; j < N ; ++j){
      cout <<de_y[j]<<" ";
    }
    cout <<endl;

  }
*/

  //giant step
  double bb = (double)n2/(double)r_sqrt_n2;
  int r_b = (int)bb;
  if(bb-(double)r_b > 0){
    r_b++;
  }

  //cout <<"rounding of n2/g = "<<r_b<<endl;

  vector<polynomial> RLWE_new_ct(2,polynomial(N,0));
  //RLWE_new_ct[0][0] = N-1;
  //RLWE_new_ct[1][0] = N-1;
  //cout <<"void ct constructed. "<<endl;
  //b = 0
  for(int i = 0 ; i < r_sqrt_n2;++i){
   // cout <<i<<endl;
   // cout <<"Output for m "<<i<<", rotate = "<<0<<": ";

    polynomial ecd_m = Encoding(modulus, primitive_root,N ,M2[i]);
  //  cout <<"ecd(m): "<<endl;
  //  print_polynomial(ecd_m);

/*
    vector<int64_t> de_m = Decoding(modulus,primitive_root,N,ecd_m);
    cout <<"Decoding of ecd(m): "<<endl;
      for(int j = 0 ; j < N ; ++j){
      cout <<de_m[j]<<" ";
    }
    cout <<endl;
*/
      
  //  cout <<"encoding. "<<endl;
    polynomial c_g0ecd_m = NTTMul(ecd_m,c_g[i][0],N,ct_modulus);
    polynomial c_g1ecd_m = NTTMul(ecd_m,c_g[i][1],N,ct_modulus);
/*
    vector<polynomial> tempcgecd(2);
    tempcgecd[0] = c_g0ecd_m;
    tempcgecd[1] = c_g1ecd_m;
    polynomial deccg = RLWE64_Dec(N,ct_modulus,modulus,s,tempcgecd);
   // print_polynomial(dec);
    vector<int64_t> de_cg = Decoding(modulus,primitive_root,N,deccg);
    cout <<"Decoding of ecd(m)*c_g : ";
    for(int j = 0 ; j < N ; ++j){
      cout <<de_cg[j]<<" ";
    }
    cout <<endl;
*/

   // polynomial c_g0ecd_m = mul_poly(ecd_m,c_g[i][0],N,ct_modulus);
  //  polynomial c_g1ecd_m = mul_poly(ecd_m,c_g[i][1],N,ct_modulus);
   // cout <<"multiplication. "<<endl;
    add_poly(RLWE_new_ct[0],c_g0ecd_m,N,ct_modulus);
    add_poly(RLWE_new_ct[1],c_g1ecd_m,N,ct_modulus);
   // cout <<"Addition. "<<endl;


    //test for giant step with b = 0
    polynomial dec = RLWE64_Dec(N,ct_modulus,modulus,s,RLWE_new_ct);
   // print_polynomial(dec);
    vector<int64_t> de_y = Decoding(modulus,primitive_root,N,dec);
  //  cout <<"Decoding: ";
  //  for(int j = 0 ; j < N ; ++j){
   //   cout <<de_y[j]<<" ";
   // }
   // cout <<endl;
    
  }
  //cout <<"b=0"<<endl;

  for(int j = 1; j < r_b ; ++j){
   // cout <<j <<endl;

    vector<polynomial> temp_ct(2,polynomial(N,0));
    //temp_ct[0][0] = N-1;
    //temp_ct[1][0] = N-1;
    //cout <<"void ct constructed. "<<endl;

    for (int i = 0; i < r_sqrt_n2; ++i){
     // cout <<i <<endl;
      int index = j*r_sqrt_n2+i;
      if(index >= M_l){
        break;
      }
      int index2 = j*r_sqrt_n2;
      while(index2 >= M_n/2){
        index2 -= M_n/2;
      }
      polynomial ecd_m = Encoding(modulus, primitive_root,N ,Rot_Vec(M2[index],index2));
      
      /*
      vector<int64_t> de_m = Decoding(modulus,primitive_root,N,ecd_m);
      cout <<"Decoding output for m "<<index<<", rotate = "<<index2<<endl;
      for(int k = 0 ; k < N ; ++k){
       cout <<de_m[k]<<" ";
      }
      cout <<endl;
      */
      
     // cout <<"encoding. "<<endl;
      polynomial c_g0ecd_m = NTTMul(ecd_m,c_g[i][0],N,ct_modulus);
      polynomial c_g1ecd_m = NTTMul(ecd_m,c_g[i][1],N,ct_modulus);

     // polynomial c_g0ecd_m = mul_poly(ecd_m,c_g[i][0],N,ct_modulus);
     // polynomial c_g1ecd_m = mul_poly(ecd_m,c_g[i][1],N,ct_modulus);
    // cout <<"multiplication. "<<endl;
      add_poly(temp_ct[0],c_g0ecd_m,N,ct_modulus);
      add_poly(temp_ct[1],c_g1ecd_m,N,ct_modulus);

/*
      polynomial dec = RLWE64_Dec(N,ct_modulus,s,temp_ct);
      vector<int64_t> de_y = Decoding(modulus,primitive_root,N,dec);
      cout <<"Decoding output for b = 1, i = "<<i<<endl;
      cout<<de_y[0]<<" "<<de_y[N-1]<<endl;
      */
      
    }
    int rot_index = j*r_sqrt_n2;
      while(rot_index >= M_l/2){
        rot_index -= M_l/2;
      }
    vector<polynomial>temp_rot = Rotation(temp_ct, RotK[rot_index], N, rot_index, ct_modulus, logq, b, logb,(int)primitive_root);


    add_poly(RLWE_new_ct[0],temp_rot[0],N,ct_modulus);
    add_poly(RLWE_new_ct[1],temp_rot[1],N,ct_modulus);

  }

  //vector<polynomial> out;

  if(M_l >= M_n){
 /*
    polynomial fdec = RLWE64_Dec(N,ct_modulus,s,RLWE_new_ct);
    vector<int64_t> fde_y = Decoding(modulus,primitive_root,N,fdec);
    for(int k = 0 ; k < N ; ++k){
      cout <<fde_y[k]<<" ";
    }
    cout <<endl;
*/
    return RLWE_new_ct;
  }

  else{
    cout <<"to be complete. "<<endl;
    return RLWE_new_ct;
  }
  
}

extern vector<polynomial> ecd_M(const vector<vector<int64_t>> & M,
  int N,int64_t modulus, const vector<vector<int64_t>> &ecd_matrix){

  int M_l = M.size(); 
  int M_n = M[0].size();

  //cout <<"Matrix M, Line = "<<M_l<<" , column = "<<M_n<<endl;

  int n1,n2;
  if(M_l >= M_n){
    n1 = M_l;
    n2 = M_n;
  }
  else{
    n1 = M_n;
    n2 = M_l;
  }
/*
  //cout <<"n1 = "<<n1 <<", n2 = "<<n2;
  cout <<"{vector m}_j in [n2]: "<<endl;
  for(int i = 0 ; i < n2 ; ++i){
    for(int j = 0 ; j < n1 ; ++j){
      cout <<M[i][j]<<" ";
    }
    cout <<endl;
  }
*/
  //construct n2 vectors
  vector<vector<int64_t>> M2(n2, vector<int64_t>(n1,1));

  #pragma omp parallel for

  for(int i = 0 ; i < n2 ; ++i){
    for(int j = 0 ; j < n1 ; ++j){
      //index1 = j mod M_l
      int index1 = j;
      while(index1 >= M_l){
        index1 -= M_l;
      }
      //index2 = j+i mod M_n
      int index2 = j+i;
      while(index2 >= M_n){
        index2 -= M_n;
      }

      M2[i][j] = M[index1][index2];
      //M2[i+n2][j+n2] = M_2[index1][index2];
    }
  }

  
/*
  cout <<"{vector m}_j in [n2]: "<<endl;
  for(int i = 0 ; i < n2 ; ++i){
    for(int j = 0 ; j < n1 ; ++j){
      cout <<M2[i][j]<<" ";
    }
    cout <<endl;
  }
*/

  double sqrt_n2 = sqrt((double)n2);
 // cout <<"sqrt n2 = "<<sqrt_n2<<endl;

  //rounding sqrt_n2
  int r_sqrt_n2 = (int)sqrt_n2;
  if(sqrt_n2-(double)r_sqrt_n2 > 0){
    r_sqrt_n2 ++;
  }

  //giant step
  double b = (double)n2/(double)r_sqrt_n2;
  int r_b = (int)b;
  if(b-(double)r_b > 0){
    r_b++;
  }

  //cout <<r_sqrt_n2<<" "<<r_b<<endl;

  vector<polynomial> out;


   for(int i = 0 ; i < r_sqrt_n2;++i){
    //cout <<i<<endl;
    //polynomial ecd_m = Encoding(modulus, primitive_root,N ,M2[i]);
    polynomial ecd_m = ecd_with_M(modulus, N, ecd_matrix, M2[i]);
    out.emplace_back(ecd_m);
  }



    for(int j = 1; j < r_b ; ++j){
   // cout <<j <<endl;
    for (int i = 0; i < r_sqrt_n2; ++i){
     // cout <<i <<endl;
      int index = j*r_sqrt_n2+i;
      if(index >= M_l){
        break;
      }
      int index2 = j*r_sqrt_n2;
      while(index2 >= M_n/2){
        index2 -= M_n/2;
      }
      //polynomial ecd_m = Encoding(modulus, primitive_root,N ,Rot_Vec(M2[index],index2));
      polynomial ecd_m = ecd_with_M(modulus, N, ecd_matrix, Rot_Vec(M2[index],index2));
      out.emplace_back(ecd_m);

     // out[j*r_sqrt_n2+i] = ecd_with_M(modulus, N, ecd_matrix, Rot_Vec(M2[index],index2));

    }

  }

  cout <<"Size of encoded matrix: ";
  cout <<out.size()<<endl;

  return out;

}


//Linear Transformation with encoded matrix and l >= n
//Input: ct in RLWE(Ecd(z,\delta)), \delta', plain matrix M, vector t
extern vector<polynomial> LT_ecd_M(const vector<polynomial> & RLWE_ct, const int64_t delta, const int64_t new_delta, 
  const vector<polynomial> & M, int N, int64_t ct_modulus, int logq,int64_t b, int logb,const vector<vector<vector<polynomial>>> & RotK, 
  const polynomial & s, int primitive_root){

  int n2 = M.size();

  double sqrt_n2 = sqrt((double)n2);
 // cout <<"sqrt n2 = "<<sqrt_n2<<endl;

  //rounding sqrt_n2
  int r_sqrt_n2 = (int)sqrt_n2;
  if(sqrt_n2-(double)r_sqrt_n2 > 0){
    r_sqrt_n2 ++;
  }
  //cout <<"rounding of sqrt n2 = "<<r_sqrt_n2<<endl;

  vector<vector<polynomial>> c_g(r_sqrt_n2);
  c_g[0] = RLWE_ct;

  #pragma omp parallel for

  for(int i = 1 ; i < r_sqrt_n2 ; ++i){
    //cout <<i<<endl;
    c_g[i] = Rotation(RLWE_ct, RotK[i], N, i, ct_modulus, logq, b, logb, primitive_root);
  }

  //cout <<"baby step. "<<endl;
/*
  for(int i = 0 ; i < r_sqrt_n2 ; ++i){
    polynomial dec = RLWE64_Dec(N,ct_modulus,12289,s,c_g[i]);
    vector<int64_t> de_y = Decoding(12289,11,N,dec);
    cout <<"Decoding of decryption of c_g "<<i<<": ";
    for(int j = 0 ; j < N ; ++j){
      cout <<de_y[j]<<" ";
    }
    cout <<endl;
    }
*/
  

  //giant step
  double bb = (double)n2/(double)r_sqrt_n2;
  int r_b = (int)bb;
  if(bb-(double)r_b > 0){
    r_b++;
  }

 // cout <<r_sqrt_n2<<" "<<r_b<<endl;

  //cout <<"rounding of n2/g = "<<r_b<<endl;

  vector<polynomial> RLWE_new_ct(2,polynomial(N,0));

  //cout <<"void ct constructed. "<<endl;
  //b = 0

  vector<polynomial> temp_gs_1_ct(r_sqrt_n2*2,polynomial(N,0));

  #pragma omp parallel for

  for(int i = 0 ; i < r_sqrt_n2;++i){
   // cout <<i<<endl;
    //polynomial ecd_m = Encoding(modulus, primitive_root,N ,M2[i]);

  //  cout <<"encoding. "<<endl;
  //  polynomial c_g0ecd_m = NTTMul(M[i],c_g[i][0],N,ct_modulus);
  //  polynomial c_g1ecd_m = NTTMul(M[i],c_g[i][1],N,ct_modulus);

    temp_gs_1_ct[2*i] = NTTMul(M[i],c_g[i][0],N,ct_modulus);
    temp_gs_1_ct[2*i+1] = NTTMul(M[i],c_g[i][1],N,ct_modulus);


  }

  for(int i = 0 ; i < r_sqrt_n2;++i){
   // add_poly(RLWE_new_ct[0],c_g0ecd_m,N,ct_modulus);
   // add_poly(RLWE_new_ct[1],c_g1ecd_m,N,ct_modulus);

    add_poly(RLWE_new_ct[0],temp_gs_1_ct[2*i],N,ct_modulus);
    add_poly(RLWE_new_ct[1],temp_gs_1_ct[2*i+1],N,ct_modulus);
   // cout <<"Addition. "<<endl;
  }


  vector<vector<polynomial>> temp_gs_2_ct(r_b-1,vector<polynomial>(2,polynomial(N,0)));

  #pragma omp parallel for

  for(int j = 1; j < r_b ; ++j){
    //cout <<j <<endl;

    vector<polynomial> temp_ct(2,polynomial(N,0));
    //temp_ct[0][0] = N-1;
    //temp_ct[1][0] = N-1;
    //cout <<"void ct constructed. "<<endl;

    for (int i = 0; i < r_sqrt_n2; ++i){
     // cout <<i <<endl;
      int index = j*r_sqrt_n2+i;
      if(index >= n2){
        break;
      }

    //  polynomial ecd_m = Encoding(modulus, primitive_root,N ,Rot_Vec(M2[index],index2));
      
     // cout <<"encoding. "<<endl;
      polynomial c_g0ecd_m = NTTMul(M[j*r_sqrt_n2+i],c_g[i][0],N,ct_modulus);
      polynomial c_g1ecd_m = NTTMul(M[j*r_sqrt_n2+i],c_g[i][1],N,ct_modulus);


    // cout <<"multiplication. "<<endl;
      add_poly(temp_ct[0],c_g0ecd_m,N,ct_modulus);
      add_poly(temp_ct[1],c_g1ecd_m,N,ct_modulus);

    }
    int rot_index = j*r_sqrt_n2;
      while(rot_index >= n2/2){
        rot_index -= n2/2;
      }
   // vector<polynomial>temp_rot = Rotation(temp_ct, RotK[rot_index], N, rot_index, ct_modulus, logq, b, logb,(int)primitive_root);
    temp_gs_2_ct[j-1] = Rotation(temp_ct, RotK[rot_index], N, rot_index, ct_modulus, logq, b, logb,(int)primitive_root);
}

  for(int i = 1 ; i < r_b;++i){
    add_poly(RLWE_new_ct[0],temp_gs_2_ct[i-1][0],N,ct_modulus);
    add_poly(RLWE_new_ct[1],temp_gs_2_ct[i-1][1],N,ct_modulus);

  }

    return RLWE_new_ct;
 
}




