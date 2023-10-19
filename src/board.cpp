#include "board.hpp"

#include "business.hpp"
#include "cup.hpp"
#include "util.hpp"

#include <vector>
using namespace std;

typedef vector<item> Board;

struct EconBoardSet {
  vector<item> boards[numBoards];
};

Cup<EconBoardSet> boardSets;

void resetBoards() {
  for (int i = 0; i < boardSets.size(); i++) {
    for (int j = 0; j < numBoards; j++) {
      boardSets.get(i)->boards[j].clear();
    }
  }
  boardSets.clear();
}

Board* getBoard(item econ, item boardNdx) {
  boardSets.ensureSize(econ+1);
  return &(boardSets.get(econ)->boards[boardNdx]);
}

item boardTake(item econ, item boardNdx) {
  vector<item>* board = getBoard(econ, boardNdx);
  if (board->size() == 0) {
    return 0;
  }

  item i = randItem(board->size());
  item result = board->at(i);
  board->erase(board->begin()+i);
  return result;
}

/*
void boardTake(item boardNdx, item ndx) {
  for (int i=boards[boardNdx].size()-1; i >= 0; i--) {
    if(boards[boardNdx][i] == ndx) {
      boards[boardNdx].erase(boards[boardNdx].begin()+i);
      return;
    }
  }
}
*/

void boardPut(item econ, item boardNdx, item ndx) {
  vector<item>* board = getBoard(econ, boardNdx);
  board->push_back(ndx);
}

void boardPut(item econ, item boardNdx, item ndx, item num) {
  vector<item>* board = getBoard(econ, boardNdx);

  for (int i = 0; i < num; i++) {
    board->push_back(ndx);
  }
}

int boardSize(item econ, item boardNdx) {
  vector<item>* board = getBoard(econ, boardNdx);
  return board->size();
}

void boardClean(item econ, item boardNdx, item ndx) {
  vector<item>* board = getBoard(econ, boardNdx);
  for (int i=board->size()-1; i >= 0; i--) {
    if(board->at(i) == ndx) {
      board->erase(board->begin()+i);
    }
  }
}

void writeBoards(FileBuffer* file) {
  //fwrite_int(file, numBoards);
  //for (int i = 0; i < numBoards; i++) {
    //fwrite_item_vector(file, &boards[i]);
  //}
}

void readBoards(FileBuffer* file, int version) {
  if (version < 47) {
    int nb = 0;
    if (version < 5) {
      nb = 2;
    } else if (version < 40) {
      nb = 4;
    } else {
      nb = fread_int(file);
    }
    for (int i = 0; i < nb; i++) {
      vector<item> trash;
      fread_item_vector(file, &trash, version);
    }
  }
}

