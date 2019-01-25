/*
Copyright 2018 Pierre-Edouard Portier
peportier.me
​
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
​
    http://www.apache.org/licenses/LICENSE-2.0
​
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
)
*/

#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <utility>
#include <list>
#include <functional>
#include <cmath>
#include <algorithm>
#include <utility>
#include <iomanip>
#include <limits>
#include <chrono>

using namespace std;

typedef vector<int> State;

//  .---.
//  |2|0|
//  .---. -> State b = {2,0,1,3}
//  |1|3|
//  .---.

typedef function<int( const State& pos )> Heuristic;

/**
 * Taille du côté du plateau de jeu
 */
int side( const State& board )
{
  return (int)sqrt( board.size() );
}

/**
 * Somme des distances manhattan entre les positions actuelles des pièces
 * et leurs positions finales
 */
int manh( const State& board )
{
  int sum = 0;
  int s = side(board);
  for( int i = 0 ; i < board.size() ; i++ )
  {
    if( board[i] != 0 )  // not a tile, '0' doesn't count
    {
      sum += abs( i / s - board[i] / s ) +
           abs( i % s - board[i] % s );
    }
  }
  return sum;
}

/**
 * Nombre de pièces incorrectement placées
 */
int nbmis( const State& board )
{
  int n = 0;
  for( int i = 0 ; i < board.size() ; i++ )
  {
    if( board[i] != 0 ) // not a tile, '0' doesn't count
    {
      if( board[i] != i ) n++;
    }
  }
  return n;
}

/**
 * Retourne si nous sommes dans l'état final ou non
 */
bool final_state( const State& board )
{
  return (nbmis(board) == 0); // we use nbmis for it is quick to compute
}

/**
 * Affichage d'un plateau de jeu
 */
void print( const State& state )
{
  int s = side(state);
  for( int i = 0 ; i < state.size() ; i++ )
  {
    if( i % s == 0 ) cout << endl;
    cout << setw(2) << setfill('0') << state[i] << " , ";
  }
  cout << endl;
}

/**
 * Ajoute un noeud voisin
 */
void addNeighbor( State &neighbor, vector< pair< State,int > > &neighbors,
             list<State>& path, Heuristic h )
{
  // si il n'est pas déjà dans le trajet
  if( find( path.begin(), path.end(), neighbor ) == path.end() )
  {
    neighbors.push_back( make_pair( neighbor, h(neighbor) ) );
  }

}

void search( const State& current_state,
        int          upper_bound, // upper bound over which exploration must stop
        int&         next_upper_bound,
        list<State>& path,
        list<State>& best_path,
        Heuristic    h,
        int&         nb_visited_state )

{
  nb_visited_state++;

  int f; // under-estimation of optimal length
  int g = path.size() - 1; // size of the current path to current_state

  if( final_state( current_state ) )
  {
    best_path = path;
    return;
  }

  // generating the neighbors
  int s = side( current_state );
  vector< pair< State,int > > neighbors;
  neighbors.clear();
  
  int pos0 = find( current_state.begin(), current_state.end(), 0 ) - current_state.begin();
  
  if( (pos0 + 1) < current_state.size() &&
      ((pos0 + 1) % s) != 0 )
  {
    State neighbor = current_state;
    swap( neighbor[pos0] , neighbor[pos0 + 1] );
    addNeighbor( neighbor, neighbors, path, h );
  }
  
  if( (pos0 - 1) >= 0 &&
      ((pos0 - 1) % s) != (s-1) )
  {
    State neighbor = current_state;
    swap( neighbor[pos0] , neighbor[pos0 - 1] );
    addNeighbor( neighbor, neighbors, path, h );
  }
  
  if( (pos0 + s) < current_state.size() )
  {
    State neighbor = current_state;
    swap( neighbor[pos0] , neighbor[pos0 + s] );
    addNeighbor( neighbor, neighbors, path, h );
  }
  
  if( (pos0 - s) >= 0 )
  {
    State neighbor = current_state;
    swap( neighbor[pos0] , neighbor[pos0 - s] );
    addNeighbor( neighbor, neighbors, path, h );
  }

  //end generating neighbors

  // sort the neighbors by heuristic value

  //sort( neighbors.begin(), neighbors.end(),
  //    [](const pair<State,int> &left, const pair<State,int> &right) 
  //    {
  //      return left.second < right.second;
  //    } );

  //visiting the neighbors
  for( const pair<State,int> &p : neighbors )
  {
    f = g + 1 + p.second;
    if( f > upper_bound )
    {
      if( f < next_upper_bound )
      {
        next_upper_bound = f; // update the next upper bound
      }
    }
    else
    {
      path.push_back( p.first );
      search( p.first, upper_bound, next_upper_bound, path, best_path, h, nb_visited_state );
      path.pop_back();
      if( !best_path.empty() ) return;
    }
  }
}

void ida( const State&     initial_state, 
         Heuristic    h,
         list<State>& best_path, // path from source to destination
         int&         nb_visited_state )
{
  int upper_bound;                     // current upper bound
  int next_upper_bound = h( initial_state ); // next upper bound
  list<State> path;
  path.push_back( initial_state ); // the path to the target starts with the source

  while( best_path.empty() && next_upper_bound != numeric_limits<int>::max() )
  {
    upper_bound = next_upper_bound;
    next_upper_bound = numeric_limits<int>::max();

    cout << "upper bound: " << upper_bound;
    search( initial_state, upper_bound, next_upper_bound, path, best_path, h, nb_visited_state );
    cout << " ; nb_visited_state: " << nb_visited_state << endl;
  }
}

int main()
{
  //State b = {11,5,12,14,15,2,0,9,13,7,6,1,3,10,4,8}; // hard
  //State b = {15,2,12,11,14,13,9,5,1,3,8,7,0,10,6,4};
  //State b = {10,0,2,4,5,1,6,12,11,13,9,7,15,3,14,8}; // 33 -> 59
  //State b = {14,1,9,6,4,8,12,5,7,2,3,0,10,11,13,15}; // 35 -> 45
  //State b = {7,11,8,3,14,0,6,15,1,4,13,9,5,12,2,10}; // C1 36 -> 46
  //State b = {14,10,9,4,13,6,5,8,2,12,7,0,1,3,11,15}; // C2 43 -> 59
  State b = {4,8,3,2,0,7,6,5,1}; //C0
  //State b = {3,2,5,4,1,8,6,7,0};
  list<State> best_path;
  int nb_visited_state = 0;
  
  auto start = std::chrono::high_resolution_clock::now();
  ida( b, manh, best_path, nb_visited_state );
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  cout << "Elapsed time: " << elapsed.count() << " s\n";
  cout << "nb moves: " << best_path.size() - 1 << endl;
  cout << "nb visited states: " << nb_visited_state << endl;
  
  return 0;
}
