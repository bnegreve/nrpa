#include <stdio.h>
#include <vector>
#include <iostream>
//#include <boost/random/mersenne_twister.hpp>
//#include <boost/random/uniform_01.hpp>
#include <sstream>
#include <stdlib.h>
#include <math.h>
//#define NDEBUG 0 // to comment in order to enable assert
#include <assert.h>
#include <nrpa.hpp>

const int MaxBoard=8;
const int MaxLegalMoves=MaxBoard*MaxBoard*4;
const int MaxPlayoutLength=MaxBoard*MaxBoard*4;
const int MaxMoveNumber=3 * MaxBoard * MaxBoard * MaxBoard * MaxBoard;
const int Seed=0;

const int MaxStrat=int(MaxBoard / 4) * int(MaxBoard / 4);

class Move
{
public:
      int code();
      Move();
      Move(int oX,int oY,int tX,int tY,int originCol);
      std::string display();
      int originX;
      int targetX;
      int originY;
      int targetY;

private:
      int originCol;
};
Move::Move()
{
      originX=-1;
      originY=-1;
      targetX=-1;
      targetY=-1;
      originCol=-1;
}
// Code = color + 10 * position of origin + 10000 * position of target
int Move::code()
{
  //return originCol + MaxBoard * originX + originY * 10 + MaxBoard * targetX + targetY * 1000 ;
  return  originX + originY * MaxBoard + MaxBoard * MaxBoard * targetX + MaxBoard * MaxBoard * MaxBoard * targetY + MaxBoard * MaxBoard * MaxBoard * MaxBoard * originCol;
}

Move::Move(int oX,int oY,int tX,int tY,int oCol)
{
      originX=oX;
      originY=oY;
      targetX=tX;
      targetY=tY;
      originCol=oCol;
      assert(fabs(originX -targetX)==1 || fabs(originY -targetY)==1);
      assert(originX<MaxBoard && originY<MaxBoard && originX >=0 && originY >=0);
      assert(targetX<MaxBoard && targetY<MaxBoard && targetX >=0 && targetY >=0);
      assert(originCol ==1 || originCol ==2);
}


std::string Move::display()
{
      char letterOrigin =char('A' + originY );
      char letterTarget =char('A' + targetY );
      std::ostringstream buffer;
      buffer<< '(' << letterOrigin << ',' << originX << ")x(" << letterTarget << ',' << targetX << ") ";
      return buffer.str();
}


class Board
{
public:
      // Function required by the file nested.c
      Board(int n);
      void play(Move m);
      void playout();
      bool terminal();
      int legalMoves(Move moves[MaxLegalMoves]);
      double score();
      // Public attributes required by the file nested.c
      int length;
      Move rollout [MaxPlayoutLength];

      // Functions called by the constructor 
      void init(); // to initialize the starting board configuration (checker like)
      void generatePossibleMoves(); // to get at start all possible moves 

      // Functions to update and get the possible legal moves on-line 
      std::vector<Move> newMovesAround(int x, int y);
      void addPossibleMovesAround(int x, int y);
      Move nextPossibleMove();
      // Debug function 
      bool isLegal(Move m);
      std::string display();
      std::string displayPossibleMove();
      std::string displayCurrentSequence();

      // Function  required by the file nestedStrat.c
      int stratLength[MaxStrat]; //values
      Move stratRollout [MaxStrat][MaxPlayoutLength];
      double score(int st);
      void print (FILE *fp) {
      }

private:
      int bd[MaxBoard][MaxBoard];
      int size;
      int stratByRow;// to know how many strategies by row
      std::vector<Move> possibleMoves;
      //BOOST random generator
      //boost::mt19937 randomGen;
};

Board::Board(int n=MaxBoard)
{
      size=n;
      length=0;
      for(int i=0; i <size; i++ )
      {
	    for(int j=0; j <size; j++ )
	    {
			bd[i][j] = 0;
	    }
      }
      //set a random seed
      if (Seed == -1 )
      {
//      boost::mt19937::result_type seed;


      }
      else
      {
	//randomGen.seed(Seed);	    
      }
      init();
      stratByRow = int(size /4 );
}

