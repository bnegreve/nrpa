// movemap.cpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-23 Thu>

#include "movemap.hpp"

using namespace std; 


#if 0 // testing 
int main(){


  /* Example where move is represented as an int */ 
  MoveMap<int> mm; 
  
  mm.registerMove(110);
  mm.registerMove(220);

  cout<<mm; 

  cout<<"Code for 110: "<<mm.code(110)<<endl;
  cout<<"Code for 220: "<<mm.code(220)<<endl;
  cout<<"Move for code for 110: "<<mm.move(mm.code(110))<<endl;
  //  cout<<"Code for 440 (must fail) "<<endl<<mm.code(440)<<endl;


  /* Example for a move represented as a struct */ 
  struct MyMove{

    MyMove(int a, int b):a(a), b(b){}

    int a;
    int b; 
  }; 

  // struct intHasher{
  //   int operator()(const int &a) { return a; }
  // }; 

  // struct eqOp{
  //   bool operator()(int a, int b) { return a==b; }
  // }; 


  struct MyMoveHasher{
    int operator()(const MyMove &m){ return m.a* 1000 + m.b; }
  }; 

  struct MyMoveEq{
    bool operator()(const MyMove &m1, const MyMove &m2){
      m1.a == m2.a && m1.a == m2.b; 
    }
  };

  MyMove x(1, 2); 
  MyMove y(1, 3); 

  MoveMap<MyMove, MyMoveHasher, MyMoveEq> mm2;

  
  mm2.registerMove(MyMove(1, 2));
  mm2.registerMove(MyMove(1, 4));

  cout<<mm; 


  

}
#endif 
