#include "article.hpp"

#include "../platform/lua.hpp"
#include "../platform/file.hpp"
#include "../pool.hpp"
#include "../util.hpp"

#include "spdlog/spdlog.h"
#include <unordered_map>

struct NewspaperArticle {
  uint32_t flags;
  char* code;
  char* file;
  item group;
  int scoreFunc;
};

struct NewspaperArticleGroup {
  uint32_t flags;
  char* code;
};

const int _articleExists = 1 << 0;

const int _articleGroupExists = 1 << 0;

Pool<NewspaperArticle> articles;
Pool<NewspaperArticle> adArticles;
Pool<NewspaperArticleGroup> articleGroups;
unordered_map<string, item> articleGroupsByCode;
unordered_map<string, item> articlesByCode;
vector<string> forcedArticles;

#include "articleTransform.cpp"

void resetNewspaperArticles() {
  // clean up strings
  for (int i=1; i <= articles.size(); i++) {
    NewspaperArticle* article = articles.get(i);
    if (article->code) {
      free(article->code);
    }
    if (article->file) {
      free(article->file);
    }
  }
  articles.clear();

  for (int i=1; i <= adArticles.size(); i++) {
    NewspaperArticle* article = adArticles.get(i);
    if (article->code) {
      free(article->code);
    }
    if (article->file) {
      free(article->file);
    }
  }
  adArticles.clear();
}

int addNewspaperArticleGroup(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  item ndx = articleGroups.create();
  NewspaperArticleGroup* group = articleGroups.get(ndx);
  group->code = luaFieldString(L, "code");
  group->flags = _articleGroupExists;

  articleGroupsByCode[group->code] = ndx;
  return 0;
}

int addNewspaperArticle(lua_State* L) {
  int numArgs = lua_gettop(L);
  if (numArgs <= 0) return 0;

  char* code = luaFieldString(L, "code");
  item ndx = articlesByCode[code];
  if (ndx == 0) {
    ndx = articles.create();
    articlesByCode[code] = ndx;
  }

  NewspaperArticle* article = articles.get(ndx);
  article->flags = _articleExists;
  article->code = code;
  article->file = luaFieldString(L, "file");
  article->scoreFunc = luaFieldFunction(L, "score");

  if (article->code == 0) {
    handleError("Must provide code for article");
  }

  if (article->file == 0) {
    handleError("Must provide file for article %s", article->code);
  }

  if (article->scoreFunc == -1) {
    handleError("Must provide score function for article %s", article->code);
  }

  char* groupCode = luaFieldString(L, "group");
  if (groupCode == 0) {
    handleError("Must provide group for article %s", article->code);
  }
  article->group = articleGroupsByCode[groupCode];
  if (article->group == 0) {
    handleError("Group %s not found for article %s", groupCode, article->code);
  }
  if (groupCode != 0) free(groupCode);

  return 0;
}

void initNewspaperAds() {
  // init ads
  uint32_t lookupFlags = c(CDisableDefaultNewspaperArticles) ? _lookupOnlyMods : 0;
  vector<string> dir = lookupDirectory("newspaper/ads/", ".md", lookupFlags);

  for (int i = 0; i < dir.size(); i ++) {
    item adNdx = adArticles.create();
    NewspaperArticle* article = adArticles.get(adNdx);

    string file = dir[i];
    string code = "Ad_" + file;
    string filepath = lookupFile("newspaper/ads/" + file + ".md", lookupFlags).c_str();

    article->flags = _articleExists;
    article->code = strdup_s(code.c_str());
    article->file = strdup_s(filepath.c_str());
    article->scoreFunc = 0;
  }
}

void initNewspaperLua() {
  resetNewspaperArticles();
  articleGroups.clear();
  articleGroupsByCode.clear();
  articlesByCode.clear();
  forcedArticles.clear();
  articles.clear();
  adArticles.clear();
  addLuaCallback("addNewspaperArticle", addNewspaperArticle);
  addLuaCallback("addNewspaperArticleGroup", addNewspaperArticleGroup);
  initNewspaperAds();
}

void forceNewspaperArticle(string code) {
  forcedArticles.push_back(code);
  generateNewspaperIssue();
}

vector<NewspaperIssueArticle> generateNewspaperArticles(item num) {
  vector<float> scores;
  vector<NewspaperArticle*> resultArticles;
  vector<NewspaperIssueArticle> results;
  scores.reserve(articles.size()+1);
  scores.push_back(-1);

  // Get scores for each article
  for (int i = 1; i <= articles.size(); i++) {
    NewspaperArticle* article = articles.get(i);
    float baseScore = callLuaScoringFunction(article->scoreFunc, article->code);
    float score = baseScore * (1 + randFloat() * c(CNewspaperRandomness));
    if (c(CLogNewspaperData)) {
      SPDLOG_INFO("article {} scored {}, randomized to {}",
          article->code, baseScore, score);
    }

    for (int k = 0; k < forcedArticles.size(); k++) {
      if (streql(forcedArticles[k].c_str(), article->code)) {
        resultArticles.push_back(article);
        score = -1;
      }
    }

    scores.push_back(score);
  }

  forcedArticles.clear();

  // Don't publish the same article over and over
  for (int i = 0; i < 4; i++) {
    int issueNdx = getNumNewspaperIssues()-i-1;
    if (issueNdx < 0) break;

    NewspaperIssue* issue = getNewspaperIssue(issueNdx);
    for (int a = 0; a < issue->articles.size(); a ++) {
      const char* code = issue->articles[a].code;
      if (code == 0) continue;
      item articleNdx = articlesByCode[code];
      if (articleNdx <= 0 || articleNdx >= scores.size()) continue;
      scores[articleNdx] = clamp(scores[articleNdx], -1.f, 0.5f);
    }
  }

  // Get the highest scoring articles, but no two from the same group
  for (int k = resultArticles.size(); k < num; k++) {
    float highScore = -1;
    NewspaperArticle* best = 0;
    item bestNdx = 0;
    for (int i = 1; i <= articles.size(); i++) {
      if (scores[i] > highScore) {

        NewspaperArticle* article = articles.get(i);
        for (int r = 0; r < resultArticles.size(); r ++) {
          if (resultArticles[r]->group == article->group) {
            scores[i] = -1;
            break;
          }
        }

        if (scores[i] > highScore) {
          best = article;
          bestNdx = i;
          highScore = scores[i];
        }
      }
    }

    if (best != 0) {
      scores[bestNdx] = -1;
      if (c(CLogNewspaperData)) {
        SPDLOG_INFO("pushing article {}:{}", bestNdx, best->code);
      }
      resultArticles.push_back(best);
    }
  }

  for (int k = 0; k < resultArticles.size(); k ++) {
    NewspaperArticle* article = resultArticles[k];
    NewspaperIssueArticle issueArticle =
      transformArticleToIssueArticle(article);
    results.push_back(issueArticle);
  }

  return results;
}

string getAdArticle(item ndx) {
  if (ndx <= 0 || ndx > adArticles.size()) return "";

  NewspaperArticle* article = adArticles.get(ndx);
  FileBuffer file = readNewspaperArticle(article->file);
  string data = file.data;
  // SPDLOG_INFO("getAdArticle data str: {}", data);
  trim(data);

  return transformArticleToIssueArticleInner(data, article->file);
}

item numAdArticles() {
  return adArticles.size();
}

