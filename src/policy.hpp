// policy.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef POLICY_HPP

const int SizeTablePolicy = 65535;

#if 1
class Policy{

public:

  inline double prob(int code) const {
    auto hit = _probs.find(code);
    //    assert(hit != _probs.end()); 
    if( hit != _probs.end() )
      return hit->second;
    else
      return 0.;
  }

  inline void setProb(int code, double prob){
    _probs[code] = prob; 
  }

  inline double updateProb(int code, double delta){
    _probs[code] += delta; 
    // auto hit = _probs.find(code);
    // if(hit != _probs.end()){
    //   hit->second += delta;
    //   return hit->second;
    // }
    // else{
    //   _probs[code] = delta;
    //   return delta; 
    // }
  }

  inline void print(std::ostream &os) const{
    os<<"Policy: "<<std::endl; 
    for(auto it = _probs.begin(); it != _probs.end(); ++it){
      os<<"\tCode : "<<it->first<<" prob: "<<it->second<<std::endl;
    }
    os<<"End of Policy"<<std::endl; 
  }


  inline void reset(){
    _probs.clear(); 
  }

  template <typename X, typename Y, typename Z>
  inline void print(std::ostream &os, const MoveMap<X,Y,Z> &movemap) const{
    os<<"Policy: "<<std::endl; 
    for(auto it = _probs.begin(); it != _probs.end(); ++it){
      os<<"\tCode : "<<movemap.move(it->first).hash<<" prob: "<<it->second<<std::endl;
    }
    os<<"End of Policy"<<std::endl; 
  }

private: 
  std::unordered_map<int, double> _probs; 
};
#else

class ProbabilityCode {
 public:
  int code;
  double proba;
};


class Policy {
 public:
  std::vector<ProbabilityCode> table [SizeTablePolicy + 1];

  void print(std::ostream &os) const {
    os<<"Policy "<<std::endl;
    for(int i = 0; i < SizeTablePolicy; i++)
      for(int j = 0; j < table[i].size(); j++)
	os<<table[i][j].code<<" : "<<table[i][j].proba<<std::endl;
  }


  void setProb (int code, double proba) {
    int index = code & SizeTablePolicy;
    for (int i = 0; i < table [index].size (); i++) {
      if (table [index] [i].code == code) {
	table [index] [i].proba = proba;
	return;
      }
    }
    ProbabilityCode p;
    p.code = code;
    p.proba = proba;
    table [index].push_back (p);
  }

  void updateProb(int code, double delta){
    int index = code & SizeTablePolicy;
    for (int i = 0; i < table [index].size (); i++) {
      if (table [index] [i].code == code) {
	table [index] [i].proba += delta;
	return;
      }
    }
    ProbabilityCode p;
    p.code = code;
    p.proba = delta;
    table [index].push_back (p);
  }

  double prob (int code) {
    int index = code & SizeTablePolicy;
    for (int i = 0; i < table [index].size (); i++)
      if (table [index] [i].code == code) {
	return table [index] [i].proba;
      }
    //return minNorm + (maxNorm - minNorm) * policyAMAF.get (code);
    return 0.0;
  }
  
  void reset(){
    for(int i = 0; i < SizeTablePolicy; i++){
      table[i].clear(); 
    }
  }


};

#endif 


#endif 

