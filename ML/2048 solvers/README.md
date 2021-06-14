different algorithms to solve 2048 games

algorithms written in submission.py file

* basic Greedy

* Improved Greedy

algorithms with iterative deepining:

* minimax move and index players

* alpha-beta minimax player

* expectimax move and index players

the first four algorithms assume that only tile value 2 can be added to the game.

expectimax can add tile 2 and 4 to the game, with a probablity of 0.9 ofr 2 and 0.1 for 4


included is an image of two graphs to showcase differences between minimax and alpha-beta: 
    * average score per thinking time 
    * average depth per thinking time

the hueristics (and subsequently the scores) could be improved by adding in:

      * monoticity checks
      * smoothness checks
