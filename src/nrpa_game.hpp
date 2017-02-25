#ifndef NRPA_GAME_HPP
#define NRPA_GAME_HPP


/* Example Move class */
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


#include "movemap.hpp"
#include "nrpa.hpp"


#endif
