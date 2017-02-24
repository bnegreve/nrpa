// movemap.cpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-24 Fri>

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

std::ostream &operator<<(std::ostream &os, const AbstractMove &m); 

class IntMove : public AbstractMove{
public: 
  IntMove (int move); 
  IntMove (const IntMove &cpy); 
    
  virtual int hash() const; 
  virtual bool operator==(const AbstractMove &other) const; 
  virtual void print(std::ostream &os) const; 
  virtual AbstractMove *clone() const; 

private:
  int _moveId; 
};

// struct MyMove : AbstractMove{
//   int a;
//   int b ; 

//   MyMove(int a, int b):a(a),b(b){ }
//   MyMove(const MyMove &cpy): a(cpy.a), b(cpy.b){ }
  
//   int hash() const { return a*1000+b; }
//   bool operator==(const AbstractMove &other) const{
//     const MyMove *m = dynamic_cast<const MyMove *>(&other);
//     if(m == NULL) return false;
//     else return a == m->a && b == m->b; 
//   }
//   void print(std::ostream &os) const {  os<<"A = "<<a<<" B = "<< b; }

//   virtual AbstractMove *clone() const{
//     return new MyMove(*this); 
//   }
// }; 

// std::ostream &operator<<(std::ostream &os, const MyMove &m){
//   os<<"A = "<<m.a<<" B = "<< m.b; 
//   return os; 
// }

class MoveMap{
  friend std::ostream &operator<<(std::ostream &, const MoveMap &);
public:


  inline const AbstractMove &move(int code) const {
    assert(code < _moves.size()); 
    return *_moves[code]; 
  }

  /* return move code from move, fail in DEBUG mode if code does not exist */ 
  inline int code(const AbstractMove &m) const{
    auto hit = _codes.find(&m);
    assert(hit != _codes.end()); 
    return hit->second; 
  }

  int registerMove(const AbstractMove &m); 

private:

  struct PtrHasher {
    inline size_t operator()(const AbstractMove *move) const{
      return move->hash(); 
    }
  };

  struct PtrEqOp{
    inline size_t operator()(const AbstractMove *m1, const AbstractMove *m2) const{
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


std::ostream &operator<<(std::ostream &os, const MoveMap &m); 


