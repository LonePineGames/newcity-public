#pragma once

#include "../cup.hpp"
#include "../serialize.hpp"

#include <string>
using namespace std;

struct NewspaperIssueArticle {
  char* code;
  char* title;
  char* text;
};

struct SportsScore {
  char* teamNames[2];
  item scores[2];
};

struct FinancialScore {
  char* statName;
  float score;
};

struct NewspaperIssue {
  Cup<NewspaperIssueArticle> articles;
  Cup<char*> ads;
  float date;
  float price;
  float temp;
  item weather;
  Cup<SportsScore> sportsScores;
  Cup<FinancialScore> financialScores;
};

string getNewspaperName();
string getNewspaperNameShort();
NewspaperIssue* getNewspaperIssue(item ndx);
NewspaperIssue* getLatestNewspaperIssue();
item getNumNewspaperIssues();
string getNewspaperAdvertisement(item issueNdx, item adNum);
float getNewspaperTime();
bool doesNewspaperExist();
void updateNewspaper();
void generateNewspaperIssue();
void resetNewspaper();
void writeNewspaper(FileBuffer* file);
void readNewspaper(FileBuffer* file);

