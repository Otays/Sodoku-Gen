# Sodoku Gen (1.0.0) README #

This program generates Sodoku puzzles of using a naive backtracking method.

## Options ##
__Block size:__
A standard sodoku has a block size of 3x3.  I've written Sodoku Gen to accept a block width of 2, 3, or 4.<br /><br />

<img src="https://raw.githubusercontent.com/Otays/Sodoku-Gen/master/pics/Sodoku1.png" />

<img src="https://raw.githubusercontent.com/Otays/Sodoku-Gen/master/pics/Sodoku2.png" />

<br />

__Difficulty Level:__
Typical sodoku puzzles are ranked using five levels of difficulty. 
Controlling the difficulty of a sodoku is a difficult problem, usually
involving a "hole digging" method and/or the dancing links algorithm.
<br />

I developed this project for fun as an exercise without doing research, so my implementation
 is more simplistic.  The supported difficulties are Easy and Normal, where Normal puzzles 
 involve an extra pruning phase.  It's important to note that without the "hole digging" method,
 there is a limit to how much pruning increases difficulty. 
<br /><br />

__Mass production:__
Sodoku Gen can mass produce these puzzles and their corresponding solutions. <br /><br />

<img src="https://raw.githubusercontent.com/Otays/Sodoku-Gen/master/pics/Sodoku3.png" />

<br /><br />

## Analysis ##
__Performance:__
Sodoku generating isn't easy.  Its equivalent to the graph coloring problem, which is NP-Complete
 with respect to the block size.<br /><br />

On my computer, 3x3 block size takes on average 0.1 s, while 4x4 block size takes about 6 minutes.
  My implementation starts over when it fails instead of using more sophisticated backtracking, 
  but rather than improving the algorithm I think It would be fun to try sending the job to the GPU
  to parallelize the generation attempts.
<br />

<img src="https://raw.githubusercontent.com/Otays/Sodoku-Gen/master/pics/Sodoku4.png" />


__Conclusion:__
This was fun.


## Required files: ##
 - makefile
 - sodoku.cpp
 - colorlogs.h
 - colorlogs.c
  

  
  
  
  
  
  






