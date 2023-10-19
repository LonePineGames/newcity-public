# Refactoring
* repalette
* remove phaseMaxs
* associate edges & nodes with chunks
* road building call flow
* render land as pallette texture

# Features
* circle-spline lanes and roads
* jobs, profit and decay
* lane begin and end markings
* only apply node config & price for third edge in node
* vehicle reverse
* parking spaces
* precision cut intersections
* conflict points (incl. save-load)
* control options
* graphics options
* supports
* explainations for illegal plans
* rivers
* sounds
* pricing
  * flatten & bridge roads
  * destroying buildings
  * splits
* better assignment of lanes at multilane intersections
* lane configuration at multilane intersections
* loading dialog
* minimum edge lengths
* road naming: implementing 90deg rule

# Known issues
* cursor pops up and down (unitize/pointOnLand issue?)
* building collision detection needs work
* cross T-junction parallel split issue
* problems w/ new game while bridge pillar tool selected
* when flattening, buildings don't get removed
* shadows glitch when at low camera angle
* edge collisions don't make edge illegal
* no specular
* 2-edge node rendering
* stoplight box texture out of alignment on >6 edges
* malformed designs
* designer UI issues
* findCollisions sometimes misses an intersection
* Cars get stuck at end of lane -- not in the lane they want to be in?
* cars use unfinished roads
* buildings should go deeper into ground -- visible on slopes
* corruption when saving game with illegal plans

# Testing
* re-enable testSplit() in testGraph.cpp:108

# Icons
