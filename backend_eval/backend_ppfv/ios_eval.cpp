#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include "include.hpp"
#include <curl/curl.h>
using namespace std;
using json = nlohmann::json;


//Callback for libcurl to store response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
  output->append((char*)contents, size * nmemb);
  return size * nmemb;
}

extern void eval_test(){
  int num_vec = 1;                        // number of photos for each person
  double threshold = 0.468;
  double scaler0 = 40;                         // scaler for input vector
  double scaler1 = 40;                       // Scaler for vectors in the server

  int64_t scale_threshold = (int64_t)(threshold*scaler0*scaler1);

  const int degree = 1024;//2048;                        //polynomial degree 
  const int64_t ct_modulus = 3221225473;//206158430209;//2748779069441;//576460752154525697;           //ciphertext modulus
  const int64_t modulus = 6000;//12289;                       //plaintext modulus

  //int64_t ct_modulus = 576460752154525697;
  int64_t delta = 1;
  int logq = 32;//38;//42;//60;
  //int degree = 2048;
  double var = 3.2;
  int64_t b = 2;
  int logb = 1;
  //int64_t modulus = 12289; 
  int64_t primitive_root = 3;

  //read input ct
  //vector<vector<int64_t> > ct(2,vector<int64_t>(degree,0));
  ifstream fin;
  fin.open("ios_ppfr/put.txt");
  if(!fin.is_open()){
      cout <<"Cannot open file ios_ppfr/put.txt"<<endl;
    }
  string test_name;
  // read from put.txt, be careful of input format
  string timestamp;
  getline(fin,timestamp);
  cout << "time stamp = " << timestamp << endl;
  getline(fin,test_name);
  //string test_name(a.begin()+9,a.end()-2);
  cout <<"test name = "<<test_name<<endl;


  char aaa;
  fin >>aaa; // hard code input {
  fin >>aaa; // hard code input [
  //cout <<aaa;
  vector<vector<int64_t> > ct(2,vector<int64_t>(degree,0));
  for(int i = 0 ; i < 2 ; ++i){
    for(int j = 0 ; j < degree ; ++j){
  fin >>ct[i][j];
  fin >>aaa; // hard code input ,
  //cout <<ct[i][j]<<" ";
    }
    cout <<endl;

    fin>>aaa; // hard code input ]
    fin>>aaa; // hard code input }
  }
  fin.close();
  cout <<"Read input ciphertext from /share/put.txt"<<endl;

  //read encrypted vectors
  vector<vector<vector<int64_t> > > ct_server(num_vec, vector<vector<int64_t>>(2,vector<int64_t>(degree,0)));
  string api_url = "http://backend_database:8001/retrieve/" + test_name;
  
  CURL* curl = curl_easy_init();
  string response; 

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
      cerr << "Request from database failed: " << curl_easy_strerror(res) << endl;
    }

    //Parse JSON response
    json json_data = json::parse(response);
    auto raw_data = json_data["feature_vector"];
    
    for (int k = 0; k < num_vec; ++k) {
      for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < degree; ++j) {
          ct_server[k][i][j] = raw_data[i][j];
	}
      }
    }
  }
  
/* 
  vector<vector<vector<int64_t> > > ct_server(num_vec, vector<vector<int64_t>>(2,vector<int64_t>(degree,0)));

  fin.open("weights/"+test_name+".txt");
  if(!fin.is_open()){
      cout <<"Cannot open file weights/"+test_name+".txt"<<endl;
    }
  for(int k = 0 ; k < num_vec ; ++k){
    for(int i = 0 ; i < 2 ; ++i){
      for(int j = 0 ; j < degree ; ++j){
        fin >>ct_server[k][i][j];
      }
    }
  }
  fin.close();
 */
  cout <<"read encrypted registered vector from weights/"+test_name+".txt"<<endl;
  //read RelK
  vector<vector<polynomial>> RelK(logq, vector<polynomial>(2, polynomial(degree,0)));
  //fin.open("rlwe_relk.txt");
  fin.open("rlwe_relk.txt");
  if(!fin.is_open()){
      cout <<"Cannot open file ./rlwe_relk.txt"<<endl;
    }
  for (int i = 0; i < logq; ++i){
    for (int j = 0; j < 2; ++j){
      for(int k = 0 ; k < degree ; ++k){
        fin >>RelK[i][j][k];
      }
    }
  }
  fin.close();

  struct timeval tstart1, tend1;

  gettimeofday(&tstart1,NULL);

  vector<vector<int64_t>> lwe_ip(num_vec,vector<int64_t>(degree+1,0));
  for (int i = 0; i < num_vec; ++i){
    vector<polynomial> RLWE_mul1 = ct_multiplication(ct,ct_server[i], RelK, degree, ct_modulus,modulus, 
    logq, b, logb);
    lwe_ip[i] = extract_0(RLWE_mul1,degree);
    lwe_ip[i][degree] = modq_64(lwe_ip[i][degree]-scale_threshold, ct_modulus);
  }

  gettimeofday(&tend1,NULL);
  double eval_time = tend1.tv_sec-tstart1.tv_sec+(tend1.tv_usec-tstart1.tv_usec)/1000000.0;
  cout <<"evaluation time = "<<eval_time<<endl;
  ofstream fout;
  fout.open("ios_ppfr/"+test_name+"_"+timestamp+".json");


  fout <<"{"<<endl;

  fout<<"\"result\" : ";
  fout <<"[";
  for(int j = 0 ; j < num_vec ; ++j){
    for(int i = 0 ; i < (degree) ; ++i){
      fout <<lwe_ip[j][i]<<",";
    }
    fout << lwe_ip[j][degree];
    fout <<endl;
  }
  //fout <<endl;
  fout<<"]";
  fout<<"}"<<endl;
  fout.close();
  cout <<"Stored the encrypted result in .json"<<endl;
  
}


int main(){
  eval_test();
}
  
