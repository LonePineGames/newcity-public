#include "newspaper.hpp"

#include "article.hpp"

#include "../game/game.hpp"
#include "../economy.hpp"
#include "../money.hpp"
#include "../name.hpp"
#include "../option.hpp"
#include "../parts/messageBoard.hpp"
#include "../platform/event.hpp"
#include "../time.hpp"
#include "../util.hpp"
#include "../weather.hpp"

#include "spdlog/spdlog.h"

bool newspaperExists = false;
string newspaperName = "";
Cup<NewspaperIssue> issues;
float newspaperTime = 0;

void resetNewspaper() {
  newspaperName = "";
  newspaperExists = false;
  newspaperTime = 0;

  for (int i = 0; i < issues.size(); i++) {
    NewspaperIssue* issue = getNewspaperIssue(i);

    for (int j = 0; j < issue->articles.size(); j++) {
      NewspaperIssueArticle* article = issue->articles.get(j);
      free(article->code);
      free(article->title);
      free(article->text);
    }
    issue->articles.clear();

    for (int j = 0; j < issue->ads.size(); j++) {
      if (issue->ads[j] != 0) {
        free(issue->ads[j]);
      }
    }
    issue->ads.clear();

    for (int j = 0; j < issue->sportsScores.size(); j++) {
      free(issue->sportsScores[j].teamNames[0]);
      free(issue->sportsScores[j].teamNames[1]);
    }

    for (int j = 0; j < issue->financialScores.size(); j++) {
      free(issue->financialScores[j].statName);
    }
  }
  issues.clear();
}

string getNewspaperName() {
  string result = "The ";
  return result + getCityName() + " " + newspaperName;
}

string getNewspaperNameShort() {
  string result = "The ";
  return result + newspaperName;
}

NewspaperIssue* getNewspaperIssue(item ndx) {
  return issues.get(ndx);
}

NewspaperIssue* getLatestNewspaperIssue() {
  return getNewspaperIssue(issues.size()-1);
}

item getNumNewspaperIssues() {
  return issues.size();
}

bool doesNewspaperExist() {
  return newspaperExists;
}

void setNewspaperTime(float time) {
  newspaperTime = time;
}

float getNewspaperTime() {
  return newspaperTime;
}

void addFinancialScore(NewspaperIssue* issue, const char* name, float mult) {
  float val = getStatistic(nationalEconNdx(), NationalStockIndex);
  val *= mult;
  val *= randFloat(0.8, 1.2);
  issue->financialScores.push_back(FinancialScore { strdup_s(name), val });
}

string getNewspaperAdvertisement(item issueNdx, item adNum) {
  if (numAdArticles() <= 0) return "";

  NewspaperIssue* issue = issues.get(issueNdx);

  // check cache
  if (issue->ads.size() > adNum) {
    char* ch = issue->ads[adNum];
    if (ch != 0) {
      return ch;
    }
  }

  item adArticleNum = (adNum*7 + issueNdx*97 + issue->weather + issue->sportsScores[0].scores[0])%numAdArticles() + 1;

  setNewspaperTime(issue->date);

  string result = getAdArticle(adArticleNum);
  issue->ads.resize(adNum+1);
  issue->ads.set(adNum, strdup_s(result.c_str())); // write to cache
  return result;
}

void fixMoon(NewspaperIssue* issue) {
  if (issue->weather == WeatherMoon) {
    if (issue->temp > 30) {
      issue->weather = WeatherSun;
    } else if (issue->temp < 0) {
      issue->weather = WeatherCold;
    } else {
      issue->weather = WeatherNull;
    }
  }
}

void generateNewspaperIssue() {
  fireEvent(EventNewspaper);

  NewspaperIssue* issue = issues.next();
  issue->articles.fromVector(
      generateNewspaperArticles(c(CNumNewspaperArticles)));
  issue->date = getCurrentDateTime();
  setNewspaperTime(issue->date);
  newspaperExists = true;

  float price = c(CNewspaperPrice) * getInflation();
  if (price >= 3) {
    price = round(price);
  } else if (price > 0.5) {
    price = round(price*4)/4.f;
  } else if (price > 0.8) {
    price = round(price*20)/20.f;
  } else {
    price = round(price*100)/100.f;
  }
  issue->price = price;

  issue->temp = getWeather().temp;
  issue->weather = getWeatherIcon();
  fixMoon(issue);

  issue->sportsScores.clear();
  issue->financialScores.clear();
  issue->financialScores.push_back(FinancialScore { strdup_s("NSMI"),
      getStatistic(nationalEconNdx(), NationalStockIndex) });
  if (getCurrentYear() > 1970) {
    addFinancialScore(issue, "NeTeK", 0.5);
  }
  addFinancialScore(issue, "EuMark", 0.08);
  if (getCurrentYear() > 1980) {
    addFinancialScore(issue, "NikkL", 2.5);
  }
  issue->financialScores.push_back(FinancialScore { strdup_s("Interest Rate"),
      getNationalInterestRate()*100 });

  for (int i = 0; i < 3; i++) {
    SportsScore score;
    score.teamNames[0] = randomName(SportsTeamName);
    score.teamNames[1] = randomName(SportsTeamName);
    while (streql(score.teamNames[0], score.teamNames[1])) {
      free(score.teamNames[1]);
      score.teamNames[1] = randomName(SportsTeamName);
    }

    item maxScore = randItem(4)+2;
    maxScore *= maxScore * maxScore;
    score.scores[0] = randItem(maxScore)+1;
    score.scores[1] = randItem(score.scores[0]);
    issue->sportsScores.push_back(score);
  }

  if (isShowNewspaperMessage()) {
    addMessage(NewspaperMessage, 0);
  }

  if (c(CLogNewspaperData)) {
    for (int i = 0; i < issue->articles.size(); i++) {
      SPDLOG_INFO("article: {}", issue->articles.get(i)->title);
      SPDLOG_INFO("{}", issue->articles.get(i)->text);
    }
  }
}