double Board::score()
{
      return length;
}
double Board::score(int st)
{
      assert(st >=0 && st < int(size /4 )*int(size /4 ) +1 );
      if(st==0)
	    return score();
      else
      {

	    // set starting end ending position
	    int startY = (int((st -1)/ stratByRow)) * 4; 
    	    int startX = ((st -1) % stratByRow )*4;
	    int endY = startY + 4;
	    int endX = startX + 4;
	    if(size - endY < 4 )
	    {
		  endY = size;
	    }
	    if(size - endX < 4 )
	    {
		  endX = size;
	    }
	    int nb=0;
	    for(int i = startX ; i < endX ; i++)
	    {
		  for(int j=startY ; j < endY ; j++)
		  {
			if(bd[i][j]==0)
			{
			      nb++;
			}
		  }
	    }
	    return nb;
      }
}



std::string Board::displayPossibleMove()
{
     std::ostringstream buffer;
     buffer << "Coup possible restants : \n";
      for(int i=0;i <possibleMoves.size();i++)
      {
	    buffer << possibleMoves[i].display();
      }
      buffer <<'\n';
      return buffer.str();
}

std::string Board::displayCurrentSequence()
{
    std::ostringstream buffer;
    buffer << "Sequence : ";
    for(int i=0; i <length;i++)
    {
	  buffer << rollout[i].display() << ' ';
    }
    buffer <<'\n';
    return buffer.str();
}

bool Board::isLegal(Move m)
{
      bool res = false;
      //check only if there is a black and a white stones in both Origin and target
      //DO NOT check if moves are inside the board 
      //DO NOT check if moves are adjacent
      assert(fabs(m.originX -m.targetX)==1 || fabs(m.originY -m.targetY)==1);
      assert(m.originX<MaxBoard && m.originY<MaxBoard && m.originX >=0 && m.originY >=0);
      assert(m.targetX<MaxBoard && m.targetY<MaxBoard && m.targetX >=0 && m.targetY >=0);

      if(bd[m.originX][m.originY] == 1 && bd[m.targetX][m.targetY] ==2 )
      {
	    res =true;
      }
      if(bd[m.originX][m.originY] == 2 && bd[m.targetX][m.targetY] ==1 )
      {
	    res =true;
      }
      return res;    
}

// Add all moves around this position (check color and existence)
std::vector<Move> Board::newMovesAround(int x, int y)
{
      std::vector<Move> moves;
      int oCol = bd[x][y];
      // Try to add left around move 
      if(x>0 && isLegal(Move(x,y,x-1,y,oCol)))
	    moves.push_back(Move(x,y,x-1,y,oCol));
      // Try to add right around move
      if(x<size-1 && isLegal(Move(x,y,x+1,y,oCol)))
	    moves.push_back(Move(x,y,x+1,y,oCol));
      // Try to add down around move
      if(y>0 && isLegal(Move(x,y,x,y-1,oCol)))
	    moves.push_back(Move(x,y,x,y-1,oCol));
      // Try to add up around move
      if(y<size-1 && isLegal(Move(x,y,x,y+1,oCol)))
	    moves.push_back(Move(x,y,x,y+1,oCol));
      return moves;
}



void Board::init()
{
      //Default start with checkerBoard
      for(int i=0; i <size; i++ )
      {
	    for(int j=0; j <size; j++ )
	    {
		  if( (i + j)% 2 ==0)
			bd[i][j] = 1;// black =1
		  else
			bd[i][j] = 2;// white =2
	    }
      }
      generatePossibleMoves();
}

void Board::addPossibleMovesAround(int x, int y)
{
      //generate moves around x and y
      std::vector<Move> newMoves = newMovesAround(x,y);
      //add all the moves around
      for(int k =0; k < newMoves.size(); k++)
      {
	    //number of possible moves (need for swap)
	    size_t size = possibleMoves.size();
	    if (size == 0)
	    {
		  possibleMoves.push_back(newMoves[k]);
	    }
	    else
	    {

		  //generate a random position among possible moves
		  //int pos = randomGen() % size;
		  int pos = rand () % size;
		  //swap the move from this position at the end
		  Move swapPoint = possibleMoves[pos];
		  possibleMoves.push_back(swapPoint);
		  //insert the new move at this position
		  possibleMoves[pos] = newMoves[k];
	    }
      }
}

void Board::generatePossibleMoves()
{
      //parse all the stones of the board and add all possible moves around
      for(int i=0; i <size; i++ )
      {
	    for(int j=0; j <size; j++ )
	    {
		  //add moves
		  addPossibleMovesAround(i,j);
	    }
      }
}

