// movemap.cpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-23 Thu>

#include "movemap.hpp"

using namespace std; 


ostream &operator<<(ostream &os, const AbstractMove &m){
  m.print(os); 
  return os; 
}

IntMove::IntMove (int move): _moveId(move){ }
IntMove::IntMove (const IntMove &cpy):_moveId(cpy._moveId){ }
    
int IntMove::hash() const{ return _moveId; }

bool IntMove::operator==(const AbstractMove &other) const { 
  const IntMove *m = dynamic_cast<const IntMove *>(&other);
  if(m == NULL) return false;
  else return _moveId == m->_moveId; 
}

void IntMove::print(ostream &os) const {
  os<<_moveId;
}

AbstractMove *IntMove::clone() const{
  return new IntMove(*this); 
}


int MoveMap::registerMove(const AbstractMove &m){
  auto hit = _codes.find(&m);

  if(hit == _codes.end()){
    /* this is a new move */ 
    int newCode = _moves.size(); 
    _moves.push_back(m.clone()); 
    _codes[ _moves.back() ] = newCode; 
    return newCode; 
  }
  else{
    return hit->second; 
  }
}

ostream &operator<<(ostream &os, const MoveMap &m){
  for(int i = 0; i < m._moves.size(); i++){
    os<<"Code : "<<i<<" "<<" Move: "<<(*m._moves[i])<<endl;
  }
  return os; 
}





#if 1 // testing 
int main(){
  MoveMap mm;

  // MyMove m(1,2); 
  // MyMove m2(1,3);
  
  // mm.registerMove(m);
  // mm.registerMove(m2);



  // cout<<mm; 

  // cout<<mm.move(0)<<endl;


  // MyMove m3(1,3); 
  // cout<<mm.code(m3)<<endl;


  MoveMap intmap;

  intmap.registerMove(IntMove(1231));
  intmap.registerMove(IntMove(431));

  cout<<intmap<<endl;

  cout<<intmap.code(IntMove(431))<<endl;


}
#endif 
