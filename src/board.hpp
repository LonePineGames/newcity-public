#ifndef BOARD_H
#define BOARD_H

#include "item.hpp"
#include "serialize.hpp"

enum Boards {
  Homes, RetailTenancies, OfficeTenancies, FarmTenancies, FactoryTenancies,
  GovernmentTenancies, DormBunks, HotelRooms,

  numBoards
};

item boardTake(item econ, item boardNdx);
item boardTake(item econ, item boardNdx, item ndx);
void boardPut(item econ, item boardNdx, item ndx);
void boardPut(item econ, item boardNdx, item ndx, item num);
int boardSize(item econ, item boardNdx);
void boardClean(item econ, item boardNdx, item ndx);
void resetBoards();
void writeBoards(FileBuffer* file);
void readBoards(FileBuffer* file, int version);

#endif
