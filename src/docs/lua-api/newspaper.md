Lua API - Newspaper

Newspaper articles are configured in newspaper/articles.lua. The newspaper appears every day at 6am, and contains CNumNewspaperArticles ($CNumNewspaperArticles$) randomly selected articles. To ease modding, you can set the newspaper to appear every hour with CHourlyNewspaper = true. You can also force a particular article to appear by using the cheat code "propaganda <article code>".

The easiest way to test tricky Lua interpolations for use in articles is to put them in a Citipedia document, such as this one, which is docs/lua-api/newspaper.md. That way you don't have to wait a game hour for the next newspaper to generate, since the Citipedia is reparsed every frame.

#### Example of a newspaper article markdown file.
> \##### \$(function() team1 = randomTeam(); return team1 end)()\$ Blank \$(function() team2 = randomTeam(); return team2 end)()\$ \$formatInt(randomInt(5,30))\$-0
>
> A stunning blow out as The \$team1\$ deny The \$team2\$ any opportunity to make a run. The \$team1\$ played extremely good defense, while still being strong at bat, and the \$team2\$ were simply wiped out. The \$team2\$ will have to wait until next year to regain their glory. The \$team1\$ go on to play The \$randomTeam()\$.

---

##### addNewspaperArticle(_object_) => void

Takes the configuration _object_ and adds a newspaper article to the list of possible newspaper articles. _object_ should have properties _code_, _group_, _file_, and _score_.

_code_ is a unique alphanumeric code for the newspaper article. If a modpack has an article with the same _code_ as a vanilla article, the modpack article will overwrite the vanilla version. When adding new newspaper articles, modders should avoid naming conflicts by adding "<your name>::" before the code, as a form of namespacing.

_group_ is one of the article groups configured by addNewspaperArticleGroup(). The game will not include two articles with the same group in the same issue of the newspaper.

_file_ is the filename of a markdown file which defines the newspaper. The first line of this file is used as a headline and must not contain images or formatting, but may contain a Lua interpolation. The Lua interpolations in the file are evaluated at the moment the newspaper issue is generated (6AM).

_score_ is a function which returns a score for the article, based on the situation. The score determines how likely the article is to be selected for any given issue. This score can be based on statistics about the city or anything else. If the article is applicable to any situation (a "fluff" piece) then it's score should be 1. Otherwise, it's score should vary between 1 and 2, based on it's likelyhood and importance. If an article has a score which is too high, it is likely to appear too often. The game applies a randomization to the score, as follows: randomizedScore = baseScore * (1 + randFloat() * CNewspaperRandomness). If an article is not applicable to a situtation, it's score should be 0 or negative.

This function should be used in modpacks/<your modpack>/newspaper/articles.lua. See newspaper/articles.lua for more examples.

---

##### addNewspaperArticleGroup(_object_) => void

Takes the configuration _object_ and adds a newspaper article group to the list of groups. _object_ should have the property _code_.

_code_ is a unique alphanumeric code for the group. Modders should avoid naming conflicts by adding "<your name>::" before the code, as a form of namespacing.


---

##### newspaperNameShort() => string

Returns the short form of the newspaper's name, including "The".

newspaperNameShort() => $newspaperNameShort()$

---

##### newspaperNameLong() => string

Returns the long form of the newspaper's name, including "The" and the name of the city.

newspaperNameLong() => $newspaperNameLong()$

---

##### maxNewspaperNdx() => number

Returns the number of issues of the newspaper.

maxNewspaperNdx() => $maxNewspaperNdx()$

---

##### hasArticleAppeared(_articleCode_) => number

Returns true if the article has appeared in the newspaper before. _articleCode_ is the newspaper article code to match, matching any article with a code that starts with _articleCode_.

hasArticleAppeared("StocksDown01") => $hasArticleAppeared("StocksDown01")$
hasArticleAppeared("StocksDown") => $hasArticleAppeared("StocksDown")$

---

##### timeSinceArticleAppeared() => number

Returns the amount of time since the article has last appeared in the newspaper. _articleCode_ is the newspaper article code to match, matching any article with a code that starts with _articleCode_. If there are no matches, timeSinceArticleAppeared() returns the time since 3000BC.

formatDuration(timeSinceArticleAppeared("StocksDown01")) => $formatDuration(timeSinceArticleAppeared("StocksDown01"))$
formatDuration(timeSinceArticleAppeared("StocksDown")) => $formatDuration(timeSinceArticleAppeared("StocksDown"))$

