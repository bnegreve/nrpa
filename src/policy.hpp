// policy.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef POLICY_HPP

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

#endif 

