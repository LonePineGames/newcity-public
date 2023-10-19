Amenity Effect: Education

#### $cityName()$ has +$formatInt(amenityEffectScore(EducationScore))$![](IconEducation)Education, so it can be home to up to $formatInt(amenityEffectMultiplier(EducationScore))$ college graduates.

![](docs/images/education.png)

In order to advance economically, your city must have educated citizens. Educated citizens also pay more taxes, [[heatmaps/hmValue|buy nicer homes]], and commit less crime. (Not counting white-collar crimes, of course.)

![](IconEducation)Education is both a local area and a city-wide effect.

##### Benefits of ![](IconEducation)Education
* See the article on [[concepts/education|Education]].
* See the article on [[heatmaps/hmEducation|the Education heatmap]].
* Some Education amenities grant high-school diplomas or college degrees. These buildings will randomly educate (level up) people within their radius of effect.
* Other Education amenities, such as the Library, do not grant diplomas or degrees, but they do increase how many total diplomas and degrees you can have in the city. (Bookworms don't live in cities without libraries.)
* Amenities which give Education do not influence the Education heatmap directly. The Education heatmap is only affected by the education level of the citizens themselves.

![](Chart::StatHSEduPercent)

* Your Education Score determines how many people with high-school diplomas, bachelor's degrees or PhDs can live in your in city. Once this cap is reached, more citizens cannot be educated to that level and people with that education level will stop moving to your city.
* There is a hard cap on what percent of people can have certain education levels, no matter how many Amenities you build:
  * No more than $formatPercent(CMaxHSEdu)$ with High School Diplomas.
  * No more than $formatPercent(CMaxBclEdu)$ with Bachelor's Degrees.
  * No more than $formatPercent(CMaxPhdEdu)$ with Doctorates (PhDs).

[[index|Back to Index]]