Move Board::nextPossibleMove()
{
    size_t size = possibleMoves.size();
    int start = rand () % size;
    int i = start;
    while (true) 
     { 
       Move p = possibleMoves[i];
       if (! isLegal(p))
        {
	      //the last possible move erase the current one and we pop back
	    possibleMoves[i] = possibleMoves[possibleMoves.size() -1];
            possibleMoves.pop_back();
	    --i;
	    if (i == -1) 
	      i = possibleMoves.size() - 1;
	    continue;
        }
        else
        {
//            CheckConsistency(); check if legal moves are in possibleMoves
            return p;
        }
    }

}


int Board::legalMoves(Move moves[MaxLegalMoves])
{
      int j=0;
      for(int i = possibleMoves.size() -1; i >= 0 ;i--)
      {
	    if(isLegal(possibleMoves[i]))
	    {
		  moves[j] = possibleMoves[i];
		  j++;
	    }
	    /* else */
	    /* { */
	    /* 	  possibleMoves.p; */
	    /* } */
      }
      return j;
}

bool Board::terminal()
{
      bool res=true;
      for(int i = possibleMoves.size() -1; i >= 0 ;i--)
      {
	    if(isLegal(possibleMoves[i]))
	    {
		  /* printf("Je suis legal !"); */
		  possibleMoves[i].display();
		  res=false;
		  return res;
	    }
	    /* else */
	    /* { */
	    /* 	  possibleMoves.pop_back(); */
	    /* } */
      }
      return res;
}

void Board::play(Move m)
{
      if(isLegal(m))
      {
	    //    std::cout << m.display();
	    bd[m.targetX][m.targetY] = bd[m.originX][m.originY];
	    bd[m.originX][m.originY] = 0;
	    addPossibleMovesAround(m.targetX,m.targetY);
	    rollout[length] = m;
	    length++;
	    
      }
      else
      {
	    std::cout <<"Coup non légal :" << m.display() <<'\n';
	     std::cout << display();
	     std::cout << displayPossibleMove();
	    exit(0);
      }
}

void Board::playout()
{
      while(! terminal())
      {
	    Move m= nextPossibleMove();
	    play(m);
      }
}
//code coming from Fuego
std::string Board::display()
{

    // Write board to a buffer first to avoid intermingling if boards are
    // dumped from different threads at the same time (e.g. debugging
    // output after an assertion)
    std::ostringstream buffer;
    if (size > 9)
        buffer << "   ";
    else
        buffer << "  ";
    int col;
    char c;
    for (col = 1, c = 'A'; col <= size; ++col, ++c)
    {
        if (c == 'I')
            ++c;
        buffer << c << ' ';
    }
    buffer << '\n';
    for (int row = size -1; row >= 0; --row)
    {
        if (size > 9 && row < 10)
            buffer << ' ';
        buffer << row << ' ';
        for (int col = 0; col < size; ++col)
        {
//	      printf("Position %d,%d  et couleur %d\n",row,col,bd[row][col]);
            switch (bd[row][col])
            {
            case 1:
                buffer << 'X';
                break;
            case 2:
                buffer << 'O';
                break;
            case 0:
                    buffer << '.';
                break;
            default:
		  printf("Erreur dans le plateau \n");
		  exit(0);
            }
            buffer << ' ';
        }
        buffer << row;
        if (row <= 2)
        {
            if (size < 10)
                buffer << "  ";
            else
                buffer << "   ";
            // More important info first, because the number of infos shown
            // depends on the board size
            if (row == 1)
                buffer << " Next Move";
        }
        buffer << '\n';
    }
    if (size > 9)
        buffer << "   ";
    else
        buffer << "  ";
    for (col = 1, c = 'A'; col <= size; ++col, ++c)
    {
        buffer << c << ' ';
    }
    buffer << '\n';
    return buffer.str();
}

using namespace std;

//#include "nestedSH.c"
//#include "nested.c"
//#include "nrpa.c"

int main(int argc, char *argv[])
{
  srand (time(NULL));
  double pol [MaxMoveNumber];//initialize the table pol
  for(int i=0 ; i < MaxMoveNumber ; i++)
  {
	pol[i]=0;
  }
  Nrpa<Board, Move, 5, MaxPlayoutLength, MaxLegalMoves>::test(Options::parse(argc, argv));

  // testTimeNested (2);
  //nrpa(4,pol);
  /*
  Board b;
  std::cout << b.display();
  nested(b,2);
  std::cout << b.display();
  /**/
 }
