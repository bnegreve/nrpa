// movemap.cpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-23 Thu>

#include <unordered_map>
#include <vector>
#include <iostream>
#include <cassert>


/* TODO: use template instead of inheritence to avoid virual function calls and other costly dynamic things */ 

class AbstractMove{
public: 
  virtual int hash() const = 0; 
  virtual bool operator==(const AbstractMove &other) const = 0; 
  virtual void print(std::ostream &os) const = 0; 
  virtual AbstractMove *clone() const = 0; 
}; 

std::ostream &operator<<(std::ostream &os, const AbstractMove &m){
  m.print(os); 
  return os; 
}



class IntMove : public AbstractMove{
public: 
  IntMove (int move): _moveId(move){ }
  IntMove (const IntMove &cpy):_moveId(cpy._moveId){ }
    
  virtual int hash() const{ return _moveId; }
  virtual bool operator==(const AbstractMove &other) const { 
    const IntMove *m = dynamic_cast<const IntMove *>(&other);
    if(m == NULL) return false;
    else return _moveId == m->_moveId; 
 }
  virtual void print(std::ostream &os) const {
    os<<_moveId;
  }
  virtual AbstractMove *clone() const{
    return new IntMove(*this); 
  }
  
private:
  int _moveId; 
};

// template <typename T>
// class AbstractMove<T>{
// public:
//   AbstractMove(const T&);
//   int hash(const T &move);
//   operator==
// private:
//   T _move; 
// }
  
//   class AbstractMove<int>{
//   }; 


struct MyMove : AbstractMove{
  int a;
  int b ; 

  MyMove(int a, int b):a(a),b(b){ }
  MyMove(const MyMove &cpy): a(cpy.a), b(cpy.b){ }
  
  int hash() const { return a*1000+b; }
  bool operator==(const AbstractMove &other) const{
    const MyMove *m = dynamic_cast<const MyMove *>(&other);
    if(m == NULL) return false;
    else return a == m->a && b == m->b; 
  }
  void print(std::ostream &os) const {  os<<"A = "<<a<<" B = "<< b; }

  virtual AbstractMove *clone() const{
    return new MyMove(*this); 
  }
}; 

std::ostream &operator<<(std::ostream &os, const MyMove &m){
  os<<"A = "<<m.a<<" B = "<< m.b; 
  return os; 
}


// class AbstractMove{

//   virtual void 

//   virtual void print(std::ostream &os) = 0; 
// }

// std::ostream &operator<<(std::ostream &os, const Move &m){
//   os<<"A = "<<m.a<<" B = "<< m.b; 
//   return os; 
// }



// template<typename MoveType, typename MoveEqOp, typename MoveHasher>
// class MoveMap{
//   friend std::ostream &operator<<(std::ostream &, const MoveMap &);
// public:

//   const MoveType &move(int code) const{
//     assert(code < _moves.size()); 
//     return _moves[code]; 
//   }

//   /* return move code from move, fail in DEBUG mode if code does not exist */ 
//   int code(const MoveType &m) const{
//     auto hit = _codes.find(m);
//     assert(hit != _codes.end()); 
//     return hit->second; 
//   }

//   int registerMove(const MoveType &m){
//     auto hit = _codes.find(m);

//     if(hit == _codes.end()){
//       /* this is a new move */ 
//       int newCode = _moves.size(); 
//       _moves.push_back(m); 
//       _codes[ _moves.back() ] = newCode; 
//       return newCode; 
//     }
//     else{
//       return hit->second; 
//     }
//   }

// private:

//   std::unordered_map<const MoveType, int,
// 		     Hasher<MoveType>, EqOp<MoveType>> _codes; 
//   std::vector<MoveType> _moves; 

// };

// std::ostream &operator<<(std::ostream &os, const MoveMap &m){
//   for(int i = 0; i < m._moves.size(); i++){
//     os<<"Code : "<<i<<" "<<" Move: "<<m._moves[i]<<std::endl;
//   }
//   return os; 
// }


class MoveMap{
  friend std::ostream &operator<<(std::ostream &, const MoveMap &);
public:


  const AbstractMove &move(int code) const{
    assert(code < _moves.size()); 
    return *_moves[code]; 
  }

  /* return move code from move, fail in DEBUG mode if code does not exist */ 
  int code(const AbstractMove &m) const{
    auto hit = _codes.find(&m);
    assert(hit != _codes.end()); 
    return hit->second; 
  }

  int registerMove(const AbstractMove &m){
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

private:

  struct PtrHasher {
    size_t operator()(const AbstractMove *move) const{
      return move->hash(); 
    }
  };

  struct PtrEqOp{
    size_t operator()(const AbstractMove *m1, const AbstractMove *m2) const{
      std::cout<<"move cmp "<<std::endl; 
      return (*m1) == (*m2); 
    }
  };

  std::unordered_map<const AbstractMove *,
		     int,
		     PtrHasher,
		     PtrEqOp> _codes; 

  std::vector<AbstractMove *> _moves; 

};


std::ostream &operator<<(std::ostream &os, const MoveMap &m){
  for(int i = 0; i < m._moves.size(); i++){
    os<<"Code : "<<i<<" "<<" Move: "<<(*m._moves[i])<<std::endl;
  }
  return os; 
}





#if 1
int main(){
  MoveMap mm;

  MyMove m(1,2); 
  MyMove m2(1,3);
  
  mm.registerMove(m);
  mm.registerMove(m2);



  std::cout<<mm; 

  std::cout<<mm.move(0)<<std::endl;


  MyMove m3(1,3); 
  std::cout<<mm.code(m3)<<std::endl;


  MoveMap intmap;

  intmap.registerMove(IntMove(1231));

  std::cout<<intmap<<std::endl;



}
#endif 