void updateNewspaper() {
  if (!c(CEnableNewspaper)) return;
  if (getStatistic(ourCityEconNdx(), NumBusinesses) == 0) return;
  if (getCurrentDateTime() < 1) return;

  if (newspaperName.length() == 0) {
    newspaperName = randomName(NewspaperName);
  }

  if (c(CHourlyNewspaper)) {
    generateNewspaperIssue();
  } else if (issues.size() == 0) {
    generateNewspaperIssue();
  } else {
    float lastDate = getLatestNewspaperIssue()->date;
    if (int(getCurrentDateTime()) > int(lastDate) && getCurrentTime() > 0.25) {
      generateNewspaperIssue();
    }
  }
}

void writeNewspaperIssue(FileBuffer* file, item ndx) {
  NewspaperIssue* issue = getNewspaperIssue(ndx);
  fwrite_float(file, issue->date);
  fwrite_float(file, issue->price);
  fwrite_float(file, issue->temp);
  fwrite_item(file, issue->weather);

  fwrite_item(file, issue->articles.size());
  for (int i = 0; i < issue->articles.size(); i ++) {
    NewspaperIssueArticle* article = issue->articles.get(i);
    fwrite_string(file, article->code);
    fwrite_string(file, article->title);
    fwrite_string(file, article->text);
  }

  fwrite_item(file, issue->sportsScores.size());
  for (int i = 0; i < issue->sportsScores.size(); i ++) {
    fwrite_string(file, issue->sportsScores[i].teamNames[0]);
    fwrite_string(file, issue->sportsScores[i].teamNames[1]);
    fwrite_item(file, issue->sportsScores[i].scores[0]);
    fwrite_item(file, issue->sportsScores[i].scores[1]);
  }

  fwrite_item(file, issue->financialScores.size());
  for (int i = 0; i < issue->financialScores.size(); i ++) {
    fwrite_string(file, issue->financialScores[i].statName);
    fwrite_float(file, issue->financialScores[i].score);
  }

  // Future proofing
  fwrite_item(file, 0);
  fwrite_item(file, 0);
  fwrite_item(file, 0);
  fwrite_item(file, 0);
}

void readNewspaperIssue(FileBuffer* file, item ndx) {
  NewspaperIssue* issue = getNewspaperIssue(ndx);
  issue->date = fread_float(file);
  issue->price = fread_float(file);
  issue->temp = fread_float(file);
  issue->weather = fread_item(file);

  item numArticles = fread_item(file);
  for (int i = 0; i < numArticles; i ++) {
    NewspaperIssueArticle article;
    article.code = fread_string(file);
    article.title = fread_string(file);
    article.text = fread_string(file);
    issue->articles.push_back(article);
  }

  item numSportsScores = fread_item(file);
  for (int i = 0; i < numSportsScores; i ++) {
    SportsScore scores;
    scores.teamNames[0] = fread_string(file);
    scores.teamNames[1] = fread_string(file);
    scores.scores[0] = fread_item(file);
    scores.scores[1] = fread_item(file);
    issue->sportsScores.push_back(scores);
  }

  item numFinancialScores = fread_item(file);
  for (int i = 0; i < numFinancialScores; i ++) {
    FinancialScore score;
    score.statName = fread_string(file);
    score.score = fread_float(file);
    issue->financialScores.push_back(score);
  }

  // Future proofing
  fread_item(file);
  fread_item(file);
  fread_item(file);
  fread_item(file);

  fixMoon(issue);
}

void writeNewspaper(FileBuffer* file) {
  fwrite_string(file, newspaperName);

  fwrite_item(file, issues.size());
  for (int i = 0; i < issues.size(); i ++) {
    writeNewspaperIssue(file, i);
  }
}

void readNewspaper(FileBuffer* file) {
  if (file->version >= 56) {
    newspaperName = fread_cpp_string(file);

    issues.resize(fread_item(file));
    for (int i = 0; i < issues.size(); i ++) {
      readNewspaperIssue(file, i);
    }

    newspaperExists = issues.size() > 0;
  }
}

