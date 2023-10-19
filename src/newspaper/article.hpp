#pragma once

#include "newspaper.hpp"

#include <string>
using namespace std;

void resetNewspaperArticles();
void initNewspaperLua();
vector<NewspaperIssueArticle> generateNewspaperArticles(item num);
void forceNewspaperArticle(string code);

string getAdArticle(item ndx);
item numAdArticles();

