#pragma once

#include "part.hpp"
#include <string>

const char* citipediaGetArticleCode404();
std::string citipediaGet404Title();
std::string citipediaGet404Content();
Part* citipediaPanel();
void selectCitipediaArticle(char* link);

